#include "main.h"

pfnUserMsgHook pTeamInfo = NULL;
pfnUserMsgHook pCurWeapon = NULL;

int TeamInfo( const char *pszName, int iSize, void *pbuf )
{
	//add_log("pTeamInfo");
	BEGIN_READ( pbuf, iSize );
	int iIndex = READ_BYTE();
	char *szTeam = READ_STRING();
	_strlwr( szTeam );
	if( !strcmp( szTeam, "terrorist" ) )
	{
		if( iIndex == g_Local.iIndex ) { g_Local.iTeam = 1; }
		g_Player[iIndex].iTeam = 1;
	}
	else if( !strcmp( szTeam, "ct" ) )
	{
		if( iIndex == g_Local.iIndex ) { g_Local.iTeam = 2; }
		g_Player[iIndex].iTeam = 2;
	}
	else
	{
		if( iIndex == g_Local.iIndex ) { g_Local.iTeam = 0; }
		g_Player[iIndex].iTeam = 0;
	}
	return (*pTeamInfo)( pszName, iSize, pbuf );
}

int CurWeapon(const char *pszName, int iSize, void *pbuf)
{
	//add_log("CurWeapon");
	BEGIN_READ( pbuf, iSize );
	int iState = READ_BYTE();
	int iWeaponID = READ_CHAR();
	if( iState )
	{
		g_Local.iWeaponID = iWeaponID;
	}
	return pCurWeapon(pszName, iSize, pbuf);
}

int pfnHookUserMsg( char *szMsgName, pfnUserMsgHook pfn )
{
	#define HOOK_MSG(name) \
	if( !strcmp( szMsgName, #name ) ) \
	{ \
		p##name = pfn; \
		return g_Engine.pfnHookUserMsg( szMsgName, ##name ); \
	}

	HOOK_MSG(TeamInfo)
	HOOK_MSG(CurWeapon)

	return g_Engine.pfnHookUserMsg( szMsgName, pfn );
}