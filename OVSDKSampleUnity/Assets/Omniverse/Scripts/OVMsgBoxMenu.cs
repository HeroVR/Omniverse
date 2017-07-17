using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System;

[System.Serializable]
struct OVMenuItemDef
{
	public int x, y, w, h, rot, msgid, close;
	public string type, style, color, name;
	public string cmd; //format: txt_arg;txt_format;cmd_type;close_after_click;
}

[System.Serializable]
struct OVMenuDef
{
	public int x, y, w, h;
	public string name;
	public OVMenuItemDef[] items;
}

class OVMenuItem
{
	public GameObject go;
	public bool dynaTxt;
	public Text txtCompo;
	public string name, cmd;
	public int msgId;
	public bool directClose;
	public void OnSliderValueChange(float val)
	{
		if (name == "OmniCoupleRate")	{
			OVSDK.SetOmniCoupleRate(val);
		}
	}
	public void OnToggleStateChange(bool state)
	{
		if (name == "OmniCoupleMode") {
			OVSDK.SetOmniCoupleMode(state);
		}
	}
}

public class OVMsgBoxMenu : OVMsgBox
{
	public string _JsonFilePrefix = "systemmenu";
	public string _MenuName;

	Transform _BG;	

	List<OVMenuItem> _Items = new List<OVMenuItem>();
	float _UpdateTimer = 0;
	UInt32 _PreOmniCoupleRate = 0;

	public static List<OVMsgBoxMenu> _AllMsgBoxJson = new List<OVMsgBoxMenu>();

	private void refreshButtonsState()
	{
		int count = _AllMsgBoxJson.Count;
		for (int i = 0; i < count; ++i)
		{
			bool enable = i == (count - 1);


            Collider col = _AllMsgBoxJson[i].GetComponent<Collider>();
            if(col)
            {
                col.enabled = enable;
            }
		}
	}

	protected override void Start ()
	{
        base.Start();

		_nTime = 0;

		_BG = transform.Find("BG");
		if (null == _BG) {
			_BG = transform;
		}

		string path = OVSDK.GetUserInfo().sConsolePath + "\\" + _JsonFilePrefix + "_unity.json";
		string json = ReadFile(path);
		OVMenuDef def;
		def.x = def.y = def.w = def.h = 0;
		def.items = null;
		try
		{
			def = JsonUtility.FromJson<OVMenuDef>(json);
			_MenuName = def.name;
			def.items = OVJsonHelper.FromJson<OVMenuItemDef>(json);
		}
		catch(Exception e) {
			Debug.Log("Parse json menu-items failed: " + e.Message);
		}
		if (null == _MenuName) {
			_MenuName = "";
		}
		if (def.w > 0 && def.h > 0) {
			ApplyTransRect(_BG as RectTransform, def.x, def.y, def.w, def.h, 0, false);
		}
		 
		for (int i = 0; def.items != null && i < def.items.Length; ++i)
		{
			OVMenuItem item = CreateItem(def.items[i]);
			if (item != null)
			{
				if (def.items[i].type == "Button")	{
					item.directClose = (0 != def.items[i].close);
					OVButton btn = item.go.gameObject.GetComponent<OVButton>();
                    btn.m_OnClick.AddListener(delegate(){ this.OnClick(item.go.gameObject); });
                   // OVUIEventListener.Get(item.go.gameObject).onClick += this.OnClick;

                }
				else if (def.items[i].type == "Slider")
				{
					Slider slider = item.go.GetComponent<Slider>();
					if (slider != null)
					{
						if (item.name == "OmniCoupleRate")
						{
							_PreOmniCoupleRate = OVSDK.GetUserOmniCoupleRate();
							slider.value = OVSDK.GetOmniCoupleRate();
						}
						slider.onValueChanged.AddListener(item.OnSliderValueChange);
					}
				}
				else if (def.items[i].type == "Toggle")
				{
					Toggle toggle = item.go.GetComponent<Toggle>();
					if (toggle != null)
					{
						if (item.name == "OmniCoupleMode")
						{
							_PreOmniCoupleRate = OVSDK.GetUserOmniCoupleRate();
							toggle.isOn = OVSDK.GetOmniCoupleRate() < 0.5f ? false : true;
						}
						toggle.onValueChanged.AddListener(item.OnToggleStateChange);
					}
				}
			}
		}

		_AllMsgBoxJson.Add(this);
		refreshButtonsState();
	}

    // Update is called once per frame
    protected override void Update ()
	{
        base.Update();

		_UpdateTimer += Time.deltaTime;
		if (_UpdateTimer > 0.99f)
		{
			_UpdateTimer = 0;

			foreach (OVMenuItem item in _Items)
			{
				if (item.dynaTxt && item.txtCompo) {
					UpdateText(item.txtCompo, item.cmd);
				}				
			}
		}
	}

	protected override void OnDestroy()
	{
		// save user's coupling percentage;
		if (_PreOmniCoupleRate != OVSDK.GetUserOmniCoupleRate()) { 
			OVSDK.sendMsg(14, "");
		}
		OVSDK.sendMsg(15, _MenuName);
		_AllMsgBoxJson.Remove(this);
		refreshButtonsState();

		base.OnDestroy();
	}

