using UnityEngine;

public class OVCamera : MonoBehaviour
{
	void LateUpdate()
	{
		//if (OVSDK._ControllerRay._MainCamera)
		//{           
            Transform main_camera = Camera.main.transform;
			if (!SteamVR.enabled)
			{
				transform.position = main_camera.position;
				transform.rotation = main_camera.rotation;
			}
			else
			{
				Transform user = main_camera.parent;  //使用VR模式时，在HDM内显示的所有camera的position，rotation都会自动保持和HDM pose一致。
				if (user)   //所以需要同步parent的transform;
				{
					transform.parent.position = user.position;
                    transform.parent.rotation = user.rotation;
				}
			}
     //   }
    }
}
