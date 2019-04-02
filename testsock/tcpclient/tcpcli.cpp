#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include"TCPClient.hpp"
#include<thread>
bool g_bRun = true;
const int cCount = 8;
const int cThrct = 1;
CTCPClient* client[cCount];
void Input()
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
			//client->Send(&login);
		}
		else if (0 == strcmp(cmdbuf, "logout")){

			LOGOUT logout;
			strcpy(logout.name, "邢建锋");
			//client->Send(&logout);
		}
		else
		{
			printf("输入错误信息!\n");
		}
	}
}
void SendThread(int n)
{
	int c = cCount / cThrct;
	int begin = n*c;
	int end = (n + 1)*c;
	for (int i = begin; i < end; i++)
	{
		client[i] = new CTCPClient();
		int n=client[i]->Connect("127.0.0.1", 4567);
		if (n == -1)
			printf("第%d个客户端连接服务器失败", i);
		else
			printf("第%d个客户端连接服务器成功", i);
	}
	LOGIN login;
	strcpy(login.name, "xingjianfeng");
	strcpy(login.psw, "xingjianfeng");
	while (g_bRun)
	{
		for (int i = begin; i < end; i++)
		{
			if (!client[i]->OnRun())
			{
				client[i]->Close();
				delete client[i];
				client[i] = nullptr;
				g_bRun = false;
				break;
			}
			else
			client[i]->Send(&login);
		}
	}
	for (int i = begin; i < end; i++)
	{
		if (client[i])
		{
			client[i]->Close();
			delete client[i];
		}
	}

}
int main(){
	std::thread t1[cThrct];
	for (size_t i = 0; i < cThrct; i++)
	{
		t1[i]=std::thread(SendThread,i);
	}
	for (size_t i = 0; i < cThrct; i++)
	{
		t1[i].join();
	}
	//std::thread t1(Input, &client);
	//t1.detach();
	//t1.join();
	
	system("pause");
	return 0;
}