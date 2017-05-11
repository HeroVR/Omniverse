using UnityEngine;

public class HVRay : MonoBehaviour
{
	void Start()
	{
		
	}

	void OnDestroy()
	{
		HVSDK._ControllerRay.OnRayDestroy(gameObject);
	}

	void Update()
	{

	}
}
