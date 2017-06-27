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

    struct RayData
    {
        public Transform line;
        public LineRenderer line_renderer;
        public Collider hit;
        public GameObject spark;
        public OVButton press;
        public int device_index;
    };

#if UNITY_EDITOR||DEBUG
    private bool _bShowMouseRay = true;
#else
    private bool _bShowMouseRay = false;
#endif

    RayData[] _Ray = new RayData[RAY_COUNT];

    bool _bShow = false;                //当前是否显示 _bShow = (_bForceShow || OVMsgBox.GetCount() > 0);
    public bool _bForceShow = false;

    const float _fRayLen = 100;

    public int _nCollideLayerMask = 1 << 31;

	public Camera _MainCamera;
	bool _bMainCameraCullingMask = false;
	int _nMainCameraCullingMask = 0;

	public GameObject _OVSDKCamera;

    private void OnDeviceConnected(params object[] args)
    {
        uint index = (uint)(int)args[0];
        var connected = (bool)args[1];
        Debug.LogFormat("{0} conenct state = " + connected, index);
    }

    public bool IsForceShow()
    {
        return _bForceShow;
    }

    public bool IsVisible()
    {
        return _bShow;
    }

    // nForceShow > 0: force show controller-ray;
    //            = 0: not force show controller-ray;
    //            < 0: don't change current _bForceShow;
    public void CheckShow(int nForceShow = -1)
    {
        //Debug.Log("++++++++++++++++CheckShow began");
        if (nForceShow >= 0) {
            _bForceShow = nForceShow > 0;
        }
        bool value = (_bForceShow || OVMsgBox.GetCount() > 0);

        if (_bShow != value)
        {
            if (value)
            {
                //SteamVR_Utils.Event.Listen("device_connected", OnDeviceConnected);
                //SteamVR_Utils.Event.Listen("new_poses", OnControllerNewPoses);//private void OnNewPoses(TrackedDevicePose_t[] poses){...}
                SteamVR_Events.NewPoses.Listen(OnControllerNewPoses);
                for (int i = 0; i < RAY_COUNT; ++i)
                {
                    _Ray[i].device_index = i == 0 ? 0 : -1;
                }
            }
            else
            {
                //SteamVR_Utils.Event.Remove("device_connected", OnDeviceConnected);
                //SteamVR_Utils.Event.Remove("new_poses", OnControllerNewPoses);
                SteamVR_Events.NewPoses.Remove(OnControllerNewPoses);
            }
        }

        //SteamVR_Utils.Event.Listen("device_connected", OnDeviceConnected);
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
                        if (i != RAY_MOUSE || _bShowMouseRay)
                        {
                            GameObject t = GameObject.Instantiate(prefab, Vector3.zero, Quaternion.identity) as GameObject;
                            t.layer = 31;
                            _Ray[i].line = t.transform;
                            _Ray[i].line_renderer = t.transform.Find("Line").GetComponent<LineRenderer>();
                            _Ray[i].line.gameObject.SetActive(false);

                            if (_Ray[i].line_renderer && i == RAY_MOUSE) {
                                _Ray[i].line_renderer.transform.localRotation = Quaternion.identity;
                            }
                        }
                        //Debug.Log("++++++++++++++++prefab_spark：began use:" + prefab_spark.name);
                        if (prefab_spark != null)
                        {
                            //Debug.Log("++++++++++++++++prefab_spark：null to set:" + prefab_spark.name);
                            _Ray[i].spark = GameObject.Instantiate(prefab_spark, Vector3.zero, Quaternion.identity) as GameObject;
                            _Ray[i].spark.SetActive(false);
                        }
                        //Debug.Log("++++++++++++++++prefab_spark：" + prefab_spark.name);
                    }
                }
            }
        }

        for (int i = 0; i < RAY_COUNT; ++i) {
            showRay(_Ray[i], false);
        }

        _bShow = value;

		if (_bShow)
		{
			if (null == _OVSDKCamera)
			{
				GameObject prefab = Resources.Load<GameObject>("OVSDK_Camera");
				if (prefab)	{
					_OVSDKCamera = GameObject.Instantiate(prefab);
				}
			}
		}
		else
		{
			if (_OVSDKCamera)
			{
				GameObject.Destroy(_OVSDKCamera);
				_OVSDKCamera = null;
			}

			if (_MainCamera) {
				_MainCamera.cullingMask = _nMainCameraCullingMask;
				_bMainCameraCullingMask = false;
				_MainCamera = null;				
			}			
		}
        //Debug.Log("++++++++++++++++_bShow：" + _bShow);
    }

    PointerEventData _PointerEventData = new PointerEventData(null);
    private void showRay(RayData ray, bool bShow)
    {
        if (ray.line)
        {
            bool vis = bShow;
            vis &= (ray.Equals(_Ray[RAY_MOUSE]) || _bShowMouseRay);

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

    private void setHitState(Collider hit, OVButtonState state)
    {
        if (hit)
        {
            OVButton btn = hit.GetComponent<OVButton>();
            if (btn) {
                btn.SetButtonState(state);
            }
        }
    }

    Collider _hitscrollviewcollider = null;
    Collider _OnListenerCollider = null;
    float _pressTime;
    private void raycast(int index, int device_index, Ray ray)
    {
        if (!_bShowMouseRay && index == 0) {
            return;
        }
        if (device_index < 0)
        {
            return;
        }

        // update _Ray position;
        if (_Ray[index].line) {
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
            if (index != RAY_MOUSE || !SteamVR.enabled)
            {
                //if (i == 0)
                //{
                //    hit_index = i;
                //}

                OVScrollView sv = null;
                if (null != (sv = hits[i].collider.GetComponent<OVScrollView>())
                    && hits[i].collider.GetComponent<OVUIEventListener>())
                {
					RectTransform trans = sv.transform as RectTransform;
					Vector3 t = trans.InverseTransformPoint(hits[i].point);
					_PointerEventData.position = new Vector2(t.x, t.y);
					_PointerEventData.position -= trans.rect.min;

					_PointerEventData.pointerId = device_index;
                    OVUIEventListener.Get(hits[i].collider.gameObject).onHover(hits[i].collider.gameObject, _PointerEventData);
                    hitscrollviewcollider = hits[i].collider;
                    continue;
                }

                if (hits[i].collider.GetComponent<OVButton>())
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
            }
        }
 

        if (_hitscrollviewcollider == null)
        {
            _hitscrollviewcollider = hitscrollviewcollider;

        }
        if(_hitscrollviewcollider != null)
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
            int ind=0;
            for(int i=0;i<hits.Length;++i)
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
                        if (null != (a = _Ray[index].hit.GetComponent<OVUIEventListener>())) {
                            a.OnPointerDown(null);
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
                        OVUIEventListener.Get(_Ray[index].press.gameObject).OnPointerDown(null);
                        _Ray[index].press.SetButtonState(_Ray[index].press == hvb ? OVButtonState.Press : OVButtonState.Normal);
                    }
                }
            }
        }
    }


    public void Drive()
    {
        if (!_bShow)  {
            return;
        }

		if (null == _MainCamera) {
			_MainCamera = FindMainCamera();
		}

		if (!_bMainCameraCullingMask && _MainCamera && _MainCamera.cullingMask != 0)
		{
			_bMainCameraCullingMask = true;
			_nMainCameraCullingMask = _MainCamera.cullingMask;
			_MainCamera.cullingMask = (int)(0xFFFFFFFF & ~(1 << 31));
		}
        Ray ray = new Ray();
        int device_index = -1;
        int index = -1;
        for (int i = 0; i < RAY_COUNT; ++i)
        {

            //new
            //if (i > 0)
            //{
            //    if (OpenVR.System != null)
            //    {
            //        device_index = (int)OpenVR.System.GetTrackedDeviceIndexForControllerRole(i == RAY_RIGHT ? ETrackedControllerRole.RightHand : ETrackedControllerRole.LeftHand);
            //    }

            //    if ((_Ray[i].device_index = device_index) > 0)
            //    {
            //        index = i;
            //    }

            //}

            //old
            if (i == RAY_MOUSE)
            {
                if (!SteamVR.enabled)
                {
                    device_index = 0;
                }
                if (_MainCamera)
                {
                    StereoTargetEyeMask target_eye = StereoTargetEyeMask.None;
                    bool stereo = false;
                    if (_MainCamera.stereoEnabled)
                    {
                        stereo = true;
                        target_eye = _MainCamera.stereoTargetEye;
                        _MainCamera.stereoTargetEye = StereoTargetEyeMask.None;
                    }
                    index = i;
                    ray = _MainCamera.ScreenPointToRay(Input.mousePosition);
                    Debug.DrawRay(ray.origin, ray.direction);

                    if (stereo)
                    {
                        _MainCamera.stereoTargetEye = target_eye;
                    }
                }
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
                    ray.origin = _Ray[i].line_renderer.transform.position;
                    ray.direction = _Ray[i].line_renderer.transform.TransformDirection(Vector3.forward);
                }
                else if (_Ray[i].device_index >= 0)
                {
                    _Ray[i].device_index = -1;
                    showRay(_Ray[i], false);
                }
            }
        }
        raycast(index, _Ray[index].device_index, ray);
        for(int i =0;i< RAY_COUNT;++i)
        {
            if(index!=i)
            {
                showRay(_Ray[i], false);
            }
        }
    }

    SteamVR_ControllerManager _ViveControllerMgr;
    void OnControllerNewPoses(TrackedDevicePose_t[] poses)
    {
        if (null == _ViveControllerMgr)
        {
            _ViveControllerMgr = GameObject.FindObjectOfType<SteamVR_ControllerManager>();
        }

        //var poses = (Valve.VR.TrackedDevicePose_t[])args[0];

        for (int j = RAY_LEFT; j <= RAY_RIGHT; ++j)
        {
            int i = _Ray[j].device_index;
            if (i < 0 || poses.Length <= i)
                continue;

            if (!poses[i].bDeviceIsConnected)
                continue;

            if (!poses[i].bPoseIsValid)
                continue;

            var pose = new SteamVR_Utils.RigidTransform(poses[i].mDeviceToAbsoluteTracking);
            if (_Ray[j].line)
            {
                //Vector3 pos = pose.pos;
                if (_ViveControllerMgr)
                {
                    _Ray[j].line.transform.position = _ViveControllerMgr.transform.TransformPoint(pose.pos);
                    _Ray[j].line.transform.rotation = _ViveControllerMgr.transform.rotation * pose.rot;
                }               
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

		if (main) {
			Debug.Log("FindMainCamera: " + main.name);
		}

		return main;
	}
}
