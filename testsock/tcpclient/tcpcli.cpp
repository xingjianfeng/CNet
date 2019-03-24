#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include"TCPClient.hpp"
#include<thread>
bool g_bRun = true;
void Input(CTCPClient*client)
{
	while (true)
	{
		char cmdbuf[128] = {};
		scanf("%s", cmdbuf);
		if (0 == strcmp(cmdbuf, "exit"))
		{
			printf("退出线程!\n");
			client->Close();
			break;
		}

		else if (0 == strcmp(cmdbuf, "login")){
			LOGIN login;
			strcpy(login.name, "邢建锋");
			strcpy(login.psw, "86420az");
			client->Send(&login);
		}
		else if (0 == strcmp(cmdbuf, "logout")){

			LOGOUT logout;
			strcpy(logout.name, "邢建锋");
			client->Send(&logout);
		}
		else
		{
			printf("输入错误信息!\n");
		}
	}
}
int main(){
	CTCPClient client;
	client.InitSocket();
	client.Connect("127.0.0.1", 4567);
	std::thread t1(Input, &client);
	t1.detach();
	//t1.join();
	while (client.IsRun() )
	{
		if (!client.OnRun())
		{
			break;
		}
	}
	//system("pause");
	return 0;
}