#include "Main.h"

CVARlist cvar;

#pragma warning(disable:4244)

void func_alias()
{
	const char* name = cmd.argC(1);
	string& content = cmd.argS(2);
	cmd.AddAlias(name,content);
}

void set_cvar()
{
	char set_cvars[56];
	if ( cmd.names.find(g_Engine.Cmd_Argv(1)) )
	{
		sprintf(set_cvars,"%s %s",g_Engine.Cmd_Argv(1),g_Engine.Cmd_Argv(2));
		cmd.exec(set_cvars);
	}
	else
		cmd.exec(g_Engine.Cmd_Argv(1));
}

void CVARlist::init()
{

	memset((char*)this, 0, sizeof(*this));
	#define REGISTER_CVAR_FLOAT(name,defaultvalue) cmd.AddCvarFloat(#name, &##name );name=defaultvalue##f;
	#define REGISTER_CVAR_INT(name,defaultvalue) cmd.AddCvarInt(#name, &##name );name=defaultvalue;
	#define REGISTER_CVAR_STR(name,defaultvalue) cmd.AddCvarString(#name, &##name );name=defaultvalue;
	#define REGISTER_COMMAND(name) cmd.AddCommand( #name, func_##name);

	//Otomatik ayarlar

	REGISTER_CVAR_INT(aim_active, 1)
	REGISTER_CVAR_INT(aim_target, 10)
	REGISTER_CVAR_FLOAT(aim_height, 0.20)
	REGISTER_CVAR_INT(aim_autowall, 0)
	REGISTER_CVAR_INT(aim_fov, 35)
	REGISTER_CVAR_INT(aim_distancebasedfov, 1)
	REGISTER_CVAR_INT(aim_avdraw, 1)
	REGISTER_CVAR_INT(aim_prediction, 1)
	REGISTER_CVAR_INT(aim_time, 300)
	REGISTER_CVAR_INT(aim_delay, 0)
	REGISTER_CVAR_INT(aim_smoothness, 1)
	REGISTER_CVAR_INT(bhop, 1)
	//REGISTER_CVAR_INT(esp_barel, 0)
	REGISTER_CVAR_INT(aim_triggerbot, 1)
	REGISTER_CVAR_INT(aim_triggerbot_fov, 11)

	REGISTER_COMMAND(alias)

	g_Engine.pfnAddCommand("set",set_cvar);
}

void HlEngineCommand(const char* command)
{
	if(!g_Engine.pfnClientCmd) { return; }
	g_Engine.pfnClientCmd( const_cast<char*>(command) );
}

void HandleCvarInt(char* name, int* value)
{
	char* arg1 = cmd.argC(1); 
	if (!strcmp(arg1,"change"))
	{
		if(*value) *value=0; 
		else *value=1;
		return;
	}
	if (!strcmp(arg1,"up")){*value += cmd.argI(2);return;}
	if (!strcmp(arg1,"down")){*value -= cmd.argI(2);return;}
	if (!strcmp(arg1,"hide")){*value = cmd.argI(2);return;}
	if (!*arg1)
	{
		g_Engine.Con_Printf( "CVAR %s = %i\n",name,*value); 
		return;
	}
	*value = cmd.argI(1);
}

void HandleCvarStr(char* name, string value)
{
	char* arg1 = cmd.argC(1);
	if (!*arg1)
	{
		g_Engine.Con_Printf("CVAR %s = %s\n", name, value.c_str());
		return;
	}
	value = cmd.argF(1);
}

void HandleCvarFloat(char* name, float* value)
{
	char* arg1 = cmd.argC(1);  
	if (!strcmp(arg1,"change"))
	{
		if(*value) *value=0; 
		else *value=1;
		return;
	}
	if (!strcmp(arg1,"up")){*value += cmd.argF(2);return;}
	if (!strcmp(arg1,"down")){*value -= cmd.argF(2);return;}
	if (!strcmp(arg1,"hide")){*value = cmd.argI(2);return;}
	if (!*arg1)
	{
		g_Engine.Con_Printf("CVAR %s = %f\n",name,*value);
		return;
	}
	*value = cmd.argF(1);
}

bool isHlCvar(char* name)
{
	if(!g_Engine.pfnGetCvarPointer) { return false; }
	cvar_s* test = g_Engine.pfnGetCvarPointer(name);
	return (test!=NULL);
}

bool HandleHlCvar(char* name)
{
	if(!g_Engine.pfnGetCvarPointer) { return false; }
	cvar_s* ptr = g_Engine.pfnGetCvarPointer(name);
	if(!ptr) { return false; }
	HandleCvarFloat(name,&ptr->value);
	return true;
}