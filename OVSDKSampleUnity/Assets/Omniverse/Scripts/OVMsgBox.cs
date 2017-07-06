using UnityEngine;
using System.Collections;
using UnityEngine.UI;

public enum OVMsgBoxTimeoutAct
{
    Close,
    QuitGame,
}

public class OVMsgBox : MonoBehaviour
{
    static int _nCounter = 0;
	static int _nVisibleCounter = 0;
    public static int GetCount()
    {
        return _nCounter;
    }

	public static int GetVisibleCount()
	{
		return _nVisibleCounter;
	}

    string _sUsage;
    public string usage
    {
        get  {
            return _sUsage;
        }
    }

    public delegate bool EventDelegate(OVMsgBox hmb, string sEvent, GameObject go);
    public EventDelegate _onEvent; 

    Transform _parent;

    Text _textTime;
    protected float _nTime = 0;
    OVMsgBoxTimeoutAct _nTimeoutAct;
    bool _bStarted = false;

	// Use this for initialization
	protected virtual void Start ()
    {
        _bStarted = true;

        ++_nCounter;        
        if (null == _parent)
        {
            _parent = transform.parent.transform;
            _textTime = SetText("BG/Time", "");
        }

        string[] id = { "BG/Yes", "BG/No", "BG/OK" };
        for (int i = 0; i < 3; ++i)
        {
            Transform btn = transform.Find(id[i]);
            if (btn) {
                OVButton obtn = btn.gameObject.GetComponent<OVButton>();
                obtn.m_OnClick.AddListener(delegate () { this.OnClick(btn.gameObject); });
               // OVUIEventListener.Get(btn.gameObject).onClick = this.OnClick;
            }
        }

		Canvas canvas = GetComponent<Canvas>();
		if (canvas)	{
			canvas.sortingOrder = _nVisibleCounter++;
		}		
	}

    protected virtual void OnDestroy()
    {
        if (_onEvent != null) {
            _onEvent(this, "destroy", gameObject);
        }

        if (_bStarted)
        {
            --_nCounter;
            if (_nCounter == 0) {
				_nVisibleCounter = 0;
			}
        }        
    }

    // Update is called once per frame
    protected virtual void Update ()
    {   
        if (_nTime > 0)
        {
            _nTime -= Time.deltaTime;
            if (_nTime <= 0)
            {
				if (_onEvent != null && _onEvent(this, "timeout", gameObject))
				{

				}
				else
				{
					if (_nTimeoutAct == OVMsgBoxTimeoutAct.QuitGame) {
						Application.Quit();
					}

					Close();
				}                
            }
            else if (_textTime) {
                _textTime.text = ((int)(0.5f+_nTime)).ToString();
            }
        }
    }

	private void LateUpdate()
	{
		if (OVSDK._ControllerRay._OVSDKCamera)
		{
            _parent.position = OVSDK._ControllerRay._OVSDKCamera.transform.position;
            _parent.rotation = OVSDK._ControllerRay._OVSDKCamera.transform.rotation;
		}
	}

	public void Close()
    {
        GameObject.Destroy(transform.parent.transform.gameObject);
    }

    public void Reset(string sUsage = "", string sTitle = "", string sContent = "", string sYes = "", string sNo = "", int nTimeout = 0, OVMsgBoxTimeoutAct nTimeoutAct = OVMsgBoxTimeoutAct.Close)
    {
        if (null == _parent)
        {
            _parent = transform.parent.transform;
            _textTime = SetText("BG/Time", "");
        }

        if (sUsage != null) {
            _sUsage = sUsage;
        }        

        int btn_count = 0;
        if (sYes.Length != 0) {
            ++btn_count;
        }
        if (sNo.Length != 0) {
            ++btn_count;
        }

        string[] id = { "BG/Yes", "BG/No", "BG/OK" };
        int[] t = { 2, 2, 1 };
        string[] cap = { sYes, sNo, sYes };
        for (int i = 0; i < 3; ++i)
        {
            Transform btn = transform.Find(id[i]);
            if (btn)
            {
                if (btn_count != t[i]) {
                    //GameObject.DestroyImmediate(btn.gameObject);
                    btn.gameObject.SetActive(false);
                }
                else {
                    btn.gameObject.SetActive(true);
                   // OVUIEventListener.Get(btn.gameObject);
                    transform.Find(id[i] + "/Text").GetComponent<Text>().text = cap[i];
                }
            }
        }

        _nTime = nTimeout;
        _nTimeoutAct = nTimeoutAct;

        SetText("BG/Title", sTitle);
        SetText("BG/Text", sContent);
        if (nTimeout > 0) {
            _textTime = SetText("BG/Time", nTimeout.ToString());
        }
        else {
            _textTime = SetText("BG/Time", "");
        }
    }

    Text SetText(string name, string tex)
    {
        Transform t = transform.Find(name);
        if (t)
        {
            Text tx = t.GetComponent<Text>();
            if (tx)
            {
                tx.text = tex;
                return tx;
            }
        }
        return null;
    }

    void OnClick(GameObject sender)
    {
        if (_onEvent != null
            && _onEvent(this, "click", sender))
        {
            return;
        }

        Close();
    }


}
