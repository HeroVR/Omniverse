using UnityEngine;
using System.Runtime.InteropServices;
using System.IO;
using System;
using UnityEngine.UI;
using Virtuix;
using UnityEngine.VR;
using System.Xml;
using System.Collections.Generic;

public class OVSDK : MonoBehaviour
{
	public static string _SDKVersion = "0.3.0";

	//public delegate void OVSDKEventCallback(int nMsgType, string sMsgContent);
	public delegate void OVSDKBuyCallback(string sItem, string sOutTradeNo, string sInTradeNo, string sErr);
	public delegate void OVSDKSaveGameDataCallback(int nError, string sMsg);
	public delegate void OVSDKLoadGameDataCallback(int nError, string sMsg, IntPtr data, int len);

	//static OVSDKEventCallback _DllEventCallback = null;
	static OVSDKSaveGameDataCallback _SaveGameDataCB = null;
	static OVSDKLoadGameDataCallback _LoadGameDataCB = null;

	[StructLayout(LayoutKind.Sequential)]
	public struct DeviceInfo
	{
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
		public string sNo;      //Omni serial-number;
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 64)]
		public string sUID;     //Omni device UID in Omniverse backend.
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 64)]
		public string sShopName; //Shop name
		[MarshalAs(UnmanagedType.U4)]
		public int nId;         //for internal usage
		[MarshalAs(UnmanagedType.U4)]
		public int nShop;       //Shop Id
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 64)]
		public string sCategory;//for internal usage
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct UserInfo
	{
		[MarshalAs(UnmanagedType.U4)]
		public int nUserId;                 //The player's unique account id in Omniverse;
		[MarshalAs(UnmanagedType.U4)]
		public int nGameId;             //GameID from OVSDK::Init 

		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
		string sCookies;                //for internal usage 

		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 4)]
		public string sGameSDKType;     //Omniverse SDK type, "unty": unity-SDK, "ue4x": UE4-SDK;
		[MarshalAs(UnmanagedType.U4)]
		public int nGameSDKVersion;     //Omniverse SDK version, version string 0xAAAA.0xBB.0xCC, format as uint32 0xAAAABBCC;

		[MarshalAs(UnmanagedType.U4)]
		public int nGamePrepareLeft;    //Game-prepare time left, use for single game ticket mode;
		[MarshalAs(UnmanagedType.U4)]
		public int nGameDurationLeft;   //Game-play time left;
		[MarshalAs(UnmanagedType.U4)]
		public int nGamePrepare;        //Game-prepare time, each game can config its own prepare time. The time is used to choose level, match game, but the duration is limited.
										//If level or match start, you should call OVSDK.ConfirmPlayGame() to tell SDK countdown game-time now.
		[MarshalAs(UnmanagedType.U4)]
		public int nGameDuration;       //Gameplay time;

		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
		public string sBillingMode;     //Ticket mode, "timing", "timingreal", "direct_game", "shiyu_coin", "game_auth", "timescount";
		[MarshalAs(UnmanagedType.U4)]
		public int nUserProp;           //for internal usage;
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
		public string sConsolePath;     //for internal usage;
		[MarshalAs(UnmanagedType.U4)]
		public int nCoupleRate, nUserCoupleRate; //Omni couple rate (0: decoupled, 10000: coupled)
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 60)]
		string sReserved;               //for internal usage;

		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 64)]
		public string sQrcode;          //The omniverse trade number for this game ticket;
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
		public string sWeb2d;           //for internal usage;

		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 64)]
		public string sReserved2;       //for internal usage;

		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
		public string sUserName;            //Player's nick game in Omniverse;
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
		public string sUserPhone;           //Player's phone number registered in Omniverse; (maybe masked, some character replaced by '*' for privacy)
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
		public string sUserEmail;           //Player's email registered in Omniverse; (maybe masked)
		public double nUserBalance;          //Player's balance in Omniverse; (In-game purchase costs this balance)
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 124)]
		public string sUserIcon;            //Player's portrait icon url;
		[MarshalAs(UnmanagedType.U4)]
		public int nDesktopDurationLeft;//for internal usage; 
	}

	// public interface for Game-developer
	static bool _bIniting = false, _bInitDone = false, _bJustInitDone = false;
	static string _sInitResult;
	static int _nInitResultCode = 0;
	static OVSDK _Instance = null;
	public static OVSDK instance
	{
		get {
			if (null == _Instance)
			{
				_Instance = GameObject.FindObjectOfType<OVSDK>();
				if (null == _Instance)
				{
					GameObject go = new GameObject("OVSDK_GlobalInstance");
					_Instance = go.AddComponent<OVSDK>();
					DontDestroyOnLoad(go);
				}
			}

			return _Instance;
		}
	}

	public static void Init(int nGameId, string sGameKey, string sParam)
	{
		if (null == OVSDK.instance) {
			Debug.LogError("get OVSDK.instance failed.");
		}

		if (_bIniting || _bInitDone) {
			Debug.LogError("OVSDK should Init once only.");
		}

		_bIniting = true;
		_bInitDone = _bJustInitDone = false;

		sParam = sParam + ";sdk_type=unty;sdk_version=" + _SDKVersion;
		DllInit(nGameId, sGameKey, sParam, onEventFromSDK, IntPtr.Zero);
	}

	public static bool HasInitialized()
	{
		return _bInitDone;
	}

	public static bool JustInitialized()
	{
		return _bJustInitDone;
	}

	public static int GetInitResultCode()
	{
		return _nInitResultCode;
	}

	public static string GetInitResult()
	{
		return _sInitResult;
	}

    public static DeviceInfo GetDeviceInfo()
    {
        DeviceInfo info = (DeviceInfo)Marshal.PtrToStructure(DllGetDeviceInfo(), typeof(DeviceInfo)); ;
        return info;
    }

    public static UserInfo GetUserInfo()
    {
        UserInfo info = (UserInfo)Marshal.PtrToStructure(DllGetUserInfo(), typeof(UserInfo));
        return info;
    }

	public static bool IsGuest()
	{
		return DllIsGuest();
	}

    public static void Buy(string sItem, double nPrice, string sOutTradeNo, OVSDKBuyCallback cb = null)
    {
		if (IsGuest())
		{
			MsgBox("", GetString("Purchase"), GetString("PurchaseGuest"), GetString("OK"));
			return;
		}

        if (_MsgboxBuy)
        {
            _MsgboxBuy.Close();
            _MsgboxBuy = null;
        }

        _BuyParam.sItem = sItem;
        _BuyParam.nPrice = nPrice;
        _BuyParam.sOutTradeNo = sOutTradeNo;
        _BuyParam.cb = cb;
        _MsgboxBuy = MsgBox("buy.confirm", GetString("Purchase"), string.Format(GetString("PurchaseConfirm"), sItem, nPrice), GetString("Yes"), GetString("No"));
        _MsgboxBuy._onEvent = onMsgBoxBuy;
    }

    public static void ExitGame(bool bToDllDesktop)
    {
        //if (DllIsDevMode())
        //{
        //    Application.Quit();
        //}
        //else
        {
            DllSendCommand(bToDllDesktop ? 7 : 8, "", 0);
        }
    }

	public static bool SaveGameData(IntPtr data, int len, OVSDKSaveGameDataCallback cb)
	{
		if (IsGuest())
		{
			MsgBox("", "", GetString("SaveGameGuest"), GetString("OK"));
			return false;
		}

		_SaveGameDataCB = cb;
		IntPtr tt = data; 
		DllSaveGameData(tt, len);
		return true;
	}

	public static void LoadGameData(OVSDKLoadGameDataCallback cb)
	{
		_LoadGameDataCB = cb;
		DllLoadGameData();
	}

    static bool onMsgBoxBuy(OVMsgBox hmb, string sEvent, GameObject go)
    {
        if (hmb.usage == "buy.confirm" && sEvent == "click" && go.name == "Yes")
        {
            DllBuy(_BuyParam.sItem, _BuyParam.nPrice, _BuyParam.sOutTradeNo);
            hmb.Reset("buy.request", "", GetString("Wait"));
            return true;
        }

        if (sEvent == "destroy")
        {
            if (hmb == _MsgboxBuy)
            {
                _MsgboxBuy = null;
            }
        }

        return false;
    }

	public static OVMsgBox MsgBox(string sUsage = "", string sTitle = "", string sContent = "", string sYes = "", string sNo = "", int nTimeout = 0, OVMsgBoxTimeoutAct nTimeoutAct = OVMsgBoxTimeoutAct.Close)
    {
        OVMsgBox hmb = null;
        GameObject mb = GameObject.Instantiate(Resources.Load("OVSDK_MsgBox"), Vector3.zero, Quaternion.identity) as GameObject;
        if (mb)
        {
            hmb = mb.GetComponentInChildren<OVMsgBox>();
            if (hmb) {
                hmb.Reset(sUsage, sTitle, sContent, sYes, sNo, nTimeout, nTimeoutAct);
            }
        }
        return hmb;
    }

	public static int GetDlgNum()
	{
		return OVMsgBox.GetVisibleCount();
	}

	public static void ConfirmPlayGame()
    {
        DllSendCommand(6, "", 0);
    }

    struct ResumeGameInfo
    {
        public int retCode;
        public string retMsg;
        public int isAgain;     //Can resume game? (0:no 1:have another tickets 2:have enough balance)
        public double reAmt;    //Player's balance on Omniverse
        public int reTicket;    //Left other tickets.
    }
    struct BuyParam
    {
        public string sItem;
        public double nPrice;
        public string sOutTradeNo;
        public OVSDKBuyCallback cb;
    }
    struct BuyResult
    {
        public int retCode;
        public string retMsg;
        public string intradeno;
    }
    static ResumeGameInfo _ResumeGameInfo;
    static OVMsgBox _MsgboxBuy;
    static BuyParam _BuyParam = new BuyParam();
    static OVMsgBox _MsgboxResumeGame;
	public static bool _DisableResumeGame = false;

    //public static int _nMsgboxLayer = 31;
    public static OVControllerRay _ControllerRay = new OVControllerRay();

    private static void onEventFromSDK(IntPtr sType, int nRetCode, IntPtr sRet, int nLen, IntPtr pUserData)
    {
		//string type = bytesToString(sType);
		string type = Marshal.PtrToStringAnsi(sType);
		string ret = "";

		// load game data
		if ("ldgd" == type)	
		{
			Debug.Log("OVSDK.OnEventFromSDK(" + type + ", " + nRetCode + ")");
			if (_LoadGameDataCB != null)
			{
				try
				{
					if (nRetCode != 0) {
						ret = Marshal.PtrToStringAnsi(sRet, nLen);
					}					
					_LoadGameDataCB(nRetCode, ret, sRet, nLen);
				}
				catch (Exception e)
				{
					Debug.LogError("error:" + e.Message);
				}
			}
			return;
		}

		//ret = bytesToString(sRet, nLen);
		ret = Marshal.PtrToStringAnsi(sRet, nLen);
		Debug.Log("OVSDK.OnEventFromSDK(" + type + ", " + nRetCode + ",\n" + ret + ")");

		if (type == "init")
		{
			_bInitDone = true;
			_sInitResult = ret;
			_nInitResultCode = nRetCode;			
		}
		else if (type == "ipc.")
		{
			switch (nRetCode)
			{
				case 2:
					onEventResumeGame(ret);
					break;
				case 5:
					onEventResumeGameResult(ret);
					break;
				case 9:
					onEventSystemMenu(ret);
					break;
				case 12:
					onEventMsgBoxJson(ret);
					break;
				case 13:
					onEventMsgBoxJsonCmd(ret);
					break;
			}

			//if (null != _DllEventCallback)
			//{
			//	_DllEventCallback(nRetCode, ret);
			//}
		}
		else if (type == "buy ")
		{
			onEventBuy(ret);
		}
		else if (type == "svgd")
		{
			if (_SaveGameDataCB != null) {
				_SaveGameDataCB(nRetCode, ret);
			}
		}
    }

    private static bool onMsgBoxResumeGame(OVMsgBox hmb, string sEvent, GameObject sender)
    {
        if (sEvent == "click")
        {
            if (sender.name == "Yes")
            {
                if (hmb == _MsgboxResumeGame)
                {
                    if (_ResumeGameInfo.isAgain != 0)
                    {
                        DllSendCommand(3, "", 0);
                        hmb.Reset(null, "", GetString("Wait"));
                        return true;
                    }
                    else
                    {
                        hmb.Reset(null, "", GetString("ResumeError"), "", "", 5, OVMsgBoxTimeoutAct.QuitGame);
                        return true;
                    }
                }
            }
            else if (sender.name == "No")
            {
                if (hmb == _MsgboxResumeGame)
                {
                    hmb.Reset(null, "", GetString("Bye"), "", "", 3, OVMsgBoxTimeoutAct.QuitGame);
                    return true;
                }
            }
        }
        else if (sEvent == "destroy")
        {
            if (hmb == _MsgboxResumeGame)
            {
                _MsgboxResumeGame = null;
            }
        }

        return false;
    }

    private static void onEventResumeGame(string sJson)
    {
        if (_MsgboxResumeGame)
        {
            _MsgboxResumeGame.Close();
            _MsgboxResumeGame = null;
        }

		if (!_DisableResumeGame)
		{
			_ResumeGameInfo = JsonUtility.FromJson<ResumeGameInfo>(sJson);
			string tx = string.Format(GetString("ResumeConfirm"), _ResumeGameInfo.reAmt.ToString("F2"), _ResumeGameInfo.reTicket.ToString());
			_MsgboxResumeGame = MsgBox("ResumeGame", GetString("Resume"), tx, GetString("ResumeYes"), GetString("ResumeNo"), 15, OVMsgBoxTimeoutAct.Close);
			_MsgboxResumeGame._onEvent = onMsgBoxResumeGame;
		}        
    }

    private static void onEventResumeGameResult(string sRet)
    {
        if (sRet.Length > 0) {
            MsgBox("", "", string.Format(GetString("ResumeFailed"), sRet), "", "", 10, OVMsgBoxTimeoutAct.Close);
        }
        else {
            MsgBox("", "", GetString("ResumeSuccess"), "", "", 2);
        }

        if (_MsgboxResumeGame) {
            _MsgboxResumeGame.Close();
            _MsgboxResumeGame = null;
        }
    }

    private static void onEventBuy(string sRet)
    {
        BuyResult res = new BuyResult();
        res.intradeno = "";
        res.retCode = -1;
        res.retMsg = GetString("PurchaseConnectFailed");
        try {
            res = JsonUtility.FromJson<BuyResult>(sRet);
        }
        catch (Exception)
        {
            Debug.Log("onEventBuy " + sRet);
        }

        string txt;
        if (res.retCode != 0) {
            txt = String.Format(GetString("PurchaseFailed"), res.retMsg);
        }
        else {
            txt = String.Format(GetString("PurchaseSuccess"), res.intradeno);
        }

        if (_MsgboxBuy) {
            _MsgboxBuy.Reset("buy.done", GetString("Purchase"), txt, GetString("Close"), "");
        }
        else {
            MsgBox("buy.done", GetString("Purchase"), txt, GetString("Close"));
        }

        if (_BuyParam.cb != null) {
            _BuyParam.cb(_BuyParam.sItem, _BuyParam.sOutTradeNo, res.intradeno, res.retMsg);
        }
    }

    // HeroVR System Menu
    static OVMsgBox _MsgboxSystemMenu = null;
	public static bool _DisableSystemMenu = false;    //for test System-Menu in development environment.

    static void onEventSystemMenu(string param)
    {
        if (!_DisableSystemMenu && _MsgboxSystemMenu == null)
        {
            GameObject mb = GameObject.Instantiate(Resources.Load("OVSDK_MsgboxMenu"), Vector3.zero, Quaternion.identity) as GameObject;
            if (mb)
            {
                _MsgboxSystemMenu = mb.GetComponentInChildren<OVMsgBoxMenu>();
                _MsgboxSystemMenu._onEvent = onMsgBoxSystemMenuEvent;
            }
        }
    }

    static bool onMsgBoxSystemMenuEvent(OVMsgBox hmb, string sEvent, GameObject go)
    {
        if (sEvent == "destroy")
        {
            if (hmb == _MsgboxSystemMenu)
            {
                _MsgboxSystemMenu = null;
            }
        }
        else if (sEvent == "click")
        {
            if (go.name == "Yes")
            {
                ExitGame(true);
                if (!DllIsDevMode())
                {
                    MsgBox("", "", GetString("Wait"));
                }
            }
            else if (go.name == "No")
            {
                ExitGame(false);
                if (!DllIsDevMode())
                {
                    MsgBox("", "", GetString("Wait"));
                }
            }

            hmb.Close();
            return true;
        }

        return false;
    }

	static void onEventMsgBoxJson(string param)
	{
		MsgBoxJson(param);
	}

	public static void MsgBoxJson(string param)
	{
		GameObject mb = GameObject.Instantiate(Resources.Load("OVSDK_MsgboxMenu"), Vector3.zero, Quaternion.identity) as GameObject;
		if (mb)
		{
			OVMsgBoxMenu mbm = mb.GetComponentInChildren<OVMsgBoxMenu>();
			if (null != mbm) {
				mbm._JsonFilePrefix = param;
			}
		}
	}

	static void onEventMsgBoxJsonCmd(string param)
	{
		string prefix = null, name = null, cmd = null;
		int sep = param.IndexOf(' ');
		if (sep > 0)
		{
			prefix = param.Substring(0, sep);
			int sep2 = param.IndexOf(' ', sep + 1);
			if (sep2 > 0) {
				name = param.Substring(sep + 1, sep2 - sep - 1);
				cmd = param.Substring(sep2 + 1);
			}
		}

		if (cmd != null)
		{
			foreach (OVMsgBoxMenu mbm in OVMsgBoxMenu._AllMsgBoxJson)
			{
				if (mbm._JsonFilePrefix == prefix) {
					mbm.UpdateItemCmd(name, cmd);
				}
			}
		}		
	}

	public static void CheckOmniViveAvailable(bool bSwitchToDevModeIfNA = true)
    {
        OmniMovementComponent omniMovementComponent = null;
        if (bSwitchToDevModeIfNA)
        {
            omniMovementComponent = GameObject.FindObjectOfType<SteamVROmniMovementComponent>();
            if (omniMovementComponent)
            {
                Debug.Log("[CheckOmniViveAvailable]Vive(N/A), Omni("
                    + (omniMovementComponent.omniFound ? "Found" : "N/A")
                    + ") switch to dev-mode.");

				if (!VRSettings.enabled) {
					omniMovementComponent.cameraReference.gameObject.AddComponent<SmoothMouseLook>();
				}

				if (!omniMovementComponent.omniFound) {
					omniMovementComponent.developerMode = true;
				}				
			}
        }		
    }

    static int _SysLang = -1;
    static Dictionary<string, string> _LangStr = new Dictionary<string, string>();
    static void CheckLang()
    {
        if (_SysLang == -1)
        {
            _SysLang = (int)Application.systemLanguage;
            if (_SysLang == (int)SystemLanguage.Chinese)
            {
                _SysLang = (int)SystemLanguage.ChineseSimplified;
            }

            LoadString("OVSDK_String");
        }
    }
    public static string GetString(string id)
    {
        CheckLang();

        string ret = null;
        try
        {
            ret = _LangStr[id];
        }
        catch (Exception/* e*/)
        {
            ret = id;
        }

        return ret;
    }

    public static void LoadString(string xml)
    {
        CheckLang();

        string lang_xml = "";
        TextAsset lang_data = null;
        if (_SysLang != (int)SystemLanguage.English && _SysLang != (int)SystemLanguage.Unknown)
        {
            lang_xml = xml + _SysLang;
            lang_data = (TextAsset)Resources.Load(lang_xml);
        }

        if (null == lang_data)
        {
            lang_xml = xml;
            lang_data = (TextAsset)Resources.Load(lang_xml);
        }

        if (null == lang_data)
        {
            Debug.LogError(string.Format("Resources.Load({0}) failed.", xml));
        }
        else
        {
            Debug.Log(string.Format("Resources.Load({0}) success.", lang_xml));

            XmlDocument doc = new XmlDocument();
            doc.LoadXml(lang_data.text);

            XmlNodeList rows = doc.DocumentElement.GetElementsByTagName("r");
            for (int i = 0; i < rows.Count; ++i)
            {
                XmlAttributeCollection r = rows.Item(i).Attributes;
                XmlAttribute k = r["id"], v = r["str"];
                if (k != null && v != null)
                {
                    _LangStr[k.Value] = v.Value;
                    //Debug.Log(string.Format("lang [{0}] = {1}", k.Value, v.Value));
                }
            }
        }
    }

	public static float GetOmniYawOffset()
	{
		return DllGetOmniYawOffset();
	}

	public static float GetOmniCoupleRate()
	{
		int coupleRate = Marshal.ReadInt32((IntPtr)((long)DllGetUserInfo() + 452));
		int userCoupleRate = GetUserOmniCoupleRate();
		return (float)(0.0001 * (userCoupleRate > 0 ? userCoupleRate - 1 : coupleRate));
	}

	public static int GetUserOmniCoupleRate()	{
		return Marshal.ReadInt32((IntPtr)((long)DllGetUserInfo() + 456));
	}

	public static void SetOmniCoupleRate(float rate)
	{
		Marshal.WriteInt32((IntPtr)((long)DllGetUserInfo() + 456), (int)(10000*rate + 1));
	}

	//----------------------------------------------------------------------------------------------------------------------------------
	// Omniverse.Functions.dll functions.
	delegate void DllCallback(IntPtr sType, int nRetCode, IntPtr sRet, int nLen, IntPtr pUserData);
	[DllImport("Omniverse.Functions")]
	static extern bool DllInit(int nGameId, string sGameKey, string sParam, DllCallback cb, IntPtr pUserData);

	[DllImport("Omniverse.Functions")]
	public static extern bool DllLaunchGame(string exe, string workdir, string cmdline, int game_id, int prepare_time);   //启动游戏，传入参数

	[DllImport("Omniverse.Functions")]
	static extern bool DllIsDevMode();

	[DllImport("Omniverse.Functions")]
	static extern void DllDrive();

	[DllImport("Omniverse.Functions")]
	static extern void DllSendCommand(int nCmd, string sData, int nLen);

	[DllImport("Omniverse.Functions")]
	static extern void DllBuy(string sItem, double nPrice, string sOutTradeNo);

	[DllImport("Omniverse.Functions")]
	static extern IntPtr DllGetDeviceInfo();

	[DllImport("Omniverse.Functions")]
	static extern IntPtr DllGetUserInfo();

	[DllImport("Omniverse.Functions")]
	static extern float DllGetOmniYawOffset();

	[DllImport("Omniverse.Functions")]
	static extern bool DllIsGuest();

	[DllImport("Omniverse.Functions")]
	static extern void DllSaveGameData(IntPtr data, int len);

	[DllImport("Omniverse.Functions")]
	static extern void DllLoadGameData();

	[DllImport("Omniverse.Functions")]
	public static extern void DllTest(IntPtr param);

