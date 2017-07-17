using UnityEngine.UI;
using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using Valve.VR;
using System;
using UnityEngine.EventSystems;
using UnityEngine.VR;
public class OVControllerRay
{
    const int RAY_MOUSE = 0;
    const int RAY_LEFT = 1;
    const int RAY_RIGHT = 2;
    const int RAY_COUNT = 3;

	public const int RayVisibility_Auto = 0;
	public const int RayVisibility_AlwaysShow = 1;
	public const int RayVisibility_AlwaysHide = 2;

	struct RayData
    {
        public Transform line;
        public LineRenderer line_renderer;
        public Collider hit;
        public GameObject spark;
        public OVButton press;
        public int device_index;
        public bool isValid;
        public int index;
        public Ray ray;
    };




    RayData[] _Ray = new RayData[RAY_COUNT];

    bool _bShow = false;                //当前是否显示 _bShow = (_bForceShow || OVMsgBox.GetCount() > 0);
	int _nRayVisibility = RayVisibility_Auto;

    const float _fRayLen = 100;

    public int _nCollideLayerMask = 1 << 31;

   // public Camera _MainCamera;
 //   bool _bMainCameraCullingMask = false;
    int _nMainCameraCullingMask = 0;

    public Camera _OVSDKCamera;

    private void OnDeviceConnected(params object[] args)
    {
        uint index = (uint)(int)args[0];
        var connected = (bool)args[1];
        Debug.LogFormat("{0} conenct state = " + connected, index);
    }

	public int RayVisibility
	{
		get	{
			return _nRayVisibility;
		}
		set {
			_nRayVisibility = value;
		}
	}

	public bool IsVisible()
    {
        return _bShow;
    }

	void UpdateVisibility()
	{
		bool value = _nRayVisibility == RayVisibility_AlwaysShow 
			|| (_nRayVisibility == RayVisibility_Auto && OVMsgBox.GetCount() > 0);

		if (_bShow == value) {
			return;
		}

		if (_bShow != value)
		{
			if (value)
			{
				SteamVR_Events.NewPoses.Listen(OnControllerNewPoses);
			}
			else
			{
				SteamVR_Events.NewPoses.Remove(OnControllerNewPoses);
			}
		}
		if (value)
		{
			if (null == _Ray[1].line)
			{
				GameObject prefab = Resources.Load<GameObject>("OVSDK_Ray");
				GameObject prefab_spark = Resources.Load<GameObject>("OVSDK_Spark");
				if (prefab)
				{
					for (int i = 0; i < RAY_COUNT; ++i)
					{
						GameObject t = GameObject.Instantiate(prefab, Vector3.zero, Quaternion.identity) as GameObject;
						t.layer = 31;
						_Ray[i].line = t.transform;
						_Ray[i].line_renderer = t.transform.Find("Line").GetComponent<LineRenderer>();
						_Ray[i].line.gameObject.SetActive(false);

						if (_Ray[i].line_renderer && i == RAY_MOUSE)
						{
							_Ray[i].line_renderer.transform.localRotation = Quaternion.identity;
						}
						if (prefab_spark != null)
						{
							_Ray[i].spark = GameObject.Instantiate(prefab_spark, Vector3.zero, Quaternion.identity) as GameObject;
							_Ray[i].spark.SetActive(false);
						}
					}
				}
			}
		}

		_bShow = value;
		if (_bShow)
		{
			if (_OVSDKCamera == null)
			{
				CreateOVSDKCamera();
			}
			_OVSDKCamera.enabled = true;
            _nMainCameraCullingMask = Camera.main.cullingMask;
            Camera.main.cullingMask &= ~_nCollideLayerMask;
        }
		else
		{
			_OVSDKCamera.enabled = false;
            Camera.main.cullingMask = _nMainCameraCullingMask;
            foreach(RayData i in _Ray)
            {
                i.line.gameObject.SetActive(false);
                i.spark.SetActive(false);
            }
        }
	}


