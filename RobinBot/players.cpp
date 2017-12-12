#include "players.h"

#pragma warning(disable:4996)
#pragma warning(disable:4800)
#pragma warning(disable:4002)
#pragma warning(disable:4244)

CMe::CMe(void)
{
	p_sInfo.eTeam = TEAM_NONE;

}

CMe::~CMe(){}

void CMe::GetInfo(void)
{
	p_sEnt = g_Engine.GetLocalPlayer();

	g_Engine.pfnGetScreenFade(&(p_sInfo.sScreenFade));

	g_Engine.pEventAPI->EV_LocalPlayerViewheight(p_sInfo.vViewOrg);
	VectorAdd(p_sEnt->origin, p_sInfo.vViewOrg, p_sInfo.vViewOrg);

	g_Engine.pfnGetPlayerInfo(p_sEnt->index, &sPlayerInfo);

	if ( g_Local.iTeam == 1 )
		p_sInfo.eTeam = TEAM_T;
	else if ( g_Local.iTeam == 2 )
		p_sInfo.eTeam = TEAM_CT;
	else if ( g_Local.iTeam == 3 )
		p_sInfo.eTeam = TEAM_SPEC;
	else
		p_sInfo.eTeam = TEAM_NONE;
	
	p_sInfo.bIsGoodWeapon = true;
	model_s *pModel = g_Studio.GetModelByIndex(p_sEnt->curstate.weaponmodel);
	if (pModel != NULL)
	{
		strncpy(p_sInfo.cWeapon, pModel->name, sizeof(p_sInfo.cWeapon));
		char *cWeaponName = p_sInfo.cWeapon;
		if (strstr(cWeaponName, "hegrenade")		||
			strstr(cWeaponName, "flashbang")		||
			strstr(cWeaponName, "smokegrenade")		||
			strstr(cWeaponName, "knife")			||
			strstr(cWeaponName, "c4"))
			p_sInfo.bIsGoodWeapon = false;
	}
	
	int iSequenceType = iSequenceInfoTable[p_sEnt->curstate.sequence];
	if (iSequenceType == 2 || iSequenceType == 5)
		p_sInfo.bIsReloading = true;
	else
		p_sInfo.bIsReloading = false;
}

cl_entity_t *CMe::Entity(void)
{
	return p_sEnt;
}

ME_INFO *CMe::Info(void)
{
	return &p_sInfo;
}

bool CMe::Visible(Vector *vInput)
{
	pmtrace_t *sTrace = g_Engine.PM_TraceLine(p_sInfo.vViewOrg, (*vInput), 0, 2, -1);
	return (sTrace->fraction >= 1.0f);
}

float CMe::Distance(Vector *vInput)
{
	return (float)(VectorDistance(p_sInfo.vViewOrg, (*vInput)) / 22.0f);
}

bool CMe::IsFlashed(void)
{
	return (!(!(p_sInfo.sScreenFade.fader == 255 && p_sInfo.sScreenFade.fadeg == 255 && p_sInfo.sScreenFade.fadeb == 255) ||
		p_sInfo.sScreenFade.fadealpha <= 150));
}

void CMe::CalcViewAngles(Vector *vOrigin, Vector *vOutput)
{
	Vector vDelta;
	VectorSubtract(&(p_sInfo.vViewOrg), vOrigin, &vDelta);

	// vOut->x
	float adjacent = sqrt(POW(vDelta.x) + POW(vDelta.y));
	p_sInfo.vAimAngles.x = (float)RADTODEG(atan(vDelta.z / adjacent));

	// vOut->y
	if (vDelta.x >= 0)		// front
	{
		if (vDelta.y >= 0)	// right
			p_sInfo.vAimAngles.y = (float)RADTODEG(atan(fabs(vDelta.y / vDelta.x))) + 180.0f;
		else				// left
			p_sInfo.vAimAngles.y = (float)RADTODEG(atan(fabs(vDelta.x / vDelta.y))) + 90.0f;
	}
	else					// behind
	{
		if (vDelta.y >= 0)
			p_sInfo.vAimAngles.y = (float)RADTODEG(atan(fabs(vDelta.x / vDelta.y))) + 270.0f;
		else
			p_sInfo.vAimAngles.y = (float)RADTODEG(atan(fabs(vDelta.y / vDelta.x)));
	}

	// vOut->z
	p_sInfo.vAimAngles.z = 0.0f;

	if (vOutput != NULL)
		VectorCopy(&(p_sInfo.vAimAngles), vOutput);
}

