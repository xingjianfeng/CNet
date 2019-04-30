#define WIN32_LEAN_AND_MEAN
#include<Windows.h>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
struct Test{
	int a;
	char ch[10];
	double d;
}
int main(){
	WORD ver = MAKEWORD(2,2);
	WSADATA WSAData;
	WSAStartup(ver,&WSAData);
	WSACleanup();
	return 0;
}
