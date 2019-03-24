#ifndef _TCP_CLIENT_HPP_
#define _TCP_CLIENT_HPP_
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
#include"MessageHeader.hpp"
class CTCPClient
{
public:
	CTCPClient() :_sock(INVALID_SOCKET){};
	virtual ~CTCPClient(){ Close(); };
	void InitSocket()
	{
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA WSAData;
		WSAStartup(ver, &WSAData);
#endif
		//
		if (SOCKET_ERROR != _sock)
		{
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (_sock == INVALID_SOCKET)
		{
			printf("create socket failed!\n");
		}
		else
		{
			printf("create socket success!\n");
		}
	};
	int Connect(const char*ip,unsigned short port)
	{
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);//host to net unsign short
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif
		if (SOCKET_ERROR == connect(_sock, (sockaddr*)&_sin, sizeof(_sin)))
		{
			printf("connect failed!\n");
			return -1;
		}
		else{
			printf("connect success!\n");
		}
		return 0;
	};
	//接收数据
	int Recv()
	{
		//return recv(_sock, buf, len, 0);
		char chrecv[1024] = {};
		int nlen = recv(_sock + 3, chrecv, sizeof(DATAHEADER), 0);
		if (nlen <= 0)
		{
			printf("与服务器%d断开连接！\n", _sock);
			return -1;
		}

		DATAHEADER* header = (DATAHEADER*)chrecv;
		printf("收到服务器数据反馈：%d,数据长度：%d。\n", header->cmd, header->datalen);
		recv(_sock, chrecv + sizeof(DATAHEADER), header->datalen - sizeof(DATAHEADER), 0);
		OnNetMsg(header);
		return 0;
	}
	//发送数据
	int Send(DATAHEADER*header)
	{
		if (IsRun()&&header)
			return send(_sock, (char*)header, header->datalen, 0);
		return SOCKET_ERROR;
	}
	void Close()
	{
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
	bool OnRun()
	{
		if (IsRun())
		{
			fd_set fdRead;
			FD_ZERO(&fdRead);
			FD_SET(_sock, &fdRead);
			timeval t = { 1, 0 };
			int nret = select(_sock + 1, &fdRead, 0, 0, &t);
			if (nret < 0)
			{
				printf("select 任务结束1！\n");
				return false;
			}
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				if (-1 == Recv())
				{
					printf("select 任务结束2！\n");
					return false;
				}
			}
			return true;
		}
		return false;
	}
	bool IsRun()
	{
		return _sock != INVALID_SOCKET;
	}

	//消息处理
	virtual void OnNetMsg(DATAHEADER*header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
								 LOGINRESULT* loginret = (LOGINRESULT*)header;
								 printf("服务器登录回馈:result=%d，服务器请求：%d,数据长度：%d\n", loginret->nresult, loginret->cmd, loginret->datalen);
		}
			break;
		case CMD_LOGOUT_RESULT:
		{
								  LOGOUTRESULT* logoutret = (LOGOUTRESULT*)header;
								  printf("服务器登出回馈:result=%d，服务器请求：%d,数据长度：%d\n", logoutret->nresult, logoutret->cmd, logoutret->datalen);
		}
			break;
		case CMD_NEWCLIENT:
		{
							  NEWUSER* newuser = (NEWUSER*)header;
							  printf("服务器新用户连接:socket=%d，服务器请求：%d,数据长度：%d\n", newuser->sock, newuser->cmd, newuser->datalen);
		}
			break;
		case CMD_CLIENT_EXIT:
		{
								USEREXIT* userexit = (USEREXIT*)header;
								printf("服务器用户断开连接:socket=%d，服务器请求：%d,数据长度：%d\n", userexit->sock, userexit->cmd, userexit->datalen);
		}
			break;
		default:
			printf("错误信息\n");
			break;
		}
	}
private:
	SOCKET _sock;
};
#endif