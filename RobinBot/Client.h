#pragma once

typedef void (*PreS_DynamicSound_t)(int, DWORD, char *, float *, DWORD, DWORD, DWORD, DWORD);

class Client
{
private:
	static void StudioEntityLight(struct alight_s *plight);
	static void InitHack(void);
	static bool IsInFOV(float *fScreen, float fFov);
	static void PredictEntity(cl_entity_s *pEntity, Vector *vOutput, unsigned int fAmount);
	static int HUD_Reset(void);
	static void HUD_Frame(double time);
	static void FarAim();
	static void HackMenu();
	static void HUD_Redraw(float time, int intermission);
	static void HUD_PlayerMove(struct playermove_s *ppmove, int server);
	static void SmoothAimAngles(Vector *vStart, Vector *vTarget, Vector *vOutput, float fSmoothness);
	static void Bhop(float frametime, struct usercmd_s *cmd);
	static void weaponSettings();
	static int HUD_Key_Event(int down, int keynum, const char *pszCurrentBinding);
	static void CL_CreateMove(float frametime, struct usercmd_s *cmd, int active);
public:
	static void PreS_DynamicSound(int entid, DWORD u, char *szSoundFile, float *fOrigin, DWORD dont, DWORD know, DWORD ja, DWORD ck);
	void HookEngine(void);
	void HookStudio(void);
	void HookClient(void);
};

extern PreS_DynamicSound_t PreS_DynamicSound_s;