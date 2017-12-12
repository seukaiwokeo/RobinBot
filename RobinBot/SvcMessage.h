#pragma once

// SVC Hook by _or_75

#include <windows.h>
#include <time.h>
#include "AutoOffset.h"
#include "Main.h"

extern int* MSG_ReadCount;
extern int* MSG_CurrentSize;
extern int* MSG_BadRead;
extern void* MSG_Base;
extern int MSG_SavedReadCount;

typedef void (*TEmptyCallback)();

typedef int (*HL_MSG_ReadByte)();
typedef int (*HL_MSG_ReadShort)();
typedef int (*HL_MSG_ReadLong)();
typedef char* (*HL_MSG_ReadString)();

typedef void (*HL_MSG_CBuf_AddText)(char* text);

extern HL_MSG_ReadByte MSG_ReadByte;
extern HL_MSG_ReadShort MSG_ReadShort;
extern HL_MSG_ReadLong MSG_ReadLong;
extern HL_MSG_ReadString MSG_ReadString;

extern HL_MSG_CBuf_AddText CBuf_AddText_Orign;

extern TEmptyCallback SVC_StuffText_Orign;
extern TEmptyCallback SVC_SendCvarValue_Orign;
extern TEmptyCallback SVC_SendCvarValue2_Orign;

void MSG_SaveReadCount();
void MSG_RestoreReadCount();

TEmptyCallback HookServerMsg(const unsigned Index, void* CallBack,AutoOffset* Offset);

bool IsCvarGood(const char *str);
bool IsCommandGood(const char *str);
bool SanitizeCommands(char *str,char* name);

void CBuf_AddText(char* text);
void SVC_StuffText();
void SVC_SendCvarValue();
void SVC_SendCvarValue2();