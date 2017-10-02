using UnityEngine;
using System.Collections;

public class OVSetOVCameraScale : MonoBehaviour {

    SteamVR_ControllerManager steamVR_ControllerManager;
    GameObject leftController;
    GameObject rightController;

    void Start()
    {
        steamVR_ControllerManager = FindObjectOfType<SteamVR_ControllerManager>();
        leftController = steamVR_ControllerManager.left;
        rightController = steamVR_ControllerManager.right;
        if (leftController != null)
        {
            gameObject.transform.localScale = leftController.transform.lossyScale;
        }
        else if (rightController != null)
        {
            gameObject.transform.localScale = rightController.transform.lossyScale;
        }
    }
}