#if UNITY_EDITOR
	[DllImport("User32.dll", SetLastError = true, ThrowOnUnmappableChar = true, CharSet = CharSet.Auto)]
	public static extern int MessageBox(IntPtr handle, String message, String title, int type);
#endif

	//----------------------------------------------------------------------------------------------------------------------------------
	// internal utility functions.
	/*
	private static unsafe string bytesToString(byte* bytes, int nLen = -1)
	{
		int len = 0;
		if (nLen < 0)
		{
			for (int i = 0; bytes[i] != 0; i++, len++) { }
		}
		else
		{
			len = nLen;
		}

		byte[] data = new byte[len];
		using (UnmanagedMemoryStream ms = new UnmanagedMemoryStream(bytes, len))
		{
			ms.Read(data, 0, data.Length);
		}
		string ret = System.Text.Encoding.UTF8.GetString(data);

		return ret;
	}
	*/

	public static void sendMsg(int nMsgType, string sMessageContent)
	{
		byte[] data = System.Text.Encoding.UTF8.GetBytes(sMessageContent);
		DllSendCommand(nMsgType, sMessageContent, data.Length);
	}

	void Start()
	{
	}
	void Update()
	{
		_ControllerRay.Drive();

		DllDrive();

		if (_bInitDone)
		{
			if (_bIniting)
			{
				_bIniting = false;
				_bJustInitDone = true;
			}
			else if (_bJustInitDone)
			{
				_bJustInitDone = false;
			}
		}
	}
	void OnDestroy()
	{

	}
}