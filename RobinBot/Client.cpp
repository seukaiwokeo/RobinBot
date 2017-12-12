#include "SvcMessage.h"
#include "AutoOffset.h"
#include "players.h"
#include "Command.h"
#include "drawing.h"
#include "Client.h"
#include "Main.h"
#include <sstream>
#pragma warning(disable:4800)
#pragma warning(disable:4002)
#pragma warning(disable:4244)
#pragma warning(disable:4996)

player_s g_Player[33];
CPlayers g_cPlayers;
CommandInterpreter cmd;
AIMBOT_INFO g_sAimbot;

//Menu Variables
bool selectedItem1 = true;
bool selectedItem2 = false;
bool selectedItem3 = false;
bool selectedItem4 = false;
bool selectedItem5 = false;
bool selectedItem6 = false;
bool selectedItem7 = false;
bool activeItem1 = true;
bool activeItem2 = true;
bool activeItem3 = true;
bool activeItem4 = true;
bool activeItem5 = true;
bool activeItem6 = true;
bool activeItem7 = true;

int c_Player[33];
int c_PlayerDist[33];

bool bGameStart = false;

static bool iGlock = true;
static bool iUsp = true;
static bool iDeagle = true;
static bool iAk47 = true;
static bool iM4a1 = true;
static bool iAwp = true;
static bool iKnife = true;
static bool iOth = true;

#define MIN_SMOOTH 0.000001f
#define CVAR(cvar_name) cvar.##cvar_name

void Client::StudioEntityLight(struct alight_s *plight)
{
	CPlayer *cPlayer = nullptr;
	add_log("StudioEntityLight start");
	if (bGameStart && g_Local.bConnected)
	{
		add_log("StudioEntityLight 1");
		cl_entity_s *pEntity = g_pStudio->GetCurrentEntity();
		add_log("StudioEntityLight 2");
		if (pEntity && pEntity->player && pEntity->index != g_cPlayers.Me()->Entity()->index)
		{
			add_log("StudioEntityLight 3");
			if ( CVAR(aim_target) > 20 ) CVAR(aim_target) = 20;
			add_log("StudioEntityLight 4");
			cPlayer = g_cPlayers.Player(pEntity->index);
			add_log("StudioEntityLight 5");
			cPlayer->GetBoneInformation(VM_HITBOX, CVAR(aim_target));
			add_log("StudioEntityLight 6");
			cPlayer->Info()->vAimBone.z += CVAR(aim_height);
			add_log("StudioEntityLight 7");
		}
	}
	add_log("StudioEntityLight end");
	return g_Studio.StudioEntityLight(plight);
}

void strreplace(char* buf, const char* search, const char* replace)
{
	char* p = buf;
	int l1 = strlen(search);
	int l2 = strlen(replace);
	while (p = strstr(p, search))
	{
		memmove(p + l2, p + l1, strlen(p + l1) + 1);
		memcpy(p, replace, l2);
		p += l2;
	}
}

void RunScript(char* scriptName)
{
	//Devre dýþý býrakýldý.
}

void Client::InitHack(void)
{
	g_Screen.iSize = sizeof(SCREENINFO);
	g_Engine.pfnGetScreenInfo(&g_Screen);

	g_Engine.Con_Printf( "\n\t--->RobinBot has loaded.\n" );

	//HideModule(hInstance);
	//HideModuleXta(hInstance);
	//HideModuleFromPEB(hInstance);f
	//RemovePeHeader((DWORD)hInstance);
	//DestroyModuleHeader(hInstance);

	cmd.init();
	cvar.init();	
	//RunScript("config.cfg");
	//string file = szDirFile("config.cfg");
	//cmd.execFile(file.c_str());
}

bool Client::IsInFOV(float *fScreen, float fFov)
{
	if ((fFov == 0.0f)								||
		((fScreen[0] <= g_Screen.iWidth/2 + fFov)	&&
		(fScreen[0] >= g_Screen.iWidth/2 - fFov)	&&
		(fScreen[1] <= g_Screen.iHeight/2 + fFov)	&&
		(fScreen[1] >= g_Screen.iHeight/2 - fFov)))
		return true;

	return false;
}

void Client::PredictEntity(cl_entity_s *pEntity, Vector *vOutput, unsigned int fAmount)
{
	net_status_s sNetwork;
	g_Engine.pNetAPI->Status(&sNetwork);
	int nHistory = (pEntity->current_position + HISTORY_MAX - fAmount) % HISTORY_MAX;
	Vector vOldOrigin, vCurOrigin, vDeltaOrigin;
	vOldOrigin = pEntity->ph[nHistory].origin;
	vCurOrigin = pEntity->ph[pEntity->current_position].origin;
	VectorSubtract(vCurOrigin, vOldOrigin, vDeltaOrigin);
	float fPing = (float)sNetwork.latency;
	if (fPing < 0.0f) fPing = -fPing;
	VectorScale(vDeltaOrigin, fPing, vDeltaOrigin);
	VectorAdd(pEntity->origin, vDeltaOrigin, *vOutput);
}