    void CreateOVSDKCamera()
    {
        if (null == _OVSDKCamera)
        {
            GameObject prefab = Resources.Load<GameObject>("OVSDK_Camera");
            if (prefab)
            {
                GameObject OVSDKCamera = GameObject.Instantiate(prefab);
                _OVSDKCamera = OVSDKCamera.transform.GetComponentInChildren<Camera>();
                _OVSDKCamera.cullingMask = _nCollideLayerMask; ;
                _OVSDKCamera.clearFlags = CameraClearFlags.Depth;
                _OVSDKCamera.enabled = false;

            }
        }
    }
    public PointerEventData _PointerEventData = new PointerEventData(null);
    private void showRay(RayData ray, bool bShow)
    {
        if (ray.line)
        {
            bool vis = bShow;
            vis &= (ray.Equals(_Ray[RAY_MOUSE]));

            ray.line.gameObject.SetActive(vis);
        }

        if (ray.spark && !bShow)
        {
            ray.spark.SetActive(false);
        }

        if (!bShow)
        {
            ray.hit = null;
            if (ray.press)
            {
                ray.press.SetButtonState(OVButtonState.Normal);
                ray.press = null;
            }
        }
    }


    Collider _hitscrollviewcollider = null;
    Collider _OnListenerCollider = null;
    float _pressTime;

