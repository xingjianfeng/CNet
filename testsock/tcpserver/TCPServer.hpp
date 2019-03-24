#ifndef _TCPServer_hpp_
#define _TCPServer_hpp_
#ifdef _WIN32
#include<Windows.h>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include<unistd.h>
#include<arpa/inet.h>
#include<string.h>
#define SOCKET int

#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif
#include<stdio.h>
#include<vector>
#include"MessageHeader.hpp"
class CTCPServer
{
	SOCKET _sock;
public:
	CTCPServer() :_sock(INVALID_SOCKET){};
	~CTCPServer(){ Close(); };
	void InitSocket()
	{
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA WSAData;
		WSAStartup(ver, &WSAData);
#endif
		//
		if (INVALID_SOCKET != _sock)
		{
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (_sock == INVALID_SOCKET)
		{
			printf("create socket failed!\n");
		}
		else
		{
			printf("create socket success!\n");
		}
		//
	}
	int Bind(const char*ip,unsigned short port)
	{
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);//host to net unsign short
		
#ifdef _WIN32
		if (ip)
		{
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;// inet_addr(ip);
		}
#else	
		if (ip)
		{
			_sin.sin_addr.s_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.s_addr = INADDR_ANY;
		}
#endif
		int nret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == nret)
		{
			printf("绑定IP:%s,端口:%d失败\n",ip,port);
		}
		else
		{
			printf("绑定IP:%s,端口:%d成功\n", ip, port);
		}
		return nret;
	}
	int Listen(int n)
	{
		int nret = listen(_sock, n);
		if (SOCKET_ERROR == nret)
		{
			printf("Socket=<%d>监听端口失败\n", _sock);
		}
		else
		{
			printf("Socket=<%d>监听端口成功\n",_sock);
		}
		return nret;
	}
	void Close()
	{
		//程序结束 清空socket
		for (size_t n = 0; n < g_clients.size(); n++)
		{
#ifdef _WIN32
			closesocket(g_clients[n]);
#else
			close(g_clients[n]);
#endif
		}
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			closesocket(_sock);
			WSACleanup();
#else
			close(_sock);
#endif
			_sock = INVALID_SOCKET;
		}
	}
	bool IsRun()
	{
		return _sock != INVALID_SOCKET;
	}
	SOCKET Accept()
	{
		sockaddr_in clientaddr = {};
		SOCKET _clisock = INVALID_SOCKET;
		int addrlen = sizeof(clientaddr);
		_clisock = accept(_sock, (sockaddr*)&clientaddr, &addrlen);
		if (_clisock == INVALID_SOCKET)
		{
			printf("无效的链接\n");
		}
		else{
			//socktmp = _clisock;
			//FD_SET(_clisock, &fdRead);
			NEWUSER newuser;
			newuser.sock = (int)_clisock;
			SendAllClient(&newuser);
			g_clients.push_back(_clisock);
			printf("new client:<socket=%d>,ip=%s\n", _clisock, inet_ntoa(clientaddr.sin_addr));
		}
		return _clisock;
	}
	bool OnRun()
	{
		if (IsRun())
		{
			timeval tout = { 1, 0 };
			fd_set fdRead, fdWrite, fdExp;
			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);
			FD_SET(_sock, &fdRead);
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExp);
			for (int n = g_clients.size() - 1; n >= 0; n--)
			{
				FD_SET(g_clients[n], &fdRead);
			}
			int nret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, &tout);
			if (nret<0)
			{
				printf("select 任务结束!\n");
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				Accept();
			}
			//遍历可读socket
			for (size_t n = 0; n< fdRead.fd_count; n++)
			{
				if (-1 == RecvData(fdRead.fd_array[n]))
				{
					auto itr = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[n]);
					if (itr != g_clients.end())
					{
						g_clients.erase(itr);
					}
				}
			}
			printf("空闲时间处理其他业务……\n");
			return true;
		}
		return false;
	}
	int RecvData(SOCKET& _clisock)
	{
		char recvbuf[1024] = {};
		int nlen = recv(_clisock, recvbuf, sizeof(DATAHEADER), 0);
		if (nlen <= 0)
		{
			printf("客户端%d退出，程序结束！\n", _clisock);
			USEREXIT userexit;
			userexit.sock = (int)_clisock;
			SendAllClient(&userexit);
			return -1;
		}

		DATAHEADER* header = (DATAHEADER*)recvbuf;
		printf("收到客户端命令：%d,数据长度：%d。\n", header->cmd, header->datalen);
		recv(_clisock, recvbuf + sizeof(DATAHEADER), header->datalen - sizeof(DATAHEADER), 0);
		OnNetMsg(header,_clisock);
		return 0;
	}
	int Send(SOCKET _clisock, DATAHEADER*header)
	{
		if (IsRun()&&header)
			return send(_clisock, (char*)header, header->datalen, 0);
		return SOCKET_ERROR;
	}
	void SendAllClient(DATAHEADER*header)
	{
		if (IsRun()&&header)
		{
			for (int n = g_clients.size() - 1; n >= 0; n--)
			{
				Send(g_clients[n], header);
			}
		}
	}
	virtual void OnNetMsg(DATAHEADER*header,SOCKET& _clisock)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
						  LOGIN* login = (LOGIN*)header;
						  printf("登录信息:name=%s,psw=%s，客户端命令：%d,数据长度：%d\n", login->name, login->psw, login->cmd, login->datalen);

						  LOGINRESULT loginres;
						  Send(_clisock, &loginres);
		}
			break;
		case CMD_LOGOUT:
		{
						   LOGOUT* logout = (LOGOUT*)header;;
						   LOGOUTRESULT logoutres;
						   Send(_clisock, &logoutres);
		}
			break;
		default:
			printf("错误信息\n");
			break;
		}

	}
private:

	std::vector<SOCKET> g_clients;
};
#endif