int Client::HUD_Reset(void)
{
	if (bGameStart)
	{
		for (int i = 1; i < 33; i++)
		{
			PPLAYER_INFO pInfo = g_cPlayers.Player(i)->Info();
			pInfo->bIsBoneScreen	= false;
			pInfo->bIsScreen		= false;
			pInfo->bIsVulnerable	= false;
			pInfo->bValid			= false;
			pInfo->sSound.bValid	= false;
			pInfo->sOldSound.bValid	= false;
		}
	}
	return g_Client.HUD_Reset();
}

void Client::HUD_Frame(double time)
{
	if ( Init )
	{
		AutoOffset* Offset = new AutoOffset;

		while ( !Offset->GetRendererInfo() )
			Sleep(90);

		Offset->SW = g_Studio.IsHardware();
		Offset->GameInfo();
		
		Offset->Find_SVCBase();
		Offset->Find_MSGInterface();
		Offset->Find_CBuf_AddText();
		Offset->Patch_CL_ConnectionlessPacket();

		CBuf_AddText_Orign = (HL_MSG_CBuf_AddText)Offset->pCBuf_AddText;

		SVC_StuffText_Orign = HookServerMsg(SVC_STUFFTEXT,SVC_StuffText,Offset);
		SVC_SendCvarValue_Orign = HookServerMsg(SVC_SENDCVARVALUE,SVC_SendCvarValue,Offset);
		SVC_SendCvarValue2_Orign = HookServerMsg(SVC_SENDCVARVALUE2,SVC_SendCvarValue2,Offset);

		Offset->SpeedPtr = (DWORD)Offset->SpeedHackPtr();
		
		InitHack();
		Init = false;

		delete Offset;
	}
	g_Engine.pNetAPI->Status(&(g_Local.net_status));
	g_Local.bConnected = (bool)g_Local.net_status.connected;
	g_Client.HUD_Frame(time);
}
void V_CalcRefdef(struct ref_params_s *pparams)
{
	if(activeItem5) {
		pparams->punchangle[0] = 0.0f;
		pparams->punchangle[1] = 0.0f;
	}
	if(GetAsyncKeyState(VK_NUMPAD1)) {
		pparams->health++;
	}
	VectorCopy(pparams->punchangle, g_Local.vPunchAngle);
	VectorCopy(pparams->viewangles, g_Local.vViewAngle);
	g_Local.iHP = pparams->health;
	g_Client.V_CalcRefdef(pparams);
}
void Client::FarAim()
{
	CMe *cMe = g_cPlayers.Me();
	cMe->GetInfo();
	ZeroMemory(&g_sAimbot.sTarget, sizeof(TARGET_INFO));
	
	double preDistance = 999999.0;

	for (int i = 1; i < 33; i++)
	{
		if (cMe->Entity()->index == i || cMe->Entity()->curstate.iuser2 == i)
			continue;
		
		CPlayer *cPlayer = g_cPlayers.Player(i);
		cPlayer->GetInfo(i);

	
		
		if (!cPlayer->Entity() || !cPlayer->Entity()->player)
			continue;
		
		if (cPlayer->Info()->bValid)
		{
			VectorCopy(cPlayer->Entity()->origin, cPlayer->Info()->vOrigin);
		}
		
		bool bIsVisible = cMe->Visible(cPlayer->Position());
		
		if (!cPlayer->Info()->bValid && !bIsVisible)
		{
			g_Local.trigger[i] = false;
			cPlayer->Info()->sSound.bValid = false;
			continue;
		}

		cPlayer->ScreenPosition(NULL);
		if (CVAR(aim_triggerbot) && cPlayer->Info()->bValid && cPlayer->BoneScreenPosition(NULL))
		{
			if (cMe->Info()->eTeam != cPlayer->Info()->eTeam && (CVAR(aim_autowall) || cMe->Visible(cPlayer->BonePosition())) )
			{
				float tFov = CVAR(aim_triggerbot_fov) * 15.0f;
				float fDistance = cMe->Distance(cPlayer->Position());
				tFov = tFov / fDistance;
				if (IsInFOV(cPlayer->Info()->fBoneScreen, tFov))
				{
					g_Local.trigger[i] = false;
				}
				else
					g_Local.trigger[i] = false;
			}
			else
				g_Local.trigger[i] = false;
		}
		else
			g_Local.trigger[i] = false;

		if (CVAR(aim_active) && cPlayer->Info()->bValid && cPlayer->BoneScreenPosition(NULL))
		{
			if (cMe->Info()->eTeam != cPlayer->Info()->eTeam && (CVAR(aim_autowall) || cMe->Visible(cPlayer->BonePosition())))
			{
				float fFov = CVAR(aim_fov) * 15.0f;
				if (CVAR(aim_distancebasedfov))
				{
					float fDistance = cMe->Distance(cPlayer->Position());
					fFov = fFov / fDistance;
				}
				if (IsInFOV(cPlayer->Info()->fBoneScreen, fFov))
				{
					float fCursorDist = POW(g_Screen.iWidth / 2 - cPlayer->Info()->fBoneScreen[0]) +
						POW(g_Screen.iHeight / 2 - cPlayer->Info()->fBoneScreen[1]);

					PTARGET_INFO pTarget = &(g_sAimbot.sTarget);
					if (!pTarget->cPlayer || fCursorDist < pTarget->fDistance)
					{
						pTarget->fDistance = fCursorDist;
						pTarget->cPlayer = cPlayer;

						pTarget->fScreen[0] = cPlayer->Info()->fBoneScreen[0];
						pTarget->fScreen[1] = cPlayer->Info()->fBoneScreen[1];
					}
				}
			}
			
			BYTE r,g,b = 0;
			if ( cPlayer->Info()->eTeam == TEAM_T )
			{ r = 255, g = 0, b = 0; }
			else if ( cPlayer->Info()->eTeam == TEAM_CT )
			{ r = 0, g = 0, b = 255; }
			else if ( cPlayer->Info()->eTeam == TEAM_SPEC )
			{ r = 255, g = 255, b = 0; }		
			if ( bIsVisible )
			{ r = 0, g = 255, b = 0; }
					
			if (CVAR(aim_avdraw) && cPlayer->Info()->bIsScreen && cPlayer->Info()->bIsBoneScreen)
			{
				if(activeItem1) {
					if(g_Player[i].bAlive) {
						if(cPlayer->Info()->eTeam != cMe->Info()->eTeam) {
							g_Drawing.DrawBox((int)cPlayer->Info()->fBoneScreen[0] -6, (int)cPlayer->Info()->fBoneScreen[1] -6, 12, 12, 1, 255,0,0,150);
						} else {
							g_Drawing.DrawBox((int)cPlayer->Info()->fBoneScreen[0] -6, (int)cPlayer->Info()->fBoneScreen[1] -6, 12, 12, 1, 0,255,0,150);
						}
						
					}
				}
				if(activeItem2) {
					if(g_Player[i].bAlive) {
						if(cPlayer->Info()->eTeam != cMe->Info()->eTeam) {
							g_Drawing.DrawString((int)cPlayer->Info()->fBoneScreen[0] -23, (int)cPlayer->Info()->fBoneScreen[1] -25, 255,0,0, g_Player[i].Info.name);
						} else {
							g_Drawing.DrawString((int)cPlayer->Info()->fBoneScreen[0] -26, (int)cPlayer->Info()->fBoneScreen[1] -25, 0,255,0, g_Player[i].Info.name);
						}
					} else {
						g_Drawing.DrawString((int)cPlayer->Info()->fBoneScreen[0] -21, (int)cPlayer->Info()->fBoneScreen[1] -25, 255,255,255, "Dead");
					}
				}
				
				if(GetAsyncKeyState(VK_LSHIFT)) {
					if(activeItem3) {
						if(cPlayer->Info()->eTeam != cMe->Info()->eTeam) {
							if(g_Player[i].bAlive) {
								g_Engine.pfnSetMousePos((int)cPlayer->Info()->fBoneScreen[0], (int)cPlayer->Info()->fBoneScreen[1]);
								if(activeItem4) { g_Local.trigger[i] = true; } else { g_Local.trigger[i] = false; }
							} else {
								g_Local.trigger[i] = false;
							}
						} else {
							g_Local.trigger[i] = false;
						}
					}
				} else {
					g_Local.trigger[i] = false;
				}
			}
			
		}		
	}
	
	if (g_sAimbot.sTarget.cPlayer != NULL	&&	!cMe->Info()->bIsReloading	&&	cMe->Info()->bIsGoodWeapon	&&	!cMe->IsFlashed())
	{
		Vector vAimOrigin;
		VectorCopy((*g_sAimbot.sTarget.cPlayer->BonePosition()), vAimOrigin);
		
		if (CVAR(aim_prediction))
		{
			Vector vDelta;
			cl_entity_s *pEntity = g_sAimbot.sTarget.cPlayer->Entity();
			VectorSubtract(vAimOrigin, pEntity->origin, vDelta);
			PredictEntity(pEntity, &vAimOrigin, (int)CVAR(aim_prediction));
			VectorAdd(vAimOrigin, vDelta, vAimOrigin);
		}
		cMe->CalcViewAngles(&vAimOrigin, NULL);
		g_sAimbot.bActive = true;
	}
	else
	{
		g_sAimbot.bActive = false;
	}
}



