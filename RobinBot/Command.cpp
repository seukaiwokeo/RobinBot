#include "Command.h"
#include "Main.h"

pcmd_t Command::CommandByName(char* szName)
{	
	pcmd_t pCmd = g_Engine.pfnGetCmdList();	
	while( pCmd )
	{
		if( !strcmp( pCmd->pszName, szName) )
			return pCmd;
		pCmd = pCmd->pNext;
	}
	return 0;
}

void Command::EnDsCommand(pcmd_t cmd,bool enabled)
{
	DWORD OldProtect;
	static BYTE OLD[4] = { 0x00,0x00,0x00,0x00 };
	BYTE nops[4] = { 0xC3,0x90,0x90,0x90 };
	VirtualProtect((DWORD*)cmd->pfnFunc,sizeof(OLD),PAGE_EXECUTE_READWRITE,&OldProtect); // steam fix
	if ( enabled == true && OLD[0] != 0x00 )
	{
		*(DWORD*)(DWORD*)cmd->pfnFunc = *(DWORD*)OLD;
		memset(&OLD,0,sizeof(OLD));
	}
	else if ( enabled == false )
	{
		if ( OLD[0] == 0x00 )
		{
			*(DWORD*)OLD = *(DWORD*)(DWORD*)cmd->pfnFunc;
			*(DWORD*)(DWORD*)cmd->pfnFunc = *(DWORD*)nops;
		}
	}
}

extern Command* pCmd;