using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using Valve.VR;

public class OVScrollView : MonoBehaviour
{
	public delegate void OnHoverChange(OVScrollView scrollview, bool bHover);
	public OnHoverChange _OnHoverChange;

	int _Index;
    //int _RightIndex;

    public RectTransform _Content;
    //Vector2 _MaxDistance;
    //Vector2 _MinDistance = Vector2.zero;

    public enum axial
    {
        Horizontal,
        Vertical,
    };

    public axial _axial;

    void OnEnable()
    {
        this.ResetContent();
    }
    void Start()
    {
        OVUIEventListener.Get(gameObject).onHover = onHover;
        OVUIEventListener.Get(gameObject).onExit = onExit;

        if (_Content == null)
        {
            Debug.LogError("_Content is not reference");
            _Content = FindObjectOfType<GridLayoutGroup>().GetComponent<RectTransform>();
        }
    }

    float _VelocityY = 0.0F;
    //float _target = 0;
    bool _bElasticity_min;
    bool _bElasticity_max;
    Vector2 _AlignAxes;
    void Update()
    {
        RectTransform listrecttra = GetComponent<RectTransform>();
        if (_Content.rect.size.x > listrecttra.rect.size.x || _Content.rect.size.y > listrecttra.rect.size.y)
        {
            if (!_bIsmove)
            {
                if (_LastSpeed != 0f)
                {
                    _LastSpeed = (int)Mathf.SmoothDamp(_LastSpeed, 0, ref _VelocityY, 2f);
                    if (_axial == axial.Horizontal)
                    {
                        _Content.anchoredPosition += new Vector2(_LastSpeed, 0);
                    }
                    else
                    {
                        _Content.anchoredPosition += new Vector2(0, _LastSpeed);
                    }
                }
                AdjustContent();
            }
            else
            {
                _bElasticity_min = false;
                _bElasticity_max = false;
            }
        }
    }

    public void ResetContent()
    {
        _Content.anchoredPosition = new Vector2(0, 0);
    }

    void AdjustContent()
    {
        if (_axial == axial.Horizontal)
        {
            float b = _Content.anchoredPosition.x;
            float listsizex = this.GetComponent<RectTransform>().sizeDelta.x;

            if ((b > 0))
            {
                _bElasticity_min = true;
                _LastSpeed = 0f;
            }

            if (b < ((_Content.sizeDelta.x - listsizex) * -1))
            {
                _bElasticity_max = true;
                _LastSpeed = 0f;

            }

            if (_bElasticity_min)
            {
                Elasticity(new Vector2(0, _Content.anchoredPosition.y));
            }
            if (_bElasticity_max)
            {
                Elasticity(new Vector2((_Content.sizeDelta.x - listsizex) * -1, _Content.anchoredPosition.y));
            }
        }
        if (_axial == axial.Vertical)
        {
            float b = _Content.anchoredPosition.y;
            float listsizey = this.GetComponent<RectTransform>().sizeDelta.y;

            if ((b < 0))
            {
                _bElasticity_min = true;
                _LastSpeed = 0f;
            }

            if (b > ((_Content.sizeDelta.y - listsizey)))
            {
                _bElasticity_max = true;
                _LastSpeed = 0f;

            }

            if (_bElasticity_min)
            {
                Elasticity(new Vector2(_Content.anchoredPosition.x, 0));
            }
            if (_bElasticity_max)
            {
                Elasticity(new Vector2(_Content.anchoredPosition.x, (_Content.sizeDelta.y - listsizey)));
            }
        }
    }


    Vector2 _Velocity;
    void Elasticity(Vector2 originPos)
    {
        float k = 0.05f;
        var f = (originPos - _Content.anchoredPosition) * k;
        _Velocity += f;
        _Velocity = Vector2.Lerp(_Velocity, Vector2.zero, 0.2f);

        _Content.anchoredPosition += _Velocity;
        if (Mathf.Abs(_Velocity.x) < 0.1 && Mathf.Abs(_Velocity.y) < 0.1)
        {
            _Content.anchoredPosition = originPos;
            _bElasticity_min = false;
            _bElasticity_max = false;
        }

    }

    void onExit(GameObject sender)
    {
        //FindObjectOfType<MainEyeUi>()._bHitListView = false;
        _bIsmove = false;

		if (null != _OnHoverChange)	{
			_OnHoverChange(this, false);
		}
    }


    Vector2 _StartPos;
    bool _bIsmove;
    float _Triggerdowntime;
    float _Timer;
    float _Speed;

#if UNITY_EDITOR
    void OnGUI()
    {
        GUI.Label(new Rect(25, 15, 125, 550), "_Speed : " + _Speed.ToString("0.00") +
            "\n_Timer:" + _Timer.ToString("0.00") +
            "\n_LastSpeed:" + _LastSpeed +
            //"\n_target:" + _target +
            "\n_bIsmove:" + _bIsmove +
            "\n_starPos:" + _StartPos);
    }
#endif

