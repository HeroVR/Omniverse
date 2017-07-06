using UnityEngine;

public class OVRay : MonoBehaviour
{
	void Start()
	{
		
	}

	void OnDestroy()
	{
		OVSDK._ControllerRay.OnRayDestroy(gameObject);
	}


}
