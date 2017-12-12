#include "SetupHooks.h"
#include "Command.h"
#include "Client.h"
#include "detours.h"

int SetupHooks::AddCommand(char *cmd_name,void(*function)(void))
{
	return 0;
}

cvar_t* SetupHooks::RegisterVariable (char *szName,char *szValue, int flags)
{
	cvar_t* pResult = g_Engine.pfnGetCvarPointer(szName);
	if(pResult != NULL)
		return pResult;
	return g_Engine.pfnRegisterVariable(szName, szValue, flags);
}

DWORD SetupHooks::StartFunc(LPSTARTUP_PARAM pStartup)
{
    SetupHooks* pClass        = pStartup->pClass;
    LPTHREAD_METHOD pMethod = pStartup->pMethod;
    LPVOID pParam            = pStartup->pParam;
    DWORD dwResult = (pClass->*pMethod)(pParam);
    delete pStartup;
    return dwResult;
}

void SetupHooks::StartThread(LPTHREAD_METHOD pMethod, LPVOID pParam, 
                             LPDWORD pdwThreadID /* = NULL */, 
                             LPSECURITY_ATTRIBUTES pSecurity /* = NULL */, 
                             DWORD dwStackSize /* = 0 */, 
                             DWORD dwFlags /* = 0 */)
{
    LPSTARTUP_PARAM pStartup = new STARTUP_PARAM;
    pStartup->pClass    = this;
    pStartup->pMethod    = pMethod;
    pStartup->pParam    = pParam;
    CreateThread(pSecurity, dwStackSize, (LPTHREAD_START_ROUTINE)StartFunc, pStartup, dwFlags, pdwThreadID);
}

DWORD SetupHooks::Initialize(LPVOID pParam)
{
	Client*	pClient = new Client;
	AutoOffset* Offset = new AutoOffset;

	while ( !Offset->GetRendererInfo() )
		Sleep(90);

	for (int i = 0;i < 10;i++)
	{
		g_pClient = (cl_clientfunc_t*)Offset->ClientFuncs();
		g_pEngine = (cl_enginefunc_t*)Offset->EngineFuncs();
		if(g_pClient && g_pEngine)
			goto Return;
	}
	Offset->Error("Couldn't find default scanning pattern.");

Return:
		
	g_pStudio = (engine_studio_api_t*)Offset->EngineStudio();

	Sleep(500);

	if(!g_pClient || !g_pEngine || !g_pStudio) 
		Offset->Error("OFFSET ERROR #1");
		
	RtlCopyMemory(&g_Engine, g_pEngine, sizeof(cl_enginefunc_t));
	RtlCopyMemory(&g_Studio, g_pStudio, sizeof(engine_studio_api_t));
	RtlCopyMemory(&g_Client, g_pClient, sizeof(cl_clientfunc_t));	
	
	pClient->HookClient();

	Sleep(500);
		
	g_pEngine->pfnAddCommand = &AddCommand;
	g_pEngine->pfnRegisterVariable = &RegisterVariable;
	g_pEngine->pfnHookUserMsg = &pfnHookUserMsg;

	Command* pCmd = new Command;
	pcmd_t cmd = pCmd->CommandByName("cmd");
	pCmd->EnDsCommand(cmd,false); // Fix - Can't "cmd", not connected

	g_Client.Initialize(g_pEngine, CLDLL_INTERFACE_VERSION);
	g_Client.HUD_Init();
	
	pCmd->EnDsCommand(cmd,true);
	
	g_pEngine->pfnAddCommand = g_Engine.pfnAddCommand;
	g_pEngine->pfnRegisterVariable = g_Engine.pfnRegisterVariable;
	g_pEngine->pfnHookUserMsg = g_Engine.pfnHookUserMsg;
	
	Sleep(500);

	pClient->HookEngine();
	pClient->HookStudio();
	
	delete pCmd;
	delete Offset;
	delete pClient;

	return 0;
}