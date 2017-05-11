using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Reflection.Emit;
using System;
using UnityEngine.Events;
using UnityEngine.Serialization;

public enum HVButtonState
{
    Normal,
    Hover,
    Press,
    Disable,
    Pressed,
}

public class HVButton : MonoBehaviour
{
    [SerializeField]
    public Sprite _StyleNormal;
    [SerializeField]
    private Sprite _StyleHover;
    [SerializeField]
    private Sprite _StylePress;    
    [SerializeField]
    private Sprite _StyleDisable;
    [SerializeField]
    private Sprite _StylePressed;
    //private GameObject GameControl;
    Collider _buttonCollider;
    public bool _isCheckBox;

    public bool _isChecked { get; set; }

    [FormerlySerializedAs("onClick"), SerializeField]
    public Button.ButtonClickedEvent m_OnClick = new Button.ButtonClickedEvent();

    void Start()
    {
        if(!_isCheckBox)
        {
            SetButtonState(HVButtonState.Normal);
        }

        HVUIEventListener listener = HVUIEventListener.Get(this.gameObject);
        listener.onClick += this.OnClick;
        listener.onUp += this.onUp;
        listener.onDown += this.onDown;
        listener.onExit += this.onExit;
        listener.onEnter += this.onEnter;
    }

    public void SetButtonState(HVButtonState state)
    {
        switch (state)
        {
            case HVButtonState.Hover:
                if(_StyleHover!=null)
                {
                transform.GetComponent<Image>().sprite = _StyleHover;
                transform.localScale = new Vector3(1.02f, 1.02f, 1.02f);
                }
                break;
            case HVButtonState.Press:
                if (_StylePress != null)
                {
                    transform.GetComponent<Image>().sprite = _StylePress;
                    transform.localScale = new Vector3(0.9f, 0.9f, 0.9f);
                }
                break;
            case HVButtonState.Disable:
                if (_StyleDisable != null)
                {
                    transform.GetComponent<Image>().sprite = _StyleDisable;
                    transform.localScale = new Vector3(0.9f, 0.9f, 0.9f);
                }
                break;
            case HVButtonState.Pressed:
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
        this.SetButtonState(HVButtonState.Normal);
        this.m_OnClick.Invoke();
        if(_isCheckBox)
        {
            IsChecked(sender);
        }    
    }
    public virtual void onUp(GameObject sender)
    {
     //   Debug.Log("onUp" + sender.name);
        this.SetButtonState(HVButtonState.Normal);
    }
    public virtual void onDown(GameObject sender)
    {
      //  Debug.Log("onDown" + sender.name);
        this.SetButtonState(HVButtonState.Press);
    }
    public virtual void onExit(GameObject sender)
    {
      //  Debug.Log("onExit"+sender.name);
        if (_isCheckBox)
        {
            if (_isChecked)
            {
                this.SetButtonState(HVButtonState.Pressed);
            }
            else
            {
                this.SetButtonState(HVButtonState.Normal);
            }

        }
        else
        {
            this.SetButtonState(HVButtonState.Normal);

        }
       // transform.localPosition -= _tra;
    }
    public virtual void onEnter(GameObject sender)
    {
     //   Debug.Log("onEnter" + sender.name);
        this.SetButtonState(HVButtonState.Hover);
      //  transform.localPosition += _tra;
    }
    public virtual void IsChecked(GameObject sender)
    {
        transform.parent.BroadcastMessage("CloseCheck",sender);//让同级其它的关闭
        if (!_isChecked)
        {
            _isChecked = true;
        }

    }

    public virtual void CloseCheck(GameObject sender)
    {
        if(gameObject!= sender)
        {
            _isChecked = false;
            this.SetButtonState(HVButtonState.Normal);
        }

    }
}
