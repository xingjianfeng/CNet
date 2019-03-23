#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include<Windows.h>
#include<WinSock2.h>
#include<stdio.h>
#include<vector>
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
std::vector<SOCKET> g_clients;
int Processor(SOCKET _clisock)
{
	DATAHEADER header;
	int nlen = recv(_clisock + 1, (char*)&header, sizeof(header), 0);
	if (nlen <= 0)
	{
		printf("�ͻ���%d�˳������������\n",_clisock);
		return -1;
	}

	printf("�յ��ͻ������%d,���ݳ��ȣ�%d��\n", header.cmd, header.datalen);
	switch (header.cmd)
	{
	case CMD_LOGIN:
	{
					  LOGIN login = {};
					  recv(_clisock, (char*)&login + sizeof(DATAHEADER), sizeof(LOGIN)-sizeof(DATAHEADER), 0);
					  printf("��¼��Ϣ:name=%s,psw=%s���ͻ������%d,���ݳ��ȣ�%d\n", login.name, login.psw, login.cmd, login.datalen);

					  LOGINRESULT loginres;
					  send(_clisock, (char*)&loginres, sizeof(LOGINRESULT), 0);
	}
		break;
	case CMD_LOGOUT:
	{
					   LOGOUT logout = {};
					   recv(_clisock, (char*)&logout + sizeof(DATAHEADER), sizeof(LOGOUT)-sizeof(DATAHEADER), 0);
					   printf("�ǳ���Ϣ:name=%s���ͻ������%d,���ݳ��ȣ�%d\n", logout.name, logout.cmd, logout.datalen);
					   LOGOUTRESULT logoutres;
					   send(_clisock, (char*)&logoutres, sizeof(LOGOUTRESULT), 0);
	}
		break;
	default:
		printf("������Ϣ\n");
		break;
	}

	return 0;
}
int main(){
	WORD ver = MAKEWORD(2, 2);
	WSADATA WSAData;
	WSAStartup(ver, &WSAData);
	//
	SOCKET _sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	//
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);//host to net unsign short
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;// inet_addr("127.0.0.1");

	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin)))
	{
		printf("�󶨶˿�ʧ��\n");
	}
	else
	{
		printf("�󶨶˿ڳɹ�\n");
	}
	//
	if (SOCKET_ERROR == listen(_sock, 5))
	{
		printf("�����˿�ʧ��\n");
	}
	else
	{
		printf("�����˿ڳɹ�\n");
	}
	//

	
	char cmdbuf[128] = {};
	SOCKET socktmp = 0;
	timeval tout = { 1, 0 };
	//
	while (true)
	{

		fd_set fdRead, fdWrite, fdExp;
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);
		FD_SET(_sock, &fdRead);
		FD_SET(_sock, &fdWrite);
		FD_SET(_sock, &fdExp);
		for (int n = g_clients.size()-1; n >=0; n--)
		{
			FD_SET(g_clients[n],&fdRead);
		}
		//if (socktmp != 0)
		//{
		//	FD_SET(socktmp, &fdRead);
		//	socktmp = 0;
		//}
		int nret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, &tout);
		if (nret<0)
		{
			printf("select �������!\n");
		}
		if (FD_ISSET(_sock, &fdRead))
		{
			FD_CLR(_sock, &fdRead);
			sockaddr_in clientaddr = {};
			int addrlen = sizeof(clientaddr);
			SOCKET _clisock = INVALID_SOCKET;
			//char buf[] = "HELLO!  I am server!\n";
			_clisock = accept(_sock, (sockaddr*)&clientaddr, &addrlen);
			if (_clisock == INVALID_SOCKET)
			{
				printf("��Ч������\n");
			}
			else{
			//socktmp = _clisock;
			//FD_SET(_clisock, &fdRead);
				for (int n = g_clients.size() - 1; n >= 0; n--)
				{
					NEWUSER newuser;
					newuser.sock = (int)_clisock;
					send(g_clients[n], (char*)&newuser, sizeof(NEWUSER), 0);
				}
				g_clients.push_back(_clisock);
				printf("new client:<socket=%d>,ip=%s\n", _clisock, inet_ntoa(clientaddr.sin_addr));
			}
		}
		//�����ɶ�socket
		for (size_t n = 0; n< fdRead.fd_count; n++)
		{
			if (-1 == Processor(fdRead.fd_array[n]))
			{
				auto itr = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[n]);
				if (itr != g_clients.end())
				{
					g_clients.erase(itr);
				}
			}
		}
		printf("����ʱ�䴦������ҵ�񡭡�\n");
	}
	//������� ���socket
	for (size_t n = 0; n < g_clients.size(); n++)
	{
		closesocket(g_clients[n]);
	}
	closesocket(_sock);
	WSACleanup();
	return 0;
}