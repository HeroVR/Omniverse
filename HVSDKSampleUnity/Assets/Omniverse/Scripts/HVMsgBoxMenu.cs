using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System;

[System.Serializable]
struct HVMenuItemDef
{
	public int x, y, w, h, rot;
	public string type, style, color;
	public string cmd; //format: txt_arg;txt_format;cmd_type;close_after_click;
}

[System.Serializable]
struct HVMenuDef
{
	public int x, y, w, h;
	public HVMenuItemDef[] items;
}

struct HVMenuItem
{
	public Text txtCompo;
	public string cmd;
}

public class HVMsgBoxMenu : HVMsgBox
{
	Transform _BG;

	Dictionary<GameObject, HVMenuItem> _Items = new Dictionary<GameObject, HVMenuItem>();
	float _UpdateTimer = 0;

	protected override void Start ()
	{
        base.Start();

		_nTime = 0;

		_BG = transform.Find("BG");
		if (null == _BG) {
			_BG = transform;
		}

		string json = ReadFile(HVSDK.GetUserInfoImpl().sConsolePath + "\\systemmenu_unity.json");
		HVMenuDef def;
		def.x = def.y = def.w = def.h = 0;
		def.items = null;
		try	{
			def = JsonUtility.FromJson<HVMenuDef>(json);
			def.items = HVJsonHelper.FromJson<HVMenuItemDef>(json);
		}
		catch(Exception e) {
			Debug.Log("Parse json menu-items failed: " + e.Message);
		}

		if (def.w > 0 && def.h > 0) {
			ApplyTransRect(_BG as RectTransform, def.x, def.y, def.w, def.h, 0, false);
		}
		 
		for (int i = 0; def.items != null && i < def.items.Length; ++i)
		{
			GameObject go = CreateItem(def.items[i]);
			if (go && def.items[i].type == "Button") {
				HVUIEventListener.Get(go.gameObject).onClick = this.OnClick;
			}					
		}
    }

    // Update is called once per frame
    protected override void Update ()
	{
        base.Update();

		_UpdateTimer += Time.deltaTime;
		if (_UpdateTimer > 0.99f)
		{
			_UpdateTimer = 0;

			foreach (KeyValuePair<GameObject, HVMenuItem> item in _Items)
			{
				if (item.Value.txtCompo) {
					UpdateText(item.Value.txtCompo, item.Value.cmd);
				}				
			}
		}
	}

	bool UpdateText(Text compo, string cmd)
	{
		bool need_update = false;
		char[] sep = { '|' };
		string[] cmds = cmd.Split(sep);
		if (cmds.Length > 1)
		{
			string content = "";
			HVSDK.UserInfo ui = HVSDK.GetUserInfoImpl();
			HVSDK.DeviceInfo di = HVSDK.GetDeviceInfoImpl();
			int type = cmds[0].Length == 0 ? 0 : int.Parse(cmds[0]);
			if (type == 0)	{
				compo.text = cmds[1];
			}
			else
			{
				switch (type)
				{
					case 1:
						content = string.Format("{0}", ui.nGameDurationLeft);
						need_update = true;
						break;
					case 2:
						content = string.Format("{0}", ui.nGamePrepareLeft);
						need_update = true;
						break;
					case 3:
						content = string.Format("{0}", ui.nId);
						break;
					case 4:
						content = ui.sName;
						break;
					case 5:
						content = string.Format("{0}", di.nShop);
						break;
					case 6:
						content = di.sShopName;
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

		bool close = true;

		char[] sep = { ';' };
		string[] cmds = sender.tag.Split(sep);
		if (cmds.Length > 2)
		{
			if (cmds[2].Length != 0) {
				HVSDK.sendMsg(int.Parse(cmds[2]), "");
			}

			if (cmds.Length > 3 && cmds[3].Length != 0)	{
				close = int.Parse(cmds[3]) != 0;
			}
		}

		if (close)	{
			Close();
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

	GameObject CreateItem(HVMenuItemDef def)
	{
		GameObject go = null;
		GameObject prefab = Resources.Load<GameObject>("HV" + def.type);
		if (prefab)
		{
			go = GameObject.Instantiate(prefab, _BG) as GameObject;
			HVMenuItem item = new HVMenuItem();
			item.cmd = def.cmd;

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

			if (txt_compo && UpdateText(txt_compo, def.cmd))
			{
				item.txtCompo = txt_compo;
			}

			_Items.Add(go, item);
		}
		return go;
	}
}
