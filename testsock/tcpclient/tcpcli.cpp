#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include<Windows.h>
#include<WinSock2.h>
#include<stdio.h>
#include<iostream>
#include<thread>
#pragma comment(lib,"ws2_32.lib")
enum CMDINFO
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEWCLIENT,
	CMD_ERROR
};
struct DATAHEADER{
	DATAHEADER()
	{
		datalen = 0;
		cmd = CMD_ERROR;
	}
	int datalen;
	CMDINFO cmd;
};
struct LOGIN :public DATAHEADER
{
	LOGIN()
	{
		datalen = sizeof(LOGIN);
		cmd = CMD_LOGIN;
	}
	char name[32];
	char psw[32];
};
struct LOGINRESULT :public DATAHEADER{
	LOGINRESULT()
	{
		datalen = sizeof(LOGINRESULT);
		cmd = CMD_LOGOUT_RESULT;
		nresult = 0;
	}
	int nresult;
};
struct LOGOUT :public DATAHEADER
{
	LOGOUT()
	{
		datalen = sizeof(LOGOUT);
		cmd = CMD_LOGOUT;
	}
	char name[32];
};
struct LOGOUTRESULT :public DATAHEADER{
	LOGOUTRESULT()
	{
		datalen = sizeof(LOGOUTRESULT);
		cmd = CMD_LOGOUT_RESULT;
		nresult = 1;
	}
	int nresult;
};
struct NEWUSER :public DATAHEADER
{
	NEWUSER()
	{
		datalen = sizeof(LOGIN);
		cmd = CMD_NEWCLIENT;
		sock = 0;
	}
	int sock;
};
bool g_bRun = true;
void Input(SOCKET _sock)
{
	while (true)
	{
		char cmdbuf[128] = {};
		scanf("%s", cmdbuf);
		if (0 == strcmp(cmdbuf, "exit"))
		{
			printf("退出线程!\n");
			g_bRun = false;
			break;
		}

		else if (0 == strcmp(cmdbuf, "login")){
			LOGIN login;
			strcpy(login.name, "邢建锋");
			strcpy(login.psw, "86420az");
			send(_sock, (char*)&login, sizeof(login), 0);
			LOGINRESULT loginres = {};
			recv(_sock, (char*)&loginres, sizeof(loginres), 0);
			printf("收到登录返回消息res=%d，客户端命令：%d,数据长度：%d\n", loginres.nresult, loginres.cmd, loginres.datalen);
		}
		else if (0 == strcmp(cmdbuf, "logout")){

			LOGOUT logout;
			strcpy(logout.name, "邢建锋");
			send(_sock, (char*)&logout, sizeof(logout), 0);
			LOGOUTRESULT logoutres = {};
			recv(_sock, (char*)&logoutres, sizeof(logoutres), 0);
			printf("收到登录返回消息res=%d，客户端命令：%d,数据长度：%d\n", logoutres.nresult, logoutres.cmd, logoutres.datalen);
		}
		else
		{
			printf("输入错误信息!\n");
		}
	}
}
int Processor(SOCKET _clisock)
{
	DATAHEADER header;
	int nlen = recv(_clisock + 1, (char*)&header, sizeof(header), 0);
	if (nlen <= 0)
	{
		printf("与服务器%d断开连接！\n", _clisock);
		return -1;
	}

	printf("收到服务器数据反馈：%d,数据长度：%d。\n", header.cmd, header.datalen);
	switch (header.cmd)
	{
	case CMD_LOGIN_RESULT:
	{
							 LOGINRESULT loginret = {};
							 recv(_clisock, (char*)&loginret + sizeof(DATAHEADER), sizeof(LOGINRESULT)-sizeof(DATAHEADER), 0);
							 printf("服务器登录回馈:socket=%d，服务器请求：%d,数据长度：%d\n", loginret.nresult, loginret.cmd, loginret.datalen);
	}
		break;
	case CMD_LOGOUT_RESULT:
	{
							  LOGOUTRESULT logoutret = {};
							  recv(_clisock, (char*)&logoutret + sizeof(DATAHEADER), sizeof(LOGOUTRESULT)-sizeof(DATAHEADER), 0);
							  printf("服务器登出回馈:socket=%d，服务器请求：%d,数据长度：%d\n", logoutret.nresult, logoutret.cmd, logoutret.datalen);
	}
		break;
	case CMD_NEWCLIENT:
	{
						NEWUSER newuser = {};
						recv(_clisock, (char*)&newuser + sizeof(DATAHEADER), sizeof(NEWUSER)-sizeof(DATAHEADER), 0);
					   printf("服务器新用户登录:socket=%d，服务器请求：%d,数据长度：%d\n", newuser.sock, newuser.cmd, newuser.datalen);
	}
		break;
	default:
		printf("错误信息\n");
		break;
	}

	return 0;
}
int main(){
	WORD ver = MAKEWORD(2, 2);
	WSADATA WSAData;
	WSAStartup(ver, &WSAData);
	//
	//
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);//host to net unsign short
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	if (SOCKET_ERROR==connect(_sock, (sockaddr*)&_sin, sizeof(_sin)))
	{
		printf("connect failed!\n");
	}
	else{
		printf("connect success!\n");
	}

	std::thread t1(Input, _sock);
	t1.detach();
	//t1.join();
	while (g_bRun)
	{
		fd_set fdRead;
		FD_ZERO(&fdRead);
		FD_SET(_sock, &fdRead);
		timeval t = {1,0};
		int nret = select(_sock + 1, &fdRead, 0, 0, &t);
		if (nret < 0)
		{
			printf("select 任务结束1！\n");
			break;
		}
		if (FD_ISSET(_sock,&fdRead))
		{
			FD_CLR(_sock, &fdRead);
			if (-1 == Processor(_sock))
			{

				printf("select 任务结束2！\n");
				break;
			}
		}
		//LOGIN login;
		//strcpy(login.name, "邢建锋");
		//strcpy(login.psw, "86420az");
		//send(_sock, (char*)&login, sizeof(login), 0);
		//Sleep(1000);
		
	}
	closesocket(_sock);
	WSACleanup();
	printf("退出！\n");
	//system("pause");
	return 0;
}