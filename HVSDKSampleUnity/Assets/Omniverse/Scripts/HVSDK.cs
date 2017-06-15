using UnityEngine;
using System.Runtime.InteropServices;
using System.IO;
using System;
using UnityEngine.UI;
using Virtuix;
using UnityEngine.VR;
using System.Xml;
using System.Collections.Generic;

public class HVSDK : MonoBehaviour
{
    public static string _SDKVersion = "0.3.0";

    public unsafe delegate void plugin_callback(byte* sType, byte* sRet, int nLen, UnityEngine.Object pUserData);
    public unsafe delegate void hv_callback(int nMsgType, string sMsgContent);

    [DllImport("desktop_plugin")]
    private static extern bool HvInit(int nGameId, string sGameKey, string sParam, plugin_callback cb, UnityEngine.Object pUserData);

    [DllImport("desktop_plugin")]
    public static extern bool HvLaunchGame(string exe, string workdir, string cmdline, int game_id, int prepare_time);   //启动游戏，传入参数

    [DllImport("desktop_plugin")]
    private static extern bool HvIsDevMode();

    [DllImport("desktop_plugin")]
    private static extern void HvDrive();

    [DllImport("desktop_plugin")]
    private static extern void HvSendCommand(int nCmd, string sData, int nLen);

    [DllImport("desktop_plugin")]
    private static extern void HvBuy(string sItem, double nPrice, string sOutTradeNo);

    [DllImport("desktop_plugin")]
    private static extern IntPtr HvGetDeviceInfo();

    [DllImport("desktop_plugin")]
    private static extern IntPtr HvGetUserInfo();

    [DllImport("desktop_plugin")]
    public static extern float HvGetOmniYawOffset();

	[DllImport("desktop_plugin")]
	public static extern bool HvIsGuest();

#if UNITY_EDITOR
    [DllImport("User32.dll", SetLastError = true, ThrowOnUnmappableChar = true, CharSet = CharSet.Auto)]
    public static extern int MessageBox(IntPtr handle, String message, String title, int type);
#endif

