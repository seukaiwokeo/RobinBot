#include "AutoOffset.h"
#include "SvcMessage.h"
#include "Command.h"

typedef void* Pointer;
typedef void** PPointer;
typedef unsigned Cardinal;
typedef unsigned* PCardinal;
typedef unsigned** PPCardinal;
typedef int** PPLongint;

#pragma warning(disable:4996)

void AutoOffset::MsgDump(DWORD Address)
{
	char msg[64];
	sprintf_s(msg,"%X",Address);
	MessageBoxA(0,msg,"MsgDump",MB_OK);
}

DWORD AutoOffset::MessagePtr(char Message[],BYTE size)
{
	BYTE bPushAddrPattern[5] = { 0x68, 0x90, 0x90, 0x90, 0x90 };
	char* String = new char[size];
	strcpy(String,Message);
	DWORD Address = FindMemoryClone(HwBase, HwBase+HwSize, (PBYTE)String, size - 1);
	*(PDWORD)(bPushAddrPattern + 1) = Address;
	Address = FindMemoryClone(HwBase, HwBase+HwSize, (PBYTE)bPushAddrPattern, 5);
	return Address;
}

bool AutoOffset::GetRendererInfo(void)
{
	DWORD GameUI = (DWORD)GetModuleHandle(L"GameUI.dll");
	DWORD vgui = (DWORD)GetModuleHandle(L"vgui.dll");
	DWORD vgui2 = (DWORD)GetModuleHandle(L"vgui2.dll");

	HLType = RENDERTYPE_UNDEFINED;

	HwBase = (DWORD)GetModuleHandle(L"hw.dll");
	if ( HwBase == NULL )
	{
		HwBase = (DWORD)GetModuleHandle(L"sw.dll");
		if ( HwBase == NULL )
		{
			HwBase = (DWORD)GetModuleHandle(NULL); // Non-Steam?
			if ( HwBase == NULL )
			{
				Error("Invalid module handle.");
			}
			else
				HLType = RENDERTYPE_UNDEFINED;
		}
		else
			HLType = RENDERTYPE_SOFTWARE;
	}
	else
		HLType = RENDERTYPE_HARDWARE;

	HwSize = (DWORD)GetModuleSize(HwBase);

	if ( HwSize == NULL )
	{
		switch(HwSize)
		{
		case RENDERTYPE_HARDWARE: HwSize = 0x122A000;break;
		case RENDERTYPE_UNDEFINED: HwSize = 0x2116000;break;
		case RENDERTYPE_SOFTWARE: HwSize = 0xB53000;break;
		default:Error("Invalid renderer type.");
		}
	}
	HwEnd = HwBase + HwSize - 1;

	return (HwBase && GameUI && vgui && vgui2);
}

void AutoOffset::Error(const PCHAR Msg)
{
	MessageBoxA(0,Msg,"Fatal Error",MB_OK|MB_ICONERROR);
	ExitProcess(0);
}

DWORD AutoOffset::GetModuleSize(const DWORD Address)
{
	return PIMAGE_NT_HEADERS(Address + (DWORD)PIMAGE_DOS_HEADER(Address)->e_lfanew)->OptionalHeader.SizeOfImage;
}

DWORD AutoOffset::FarProc(const DWORD Address, DWORD LB, DWORD HB)
{
	return ( (Address < LB) || (Address > HB) );
}

BOOL AutoOffset::__comparemem(const UCHAR *buff1, const UCHAR *buff2, UINT size)
{
	for (UINT i = 0; i < size; i++, buff1++, buff2++)
	{
		if ((*buff1 != *buff2) && (*buff2 != 0xFF))
			return FALSE;
	}
	return TRUE;
}

ULONG AutoOffset::__findmemoryclone(const ULONG start, const ULONG end, const ULONG clone, UINT size)
{
	for (ULONG ul = start; (ul + size) < end; ul++)
	{
		if (CompareMemory(ul, clone, size))
			return ul;
	}
	return NULL;
}

ULONG AutoOffset::__findreference(const ULONG start, const ULONG end, const ULONG address)
{
	UCHAR Pattern[5];
	Pattern[0] = 0x68;
	*(ULONG*)&Pattern[1] = address;
	return FindMemoryClone(start, end, Pattern, sizeof(Pattern)-1);
}

void* AutoOffset::SpeedHackPtr(void)
{
	DWORD Old = NULL;
	PCHAR String = "Texture load: %6.1fms";
	DWORD Address = (DWORD)FindMemoryClone(HwBase, HwBase+HwSize, String, strlen(String));
	PVOID SpeedPtr = (PVOID)*(DWORD*)(FindReference(HwBase, HwBase+HwSize, Address) - 7);
	
	if ( FarProc((DWORD)SpeedPtr,HwBase,HwEnd) )
		Error("Couldn't find SpeedPtr pointer.");
	else
		VirtualProtect(SpeedPtr,sizeof(double),PAGE_READWRITE,&Old);
	
	return SpeedPtr;
}