    private void raycast(int index, int device_index, Ray ray)
    {
        if (device_index < 0)
        {
            return;
        }

        // update _Ray position;
        if (_Ray[index].line)
        {
            _Ray[index].line.gameObject.SetActive(device_index >= 0);
        }



        if (_Ray[index].line && index == RAY_MOUSE)
        {
            _Ray[index].line.transform.position = ray.origin;
            _Ray[index].line.transform.LookAt(ray.origin + ray.direction * 100);
        }
        Debug.DrawRay(ray.origin, ray.direction * 100);
        // hit test
        int mask = _nCollideLayerMask != 0 ? _nCollideLayerMask : -1;
        RaycastHit[] hits = Physics.RaycastAll(ray, 1000, mask);
        int hit_index = -1;
        OVButton hvb = null;
        Collider hitscrollviewcollider = null;
        OVScrollView hitsv = null;
        float len = _fRayLen;

        for (int i = 0; i < hits.Length; i++)
        {
            if (hits[i].collider.GetComponent<Canvas>())
            {
                RectTransform trans = hits[i].collider.GetComponent<Canvas>().transform as RectTransform;
                Vector3 t = trans.InverseTransformPoint(hits[i].point);
                Vector2 v2 = new Vector2(t.x, t.y);
                v2 -= trans.rect.min;
                _PointerEventData.delta = v2 - _PointerEventData.position;
                _PointerEventData.position = v2;
                //Debug.Log("_PointerEventData.delta:=" + _PointerEventData.delta);
                //Debug.Log(_PointerEventData.position);
            }

            if (null != hits[i].collider.GetComponent<OVScrollView>()
                && hits[i].collider.GetComponent<OVUIEventListener>())
            {
                _PointerEventData.pointerId = device_index;
                OVUIEventListener.Get(hits[i].collider.gameObject).onHover(hits[i].collider.gameObject, _PointerEventData);
                hitscrollviewcollider = hits[i].collider;
                continue;
            }

            OVButton btn_tmp = hits[i].collider.GetComponent<OVButton>();
            if (btn_tmp && !btn_tmp._bIgnoreRay)
            {
                hitsv = hits[i].collider.GetComponentInParent<OVScrollView>();
                if (hitsv == null)
                {
                    hvb = hits[i].collider.GetComponent<OVButton>();  // 优先button
                    OVUIEventListener.Get(hvb.gameObject);                    //确保button有UIEventListener;
                    hit_index = i;
                }
                else
                {
                    hvb = hits[i].collider.GetComponent<OVButton>();
                    hit_index = i;// 优先button
                }

            }
            //}
        }


        if (_hitscrollviewcollider == null)
        {
            _hitscrollviewcollider = hitscrollviewcollider;

        }
        if (_hitscrollviewcollider != null)
        {
            if (_hitscrollviewcollider != hitscrollviewcollider)
            {

                OVUIEventListener.Get(_hitscrollviewcollider.gameObject).onExit(null);
            }
            _hitscrollviewcollider = hitscrollviewcollider;

        }


        if (hitsv)
        {
            if (hitscrollviewcollider == null)
            {
                hvb = null;

            }
            if (hitscrollviewcollider)
            {
                if (hitsv.name == hitscrollviewcollider.name)
                {
                    OVUIEventListener.Get(hvb.gameObject);
                }
            }

        }

        //spark 处理
        if (hits.Length <= 0)
        {
            if (_Ray[index].spark)
            {
                _Ray[index].spark.SetActive(false);
            }
            if (_Ray[index].line_renderer)
            {
                _Ray[index].line_renderer.SetPosition(1, new Vector3(0, 0, 100));
            }

        }
        else
        {
            float f = hits[0].distance;
            int ind = 0;
            for (int i = 0; i < hits.Length; ++i)
            {

                if (hits[i].distance < f)
                {
                    f = hits[i].distance;
                    ind = i;
                }
            }
            if (_Ray[index].spark)
            {
                _Ray[index].spark.SetActive(true);
                {
                    _Ray[index].spark.transform.position = hits[ind].point; ;
                    _Ray[index].spark.transform.LookAt(hits[ind].point - ray.direction, Vector3.up);
                }
            }

            len = Vector3.Distance(ray.origin, hits[ind].point);

            if (_Ray[index].line_renderer)
            {
                _Ray[index].line_renderer.SetPosition(1, new Vector3(0, 0, len));
            }
        }




        Collider col = hit_index >= 0 ? hits[hit_index].collider : null;

        if (col != _Ray[index].hit)
        {
            _Ray[index].hit = col;
            if (_OnListenerCollider != col && _OnListenerCollider != null)
            {
                OVUIEventListener.Get(_OnListenerCollider.gameObject).OnPointerExit(null);
                //Debug.Log("OnPointerExit" + _OnListenerCollider);
                _OnListenerCollider = null;
                SteamVR_Controller.Input(device_index).TriggerHapticPulse(500);

            }

            if (col != null)
            {
                //OVUIEventListener a;
                if (null != (_Ray[index].hit.GetComponent<OVUIEventListener>()))
                {
                    // Debug.Log("OnPointerEnter" + col.gameObject);
                    OVUIEventListener.Get(col.gameObject).OnPointerEnter(null);
                    SteamVR_Controller.Input(device_index).TriggerHapticPulse(1500);
                    _OnListenerCollider = col;
                }
            }
        }

        if (col)
        {
            if (hvb)
            {
                int act = 0;
                if (index == RAY_MOUSE)
                {
                    if (Input.GetMouseButtonDown(0))
                    {
                        act = 1;
                    }
                    else if (Input.GetMouseButtonUp(0))
                    {
                        act = 2;
                    }
                    else if (Input.GetMouseButton(0))
                    {
                        act = 3;
                    }
                }
                else
                {
                    if (SteamVR_Controller.Input(device_index).GetPressDown(SteamVR_Controller.ButtonMask.Trigger))
                    {
                        act = 1;
                    }
                    else if (SteamVR_Controller.Input(device_index).GetPressUp(SteamVR_Controller.ButtonMask.Trigger))
                    {
                        act = 2;
                    }
                    else if (SteamVR_Controller.Input(device_index).GetPress(SteamVR_Controller.ButtonMask.Trigger))
                    {
                        act = 3;
                    }
                }

                if (act == 1)
                {
                    _pressTime = 0;
                    _Ray[index].press = hvb;

                    if (col != null)
                    {
                        OVUIEventListener a;
                        if (null != (a = _Ray[index].hit.GetComponent<OVUIEventListener>()))
                        {

                            a.OnPointerDown(_PointerEventData);
                        }
                    }

                }
                else if (act == 2)
                {
                    //  hvb.SetButtonState(OVButtonState.Normal);
                    if (_Ray[index].press == hvb)
                    {
                        if (_Ray[index].spark)
                        {
                            //_Ray[index].spark.transform.SetParent(null);
                            _Ray[index].spark.SetActive(false);
                        }
                        if (_pressTime < 0.3)
                        {
                            OVUIEventListener.Get(hvb.gameObject).OnPointerClick(null);
                        }
                        OVUIEventListener.Get(hvb.gameObject).OnPointerUp(null);
                        _Ray[index].press = null;
                    }
                }
                else if (act == 3)
                {
                    _pressTime += Time.deltaTime;
                    if (_Ray[index].press)
                    {
                        if (_PointerEventData.IsPointerMoving())
                        {
                            _PointerEventData.pointerDrag = _Ray[index].hit.gameObject;
                            OVUIEventListener.Get(_Ray[index].press.gameObject).OnDrag(_PointerEventData);
                            //	Debug.Log("OnDrag++++++++++++++++------------------------------------------");

                        }
                        OVUIEventListener.Get(_Ray[index].press.gameObject).OnPointerDown(null);
                        _Ray[index].press.SetButtonState(_Ray[index].press == hvb ? OVButtonState.Press : OVButtonState.Normal);
                    }
                }
            }
        }
    }


