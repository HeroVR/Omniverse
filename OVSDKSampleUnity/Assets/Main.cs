using UnityEngine;
using System.Collections;
using UnityEngine.UI;
using Virtuix;
using System.Runtime.InteropServices;
using System.IO;
using System;

public class Main : MonoBehaviour {

	[StructLayout(LayoutKind.Sequential, Pack = 4)]
	struct GameData
	{
		public Matrix4x4 red, green;
	}

	public GameObject _goRed, _goGreen;

    OVMsgBox _MsgboxIniting;
	OVMsgBox _MsgboxLoadingGame;

	void Start ()
	{
        OVSDK.Init(1000, "01ec17dac7140c0fbe936ee128310000", "omni=1");

		OVSDK.LoadString("SampleString");
        _MsgboxIniting = OVSDK.MsgBox("", "", OVSDK.GetString("WaitInit"));
	}

    void Update ()
    {
        //OVSDK.Drive();
        if (OVSDK.JustInitialized()) {
            OnOVSDKInit();
        }
	}

	void OnOVSDKInit()
    {
        Debug.Log("OnOVSDKInit: " + OVSDK.GetInitResult());

        OVSDK.DeviceInfo dev = OVSDK.GetDeviceInfo();
        Debug.Log("shop=" + dev.sShopName + ";shopid=" + dev.nShop + ";shop_category=" + dev.sCategory + ";device_sn=" + dev.sNo);

        OVSDK.UserInfo user = OVSDK.GetUserInfo();
        Debug.Log(string.Format("{0}name={1};id={2};phone={3};qrcode={4};balance={5}", OVSDK.IsGuest() ? "[GUEST]" : "", user.sUserName, user.nUserId, user.sUserPhone, user.sQrcode, user.nUserBalance));

		OnClickLoadGame();

//#if UNITY_EDITOR
        OVSDK.CheckOmniViveAvailable();
//#endif

        // buttons in sample-scene
        OVSDK._ControllerRay.CheckShow(1);

        if (_MsgboxIniting != null)
        {
            _MsgboxIniting.Close();
            _MsgboxIniting = null;
        }

        if (OVSDK.GetUserInfo().nGamePrepareLeft > 0)
        {
            //OVMsgBox hmb = OVSDK.MsgBox("prepare_game", OVSDK.GetString("PrepareStage"), OVSDK.GetString("PrepareConfirm"), OVSDK.GetString("Close"), "", OVSDK.GetUserInfo().nGamePrepareLeft);
			//hmb._onEvent = OnPrepareMsgBoxEvent;
        }
        else
        {
            //OVSDK.MsgBox("", OVSDK.GetString("InitResult"), OVSDK._sInitResult, OVSDK.GetString("Close"));
        }        
    }
	bool OnPrepareMsgBoxEvent(OVMsgBox hmb, string sEvent, GameObject sender)
	{
		if (sEvent == "timeout")
		{
			hmb.Reset("prepare_game_done", null, OVSDK.GetString("PrepareEnd"), OVSDK.GetString("OK"));
			return true;
		}

		if (sEvent == "click" && sender.name == "OK" && hmb.usage == "prepare_game")
		{
			OVSDK.ConfirmPlayGame();
		}
		return false;
	}

	public void OnClickSampleMsgBox()
	{
        OVMsgBox hmb = OVSDK.MsgBox("", "OVSDK", "Hello World!!", OVSDK.GetString("OK"));
        hmb._onEvent = OnClickSampleMsgBoxOK;
		// test MsgBoxJson;
		//if (OVMsgBoxMenu._AllMsgBoxJson.Count == 0)	{
		//	OVSDK.MsgBoxJson("test");
		//}
		//else {
		//	OVSDK.onEventMsgBoxJsonText("test ExtendButton 1|<size=14><color=#FFFFFF>Remaining Game time : </color><color=#00FFFF>{0} Min.</color></size>");
		//}		
    }