void* AutoOffset::ClientFuncs(void)
{
	PCHAR String = "ScreenFade";
	DWORD Address = (DWORD)FindMemoryClone(HwBase, HwBase+HwSize, String, strlen(String));
	PVOID ClientPtr = (PVOID)*(PDWORD)(FindReference(HwBase, HwBase+HwSize, Address) + 0x13);
	
	if ( FarProc((DWORD)ClientPtr,HwBase,HwEnd) )
		Error("Couldn't find ClientPtr pointer.");
	
	return ClientPtr;
}

void* AutoOffset::EngineFuncs(void)
{
	PCHAR String = "ScreenFade";
	DWORD Address = FindMemoryClone(HwBase, HwBase+HwSize, String, strlen(String));
	PVOID EnginePtr = (PVOID)*(PDWORD)(FindReference(HwBase, HwBase+HwSize, Address) + 0x0D);
	
	if ( FarProc((DWORD)EnginePtr,HwBase,HwEnd) )
		Error("Couldn't find EnginePtr pointer.");
	
	return EnginePtr;
}

DWORD AutoOffset::EngineStudio(void)
{
	start_ptr:
	bool badptr = false;
	DWORD OLD = 0;
	DWORD dwStudioone = 0;
	PCHAR String = "Couldn't get client .dll studio model rendering interface.";
	DWORD Address = FindMemoryClone(HwBase, HwBase+HwSize, String, strlen(String));
	PVOID EngineStudioPtr = (PVOID)*(PDWORD)(FindReference(HwBase, HwBase+HwSize, Address) - 0x14);
	
	if ( FarProc((DWORD)EngineStudioPtr,HwBase,HwEnd) )
		Error("Couldn't find EngineStudioPtr #1 pointer.");
	
	for(DWORD i = 0;i <= 60;i++)
	{
		dwStudioone = (DWORD)g_pClient->HUD_GetStudioModelInterface + i;
		if ( CheckByte(dwStudioone,0xB8,0) || CheckByte(dwStudioone,0xBF,0) )
		{
			dwStudioone++;

			if ( IsBadReadPtr((PDWORD)(*(PDWORD)dwStudioone),sizeof(engine_studio_api_s)) )
				badptr = true;
			else
				badptr = false;

			if ( badptr == false ) 
			{
				if ( *(PWORD)(PDWORD)EngineStudioPtr == *(PWORD)(PDWORD)(*(PDWORD)dwStudioone) )
				{
					return *(PDWORD)dwStudioone;
				}
			}
		}
	}
	goto start_ptr;
	return  0;
}

unsigned AutoOffset::Relative(const Cardinal Addr,const Cardinal NewFunc)
{
	return NewFunc - Addr - 5;
}

unsigned AutoOffset::Absolute(DWORD Addr)
{
	return Cardinal(Addr) + *(PCardinal)(Addr) + 4;
}

bool AutoOffset::CheckByte(DWORD Address,BYTE Value,int Offset)
{
	if ( *(PBYTE)((DWORD)Address + Offset) == Value )
		return true;
	else
		return false;
}

DWORD AutoOffset::GetCallback(unsigned Index)
{
	return SVCBase[Index]->Callback;
}

void AutoOffset::GameInfo(void)
{
	bool search = false;
	DWORD Address = MessagePtr("fullinfo <complete info string>",32);
	
	if ( FarProc(Address,HwBase,HwEnd) )
		Error("Couldn't find GameInfo #1 pointer.");

	for(int i = 1;i < 40;i++)
	{
		Address--;
		if( *(PBYTE)(PDWORD)Address == 0xC3 )
			search = true;
		if ( search == true )
			for(int i = 1;i < 50;i++)
			{
				Address--;
				if( CheckByte(Address,0x90,0) && CheckByte(Address,0x68,1) )
				{
					Address = Address+1;
					goto finish;
				}
			}
	}
	Error("Couldn't find GameInfo #2 pointer.");

	finish:
	BuildInfo.GameName = *(PCHAR*)(Cardinal(Address)+1);
	BuildInfo.GameVersion = *(PCHAR*)(Cardinal(Address)+6);
	BuildInfo.Protocol = *(PBYTE)(Cardinal(Address)+11);
	Address = (DWORD)Absolute(Cardinal(Address)+23);
	
	if ( FarProc(Address,HwBase,HwEnd) )
		Error("Couldn't find GameInfo #3 pointer.");

	__asm
	{
		call Address
		mov BuildInfo.Build,eax
	}
}