void Client::HUD_Redraw(float time, int intermission)
{
	g_Client.HUD_Redraw(time, intermission);
	
	if ( !bGameStart )
	{
		HUD_Reset();
		bGameStart = true;
	}
	
	cl_entity_t *pLocal = g_Engine.GetLocalPlayer();
	g_Local.iIndex = pLocal->index;

	for (int i = 1; i < 33; i++)
	{
		cl_entity_s *ent = g_Engine.GetEntityByIndex(i);
		g_Engine.pfnGetPlayerInfo(i, &g_Player[i].Info);
		g_Player[i].bAlive = ent && !(ent->curstate.effects & EF_NODRAW) && ent->player && ent->curstate.movetype != 6 && ent->curstate.movetype != 0;
		g_Player[i].vOrigin = ent->origin;
		g_Player[i].Angles = ent->angles;
	}

	if (g_Local.bConnected) {
		FarAim();
		HackMenu();
	}
}

void Client::HUD_PlayerMove(struct playermove_s *ppmove, int server)
{
	g_Client.HUD_PlayerMove( ppmove, server );

	g_Engine.pEventAPI->EV_LocalPlayerViewheight(g_Local.vEye);
	g_Local.vOrigin = ppmove->origin;
	g_Local.vEye = g_Local.vEye + ppmove->origin;

	g_Local.iFlags = ppmove->flags;
	g_Local.iWaterLevel = ppmove->waterlevel;

	g_Local.fOnLadder = ppmove->movetype == 5;
	g_Local.iUseHull=ppmove->usehull;
	
	Vector vTemp1 = g_Local.vOrigin;
	vTemp1[2] -= 8192;
	pmtrace_t *trace = g_Engine.PM_TraceLine(g_Local.vOrigin, vTemp1, 1, ppmove->usehull, -1); 

	g_Local.flHeight=abs(trace->endpos.z - g_Local.vOrigin.z);

	if(g_Local.flHeight <= 60) g_Local.flGroundAngle=acos(trace->plane.normal[2])/M_PI*180; 
	else g_Local.flGroundAngle = 0;

	Vector vTemp2=trace->endpos;
	pmtrace_t pTrace;
	
	g_Engine.pEventAPI->EV_SetTraceHull( ppmove->usehull );
	g_Engine.pEventAPI->EV_PlayerTrace( g_Local.vOrigin, vTemp2, PM_GLASS_IGNORE | PM_STUDIO_BOX, g_Local.iIndex, &pTrace );
	
	if( pTrace.fraction < 1.0f )
	{
		g_Local.flHeight=abs(pTrace.endpos.z - g_Local.vOrigin.z);
		int ind=g_Engine.pEventAPI->EV_IndexFromTrace(&pTrace);
		if(ind>0&&ind<33)
		{
			float dst=g_Local.vOrigin.z-(g_Local.iUseHull==0?32:18)-g_Player[ind].vOrigin.z-g_Local.flHeight;
			if(dst<30)
			{
				g_Local.flHeight-=14.0;
			}
		}
	}
}