    bool OnClickSampleMsgBoxOK(OVMsgBox hmb, string sEvent, GameObject sender)
    {
        if (sEvent == "click")
        {
            Debug.Log("sample msgbox ok clicked.");
            hmb.Close();
            return true;
        }
        return false;
    }

	public void OnClickSaveGame()
	{
		GameData gd = new GameData();
		gd.red = _goRed.transform.localToWorldMatrix;
		gd.green = _goGreen.transform.localToWorldMatrix;

		int len = Marshal.SizeOf(typeof(GameData));
		IntPtr data = Marshal.AllocHGlobal(len);
		Marshal.StructureToPtr(gd, data, true);

		OVSDK.SaveGameData(data, len, OnSaveGameDone);
		//OVSDK.DllTest(data);

		Marshal.FreeHGlobal(data);
		data = IntPtr.Zero;		
	}

	public void OnClickLoadGame()
	{
		OVSDK.LoadGameData(OnLoadGameDone);
		_MsgboxLoadingGame = OVSDK.MsgBox("LoadGame", "Loading", "Loading saved game data...");
	}

	void OnSaveGameDone(int nRetCode, string sRetMsg)
	{
		OVSDK.MsgBox("SaveGame", "Save", nRetCode == 0 ? "Save Success." : "Save failed\n" + sRetMsg, OVSDK.GetString("OK"));	
	}

	void OnLoadGameDone(int nRetCode, string sRetMsg, IntPtr data, int len)
	{	
		_MsgboxLoadingGame.Close();

		if (data != IntPtr.Zero && len == Marshal.SizeOf(typeof(GameData)))
		{			
			GameData gd = (GameData)Marshal.PtrToStructure(data, typeof(GameData));

			_goRed.transform.localPosition = gd.red.GetPosition();
			_goRed.transform.localRotation = gd.red.GetRotation();
			_goGreen.transform.localPosition = gd.green.GetPosition();
			_goGreen.transform.localRotation = gd.green.GetRotation();
		}		
	}

	void OnBuyResult(string sItem, string sOutTradeNo, string sInTradeNo, string sErr)
    {
        if (sInTradeNo != null && sInTradeNo != "")
        {
            var prefab = Resources.Load("OVSDK_SampleCube");
            if (prefab)
            {
                string[] strs = sOutTradeNo.Split(',');
                Vector3 pos = new Vector3(float.Parse(strs[0]), float.Parse(strs[1]), float.Parse(strs[2]));

                GameObject go = GameObject.Instantiate(prefab, pos, Quaternion.identity) as GameObject;
                Rigidbody rb = go.GetComponent<Rigidbody>();
                if (rb)
                {
                    float angle = UnityEngine.Random.Range(-0.7f, 0.7f);
                    Vector3 v = new Vector3(Mathf.Sin(angle), 1, Mathf.Cos(angle)) * 5;
                    rb.velocity = v;
                }
            }            
        }
    }

    public void OnClickSampleBuy(GameObject sender)
    {
        // Game customized trader-no
        string out_trade_no = sender.transform.position.x + "," + sender.transform.position.y + "," + sender.transform.position.z + "," + Time.time;

        OVSDK.Buy("Box", 0.88, out_trade_no, OnBuyResult);
    }

	public void OnClickSampleSwitchControllerRay(GameObject sender)
    {
		OVSDK._ControllerRay.CheckShow(OVSDK._ControllerRay.IsForceShow() ? 0 : 1);

		UpdateButtonSwitchControllerRay(sender);
    }

	void UpdateButtonSwitchControllerRay(GameObject btn)
	{
		if (btn)
		{
			Transform child = btn.transform.FindChild("Text");
			if (child)
			{
				child.GetComponent<Text>().text = OVSDK._ControllerRay.IsForceShow()
												? "Auto show/hide controller-ray"
												: "Always show controller-ray";
			}

			btn.GetComponent<Image>().color = OVSDK._ControllerRay.IsVisible() ? Color.white : Color.red;
		}		
	}
}
