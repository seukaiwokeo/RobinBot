#ifndef __PLAYERS_H__
#define __PLAYERS_H__

#include <windows.h>
#include "Main.h"

#ifndef RADTODEG
#define RADTODEG(x) (x * (180 / M_PI))
#endif

typedef float TRANSFORM_MATRIX[MAXSTUDIOBONES][3][4];
typedef TRANSFORM_MATRIX *PTRANSFORM_MATRIX;

typedef struct _SOUND_INFO
{
	Vector	vOrigin;
	DWORD	dwTime;
	bool	bValid;
}
SOUND_INFO, *PSOUND_INFO;

typedef struct _SCREENSHOT_INFO
{
	DWORD	dwStartTime;
	int		nCounter;
}
SCREENSHOT_INFO, *PSCREENSHOT_INFO;

typedef enum _TEAM_INFO
{
	TEAM_T,
	TEAM_CT,
	TEAM_SPEC,
	TEAM_NONE
}
TEAM_INFO, *PTEAM_INFO;

typedef struct ME_INFO_
{
	TEAM_INFO				eTeam;

	screenfade_t			sScreenFade;

	bool					bIsReloading;
	bool					bIsGoodWeapon;

	char					cWeapon[64];

	Vector					vViewOrg;
	Vector					vViewAngles;
	Vector					vAimAngles;
	Vector					vPreAimAngles;

	bool					bPanic;
	int						nScreenShot;
	SCREENSHOT_INFO			sAntiCheat;
}
ME_INFO, *PME_INFO;

typedef enum _VECTOR_MODE
{
	VM_BONE,
	VM_HITBOX
}
VECTOR_MODE, *PVECTOR_MODE;

static int iSequenceInfoTable[] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 2, 0, 1, 2, 0, 1, 2, 0,
	1, 2, 0, 1, 1, 2, 0, 1, 1, 2,
	0, 1, 2, 0, 1, 2, 0, 1, 2, 0,
	1, 2, 0, 1, 2, 0, 1, 2, 0, 1,
	2, 0, 1, 2, 0, 0, 0, 4, 0, 4,
	0, 5, 0, 5, 0, 0, 1, 1, 2, 0,
	1, 1, 2, 0, 1, 0, 1, 0, 1, 2,
	0, 1, 2, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3
};

class CMe	// todo: derive this from cplayer
{
	public:
							CMe(void);
							~CMe();
		void				GetInfo(void);
		hud_player_info_t	sPlayerInfo;
		cl_entity_t			*Entity(void);
		PME_INFO			Info(void);
		bool				Visible(Vector *vInput);
		float				Distance(Vector *vInput);
		bool				IsFlashed(void);
		void				CalcViewAngles(Vector *vOrigin, Vector *vOutput);
	private:
		cl_entity_t			*p_sEnt;
		ME_INFO				p_sInfo;
		ME_INFO				p_sPreInfo;
};

typedef struct PLAYER_INFO_
{
	Vector				vOrigin;

	bool				bValid;
	bool				bIsVulnerable;

	float				fScreen[2];
	bool				bIsScreen;
	float				fBoneScreen[2];
	bool				bIsBoneScreen;

	TEAM_INFO			eTeam;

	bool				bGotBone;
	Vector				vAimBone;

	SOUND_INFO			sSound;
	SOUND_INFO			sOldSound;
}
PLAYER_INFO, *PPLAYER_INFO;

class CPlayer
{
	public:
							CPlayer(void);
							~CPlayer();
		void				GetInfo(int nID);
		hud_player_info_t	sPlayerInfo;
		cl_entity_t			*Entity(void);
		PPLAYER_INFO		Info(void);
		bool				ValidateEntity(void);
		float				Distance(Vector *vInput);
		float				BoneDistance(Vector *vInput);
		bool				IsInFov(float fFov);
		Vector				*Position(void);
		Vector				*BonePosition(void);
		bool				ScreenPosition(float *fOutput);
		bool				BoneScreenPosition(float *fOutput);
		void				GetBoneInformation(VECTOR_MODE eMode, int nNum);

	private:
		cl_entity_t			*p_sEnt;
		PLAYER_INFO			p_sInfo;
};

#define MAX_VPLAYERS 33

class CPlayers
{
	public:
					CPlayers(void);
					~CPlayers();
		int			Size(void);
		CMe			*Me(void);
		CPlayer		*Player(int i);

	private:
		CMe			*p_cMe;
		CPlayer		*p_cPlayers;
};
extern CPlayers	g_cPlayers;

typedef struct _TARGET_INFO
{
	float		fDistance;
	CPlayer		*cPlayer;
	float		fScreen[2];
}
TARGET_INFO, *PTARGET_INFO;

typedef struct _AIMBOT_INFO
{
	TARGET_INFO	sTarget;
	bool		bActive;
	DWORD		dwStartTime;
}
AIMBOT_INFO, *PAIMBOT_INFO;


#endif