CPlayer::CPlayer(void){}
CPlayer::~CPlayer(){}

void CPlayer::GetInfo(int nID)
{
	p_sEnt				= g_Engine.GetEntityByIndex(nID);
	p_sInfo.bGotBone	= false;
	p_sInfo.bValid		= ValidateEntity();

	if (p_sInfo.bValid)
	{
		//SoundCopy(g_cPlayers.player(i)->cursound, g_cPlayers.player(i)->oldsound);

		VectorCopy(p_sEnt->origin, p_sInfo.sSound.vOrigin);	// todo p_sEnt -> p_pEnt
		p_sInfo.sSound.dwTime = GetTickCount();
		p_sInfo.sSound.bValid = true;

		int iSequenceType = iSequenceInfoTable[p_sEnt->curstate.sequence];
		if (iSequenceType == 2 || iSequenceType == 5)
			p_sInfo.bIsVulnerable = true;
		else
			p_sInfo.bIsVulnerable = false;

		g_Engine.pfnGetPlayerInfo(nID, &sPlayerInfo);

		if ( g_Player[nID].iTeam == 1 )
			p_sInfo.eTeam = TEAM_T;
		else if ( g_Player[nID].iTeam == 2 )
			p_sInfo.eTeam = TEAM_CT;
		else if ( g_Player[nID].iTeam == 3 )
			p_sInfo.eTeam = TEAM_SPEC;
		else
			p_sInfo.eTeam = TEAM_NONE;
	}
}

cl_entity_t *CPlayer::Entity(void)
{
	return p_sEnt;
}

PPLAYER_INFO CPlayer::Info(void)
{
	return &p_sInfo;
}

bool CPlayer::ValidateEntity(void)
{
	if( (p_sEnt->index != g_Local.iIndex) && !(p_sEnt->curstate.effects & EF_NODRAW) && p_sEnt->player && (p_sEnt->model->name !=0 || p_sEnt->model->name !="")  && !(p_sEnt->curstate.messagenum < g_Engine.GetLocalPlayer()->curstate.messagenum))
		return true;
	else
		return false;
}

float CPlayer::Distance(Vector *vInput)
{
	return (float)(VectorDistance(p_sInfo.vOrigin, (*vInput)) / 22.0f);
}

float CPlayer::BoneDistance(Vector *vInput)
{
	return (float)(VectorDistance(p_sInfo.vAimBone, (*vInput)) / 22.0f);	// todo: use dyn vecs here
}

bool CPlayer::IsInFov(float fFov)
{
	if ((fFov == 0.0f)										||
		((p_sInfo.fScreen[0] <= g_Screen.iWidth/2 + fFov)	&&
		(p_sInfo.fScreen[0] >= g_Screen.iWidth/2 - fFov)	&&
		(p_sInfo.fScreen[1] <= g_Screen.iHeight/2 + fFov)	&&
		(p_sInfo.fScreen[1] >= g_Screen.iHeight/2 - fFov)))
		return true;

	return false;
}

Vector *CPlayer::Position(void)
{
	return &(p_sInfo.vOrigin);
}

Vector *CPlayer::BonePosition(void)
{
	return &(p_sInfo.vAimBone);
//	vOutput->x = (p_sInfo.sBones)[nBoneID][0][3];
//	vOutput->y = (p_sInfo.sBones)[nBoneID][1][3];
//	vOutput->z = (p_sInfo.sBones)[nBoneID][2][3];
}