void Client::SmoothAimAngles(Vector *vStart, Vector *vTarget, Vector *vOutput, float fSmoothness)
{
	if(fSmoothness == MIN_SMOOTH)
		return;
	Vector vDelta;
	VectorSubtract((*vTarget), (*vStart), vDelta);
	if (vDelta[1] > 180.0f) vDelta[1] -= 360.0f;
	if (vDelta[1] < -180.0f) vDelta[1] += 360.0f;
	vDelta[0] = vDelta[0] / fSmoothness;
	vDelta[1] = vDelta[1] / fSmoothness;
	VectorAdd((*vStart), vDelta, (*vOutput));
}

void Client::Bhop(float frametime, struct usercmd_s *cmd)
{
	static bool lastFramePressedJump=false;
	static bool JumpInNextFrame=false;
	int curFramePressedJump=cmd->buttons&IN_JUMP;
	if(JumpInNextFrame)
	{
		JumpInNextFrame=false;
		cmd->buttons|=IN_JUMP;
		goto bhopfuncend;
	}
	static int inAirBhopCnt=0;bool isJumped=false;

	if (curFramePressedJump)
	{
		cmd->buttons &= ~IN_JUMP;
		if( ((!lastFramePressedJump)|| g_Local.iFlags&FL_ONGROUND || g_Local.iWaterLevel >= 2 || g_Local.fOnLadder==1 || g_Local.flHeight<=2))
		{
			if(true)
			{
				static int bhop_jump_number=0;
				bhop_jump_number++;
				if(bhop_jump_number>=2)
				{
					bhop_jump_number=0;
					JumpInNextFrame=true; 
					goto bhopfuncend;
				}
			}
			{
				inAirBhopCnt=4;isJumped=true;
				cmd->buttons |= IN_JUMP;
			}
		} 
	}
	if(!isJumped)
	{
		if(inAirBhopCnt>0)
		{
			if(inAirBhopCnt%2==0) {cmd->buttons |= IN_JUMP;}
			else cmd->buttons &= ~IN_JUMP;
			inAirBhopCnt--;
		}
	}
bhopfuncend:
	lastFramePressedJump=curFramePressedJump;
}

