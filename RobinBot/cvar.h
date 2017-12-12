#ifndef CVAR_H
#define CVAR_H

class CVARlist
{
public:
	void init();
public:
	int aim_active;
	int aim_target;
	float aim_height;
	int aim_autowall;
	int aim_fov;
	int aim_distancebasedfov;
	int aim_avdraw;
	int aim_prediction;
	int aim_time;
	int aim_delay;
	int aim_smoothness;
	int bhop;
	//int esp_barel;
	int aim_triggerbot;
	int aim_triggerbot_fov;
};

extern CVARlist cvar;

#endif