    void Proessdata(int index, int device_index)
    {
        _PointerEventData.button = PointerEventData.InputButton.Left;
        GameObject gameobject = _PointerEventData.pointerCurrentRaycast.gameObject;
        HandlePointerExitAndEnter(_PointerEventData, gameobject);

        bool pressframe = Input.GetMouseButtonDown(0) || SteamVR_Controller.Input(device_index).GetPressDown(SteamVR_Controller.ButtonMask.Trigger);
        if (pressframe)
        {
            float unscaledTime = Time.unscaledTime;
            _PointerEventData.eligibleForClick = true;
            _PointerEventData.delta = Vector2.zero;
            //   _PointerEventData.
            _PointerEventData.dragging = false;
            _PointerEventData.useDragThreshold = true;
            _PointerEventData.pressPosition = _PointerEventData.position;

            GameObject gameObject2 = ExecuteEvents.ExecuteHierarchy<IPointerDownHandler>(gameobject, _PointerEventData, ExecuteEvents.pointerDownHandler);

            if (gameObject2 == null)
            {
                gameObject2 = ExecuteEvents.GetEventHandler<IPointerClickHandler>(gameobject);
            }
            if (gameObject2 == _PointerEventData.lastPress)
            {
                float num = unscaledTime - _PointerEventData.clickTime;
                if (num < 0.3f)
                {
                    _PointerEventData.clickCount++;
                }
                else
                {
                    _PointerEventData.clickCount = 1;
                }
                _PointerEventData.clickTime = unscaledTime;
            }
            else
            {
                _PointerEventData.clickCount = 1;
            }
            _PointerEventData.pointerPress = gameObject2;
            _PointerEventData.pointerPressRaycast = _PointerEventData.pointerCurrentRaycast;
            _PointerEventData.rawPointerPress = gameobject;
            _PointerEventData.clickTime = unscaledTime;
            _PointerEventData.pointerDrag = ExecuteEvents.GetEventHandler<IDragHandler>(gameobject);
            if (_PointerEventData.pointerDrag != null)
            {
                ExecuteEvents.Execute<IInitializePotentialDragHandler>(gameobject, _PointerEventData, ExecuteEvents.initializePotentialDrag);
            }
        }
        bool ReleasedThisFrame = Input.GetMouseButtonUp(0) || SteamVR_Controller.Input(device_index).GetPressUp(SteamVR_Controller.ButtonMask.Trigger);
        if (ReleasedThisFrame)
        {

            ExecuteEvents.Execute<IPointerUpHandler>(_PointerEventData.pointerPress, _PointerEventData, ExecuteEvents.pointerUpHandler);
            GameObject eventHandler = ExecuteEvents.GetEventHandler<IPointerClickHandler>(gameobject);
            if (_PointerEventData.pointerPress == eventHandler)
            {
                ExecuteEvents.Execute<IPointerClickHandler>(_PointerEventData.pointerPress, _PointerEventData, ExecuteEvents.pointerClickHandler);
            }
            else if (_PointerEventData.pointerDrag != null && _PointerEventData.dragging)
            {
                ExecuteEvents.ExecuteHierarchy<IDropHandler>(gameobject, _PointerEventData, ExecuteEvents.dropHandler);
            }
            _PointerEventData.eligibleForClick = false;
            _PointerEventData.pointerPress = null;
            _PointerEventData.rawPointerPress = null;
            if (_PointerEventData.pointerDrag != null && _PointerEventData.dragging)
            {
                ExecuteEvents.Execute<IEndDragHandler>(_PointerEventData.pointerDrag, _PointerEventData, ExecuteEvents.endDragHandler);
            }
            _PointerEventData.dragging = false;
            _PointerEventData.pointerDrag = null;
            // _PointerEventData.useDragThreshold = false;
            if (gameobject != _PointerEventData.pointerEnter)
            {
                this.HandlePointerExitAndEnter(_PointerEventData, null);
                this.HandlePointerExitAndEnter(_PointerEventData, gameobject);
            }
        }


    }
    void ProsseDrag()
    {
        bool flag = (Time.unscaledTime - _PointerEventData.clickTime) > 0.3;
        if (flag && _PointerEventData.pointerDrag != null && !_PointerEventData.dragging)
        {
            ExecuteEvents.Execute<IBeginDragHandler>(_PointerEventData.pointerDrag, _PointerEventData, ExecuteEvents.beginDragHandler);
            _PointerEventData.dragging = true;
            //  Debug.Log("111");
        }
        if (_PointerEventData.dragging && flag && _PointerEventData.pointerDrag != null)
        {
            if (_PointerEventData.pointerPress != _PointerEventData.pointerDrag)
            {
                ExecuteEvents.Execute<IPointerUpHandler>(_PointerEventData.pointerPress, _PointerEventData, ExecuteEvents.pointerUpHandler);
                _PointerEventData.eligibleForClick = false;
                _PointerEventData.pointerPress = null;
                _PointerEventData.rawPointerPress = null;
                //   Debug.Log("222");
            }
            ExecuteEvents.Execute<IDragHandler>(_PointerEventData.pointerDrag, _PointerEventData, ExecuteEvents.dragHandler);
            //  Debug.Log("0000");

        }
    }
    private void HandlePointerExitAndEnter(PointerEventData currentPointerData, GameObject newEnterTarget)
    {
        if (newEnterTarget == null || currentPointerData.pointerEnter == null)
        {
            for (int i = 0; i < currentPointerData.hovered.Count; i++)
            {
                ExecuteEvents.Execute<IPointerExitHandler>(currentPointerData.hovered[i], currentPointerData, ExecuteEvents.pointerExitHandler);
            }
            currentPointerData.hovered.Clear();
            if (newEnterTarget == null)
            {
                currentPointerData.pointerEnter = newEnterTarget;
                return;
            }
        }
        if (currentPointerData.pointerEnter == newEnterTarget && newEnterTarget)
        {
            return;
        }
        GameObject gameObject = this.FindCommonRoot(currentPointerData.pointerEnter, newEnterTarget);
        if (currentPointerData.pointerEnter != null)
        {
            Transform transform = currentPointerData.pointerEnter.transform;
            while (transform != null)
            {
                if (gameObject != null && gameObject.transform == transform)
                {
                    break;
                }
                ExecuteEvents.Execute<IPointerExitHandler>(transform.gameObject, currentPointerData, ExecuteEvents.pointerExitHandler);
                currentPointerData.hovered.Remove(transform.gameObject);
                transform = transform.parent;
            }
        }
        currentPointerData.pointerEnter = newEnterTarget;
        if (newEnterTarget != null)
        {
            Transform transform2 = newEnterTarget.transform;
            while (transform2 != null && transform2.gameObject != gameObject)
            {
                ExecuteEvents.Execute<IPointerEnterHandler>(transform2.gameObject, currentPointerData, ExecuteEvents.pointerEnterHandler);
                currentPointerData.hovered.Add(transform2.gameObject);
                transform2 = transform2.parent;
            }
        }
    }
    private GameObject FindCommonRoot(GameObject g1, GameObject g2)
    {
        if (g1 == null || g2 == null)
        {
            return null;
        }
        Transform transform = g1.transform;
        while (transform != null)
        {
            Transform transform2 = g2.transform;
            while (transform2 != null)
            {
                if (transform == transform2)
                {
                    return transform.gameObject;
                }
                transform2 = transform2.parent;
            }
            transform = transform.parent;
        }
        return null;
    }



