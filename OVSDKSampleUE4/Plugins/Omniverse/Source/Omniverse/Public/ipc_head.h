#ifndef __OV_IPC_HEAD_H__
#define __OV_IPC_HEAD_H__

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
	char sNo[128];        //Omni serial-number;
	char sUID[64];        //Omni device UID in Omniverse backend.
	char sShopName[64];   //Shop name
	Uint32 nId;           //for internal usage
	Uint32 nShopId;	      //Shop Id
	char sCategory[64];   //for internal usage
};

enum USER_PROP_TYPE
{
	USER_PROP_NONE = 0,
	USER_PROP_GUEST = 1<<0,
	USER_PROP_ALLOW_LOGIN = 1<<1,
	USER_PROP_ALLOW_CALL_HELP = 1<<2,
};

struct IPCUser
{
	Uint32 nUserId;                 //The player's unique account id in Omniverse; 
	Uint32 nGameId;                 //GameID from UOVInterface::init 
	char sCookies[256];             //for internal usage 
	char sGameSDKType[4];           //Omniverse SDK type, "unty": unity-SDK, "ue4x": UE4-SDK;
	Uint32 nGameSDKVersion;         //Omniverse SDK version, version string 0xAAAA.0xBB.0xCC, format as uint32 0xAAAABBCC;
	Uint32 nGamePrepareLeft;        //Game-prepare time left, use for single game ticket mode;
	Uint32 nGameDurationLeft;       //Game-play time left;
	Uint32 nGamePrepare;            //Game-prepare time, each game can config its own prepare time. The time is used to choose level, match game, but the duration is limited.
									//If level or match start, you should call HVSDK.ConfirmPlayGame() to tell SDK countdown game-time now.
	Uint32 nGameDuration;           //Gameplay time;
	char sBillingMode[32];          //Ticket mode, "timing", "timingreal", "direct_game", "shiyu_coin", "game_auth", "timescount";
	Uint32 nUserProp;               //for internal usage;
	char sConsolePath[128];         //for internal usage;
	Sint32 nCoupleRate, nUserCoupleRate; //Omni couple rate (0: decoupled, 10000: coupled)
	char sReserved[60];             //for internal usage;
	char sQrcode[64];               //The omniverse trade number for this game ticket;
	char sWeb2d[128];               //for internal usage;
	Uint32 nCurGameId;              //for internal usage;
	char sReserved2[60];            //for internal usage; 
	char sUserName[128];            //Player's nick game in Omniverse;
	char sUserPhone[32];            //Player's phone number registered in Omniverse; (maybe masked, some character replaced by '*' for privacy)
	char sUserEmail[128];           //Player's email registered in Omniverse; (maybe masked)
	double nUserBalance;           //Player's balance in Omniverse; (In-game purchase costs this balance)
	char sUserIcon[128];            //Player's portrait icon url;
	Uint32 nDesktopDurationLeft;    //for internal usage; 
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

#endif // __OV_IPC_HEAD_H__