void Client::weaponSettings()
{
	//Silahlarý engellemek için yapýlmýþtýr fakat tam performans için devredýþý býrakýlmýþtýr.
	/*if ( g_Local.iWeaponID == WEAPONLIST_GLOCK18 )
	{
		if ( iGlock )
		{
			g_Engine.pfnClientCmd("wpn_glock");
			iGlock = false;
			iUsp = true;
			iDeagle = true;
			iAk47 = true;
			iM4a1 = true;
			iAwp = true;
			iKnife = true;
			iOth = true;
		}
	}
	else if ( g_Local.iWeaponID == WEAPONLIST_USP )
	{
		if ( iUsp )
		{
			g_Engine.pfnClientCmd("wpn_usp");
			iGlock = true;
			iUsp = false;
			iDeagle = true;
			iAk47 = true;
			iM4a1 = true;
			iAwp = true;
			iKnife = true;
			iOth = true;
		}
	}
	else if ( g_Local.iWeaponID == WEAPONLIST_DEAGLE )
	{
		if ( iDeagle )
		{
			g_Engine.pfnClientCmd("wpn_deagle");
			iGlock = true;
			iUsp = true;
			iDeagle = false;
			iAk47 = true;
			iM4a1 = true;
			iAwp = true;
			iKnife = true;
			iOth = true;
		}
	}
	else if ( g_Local.iWeaponID == WEAPONLIST_AK47 )
	{
		if ( iAk47 )
		{
			g_Engine.pfnClientCmd("wpn_ak47");
			iGlock = true;
			iUsp = true;
			iDeagle = true;
			iAk47 = false;
			iM4a1 = true;
			iAwp = true;
			iKnife = true;
			iOth = true;
		}
	}
	else if ( g_Local.iWeaponID == WEAPONLIST_M4A1 )
	{
		if ( iM4a1 )
		{
			g_Engine.pfnClientCmd("wpn_m4a1");
			iGlock = true;
			iUsp = true;
			iDeagle = true;
			iAk47 = true;
			iM4a1 = false;
			iAwp = true;
			iKnife = true;
			iOth = true;
		}
	}
	else if ( g_Local.iWeaponID == WEAPONLIST_AWP )
	{
		if ( iAwp )
		{
			g_Engine.pfnClientCmd("wpn_awp");
			iGlock = true;
			iUsp = true;
			iDeagle = true;
			iAk47 = true;
			iM4a1 = true;
			iAwp = false;
			iKnife = true;
			iOth = true;
		}
	}
	else if ( g_Local.iWeaponID == WEAPONLIST_KNIFE )
	{
		if ( iKnife )
		{
			g_Engine.pfnClientCmd("wpn_knf");
			iGlock = true;
			iUsp = true;
			iDeagle = true;
			iAk47 = true;
			iM4a1 = true;
			iAwp = true;
			iKnife = false;
			iOth = true;
		}
	}
	else
	{
		if ( g_Local.iWeaponID != WEAPONLIST_GLOCK18 && g_Local.iWeaponID != WEAPONLIST_USP && 
			g_Local.iWeaponID != WEAPONLIST_DEAGLE && g_Local.iWeaponID != WEAPONLIST_AK47 &&
			g_Local.iWeaponID != WEAPONLIST_M4A1 && g_Local.iWeaponID != WEAPONLIST_AWP 
			&& g_Local.iWeaponID != WEAPONLIST_KNIFE )
		{
			if ( iOth )
			{
				g_Engine.pfnClientCmd("wpn_other");
				iGlock = true;
				iUsp = true;
				iDeagle = true;
				iAk47 = true;
				iM4a1 = true;
				iAwp = true;
				iKnife = true;
				iOth = false;
			}
		}
	}*/
}

