/*
Original Project: Far Aimbot 0.5
--------------------------------
Author: WithCreative & HLRTeam
Created: HLRTeam
Updated: WithCreative & 10.12.2017
Extras: RobinLoader [DLL Injector] by WithCreative
Version 1: Trigger Bot, ESP Box (HLRTeam)
Version 2: Aimbot, Trigger Bot, ESP Box, Name ESP, No Recoil, Menu (WithCreative)
Special Thanks to: HLRTeam for ValveSDK & Source Base
Category: Game Hacking
Description: Counter-Strike - Engine SDK Hack
*/


#include "Main.h"
#include "SetupHooks.h"

char g_szBaseDir[256];
local_s g_Local;
SCREENINFO g_Screen;
GameInfo_s BuildInfo;
HINSTANCE hInstance;
cl_clientfunc_t *g_pClient = nullptr;
cl_clientfunc_t g_Client;
cl_enginefunc_t *g_pEngine = nullptr;
cl_enginefunc_t g_Engine;
engine_studio_api_t *g_pStudio = nullptr;
engine_studio_api_t g_Studio;
ofstream ofile;

string szDirFile( char* pszName )
{
	string szRet = g_szBaseDir;
	return (szRet + pszName);
}


void add_log(const char *fmt, ...)
{
	if(!fmt)
		return;
	va_list va_alist;
	char logbuf[256] = { 0 };
	va_start(va_alist, fmt);
	_vsnprintf(logbuf + strlen(logbuf), sizeof(logbuf)-strlen(logbuf), fmt, va_alist);
	va_end(va_alist);
	ofile << logbuf << endl;
}

BOOL APIENTRY DllMain( HINSTANCE hModule, DWORD dwReason, LPVOID lpReserved )
{
	if( dwReason == DLL_PROCESS_ATTACH )
	{
		if ( GetLastError() != ERROR_ALREADY_EXISTS )
		{
			AllocConsole();
			GetModuleFileName( hModule, (LPWSTR)g_szBaseDir, sizeof( g_szBaseDir ) );
			char* pos = g_szBaseDir + strlen( g_szBaseDir );
			while( pos >= g_szBaseDir && *pos!='\\' ) --pos; pos[ 1 ]=0;

			hInstance = hModule;
			DisableThreadLibraryCalls((HINSTANCE)hModule);
			SetupHooks* Hook = new SetupHooks;
			Hook->StartThread(&SetupHooks::Initialize, 0);
			delete Hook;
		}
	}
	return TRUE;
}