    private void MyRayCast(int index, int device_index, Ray ray)
    {
        if (index < 0 || device_index < 0)
        {
            _Ray[index].line.gameObject.SetActive(false);
            return;
        }

        int mask = _nCollideLayerMask != 0 ? _nCollideLayerMask : -1;
        RaycastHit[] hits = Physics.RaycastAll(ray, 1000, mask);
        if (hits.Length <= 0)
        {
            _Ray[index].spark.gameObject.SetActive(false);
            return;
        }
        Vector2 screenpos = new Vector2(-1, -1);
        GraphicRaycaster grarcater = null;
        Canvas ca = null;

        for (int i = 0; i < hits.Length; ++i)
        {
            ca = hits[i].collider.transform.GetComponentInChildren<Canvas>();
            if (ca != null)
            {
                ca.worldCamera = _OVSDKCamera;
                grarcater = ca.GetComponent<GraphicRaycaster>();
                if (grarcater == null)
                {
                    ca.gameObject.AddComponent<GraphicRaycaster>();
                }
                Vector3 ve = ca.transform.worldToLocalMatrix.MultiplyPoint3x4(hits[i].point);
                screenpos = Camera.main.WorldToScreenPoint(ca.transform.localToWorldMatrix.MultiplyPoint3x4(ve));
                _Ray[index].line.gameObject.SetActive(true);
                if(index==RAY_MOUSE)
                {
                    _Ray[index].line_renderer.gameObject.SetActive(false);
                }
                _Ray[index].spark.gameObject.SetActive(true);
                _Ray[index].spark.gameObject.transform.position = hits[i].point;
                break;
            }
        }

        if (grarcater == null || ca == null)
        {
            return;
        }

        _PointerEventData.delta = screenpos - _PointerEventData.position;
        _PointerEventData.position = screenpos;
        _PointerEventData.scrollDelta = SteamVR_Controller.Input(device_index).GetAxis(EVRButtonId.k_EButton_SteamVR_Touchpad);
        List<RaycastResult> resultAppendList = new List<RaycastResult>();
        grarcater.Raycast(_PointerEventData, resultAppendList);
        RaycastResult CurrentRaycas = this.FindFristRayCast(resultAppendList);
        _PointerEventData.pointerCurrentRaycast = CurrentRaycas;
    }