void AutoOffset::Find_SVCBase(void)
{
	DWORD Address = MessagePtr("-------- Message Load ---------",32);
	
	if ( FarProc(Address,HwBase,HwEnd) )
		Error("Couldn't find SVCBase #1 pointer.");
		
	if ( !CheckByte(Address,0xBF,0x11) )
		Error("Couldn't find SVCBase #2 pointer.");
	else
		Address = Address+0x12;

	Pointer Addr;
	Addr = (Pointer)Address;
	DWORD SvPtr = Address;
	Addr = *PPCardinal(Addr);

	SVCBase = (server_msg_array_s)Pointer(Cardinal(Addr) - sizeof(Cardinal));

	if ( !CheckByte(SvPtr,0x81,0x9) && !CheckByte(SvPtr,0xFF,0x10) )
		Error("Couldn't find SVCBase #3 pointer.");
	else
		SvPtr = SvPtr+0x11;

	Addr = (Pointer)SvPtr;
	SVCBase_End = Pointer(*PCardinal(Addr) + sizeof(Cardinal));
	SVCCount = (Cardinal(SVCBase_End) - Cardinal(SVCBase)) / sizeof(server_msg_t);
}

void AutoOffset::Find_MSGInterface(void)
{
	MSG_ReadByte = (HL_MSG_ReadByte)(DWORD)(PVOID)Pointer(Absolute(GetCallback(SVC_CDTRACK) + 1));
	MSG_ReadShort = (HL_MSG_ReadShort)(DWORD)(PVOID)Pointer(Absolute(GetCallback(SVC_STOPSOUND) + 1));
	MSG_ReadLong = (HL_MSG_ReadLong)(DWORD)(PVOID)Pointer(Absolute(GetCallback(SVC_VERSION) + 1));
	MSG_ReadString = (HL_MSG_ReadString)(DWORD)(PVOID)Pointer(Absolute(GetCallback(SVC_PRINT) + 1));

	MSG_ReadCount = *PPLongint(Cardinal(MSG_ReadByte) + 1);
	MSG_CurrentSize = *PPLongint(Cardinal(MSG_ReadByte) + 7);
	MSG_BadRead = *PPLongint(Cardinal(MSG_ReadByte) + 20);

	DWORD D_MSG_Base = MessagePtr("Error reading demo message data.",33);
	MSG_Base = *PPointer(Cardinal(D_MSG_Base) + ( SW ? 27 : 24 ));
}

void AutoOffset::Find_CBuf_AddText(void)
{
	Pointer Addr;
	Command* pCmd = new Command;
	pcmd_t cmd = pCmd->CommandByName("escape");
	
	Addr = Pointer(Cardinal(cmd->pfnFunc) + 15);
	pCBuf_AddText = (DWORD)Pointer(Absolute((DWORD)Addr));

	if ( FarProc((DWORD)Addr,HwBase,HwEnd)  )
		Error("Couldn't find CBuf_AddText #1 pointer.");

	if ( FarProc((DWORD)pCBuf_AddText,HwBase,HwEnd)  )
		Error("Couldn't find CBuf_AddText #2 pointer.");

	delete pCmd;
}

void AutoOffset::Patch_CL_ConnectionlessPacket(void)
{
	DWORD OLD;
	DWORD Address = MessagePtr("Redirecting connection to",26);

	if ( FarProc(Address,HwBase,HwEnd) )
		Error("Couldn't find CL_ConnectionlessPacket #1 pointer.");

	if ( !CheckByte(Address,0xE8,-6) )
		Error("Couldn't find CL_ConnectionlessPacket #2 pointer.");
	else
		Address = Address-5;

	VirtualProtect((PVOID)Address,4,PAGE_EXECUTE_READWRITE,&OLD);
	*PCardinal(Address) = Relative(Cardinal(Address) - 1, Cardinal(&CBuf_AddText));
	VirtualProtect((PVOID)Address,4,OLD,0);
}

void *AutoOffset::Sound(void)
{
	DWORD Addres = MessagePtr("S_StartDynamicSound: %s volume > 255",37);
	
	for(int i = 1;i < 200;i++)
	{
		Addres--;
		if( CheckByte(Addres,0x90,0) )
		{
			Addres++;
			if ( CheckByte(Addres,0x55,0) || CheckByte(Addres,0x83,0) )
			{
				return (void*)Addres;
			}
			else
				Error("Couldn't find PreS_DynamicSound #1 pointer.");
		}
	}
	Error("Couldn't find PreS_DynamicSound #2 pointer.");
	return 0;
}