    void onHover(GameObject sender, UnityEngine.EventSystems.PointerEventData ped)
    {
     //_MaxDistance = _Content.rect.size;
     RectTransform listrecttra = GetComponent<RectTransform>();
        if (_Content.rect.size.x > listrecttra.rect.size.x || _Content.rect.size.y > listrecttra.rect.size.y)
        {
			//显滑动提示
			//FindObjectOfType<MainEyeUi>()._bHitListView = true;
			if (null != _OnHoverChange)	{
				_OnHoverChange(this, true);
			}

			if (OpenVR.System != null)
            {
                _Index = ped.pointerId;
                if (_Index > 0)
                {
                    if (SteamVR_Controller.Input(_Index).GetTouchDown(SteamVR_Controller.ButtonMask.Touchpad))
                    {
                        _Timer = 0;

                    }
                    if (SteamVR_Controller.Input(_Index).GetPressDown(SteamVR_Controller.ButtonMask.Trigger))
                    {

                        _Timer = 0;
                    }
                    if (SteamVR_Controller.Input(_Index).GetPress(SteamVR_Controller.ButtonMask.Trigger))
                    {
                        _Triggerdowntime += Time.deltaTime;

                        if (_Triggerdowntime > Time.fixedDeltaTime)
                        {
                            if (_Timer <= Time.fixedDeltaTime)
                            {
                                _Timer += Time.fixedDeltaTime;
                            }
                            else
                            {
                                _Timer = 0;
                                _StartPos = ped.position;
                                _bIsmove = true;
                            }

                        }
                    }
                    if (SteamVR_Controller.Input(_Index).GetTouch(SteamVR_Controller.ButtonMask.Touchpad))
                    {
                        _Triggerdowntime += Time.deltaTime;

                        if (_Triggerdowntime > Time.fixedDeltaTime)
                        {
                            if (_Timer <= Time.fixedDeltaTime)
                            {
                                _Timer += Time.fixedDeltaTime;
                            }
                            else
                            {
                                _Timer = 0;
                                _StartPos = SteamVR_Controller.Input(_Index).GetAxis() * 250;
                                _bIsmove = true;
                            }
                        }
                    }

                    if (SteamVR_Controller.Input(_Index).GetPressUp(SteamVR_Controller.ButtonMask.Trigger))
                    {
                        _Triggerdowntime = 0;
                        _bIsmove = false;
                    }

                    if (SteamVR_Controller.Input(_Index).GetTouchUp(SteamVR_Controller.ButtonMask.Touchpad))
                    {
                        _bIsmove = false;
                    }
                }
            }
            else
            {

                if ((Input.GetAxis("Mouse ScrollWheel")) != 0)
                {

                    _bIsmove = true;
                }
                else
                {
                    if (Input.GetMouseButton(0))
                    {
                        _Triggerdowntime += Time.deltaTime;

                        if (_Triggerdowntime > Time.fixedDeltaTime)
                        {
                            if (_Timer <= Time.fixedDeltaTime)
                            {
                                _Timer += Time.fixedDeltaTime;
                            }
                            else
                            {
                                _Timer = 0;

                                _bIsmove = true;
                                _StartPos = ped.position;

                            }
                        }
                    }
                    else
                    {
                        _bIsmove = false;
                        _Triggerdowntime = 0;
                        _StartPos = ped.position;
                    }
                }
            }

            if (_bIsmove)
            {
                Vector2 newpos = new Vector2();

                if (SteamVR.active)
                {
                    if (SteamVR_Controller.Input(_Index).GetTouch(SteamVR_Controller.ButtonMask.Touchpad))
                    {
                        newpos = SteamVR_Controller.Input(_Index).GetAxis() * 250;
                    }
                    if (SteamVR_Controller.Input(_Index).GetPress(SteamVR_Controller.ButtonMask.Trigger))
                    {
                        newpos = ped.position;
                    }
                }
                if (Input.GetMouseButton(0))
                {
                    newpos = ped.position;
                  //  Debug.Log("newpos:" + newpos + "_StartPos:" + _StartPos + "=" + Vector2.Distance(newpos, _StartPos));

                }


                if (_StartPos != newpos)
                {
                    float xMoveDistance = Mathf.Abs(newpos.x - _StartPos.x);
                    float yMoveDistance = Mathf.Abs(newpos.y - _StartPos.y);
                    if (_axial == axial.Horizontal)
                    {
                        if (xMoveDistance > yMoveDistance)
                        {
                            _Speed = (int)((newpos.x - _StartPos.x));
                            // Debug.Log("newpos.x:" + newpos.x + "_StartPos.x:" + _StartPos.x + "_Speed========" + (newpos.x- _StartPos.x));
                        }
                    }
                    if (_axial == axial.Vertical)
                    {
                        if (yMoveDistance > xMoveDistance)
                        {
                            _Speed = (int)((newpos.y - _StartPos.y));
                        }
                    }
                    Move();
                }

                float a;//鼠标滚轮
                if ((a = Input.GetAxis("Mouse ScrollWheel")) != 0)
                {
                    _Speed = (int)(a * 250);

                }
                Move();

            }
            else
            {
                _Speed = 0;
            }
        }

    }

    float _LastSpeed;

    void Move()
    {
        _LastSpeed = _Speed;

        if (_Speed != 0 && _bIsmove)
        {
            if (_axial == axial.Horizontal && _Content.rect.size.x > this.GetComponent<RectTransform>().rect.size.x) {
                _Content.anchoredPosition += new Vector2(_Speed, 0);
            }

            if (_axial == axial.Vertical && _Content.rect.size.y > this.GetComponent<RectTransform>().rect.size.y) {
                _Content.anchoredPosition += new Vector2(0, _Speed);
            }
        }
    }


}