    [StructLayout(LayoutKind.Sequential)]
    public struct DeviceInfo
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string sNo;      //设备序列号
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 64)]
        public string sUID;     //设备在平台UID字符串
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 64)]
        public string sShopName; //店铺名字
        [MarshalAs(UnmanagedType.U4)]
        public int nId;         //设备在平台的编号
        [MarshalAs(UnmanagedType.U4)]
        public int nShop;       //店铺在平台的编号
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 64)]
        public string sCategory; //设备类型
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct UserInfo
    {
        [MarshalAs(UnmanagedType.U4)]
        public int nId;             //用户在平台ID
        [MarshalAs(UnmanagedType.U4)]
        public int nGameId;         //启动的游戏ID

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        string sCookies;     //保留

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 4)]
        public string sGameHVSDKType;
        [MarshalAs(UnmanagedType.U4)]
        public int nGameHVSDKVersion;

        [MarshalAs(UnmanagedType.U4)]
        public int nGamePrepareLeft;	//剩余准备时间；
        [MarshalAs(UnmanagedType.U4)]
        public int nGameDurationLeft;	//剩余游戏时间
        [MarshalAs(UnmanagedType.U4)]
        public int nGamePrepare;    //准备时间, 可以在平台配置，每个游戏不一样。（准备时间结束后，开始正式计时，如果提前开始，请调用HVSDK.ConfirmPlayGame()）
        [MarshalAs(UnmanagedType.U4)]
        public int nGameDuration;   //游戏时间(不包括准备时间)

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string sBillingMode;  //timescount: 次票; timing: 包时
		[MarshalAs(UnmanagedType.U4)]
		public int nUserProp;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string sConsolePath;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 68)]
        string sReserved;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 64)]
        public string sQrcode;      //校验用字段，用于校验用户的有效性；
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string sWeb2d;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 64)]
        public string sReserved2;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string sName;        //用户在平台的昵称
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string sPhone;       //用户在平台的注册手机号（已加密）
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string sEmail;       //用户在平台的注册邮箱
        public double nWallet;      //用户在平台的预充值；
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 124)]
        public string sIcon;        //用户在平台的头像url;
        [MarshalAs(UnmanagedType.U4)]
        public int nDesktopDurationLeft;
    }

    private static unsafe string bytesToString(byte* bytes, int nLen = -1)
    {
        int len = 0;
        if (nLen < 0) {
            for (int i = 0; bytes[i] != 0; i++, len++) { }
        }
        else {
            len = nLen;
        }

        byte[] data = new byte[len];
        using (UnmanagedMemoryStream ms = new UnmanagedMemoryStream(bytes, len)) {
            ms.Read(data, 0, data.Length);
        }
        string ret = System.Text.Encoding.UTF8.GetString(data);
        return ret;
    }

    // public interface for Game-developer
    public static bool _bIniting = false, _bInitDone = false, _bJustInitDone = false;
    public static string _sInitResult;

	void Start()
	{
	}
	void Update()
	{
		_ControllerRay.Drive();

		HvDrive();

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

	static HVSDK _Instance = null;
	public static HVSDK instance
	{
		get {
			if (null == _Instance)
			{
				_Instance = GameObject.FindObjectOfType<HVSDK>();
				if (null == _Instance)
				{
					GameObject go = new GameObject("HVSDK_GlobalInstance");
					_Instance = go.AddComponent<HVSDK>();
					DontDestroyOnLoad(go);
				}
			}

			return _Instance;
		}
	}

	public static void Init(int nGameId, string sGameKey, string sParam, hv_callback cbHvMsg)
    {
		if (null == HVSDK.instance)	{
			Debug.LogError("get HVSDK.instance failed.");
		}

		if (_bIniting || _bInitDone) {
			Debug.LogError("HVSDK should Init once only.");
		}

        _bIniting = true;
        _bInitDone = _bJustInitDone = false;
        _hvMsgCallBack = cbHvMsg;

        sParam = sParam + ";hvsdk_type=unty;hvsdk_version=" + _SDKVersion;
        unsafe {
            HvInit(nGameId, sGameKey, sParam, onEventFromSDK, null);
        }
    }

    public static void sendMsg(int nMsgType, string sMessageContent)
    {
        byte[] data = System.Text.Encoding.UTF8.GetBytes(sMessageContent);
        HvSendCommand(nMsgType, sMessageContent, data.Length);
    }

    public static DeviceInfo GetDeviceInfoImpl()
    {
        DeviceInfo info = new DeviceInfo();

        IntPtr ptr = HvGetDeviceInfo();
        if (ptr != IntPtr.Zero)
        {
            try {
                info = (DeviceInfo)Marshal.PtrToStructure(ptr, typeof(DeviceInfo));
            }
            catch (Exception e) {
                Debug.LogError("error:" + e.Message);
            }
        }

        return info;
    }

    public static UserInfo GetUserInfoImpl()
    {
        UserInfo info = new UserInfo();

        IntPtr ptr = HvGetUserInfo();
        if (ptr != IntPtr.Zero)
        {
            try {
                info = (UserInfo)Marshal.PtrToStructure(ptr, typeof(UserInfo));
            }
            catch (Exception e) {
                Debug.LogError("error:" + e.Message);
            }
        }

        return info;
    }

    public delegate void buy_callback(string sItem, string sOutTradeNo, string sInTradeNo, string sErr);
    public static void Buy(string sItem, double nPrice, string sOutTradeNo, buy_callback cb = null)
    {
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

    public static void ExitGame(bool bToHvDesktop)
    {
        //if (HvIsDevMode())
        //{
        //    Application.Quit();
        //}
        //else
        {
            HvSendCommand(bToHvDesktop ? 7 : 8, "", 0);
        }
    }

    static bool onMsgBoxBuy(HVMsgBox hmb, string sEvent, GameObject go)
    {
        if (hmb.usage == "buy.confirm" && sEvent == "click" && go.name == "Yes")
        {
            HvBuy(_BuyParam.sItem, _BuyParam.nPrice, _BuyParam.sOutTradeNo);
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

    public
    static HVMsgBox MsgBox(string sUsage = "", string sTitle = "", string sContent = "", string sYes = "", string sNo = "", int nTimeout = 0, HVMsgBoxTimeoutAct nTimeoutAct = HVMsgBoxTimeoutAct.Close)
    {
        HVMsgBox hmb = null;
        GameObject mb = GameObject.Instantiate(Resources.Load("HVSDK_MsgBox"), Vector3.zero, Quaternion.identity) as GameObject;
        if (mb)
        {
            hmb = mb.GetComponentInChildren<HVMsgBox>();
            if (hmb) {
                hmb.Reset(sUsage, sTitle, sContent, sYes, sNo, nTimeout, nTimeoutAct);
            }
        }
        return hmb;
    }

    public static void ConfirmPlayGame()
    {
        HvSendCommand(6, "", 0);
    }

    struct ResumeGameInfo
    {
        public int retCode;
        public string retMsg;
        public int isAgain;     //是否再来一次(0:不符合条件 1:有票 2:余额足)
        public double reAmt;    //用户剩余金额
        //public double price;
        public int reTicket;    //次游戏的用户剩余票张数
    }
    struct BuyParam
    {
        public string sItem;
        public double nPrice;
        public string sOutTradeNo;
        public buy_callback cb;
    }
    struct BuyResult
    {
        public int retCode;
        public string retMsg;
        public string intradeno;
    }
    static ResumeGameInfo _ResumeGameInfo;
    static HVMsgBox _MsgboxBuy;
    static BuyParam _BuyParam = new BuyParam();
    static HVMsgBox _MsgboxResumeGame;
	public static bool _DisableResumeGame = false;

    //public static int _nMsgboxLayer = 31;
    public static HVControllerRay _ControllerRay = new HVControllerRay();
    public static hv_callback _hvMsgCallBack = null;

    private static unsafe void onEventFromSDK(byte* sType, byte* sRet, int nLen, UnityEngine.Object pUserData)
    {
        string type = bytesToString(sType);
        string ret = bytesToString(sRet, nLen);

        Debug.Log("HVSDK.OnEventFromSDK(" + type + ", " + ret + ")");

        if (type == "init")
        {
            _bInitDone = true;
            _sInitResult = ret;
        }
        else if (type.Substring(0, 4) == "ipc.")
        {
            string id = type.Substring(4);
			if (id == "2")
			{
				onEventResumeGame(ret);
			}
			else if (id == "5")
			{
				onEventResumeGameResult(ret);
			}
			else if (id == "9")
			{
				onEventSystemMenu(ret);
			}
			else if (id == "12")
			{
				onEventMsgBoxJson(ret);
			}
			else if (id == "13")
			{
				onEventMsgBoxJsonCmd(ret);
			}

            if (null != _hvMsgCallBack)
            {
                _hvMsgCallBack(int.Parse(id), ret);
            }
        }
        else if (type == "buy ")
        {
            onEventBuy(ret);
        }
    }

    private static bool onMsgBoxResumeGame(HVMsgBox hmb, string sEvent, GameObject sender)
    {
        if (sEvent == "click")
        {
            if (sender.name == "Yes")
            {
                if (hmb == _MsgboxResumeGame)
                {
                    if (_ResumeGameInfo.isAgain != 0)
                    {
                        HvSendCommand(3, "", 0);
                        hmb.Reset(null, "", GetString("Wait"));
                        return true;
                    }
                    else
                    {
                        hmb.Reset(null, "", GetString("ResumeError"), "", "", 5, HVMsgBoxTimeoutAct.QuitGame);
                        return true;
                    }
                }
            }
            else if (sender.name == "No")
            {
                if (hmb == _MsgboxResumeGame)
                {
                    hmb.Reset(null, "", GetString("Bye"), "", "", 3, HVMsgBoxTimeoutAct.QuitGame);
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
			_MsgboxResumeGame = MsgBox("ResumeGame", GetString("Resume"), tx, GetString("ResumeYes"), GetString("ResumeNo"), 15, HVMsgBoxTimeoutAct.Close);
			_MsgboxResumeGame._onEvent = onMsgBoxResumeGame;
		}        
    }

    private static void onEventResumeGameResult(string sRet)
    {
        if (sRet.Length > 0) {
            MsgBox("", "", string.Format(GetString("ResumeFailed"), sRet), "", "", 10, HVMsgBoxTimeoutAct.Close);
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
    static HVMsgBox _MsgboxSystemMenu = null;
	public static bool _DisableSystemMenu = false;    //for test System-Menu in development environment.

    static void onEventSystemMenu(string param)
    {
        if (!_DisableSystemMenu && _MsgboxSystemMenu == null)
        {
            GameObject mb = GameObject.Instantiate(Resources.Load("HVSDK_MsgboxMenu"), Vector3.zero, Quaternion.identity) as GameObject;
            if (mb)
            {
                _MsgboxSystemMenu = mb.GetComponentInChildren<HVMsgBoxMenu>();
                _MsgboxSystemMenu._onEvent = onMsgBoxSystemMenuEvent;
            }
        }
    }

    static bool onMsgBoxSystemMenuEvent(HVMsgBox hmb, string sEvent, GameObject go)
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
                if (!HvIsDevMode())
                {
                    MsgBox("", "", GetString("Wait"));
                }
            }
            else if (go.name == "No")
            {
                ExitGame(false);
                if (!HvIsDevMode())
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
		GameObject mb = GameObject.Instantiate(Resources.Load("HVSDK_MsgboxMenu"), Vector3.zero, Quaternion.identity) as GameObject;
		if (mb)
		{
			HVMsgBoxMenu mbm = mb.GetComponentInChildren<HVMsgBoxMenu>();
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
			foreach (HVMsgBoxMenu mbm in HVMsgBoxMenu._AllMsgBoxJson)
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
        if (!VRSettings.enabled && bSwitchToDevModeIfNA)
        {
            omniMovementComponent = GameObject.FindObjectOfType<SteamVROmniMovementComponent>();
            if (omniMovementComponent)
            {
                Debug.Log("[CheckOmniViveAvailable]Vive(N/A), Omni("
                    + (omniMovementComponent.omniFound ? "Found" : "N/A")
                    + ") switch to dev-mode.");
                //Vector3 v3 = omniMovementComponent.transform.position;
                //Quaternion qu = omniMovementComponent.transform.rotation;

            }
        }

        /*
        if (null == omniMovementComponent)
        {
            omniMovementComponent = GameObject.FindObjectOfType<SteamVROmniMovementComponent>();
        }

        if (omniMovementComponent)
        {
            Transform steam_camera_rig_offset = omniMovementComponent.transform.Find("ViveSetupCameraRigParent/ViveSetupCameraRigRootOffset");
            if (steam_camera_rig_offset)
            {
              //  steam_camera_rig_offset.localPosition = Vector3.zero;
            }

            if (Input.GetKeyDown(KeyCode.D))
            {
                omniMovementComponent.developerMode = true;
                Debug.Log("not Found omni to set developerMode"+ omniMovementComponent.developerMode);
            }
            else
            {
                omniMovementComponent.developerMode = false;
            }
        }
		*/
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

            LoadString("HVSDK_String");
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
}