void Client::CL_CreateMove(float frametime, struct usercmd_s *cmd, int active)
{
	g_Client.CL_CreateMove(frametime, cmd, active);

	g_Local.frametime = frametime;

	cl_entity_t *pLocal = g_Engine.GetLocalPlayer();
	g_Local.bAlive = pLocal && !(pLocal->curstate.effects & EF_NODRAW) && pLocal->player && pLocal->curstate.movetype !=6 && pLocal->curstate.movetype != 0;

	if (g_Local.bAlive && active && !Init && g_Local.bConnected)
	{
		bool bDoSmoothAim = false;
		weaponSettings();
		if (CVAR(bhop))
		{
			Bhop(frametime, cmd);
		}
		if (cvar.aim_triggerbot)
		{
			for(int i = 1;i < 33;i++)
				if ( g_Local.trigger[i] )
					cmd->buttons |= IN_ATTACK;
		}
		if (g_sAimbot.bActive)
		{
			bDoSmoothAim = true;
			if (cmd->buttons & IN_ATTACK)
			{
				if (!g_sAimbot.dwStartTime)
					g_sAimbot.dwStartTime = GetTickCount();
				DWORD dwNow = GetTickCount();
				if ((CVAR(aim_time) == 0.0f || (g_sAimbot.dwStartTime + CVAR(aim_time)) >= dwNow) &&
					(CVAR(aim_delay) == 0.0f || (g_sAimbot.dwStartTime + CVAR(aim_delay)) <= dwNow))
				{
					VectorCopy(g_cPlayers.Me()->Info()->vAimAngles, cmd->viewangles);
				}
				else
				{
					bDoSmoothAim = false;
				}
			}
			else
			{
				g_sAimbot.dwStartTime = 0;
			}
		}
		if (CVAR(aim_active) && CVAR(aim_smoothness))
		{
			if (bDoSmoothAim)
			{
				SmoothAimAngles(&(g_cPlayers.Me()->Info()->vPreAimAngles), &(g_cPlayers.Me()->Info()->vAimAngles), &(g_cPlayers.Me()->Info()->vPreAimAngles), CVAR(aim_smoothness));
				VectorCopy(g_cPlayers.Me()->Info()->vPreAimAngles, cmd->viewangles);
			}
			else
			{
				Vector vDelta;
				VectorSubtract(g_cPlayers.Me()->Info()->vPreAimAngles, g_cPlayers.Me()->Info()->vViewAngles, vDelta);
				if (vDelta[0] == 0.0f && vDelta[1] == 0.0f)
				{
					VectorCopy(g_cPlayers.Me()->Info()->vViewAngles, g_cPlayers.Me()->Info()->vPreAimAngles);
				}
				else
				{
					SmoothAimAngles(
						&(g_cPlayers.Me()->Info()->vPreAimAngles), 
						&(cmd->viewangles), 
						&(g_cPlayers.Me()->Info()->vPreAimAngles), CVAR(aim_smoothness));
					VectorCopy(g_cPlayers.Me()->Info()->vPreAimAngles, cmd->viewangles);
				}
			}
		}
	}
}

void Client::HookEngine(void)
{
	memcpy( &g_Engine, (LPVOID)g_pEngine, sizeof( cl_enginefunc_t ) );
}

