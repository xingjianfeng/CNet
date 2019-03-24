#ifndef _MESSAGEHEAD_HPP_
#define _MESSAGEHEAD_HPP_
enum CMDINFO
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEWCLIENT,
	CMD_CLIENT_EXIT,
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
		datalen = sizeof(NEWUSER);
		cmd = CMD_NEWCLIENT;
		sock = 0;
	}
	int sock;
};
struct USEREXIT :public DATAHEADER
{
	USEREXIT()
	{
		datalen = sizeof(NEWUSER);
		cmd = CMD_CLIENT_EXIT;
		sock = 0;
	}
	int sock;
};
#endif