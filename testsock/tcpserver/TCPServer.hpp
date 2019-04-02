#ifndef _TCPServer_hpp_
#define _TCPServer_hpp_
#ifdef _WIN32
#define FD_SETSIZE 1024
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
#ifndef MAX_BUFFER_SIZE
#define MAX_BUFFER_SIZE 512
#endif
class ClientSocket
{
public:
	ClientSocket(SOCKET sock = INVALID_SOCKET)
	{
		_csock = sock;
		memset(_msgbuf, 0, MAX_BUFFER_SIZE * 10);
		_lastpos = 0;
	}
	SOCKET& CSock()
	{
		return _csock;
	}
	char* GetMsgBuf()
	{
		return _msgbuf;
	}
	int GetLastPos()
	{
		return _lastpos;
	}
	void SetLastPos(int pos)
	{
		_lastpos = pos;
	}
	void BufCopy(char*buf,int len)
	{
		memcpy(_msgbuf+_lastpos,buf,len);
		_lastpos += len;
	}
	void BufMove(int len)
	{
		memcpy(_msgbuf, _msgbuf+len,_lastpos-len);
	}
private:
	char _msgbuf[MAX_BUFFER_SIZE*10];
	int _lastpos;
	SOCKET _csock;
};
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
		for (size_t n = 0; n < _clients.size(); n++)
		{
#ifdef _WIN32
			closesocket(_clients[n]->CSock());
			delete _clients[n];
#else
			close(_clients[n]->CSock());
			delete _clients[n];
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
		/*	NEWUSER newuser;
			newuser.sock = (int)_clisock;
			SendAllClient(&newuser);*/
			_clients.push_back(new ClientSocket(_clisock));
			printf("第%d个new client:<socket=%d>,ip=%s\n",_clients.size()-1, _clisock, inet_ntoa(clientaddr.sin_addr));
		}
		return _clisock;
	}
	bool OnRun()
	{
		if (IsRun())
		{
			timeval tout = { 0, 0 };
			fd_set fdRead, fdWrite, fdExp;
			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);
			FD_SET(_sock, &fdRead);
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExp);
			for (int n = _clients.size() - 1; n >= 0; n--)
			{
				FD_SET(_clients[n]->CSock(), &fdRead);
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
				return true;
			}
			//遍历可读socket
			for (size_t n = 0; n< _clients.size(); n++)
			{
				if (FD_ISSET(_clients[n]->CSock(), &fdRead))
				{
					if (-1 == RecvData(_clients[n]))
					{
						auto itr = _clients.begin()+n;
						if (itr != _clients.end())
						{
							delete _clients[n];
							_clients.erase(itr);
						}
					}
				}
			}
			//printf("空闲时间处理其他业务……\n");
			return true;
		}
		return false;
	}
	int RecvData(ClientSocket* _clisock)
	{
		char recvbuf[MAX_BUFFER_SIZE] = {};
		int nlen = recv(_clisock->CSock(), recvbuf, MAX_BUFFER_SIZE, 0);
		if (nlen <= 0)
		{
			printf("客户端%d退出，程序结束！\n", _clisock->CSock());
			//USEREXIT userexit;
			//userexit.sock = (int)_clisock->CSock();
			//SendAllClient(&userexit);
			return -1;
		}
		printf("接收客户端<socket=%d>数据长度:%d\n", _clisock->CSock(), nlen);
		//_clisock->BufCopy(recvbuf, nlen);
		//while (_clisock->GetLastPos()>=sizeof(DATAHEADER))
		//{
		//	DATAHEADER* header = (DATAHEADER*)_clisock->GetMsgBuf();
		//	if (_clisock->GetLastPos() >= header->datalen)
		//	{
		//		OnNetMsg(header, _clisock->CSock());
		//		_clisock->BufMove(header->datalen);
		//		_clisock->SetLastPos(_clisock->GetLastPos() - header->datalen);
		//	}
		//	else
		//		break;
		//}
		/*
		DATAHEADER* header = (DATAHEADER*)recvbuf;
		printf("收到客户端命令：%d,数据长度：%d。\n", header->cmd, header->datalen);
		recv(_clisock, recvbuf + sizeof(DATAHEADER), header->datalen - sizeof(DATAHEADER), 0);
		OnNetMsg(header,_clisock);*/
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
			for (int n = _clients.size() - 1; n >= 0; n--)
			{
				Send(_clients[n]->CSock(), header);
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
						 // printf("登录信息:name=%s,psw=%s，客户端命令：%d,数据长度：%d\n", login->name, login->psw, login->cmd, login->datalen);

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

	std::vector<ClientSocket*> _clients;
};
#endif