void Client::HookStudio(void)
{
	memcpy( &g_Studio, (LPVOID)g_pStudio, sizeof( engine_studio_api_t ) );
	g_pStudio->StudioEntityLight = StudioEntityLight;
}
/*
int HUD_AddEntity(int type, struct cl_entity_s *ent, const char *modelname)
{
	int iRet = g_Client.HUD_AddEntity(type, ent, modelname);
	
	CPlayer *cPlayer;
	CMe *cMe = g_cPlayers.Me();
	cMe->GetInfo();

	for (int i = 1; i < 33; i++)
	{
		if (cMe->Entity()->index == i || cMe->Entity()->curstate.iuser2 == i)
			continue;

		cPlayer = g_cPlayers.Player(i);
		cPlayer->GetInfo(i);

		if (!cPlayer->Entity() || !cPlayer->Entity()->player)
			continue;

		BYTE r, g, b = 0;
		if (cPlayer->Info()->eTeam == TEAM_T)
		{
			r = 255, g = 0, b = 0;
		}
		else if (cPlayer->Info()->eTeam == TEAM_CT)
		{
			r = 0, g = 0, b = 255;
		}
		else if (cPlayer->Info()->eTeam == TEAM_SPEC)
		{
			r = 255, g = 255, b = 0;
		}
		if (CVAR(esp_barel) && cPlayer->Info()->bValid && g_Player[i].bAlive)
		{
			Vector fwd;
			g_Engine.pfnAngleVectors(g_Player[i].Angles, fwd, 0, 0);
			VectorMul(fwd, 5000, fwd);
			fwd[0] = cPlayer->Info()->vAimBone.x + fwd[0];
			fwd[1] = cPlayer->Info()->vAimBone.y + fwd[1];
			fwd[2] = cPlayer->Info()->vAimBone.z + fwd[2];
			int beamindex = g_Engine.pEventAPI->EV_FindModelIndex("sprites/laserbeam.spr");
			fwd.z = -fwd.z;
			g_Engine.pEfxAPI->R_BeamPoints(cPlayer->Info()->vAimBone, fwd, beamindex,
				g_Local.frametime + 0.0001f, 1.0f, 0, 32, 0, 0, g_Local.frametime, r, g, b);
		}
	}

	return iRet;
}
*/
void Client::HookClient(void)
{
	memcpy( &g_Client, (LPVOID)g_pClient, sizeof( cl_clientfunc_t ) );
	g_pClient->HUD_Reset = HUD_Reset;
	g_pClient->HUD_Frame = HUD_Frame;
	g_pClient->HUD_Redraw = HUD_Redraw;
	g_pClient->HUD_PlayerMove = HUD_PlayerMove;
	g_pClient->CL_CreateMove = CL_CreateMove;
	g_pClient->V_CalcRefdef = V_CalcRefdef;
}
//HackMenu
bool isMenuVisible = true;
void Client::HackMenu() 
{
	int mX = 10;
	int mY = 10;
	int mW = 182;
	int mH = 123;
	//border--
	int bX = mX - 1;
	int bY = mY - 1;
	int bW = mW + 2;
	int bH = mH + 2;
	int bS = 1;
	if(GetAsyncKeyState(VK_NUMPAD5)&1) {
		isMenuVisible = !isMenuVisible;
	}
	//Functions
	if(isMenuVisible) {
		if(GetAsyncKeyState(VK_UP)&1) {
			if(selectedItem2) { selectedItem2 = false; selectedItem1 = true; } else
				if(selectedItem3) { selectedItem3 = false; selectedItem2 = true; } else
					if(selectedItem4) { selectedItem4 = false; selectedItem3 = true; } else
						if(selectedItem5) { selectedItem5 = false; selectedItem4 = true; } else
							if(selectedItem6) { selectedItem6 = false; selectedItem5 = true; } else
								if(selectedItem7) { selectedItem7 = false; selectedItem6 = true; }
		}
		if(GetAsyncKeyState(VK_DOWN)&1) {
			if(selectedItem1) { selectedItem2 = true; selectedItem1 = false; } else
				if(selectedItem2) { selectedItem3 = true; selectedItem2 = false; } else
					if(selectedItem3) { selectedItem4 = true; selectedItem3 = false; } else
						if(selectedItem4) { selectedItem5 = true; selectedItem4 = false; } else
							if(selectedItem5) { selectedItem6 = true; selectedItem5 = false; } else
								if(selectedItem6) { selectedItem7 = true; selectedItem6 = false; }
		}
		if(GetAsyncKeyState(VK_RIGHT)&1) {
			if(selectedItem1) { activeItem1 = true; }
			if(selectedItem2) { activeItem2 = true; }
			if(selectedItem3) { activeItem3 = true; }
			if(selectedItem4) { activeItem4 = true; }
			if(selectedItem5) { activeItem5 = true; }
			if(selectedItem6) { activeItem6 = true; }
		}
		if(GetAsyncKeyState(VK_LEFT)&1) {
			if(selectedItem1) { activeItem1 = false; }
			if(selectedItem2) { activeItem2 = false; }
			if(selectedItem3) { activeItem3 = false; }
			if(selectedItem4) { activeItem4 = false; }
			if(selectedItem5) { activeItem5 = false; }
			if(selectedItem6) { activeItem6 = false; }
		}
		//<menubackground>
		g_Drawing.FillArea(mX,mY,mW,mH,0,0,0,110);
		g_Drawing.DrawString(mX + 10, mY + 7, 173, 216, 230, "        --RobinBot--");
		g_Drawing.DrawBox(bX,bY,bW,bH,bS,255,255,255,155);
		//</menubackground>
		//<espbox>
		if(selectedItem1) {
			g_Drawing.DrawString(mX + 10, mY + 25, 255, 165, 0, "ESP Box"); 
		} else {
			g_Drawing.DrawString(mX + 10, mY + 25, 255, 255, 255, "ESP Name");
		}
		if(activeItem1) {
			g_Drawing.DrawString(mX + 150, mY + 25, 0, 255, 0, " ON");
		} else {
			g_Drawing.DrawString(mX + 150, mY + 25, 255, 0, 0, "OFF");
		}
		//</espbox>
		//<espteam>
		if(selectedItem2) {
			g_Drawing.DrawString(mX + 10, mY + 38, 255, 165, 0, "ESP Team");
		} else {
			g_Drawing.DrawString(mX + 10, mY + 38, 255, 255, 255, "ESP Team");
		}
		if(activeItem2) {
			g_Drawing.DrawString(mX + 150, mY + 38, 0, 255, 0, " ON");
		} else {
			g_Drawing.DrawString(mX + 150, mY + 38, 255, 0, 0, "OFF");
		}
		//</espteam>
		//<aimbot>
		if(selectedItem3) {
			g_Drawing.DrawString(mX + 10, mY + 51, 255, 165, 0, "Aimbot");
		} else {
			g_Drawing.DrawString(mX + 10, mY + 51, 255, 255, 255, "Aimbot");
		}
		if(activeItem3) {
			g_Drawing.DrawString(mX + 135, mY + 51, 0, 255, 0, "SHIFT");
		} else {
			g_Drawing.DrawString(mX + 150, mY + 51, 255, 0, 0, "OFF");
		}
		//</aimbot>
		//<attackmode>
		if(selectedItem4) {
			g_Drawing.DrawString(mX + 10, mY + 64, 255, 165, 0, "Attack Mode");
		} else {
			g_Drawing.DrawString(mX + 10, mY + 64, 255, 255, 255, "Attack Mode");
		}
		if(activeItem4) {
			g_Drawing.DrawString(mX + 135, mY + 64, 0, 255, 0, "AUTO");
		} else {
			g_Drawing.DrawString(mX + 124, mY + 64, 255, 0, 0, "MANUAL");
		}
		//</attackmode>
		//<norecoil>
		if(selectedItem5) {
			g_Drawing.DrawString(mX + 10, mY + 77, 255, 165, 0, "No Recoil");
		} else {
			g_Drawing.DrawString(mX + 10, mY + 77, 255, 255, 255, "No Recoil");
		}
		if(activeItem5) {
			g_Drawing.DrawString(mX + 150, mY + 77, 0, 255, 0, " ON");
		} else {
			g_Drawing.DrawString(mX + 150, mY + 77, 255, 0, 0, "OFF");
		}
		//</norecoil>
		//<crosshair>
		if(selectedItem6) {
			g_Drawing.DrawString(mX + 10, mY + 90, 255, 165, 0, "Crosshair");
		} else {
			g_Drawing.DrawString(mX + 10, mY + 90, 255, 255, 255, "Crosshair");
		}
		if(activeItem6) {
			g_Drawing.DrawString(mX + 150, mY + 90, 0, 255, 0, " ON");
		} else {
			g_Drawing.DrawString(mX + 150, mY + 90, 255, 0, 0, "OFF");
		}
		//</crosshair>
		//<bunny>
		if(selectedItem7) {
			g_Drawing.DrawString(mX + 10, mY + 103, 255, 165, 0, "Bunny");
		} else {
			g_Drawing.DrawString(mX + 10, mY + 103, 255, 255, 255, "Bunny");
		}
		if(activeItem7) {
			g_Drawing.DrawString(mX + 150, mY + 103, 0, 255, 0, " ON");
		} else {
			g_Drawing.DrawString(mX + 150, mY + 103, 255, 0, 0, "OFF");
		}
		//</bunny>
	}
	CMe *cMe = g_cPlayers.Me();
	cMe->GetInfo();
	//Üst kýsýmda çýkan bilgilendirme kodlarý (Silebilirsiniz)
	g_Drawing.DrawString(g_Screen.iWidth /2 -10, 15, 255,0,0,"HP: %i", g_Local.iHP);
	g_Drawing.DrawString(g_Screen.iWidth /2 -120, 28, 255,0,0,"ViewAng: x%f - y%f", g_Local.vViewAngle[0], g_Local.vViewAngle[1]);
	if(activeItem6) {
		//No recoil kodlarý
		float recoil = ((g_Screen.iWidth * g_Screen.iHeight) * 20.0f) / 2073600.0f;
		float y = g_Local.vPunchAngle[0] * recoil;
		float x = g_Local.vPunchAngle[1] * recoil;
		g_Engine.pfnFillRGBA(g_Screen.iWidth / 2 - x - 1, g_Screen.iHeight / 2 + y - 1, 3, 3, 255, 1, 1, 255);
	}
}