#pragma once

#include <Windows.h>
#include <fstream>
#include "Main.h"

#define CompareMemory(Buff1, Buff2, Size) __comparemem((const UCHAR *)Buff1, (const UCHAR *)Buff2, (UINT)Size)
#define FindMemoryClone(Start, End, Clone, Size) __findmemoryclone((const ULONG)Start, (const ULONG)End, (const ULONG)Clone, (UINT)Size)
#define FindReference(Start, End, Address) __findreference((const ULONG)Start, (const ULONG)End, (const ULONG)Address)

class AutoOffset
{
public:
	bool SW;
	BYTE HLType;
	DWORD SpeedPtr;
	DWORD HwBase, HwSize, HwEnd;

	DWORD pCBuf_AddText;
	server_msg_array_s SVCBase;
	void* SVCBase_End;
	unsigned int SVCCount;
	
	void MsgDump(DWORD Address);
	DWORD MessagePtr(char Message[],BYTE size);
	bool GetRendererInfo(void);
	void Error(const PCHAR Msg);
	DWORD GetModuleSize(const DWORD Address);
	DWORD FarProc(const DWORD Address, DWORD LB, DWORD HB);
	void *SpeedHackPtr(void);
	void *ClientFuncs(void);
	void *EngineFuncs(void);
	DWORD EngineStudio();

	unsigned Relative(const unsigned Addr,const unsigned NewFunc);
	unsigned Absolute(DWORD Addr);
	bool CheckByte(DWORD Address,BYTE Value,int Offset);
	DWORD GetCallback(unsigned Index);

	void GameInfo(void);
	void Find_SVCBase(void);
	void Find_MSGInterface(void);
	void Find_CBuf_AddText(void);
	void Patch_CL_ConnectionlessPacket(void);
	void *Sound(void);
	
	BOOL __comparemem(const UCHAR *buff1, const UCHAR *buff2, UINT size);
	ULONG __findmemoryclone(const ULONG start, const ULONG end, const ULONG clone, UINT size);
	ULONG __findreference(const ULONG start, const ULONG end, const ULONG address);
};