	bool UpdateText(Text compo, string cmd)
	{
		bool need_update = false;
		char[] sep = { '|' };
		string[] cmds = cmd.Split(sep);
		if (cmds.Length > 1)
		{
			string content = "";
			OVSDK.UserInfo ui = OVSDK.GetUserInfo();
			OVSDK.DeviceInfo di = OVSDK.GetDeviceInfo();
			int type = cmds[0].Length == 0 ? 0 : int.Parse(cmds[0]);
			if (type == 0)	{
				compo.text = cmds[1];
			}
			else
			{
				switch (type)
				{
					case 1:
						content = string.Format("{0}", ui.nGameDurationLeft / 60);
						need_update = true;
						break;
					case 2:
						content = string.Format("{0}", ui.nGamePrepareLeft);
						need_update = true;
						break;
					case 3:
						content = string.Format("{0}", ui.nUserId);
						break;
					case 4:
						content = ui.sUserName;
						break;
					case 5:
						content = string.Format("{0}", di.nShop);
						break;
					case 6:
						content = di.sShopName;
						break;
                    case 7:
                        content = string.Format("{0}", ui.nGameDuration / 60);
                        need_update = true;
                        break;
				}

				compo.text = string.Format(cmds[1], content);
			}					
		}
		return need_update;
	}
	void OnClick(GameObject sender)
	{
		if (_onEvent != null
			&& _onEvent(this, "click", sender))
		{
			return;
		}
		
		int index = 0;
		if (tryGetMenuItem(sender, ref index))
		{
			if (_Items[index].directClose) {
				Close();
			}
			else {
				string msg = "dlg=" + _MenuName + ";" + "wgt=" + _Items[index].name;
				OVSDK.sendMsg(_Items[index].msgId, msg);
			}
		}
	}

	string ReadFile(string path)
	{
		try
		{
			FileStream file = new System.IO.FileStream(path, FileMode.Open);
			byte[] content = new byte[file.Length];
			file.Seek(0, SeekOrigin.Begin);
			file.Read(content, 0, (int)file.Length);
			file.Close();

			char[] chars = new char[content.Length];
			Encoding.Default.GetDecoder().GetChars(content, 0, content.Length, chars, 0);
			return new string(chars);
		}
		catch (IOException)
		{
		}

		return string.Empty;
	}

	static Color StringToColor(string str)
	{
		byte r = 255, g = 255, b = 255, a = 255;
		if (null != str || str.Length > 0)
		{
			int t = int.Parse(str, System.Globalization.NumberStyles.HexNumber);
			a = (byte)(t & 255);
			b = (byte)((t >> 8) & 255);
			g = (byte)((t >> 16) & 255);
			r = (byte)((t >> 24) & 255);
			if (str.Length <= 6)
			{
				r = g;
				g = b;
				b = a;
				a = 255;
			}
		}

		Color32 clr = new Color32(r, g, b, a);
		return clr;
	}

	void ApplyTransRect(RectTransform trans, int x, int y, int w, int h, int rot, bool zeroz = true)
	{
		trans.offsetMin = new Vector2(w, h) * -0.5f;
		trans.offsetMax = new Vector2(w, h) * 0.5f;
		trans.localPosition = new Vector3(x, y, zeroz ? 0 : trans.localPosition.z);
		trans.localRotation = Quaternion.Euler(0, 0, rot);
		trans.localScale = new Vector3(1, 1, 1);
	}

	OVMenuItem CreateItem(OVMenuItemDef def)
	{
		OVMenuItem item = null;
		string prefab_file = "OV" + def.type;
		GameObject prefab = Resources.Load<GameObject>(prefab_file);
		if (prefab)
		{
			GameObject go = GameObject.Instantiate(prefab, _BG) as GameObject;

			item = new OVMenuItem();
			item.go = go;
			item.cmd = def.cmd;
			item.name = def.name;
			item.msgId = def.msgid;

			// Rect
			ApplyTransRect(go.transform as RectTransform, def.x, def.y, def.w, def.h, def.rot);

			// Color
			if (def.color != null && def.color.Length != 0)
			{
				Image img = go.GetComponent<Image>();
				if (img)
				{
					img.color = StringToColor(def.color);
				}
			}

			// Text
			Text txt_compo = go.GetComponent<Text>();
			if (null == txt_compo)
			{
				Transform txt_trans = go.transform.FindChild("Text");
				if (txt_trans)
				{
					txt_compo = txt_trans.GetComponent<Text>();
				}
			}

			item.txtCompo = txt_compo;
			item.dynaTxt = (txt_compo && UpdateText(txt_compo, def.cmd));

			_Items.Add(item);
		}
		return item;
	}

	public bool tryClose(string name)
	{
		if ((name.Length == 0) || (_MenuName == name)) {
			Close();
		}
		return ((name.Length != 0) && (_MenuName == name));
	}

	public bool UpdateItemCmd(string name, string cmd)
	{
		for (int i = 0; i < _Items.Count; ++i)
		{
			OVMenuItem item = _Items[i];
			if (item.name == name)
			{
				item.cmd = cmd;
				item.dynaTxt = (item.txtCompo && UpdateText(item.txtCompo, cmd));
				return true;
			}
		}

		return false;
	}

	private bool tryGetMenuItem(GameObject obj, ref int index)
	{
		for (int i = 0; i < _Items.Count; ++i)
		{
			if (obj == _Items[i].go)
			{
				index = i;
				return true;
			}
		}
		return false;
	}
}
