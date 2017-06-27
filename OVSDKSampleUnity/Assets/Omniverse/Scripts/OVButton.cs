using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Reflection.Emit;
using System;
using UnityEngine.Events;
using UnityEngine.Serialization;

public enum OVButtonState
{
    Normal,
    Hover,
    Press,
    Disable,
    Pressed,
}

public class OVButton : MonoBehaviour
{
    [SerializeField]
    public Sprite _StyleNormal;
    [SerializeField]
    public Sprite _StyleHover;
    [SerializeField]
    public Sprite _StylePress;    
    [SerializeField]
    public Sprite _StyleDisable;
    [SerializeField]
    public Sprite _StylePressed;
    [SerializeField]
    public GameObject _CheckGameObject;
    [SerializeField]
    public bool _Checknormal;
    [SerializeField]
    public string _Buttontext;
    //private GameObject GameControl;
    Collider _buttonCollider;
    public bool _isCheckBox;

    public bool _isChecked { get; set; }

    [FormerlySerializedAs("onClick"), SerializeField]
    public Button.ButtonClickedEvent m_OnClick = new Button.ButtonClickedEvent();

    void OnEnable()
    {
        if(!_isCheckBox)
        {
            SetButtonState(OVButtonState.Normal);
        }else
        {
            if(_Checknormal)
            {
                IsChecked(this.gameObject);
            }
        }

        OVUIEventListener listener = OVUIEventListener.Get(this.gameObject);
        listener.onClick += this.OnClick;
        listener.onUp += this.onUp;
        listener.onDown += this.onDown;
        listener.onExit += this.onExit;
        listener.onEnter += this.onEnter;
        if(_Buttontext != "" && OVSDK.HasInitialized())
        {
            Text t;
            if(null!=(t= this.GetComponentInChildren<Text>()))
            {
                t.text = OVSDK.GetString(_Buttontext);
            }
        }
    }
    void OnDisable()
    {
        OVUIEventListener listener = OVUIEventListener.Get(this.gameObject);
        listener.onClick -= this.OnClick;
        listener.onUp -= this.onUp;
        listener.onDown -= this.onDown;
        listener.onExit -= this.onExit;
        listener.onEnter -= this.onEnter;
    }

    public void SetButtonState(OVButtonState state)
    {
        switch (state)
        {
            case OVButtonState.Hover:
                if(_StyleHover!=null)
                {
                transform.GetComponent<Image>().sprite = _StyleHover;
                transform.localScale = new Vector3(1.02f, 1.02f, 1.02f);
                }
                break;
            case OVButtonState.Press:
                if (_StylePress != null)
                {
                    transform.GetComponent<Image>().sprite = _StylePress;
                    transform.localScale = new Vector3(0.9f, 0.9f, 0.9f);
                }
                break;
            case OVButtonState.Disable:
                if (_StyleDisable != null)
                {
                    transform.GetComponent<Image>().sprite = _StyleDisable;
                    transform.localScale = new Vector3(0.9f, 0.9f, 0.9f);
                }
                break;
            case OVButtonState.Pressed:
                if (_StylePressed != null)
                {
                    transform.GetComponent<Image>().sprite = _StylePressed;
                    transform.localScale = new Vector3(1.0f, 1.0f, 1.0f);
                }
                break;
            default:
                if (_StyleNormal != null)
                { 
                transform.GetComponent<Image>().sprite = _StyleNormal;
                transform.localScale = new Vector3(1.00f, 1.00f, 1.00f);
                }
                break;
        }
    }

    public virtual void OnClick(GameObject sender)
    {
      //  Debug.Log("OnClick" + sender.name);
        this.SetButtonState(OVButtonState.Normal);
        this.m_OnClick.Invoke();
        if(_isCheckBox)
        {
            IsChecked(sender);
        }    
    }
    public virtual void onUp(GameObject sender)
    {
     //   Debug.Log("onUp" + sender.name);
        this.SetButtonState(OVButtonState.Normal);
    }
    public virtual void onDown(GameObject sender)
    {
      //  Debug.Log("onDown" + sender.name);
        this.SetButtonState(OVButtonState.Press);
    }
    public virtual void onExit(GameObject sender)
    {
      //  Debug.Log("onExit"+sender.name);
        if (_isCheckBox)
        {
            if (_isChecked)
            {
                this.SetButtonState(OVButtonState.Pressed);
            }
            else
            {
                this.SetButtonState(OVButtonState.Normal);
            }
        }
        else
        {
            this.SetButtonState(OVButtonState.Normal);

        }
      //  transform.localPosition -= new Vector3(0,0,-20);
    }
    public virtual void onEnter(GameObject sender)
    {
     //   Debug.Log("onEnter" + sender.name);
        this.SetButtonState(OVButtonState.Hover);
      //  transform.localPosition += new Vector3(0, 0, -20);
    }
    public virtual void IsChecked(GameObject sender)
    {
        transform.parent.BroadcastMessage("CloseCheck",sender);//让同级其它的关闭
        if (!_isChecked)
        {
            _isChecked = true;
            this.SetButtonState(OVButtonState.Pressed);
            if (_CheckGameObject!=null)
            {
                _CheckGameObject.SetActive(true);
            }

        }

    }

    public virtual void CloseCheck(GameObject sender)
    {
        if (gameObject!= sender && this._isCheckBox)
        {
            if (_CheckGameObject != null)
            {
                _CheckGameObject.SetActive(false);
            }
            _isChecked = false;
            this.SetButtonState(OVButtonState.Normal);
           // this.m_OnClick.Invoke();
        }

    }
}
