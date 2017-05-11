#ifndef __HV_IPC_HEAD_H__
#define __HV_IPC_HEAD_H__

#ifndef N3_INT_DEFINED
#	define N3_INT_DEFINED
#	ifdef _MSC_VER
typedef __int8 Sint8;
typedef unsigned __int8 Uint8;
typedef __int16 Sint16;
typedef unsigned __int16 Uint16;
typedef __int32 Sint32;
typedef unsigned __int32 Uint32;
typedef __int64 Sint64;
typedef unsigned __int64 Uint64;
#	else
typedef signed char Sint8;
typedef unsigned char Uint8;
typedef signed short Sint16;
typedef unsigned short Uint16;
typedef signed int Sint32;
typedef unsigned int Uint32;
typedef signed long long Sint64;
typedef unsigned long long Uint64;
#	endif
#endif

#pragma pack(push, 4)

struct IPCDevice {
	char sNo[128];
	char sUID[64];
	char sShopName[64];
	Uint32 nId, nShopId;	
	char sCategory[64];
};

enum USER_PROP_TYPE
{
	USER_PROP_NONE = 0,
	USER_PROP_GUEST = 1<<0,
};

struct IPCUser {
	Uint32 nId;
	Uint32 nGameId;	
	char sCookies[256];
	char sGameHVSDKType[4];
	Uint32 nGameHVSDKVersion;
	Uint32 nGamePrepareLeft;
	Uint32 nGameDurationLeft;
	Uint32 nGamePrepare;	//准备时间, 可以在平台配置，每个游戏不一样。（准备时间结束后，开始正式计时，如果提前开始，请调用HVSDK.ConfirmPlayGame()）
	Uint32 nGameDuration;	//游戏时间(不包括准备时间)
	char sBillingMode[32];	//次票/包时
	Uint32 nUserProp;		//用户属性
	char sConsolePath[128];
	char sReserved[68];
	char sQrcode[64];	
	char sWeb2d[128];	
	Uint32 nCurGameId;
	char sReserved2[60];
	char sName[128];
	char sPhone[32];
	char sEmail[128];
	double nWallet;
	char sIcon[128];
	Uint32 nDesktopDurationLeft;
};

struct IPCHead
{
	char sMagic[4];
	Uint64 hHost;
	volatile int nState;	//0:listen, 1, connected
	volatile unsigned int nMsgToClient, nMsgToHost;	//spin-locker
	
	IPCDevice device;
	IPCUser user;
};

struct MsgHead {
	unsigned short nMsg;
	unsigned short nLen;	
};

#pragma pack(pop)

#endif // __HV_IPC_HEAD_H__