bool CalcScreen(Vector *vInput, float *fOutput)
{
	int iRasterizer	= g_Engine.pTriAPI->WorldToScreen((*vInput), fOutput);
	if (fOutput[0] < 1 && fOutput[1] < 1 && fOutput[0] > -1 && fOutput[1] > -1 && !iRasterizer)
	{
		int iScreenCenterX = g_Screen.iWidth  / 2;
		int iScreenCenterY = g_Screen.iHeight / 2;

		fOutput[0] =  fOutput[0] * iScreenCenterX + iScreenCenterX;
		fOutput[1] = -fOutput[1] * iScreenCenterY + iScreenCenterY;

		return true;
	}

	return false;
}
void VectorTransform(float *in1, float in2[3][4], float *out)
{
	out[0] = DotProduct(in1, in2[0]) + in2[0][3];
	out[1] = DotProduct(in1, in2[1]) + in2[1][3];
	out[2] = DotProduct(in1, in2[2]) + in2[2][3];
}

bool CPlayer::ScreenPosition(float *fOutput)
{
	p_sInfo.bIsScreen = CalcScreen(Position(), p_sInfo.fScreen);
	if (fOutput != NULL) { fOutput[0] = p_sInfo.fScreen[0]; fOutput[1] = p_sInfo.fScreen[1]; }
	return p_sInfo.bIsScreen;
}

bool CPlayer::BoneScreenPosition(float *fOutput)
{
	p_sInfo.bIsBoneScreen = CalcScreen(BonePosition(), p_sInfo.fBoneScreen);
	if (fOutput != NULL) { fOutput[0] = p_sInfo.fBoneScreen[0]; fOutput[1] = p_sInfo.fBoneScreen[1]; }
	return p_sInfo.bIsBoneScreen;
	
}

void CPlayer::GetBoneInformation(VECTOR_MODE eMode, int nNum)
{
	add_log("GetBoneInformation 1");

	model_s	*pModel = nullptr;
	studiohdr_t	*pStudioHeader = nullptr;
	mstudiobbox_t *pStudioBox = nullptr;
	PTRANSFORM_MATRIX pTransformMatrix = (PTRANSFORM_MATRIX)g_Studio.StudioGetBoneTransform();

	add_log("GetBoneInformation 2");

	// switch me!
	if (eMode == VM_BONE)
	{
		add_log("GetBoneInformation 3");
		p_sInfo.vAimBone.x = (*pTransformMatrix)[nNum][0][3];
		p_sInfo.vAimBone.y = (*pTransformMatrix)[nNum][1][3];
		p_sInfo.vAimBone.z = (*pTransformMatrix)[nNum][2][3];
		add_log("GetBoneInformation 4");
	}
	else if (eMode == VM_HITBOX)
	{
		add_log("GetBoneInformation 5");
		if (p_sInfo.bGotBone)
			return;
		add_log("GetBoneInformation 6");
		pModel = g_Studio.SetupPlayerModel(p_sEnt->index);
		add_log("GetBoneInformation 6 1");
		pStudioHeader = (studiohdr_t *)g_Studio.Mod_Extradata(pModel);
		add_log("GetBoneInformation 6 2");
		pStudioBox = (mstudiobbox_t *)((PBYTE)pStudioHeader + pStudioHeader->hitboxindex);
		add_log("GetBoneInformation 7");
		Vector vMin, vMax;
		VectorTransform(pStudioBox[nNum].bbmin, (*pTransformMatrix)[pStudioBox[nNum].bone], vMin);
		VectorTransform(pStudioBox[nNum].bbmax, (*pTransformMatrix)[pStudioBox[nNum].bone], vMax);
		add_log("GetBoneInformation 8");
		p_sInfo.vAimBone = (vMin + vMax) * 0.5f;
		p_sInfo.bGotBone = true;
		add_log("GetBoneInformation 9");
	}
}

CPlayers::CPlayers(void)
{
	p_cMe			= new CMe;
	p_cPlayers		= new CPlayer[MAX_VPLAYERS];
}

CPlayers::~CPlayers()
{
	delete[]	p_cPlayers;
	delete		p_cMe;
}

int CPlayers::Size(void)
{
	return MAX_VPLAYERS;
}

CMe *CPlayers::Me(void)
{
	return p_cMe;
}

CPlayer *CPlayers::Player(int i)
{
	if (i >= 0 && i < MAX_VPLAYERS)
		return &p_cPlayers[i];
	return &p_cPlayers[0];
}