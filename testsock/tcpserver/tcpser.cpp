#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include"TCPServer.hpp"

int main(){
	CTCPServer server;
	server.InitSocket();
	server.Bind(NULL, 4567);
	server.Listen(5);
	//
	while (server.IsRun())
	{
		if (!server.OnRun())
		{
			break;
		}
		
	}
	server.Close();
	return 0;
}