    RaycastResult FindFristRayCast(List<RaycastResult> candidates)
    {
        for (int i = 0; i < candidates.Count; ++i)
        {
            if (candidates[i].gameObject != null)
            {
                return candidates[i];
            }
        }
        return default(RaycastResult);
    }


    public void Drive()
    {
		UpdateVisibility();

        if (!_bShow)
        {
            return;
        }
        if (!_OVSDKCamera)
        {
            CreateOVSDKCamera();
        }

        RayData raydata = GetOVSDKRay();
        MyRayCast(raydata.index, raydata.device_index, raydata.ray);
        this.Proessdata(raydata.index, raydata.device_index);
        this.ProsseDrag();

    }
    RayData GetOVSDKRay()
    {
        int device_index = -1;
        int index = -1;
        for (int i = RAY_RIGHT; i >= 0; --i)
        {
            if (i == RAY_MOUSE)
            {
                index = i;
                device_index = 0;

                _Ray[i].ray = _OVSDKCamera.ScreenPointToRay(Input.mousePosition);
                _Ray[i].index = index;
                break;
            }
            else
            {
                if (OpenVR.System != null)
                {
                    device_index = (int)OpenVR.System.GetTrackedDeviceIndexForControllerRole(i == RAY_RIGHT ? ETrackedControllerRole.RightHand : ETrackedControllerRole.LeftHand);
                }

                if (device_index >= 0
                    && SteamVR_Controller.Input(device_index).connected
                    && (i != RAY_RIGHT || device_index != _Ray[RAY_LEFT].device_index)
                    && _Ray[i].line_renderer
                    && index <= 0)
                {

                    index = i;
                    _Ray[i].device_index = device_index;
                    _Ray[i].ray.origin = _Ray[i].line_renderer.transform.position;
                    _Ray[i].ray.direction = _Ray[i].line_renderer.transform.TransformDirection(Vector3.forward);
                    _Ray[i].index = index;
                    break;
                }
            }
        }
        return _Ray[index];
    }
    bool getRayData(int rayid, out RayData raydata)
    {
        bool v = false;
        raydata = _Ray[rayid];
        v = raydata.isValid;
        return v;
    }


    void OnControllerNewPoses(TrackedDevicePose_t[] poses)
    {
        for (int j = RAY_LEFT; j <= RAY_RIGHT; ++j)
        {

            int i = _Ray[j].device_index;
            if (i < 0 || poses.Length <= i)
            {
                _Ray[j].isValid = false;
                continue;
            }


            if (!poses[i].bDeviceIsConnected)
            {
                _Ray[j].isValid = false;
                continue;
            }


            if (!poses[i].bPoseIsValid)
            {
                _Ray[j].isValid = false;
                continue;
            }

            var pose = new SteamVR_Utils.RigidTransform(poses[i].mDeviceToAbsoluteTracking);
            _Ray[j].isValid = true;

            if (_Ray[j].line)
            {
                _Ray[j].line.transform.position = Camera.main.transform.parent.transform.TransformPoint(pose.pos);
                _Ray[j].line.transform.rotation = Camera.main.transform.parent.transform.rotation * pose.rot;

            }
        }
    }

    public void OnRayDestroy(GameObject ray)
    {
        for (int i = 0; i < RAY_COUNT; ++i)
        {
            if (_Ray[i].line == ray)
            {
                _Ray[i].line = null;
                _Ray[i].line_renderer = null;
            }
        }
    }

    static Camera FindMainCamera()
    {
        Camera main = null;
        Camera[] cameras = Camera.allCameras;
        for (int i = 0; i < cameras.Length; ++i)
        {
            if (cameras[i].cameraType == CameraType.Game
                && !cameras[i].orthographic)
            {
                if (cameras[i].tag == "MainCamera" || null == main)
                {
                    main = cameras[i];
                }
            }
        }

        if (main)
        {
            Debug.Log("FindMainCamera: " + main.name);
        }

        return main;
    }
}
