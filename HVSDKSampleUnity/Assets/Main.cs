using UnityEngine;
using System.Collections;
using UnityEngine.UI;
using Virtuix;

public class Main : MonoBehaviour {

    HVMsgBox _MsgboxIniting;

	void Start ()
	{
        HVSDK.Init(1000, "01ec17dac7140c0fbe936ee128310000", "omni=1", null);

		HVSDK.LoadString("SampleString");
        _MsgboxIniting = HVSDK.MsgBox("", "", HVSDK.GetString("WaitInit"));
	}

    void Update ()
    {
        //HVSDK.Drive();
        if (HVSDK._bJustInitDone) {
            OnHVSDKInit();
        }
	}

    void OnHVSDKInit()
    {
        Debug.Log("OnHVSDKInit: " + HVSDK._sInitResult);

        HVSDK.DeviceInfo dev = HVSDK.GetDeviceInfoImpl();
        Debug.Log("shop=" + dev.sShopName + ";shopid=" + dev.nShop + ";shop_category=" + dev.sCategory + ";device_sn=" + dev.sNo);

        HVSDK.UserInfo user = HVSDK.GetUserInfoImpl();
        Debug.Log(string.Format("{0}name={1};id={2};phone={3};qrcode={4};money={5}", HVSDK.HvIsGuest() ? "[GUEST]" : "", user.sName, user.nId, user.sPhone, user.sQrcode, user.nWallet));


//#if UNITY_EDITOR
        HVSDK.CheckOmniViveAvailable();
//#endif

		// use Omni decoupled-mode;
		OmniController omni_controller = GameObject.FindObjectOfType<OmniController>();
        if (omni_controller)
        {
            omni_controller.AlignOmni();
            omni_controller.couplingPercentage = omni_controller.developerMode ? 1 : 0;
            omni_controller.omniOffset = HVSDK.HvGetOmniYawOffset();
        }

        // buttons in sample-scene
        HVSDK._ControllerRay.CheckShow(1);

		GameObject go = GameObject.Find("Canvas/HVButton_SampleMsgBox") as GameObject;
        if (go) {
            HVUIEventListener.Get(go).onClick = OnClickSampleMsgBox;
        }

        go = GameObject.Find("Canvas/HVButton_SampleBuy") as GameObject;
        if (go)  {
            HVUIEventListener.Get(go).onClick = OnClickSampleBuy;
        }

        go = GameObject.Find("Canvas/HVButton_SwitchController") as GameObject;
        if (go)
        {
            HVUIEventListener.Get(go).onClick = OnClickSampleSwitchControllerRay;
			UpdateButtonSwitchControllerRay(go);
		}

        if (_MsgboxIniting != null)
        {
            _MsgboxIniting.Close();
            _MsgboxIniting = null;
        }

        if (HVSDK.GetUserInfoImpl().nGamePrepareLeft > 0)
        {
            //HVMsgBox hmb = HVSDK.MsgBox("prepare_game", HVSDK.GetString("PrepareStage"), HVSDK.GetString("PrepareConfirm"), HVSDK.GetString("Close"), "", HVSDK.GetUserInfoImpl().nGamePrepareLeft);
			//hmb._onEvent = OnPrepareMsgBoxEvent;
        }
        else
        {
            //HVSDK.MsgBox("", HVSDK.GetString("InitResult"), HVSDK._sInitResult, HVSDK.GetString("Close"));
        }        
    }
	bool OnPrepareMsgBoxEvent(HVMsgBox hmb, string sEvent, GameObject sender)
	{
		if (sEvent == "timeout")
		{
			hmb.Reset("prepare_game_done", null, HVSDK.GetString("PrepareEnd"), HVSDK.GetString("OK"));
			return true;
		}

		if (sEvent == "click" && sender.name == "OK" && hmb.usage == "prepare_game")
		{
			HVSDK.ConfirmPlayGame();
		}
		return false;
	}

	void OnClickSampleMsgBox(GameObject sender)
    {
        HVMsgBox hmb = HVSDK.MsgBox("", "HVSDK", "Hello World!!", HVSDK.GetString("OK"));
        hmb._onEvent = OnClickSampleMsgBoxOK;

		// test MsgBoxJson;
		//if (HVMsgBoxMenu._AllMsgBoxJson.Count == 0)	{
		//	HVSDK.MsgBoxJson("test");
		//}
		//else {
		//	HVSDK.onEventMsgBoxJsonText("test ExtendButton 1|<size=14><color=#FFFFFF>Remaining Game time : </color><color=#00FFFF>{0} Min.</color></size>");
		//}		
	}

    bool OnClickSampleMsgBoxOK(HVMsgBox hmb, string sEvent, GameObject sender)
    {
        if (sEvent == "click")
        {
            Debug.Log("sample msgbox ok clicked.");
            hmb.Close();
            return true;
        }
        return false;
    }

    void OnBuyResult(string sItem, string sOutTradeNo, string sInTradeNo, string sErr)
    {
        //if (sInTradeNo != "")
        {
            Object prefab = Resources.Load("HVSDK_SampleCube");
            if (prefab)
            {
                string[] strs = sOutTradeNo.Split(',');
                Vector3 pos = new Vector3(float.Parse(strs[0]), float.Parse(strs[1]), float.Parse(strs[2]));

                GameObject go = GameObject.Instantiate(prefab, pos, Quaternion.identity) as GameObject;
                Rigidbody rb = go.GetComponent<Rigidbody>();
                if (rb)
                {
                    float angle = Random.Range(-0.7f, 0.7f);
                    Vector3 v = new Vector3(Mathf.Sin(angle), 1, Mathf.Cos(angle)) * 5;
                    rb.velocity = v;
                }
            }            
        }
    }

    void OnClickSampleBuy(GameObject sender)
    {
        // Game customized trader-no
        string out_trade_no = sender.transform.position.x + "," + sender.transform.position.y + "," + sender.transform.position.z + "," + Time.time;

        HVSDK.Buy("Box", 0.88, out_trade_no, OnBuyResult);
    }

    void OnClickSampleSwitchControllerRay(GameObject sender)
    {
		HVSDK._ControllerRay.CheckShow(HVSDK._ControllerRay.IsForceShow() ? 0 : 1);

		UpdateButtonSwitchControllerRay(sender);
    }

	void UpdateButtonSwitchControllerRay(GameObject btn)
	{
		if (btn)
		{
			Transform child = btn.transform.FindChild("Text");
			if (child)
			{
				child.GetComponent<Text>().text = HVSDK._ControllerRay.IsForceShow()
												? "Auto show/hide controller-ray"
												: "Always show controller-ray";
			}

			btn.GetComponent<Image>().color = HVSDK._ControllerRay.IsVisible() ? Color.white : Color.red;
		}		
	}
}
