#pragma once

#include "Main.h"
#include "AutoOffset.h"

class SetupHooks;
typedef DWORD (SetupHooks::*LPTHREAD_METHOD)(LPVOID pParam);

typedef struct STARTUP_PARAM
{
    SetupHooks*        pClass;
    LPTHREAD_METHOD    pMethod;
    LPVOID            pParam;
} *LPSTARTUP_PARAM;

class SetupHooks
{
private:
	static int AddCommand (char *cmd_name,void (*function)(void));
	static cvar_t* RegisterVariable (char *szName,char *szValue,int flags);
public:
	DWORD Initialize(LPVOID pParam);
	static  DWORD StartFunc (LPSTARTUP_PARAM pStartup);
	void StartThread (LPTHREAD_METHOD pMethod, LPVOID pParam,LPDWORD pdwThreadID = NULL,
		LPSECURITY_ATTRIBUTES pSecurity = NULL,
		DWORD dwStackSize = NULL,
		DWORD dwFlags = 0);
};