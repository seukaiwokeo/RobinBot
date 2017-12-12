#include "Main.h"

void HlEngineCommand(const char* command);
void HandleCvarInt(char* name, int* value);
void HandleCvarFloat(char* name, float* value);
void HandleCvarStr(char* name, string value);
bool isHlCvar(char* name);
bool HandleHlCvar(char* name);

void CommandInterpreter::exec(const char* cmdlist)
{
	string my_copy = cmdlist;
	char* from = const_cast<char*>(my_copy.c_str());
	char* to = from;
	while (*from == ' ' || *from == '\t'){ ++from; ++to; }
	while (*to >= ' ' && *to != ';'){
		if (*to == '\"')
		{
			do{ ++to; } while (*to && *to != '\"');
		}
		else
		{
			++to;
		}
	}
	do{
		if (from[0] == '/' && from[1] == '/') { return; }
		if (from < to)
		{
			char oldch = *to;
			*to = 0;
			exec_one(from);
			*to = oldch;
		}
		if (!*to) { break; }
		++to;
		from = to;
		while (*from == ' ' || *from == '\t'){ ++from; ++to; }
		while (*to >= ' ' && *to != ';') ++to;
	} while (1);
}

void CommandInterpreter::extractArguments(const char* const_args)
{
	preExecArgs.clear();
	char* args = const_cast<char*>(const_args);
	while (*args)
	{
		while (*args && *args <= ' ') args++;
		if (!*args) break;
		char* start;
		if (*args == '\"') { start = ++args; while (*args != '\"' && *args) args++; }
		else            { start = args;   while (*args > ' ') args++; }
		char last = *args;
		*args = 0;
		preExecArgs.push_back(start);
		*args = last;
		if (*args == '\"') args++;
	}
}

void CommandInterpreter::collectArguments(string& dest, int from, int to)
{
	dest.erase();
	--from; --to;
	int nArgs = preExecArgs.size();
	to = (to < nArgs) ? to : (nArgs - 1);
	while (from <= to){
		dest += preExecArgs[from];
		if (from != to) dest += " ";
		++from;
	};
}

void CommandInterpreter::createRandomPrefix()
{
	static char characterBox[] = "0123456789abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ!$%&/()=?{}[]*#-.,<>~+_";
	static int len = sizeof(characterBox)-1;
	excludePrefixChar[0] = characterBox[rand() % len];
	excludePrefixChar[1] = characterBox[rand() % len];
	excludePrefixChar[2] = characterBox[rand() % len];
	excludePrefixChar[3] = characterBox[rand() % len];
	excludePrefixChar[4] = 0;
}

void CommandInterpreter::exec_one(const char* cur_cmd)
{
	if (*cur_cmd == '#' || *cur_cmd == '.')
	{
		if (false)
		{
			static string hlcommand;
			hlcommand.erase();
			hlcommand += excludePrefixChar;
			hlcommand += (cur_cmd + 1);
			HlEngineCommand(hlcommand.c_str());
		}
		else
		{
			HlEngineCommand(cur_cmd + 1);
		}
		return;
	}
	char  command[32];
	char* commandPos = command;
	int   commandCharsLeft = 31;
	while (*cur_cmd > ' ' && commandCharsLeft)
	{
		*commandPos = *cur_cmd;
		commandPos++;
		cur_cmd++;
		commandCharsLeft--;
	}
	*commandPos = 0;
	while (*cur_cmd > ' ') cur_cmd++;
	if (names.find(command))
	{
		Entry& entry = entries[names.num];
		switch (entry.type)
		{
		case Entry::ALIAS:{
							  string& content = *(string*)(entry.data);
							  exec(const_cast<char*>(content.c_str()));
		}break;
		case Entry::COMMAND:{
								typedef void(*CmdFunc)();
								CmdFunc function = (CmdFunc)(entry.data);
								extractArguments(cur_cmd);
								function();
		}break;
		case Entry::CVAR_INT:
			extractArguments(cur_cmd);
			HandleCvarInt(command, (int*)entry.data);
			break;
		case Entry::CVAR_FLOAT:
			extractArguments(cur_cmd);
			HandleCvarFloat(command, (float*)entry.data);
			break;
		case Entry::CVAR_STR: {
			extractArguments(cur_cmd);
			HandleCvarStr(command, *(string*)entry.data);
		}
			break;
		case Entry::HL_CVAR:
			extractArguments(cur_cmd);
			HandleHlCvar(command);
			break;
		}
	}
	else {
		if (!isHlCvar(command))
		{
			{
				g_Engine.Con_Printf("Unknown command: %s\n", command);
			}
		}
		else
		{
			Add(command, Entry::HL_CVAR, NULL);
			extractArguments(cur_cmd);
			HandleHlCvar(command);
		}
	}
}

void CommandInterpreter::execFile(const char* filename)
{
	if (strstr(filename, ".."))
	{
		g_Engine.Con_Printf("forbidden.\n");
		return;
	}
	ifstream ifs(filename);
	if (!ifs)
	{
		return;
	}
	char buf[1024];
	while (ifs)
	{
		ifs.getline(buf, 550, 10);
		if (!ifs) { break; }
		exec(buf);
	}
}

void  CommandInterpreter::Add(char* name, int type, void* data)
{
	if (names.find(name))
	{
		g_Engine.Con_Printf("%s is already registered.\n", name);
		return;
	}
	int index = entries.size();
	Entry tmp = { type, data, name };
	entries.push_back(tmp);
	names.add(name, index);
}

inline static void lowercase(char* str)
{
	while (*str){ *str = tolower(*str); ++str; }
}

void  CommandInterpreter::AddAlias(const char* NamE, string& newcontent)
{
	char name[36];
	strncpy_s(name, NamE, 31);
	name[31] = 0;
	lowercase(name);
	if (names.find(name))
	{
		Entry& entry = entries[names.num];
		if (entry.type != Entry::ALIAS)
		{
			g_Engine.Con_Printf("%s is already registered.\n", name);
			return;
		}
		*(string*)(entry.data) = newcontent;
		return;
	}
	if (isHlCvar(name))
	{
		g_Engine.Con_Printf("%s is a HL cvar.\n", name);
		return;
	}
	int index = entries.size();
	string* initial_content = new string;
	*initial_content = newcontent;
	Entry tmp = { Entry::ALIAS, initial_content };
	entries.push_back(tmp);
	names.add(name, index);
}