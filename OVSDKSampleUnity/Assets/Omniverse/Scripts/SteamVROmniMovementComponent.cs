using UnityEngine;
using System.Collections;


namespace Virtuix
{
    public class SteamVROmniMovementComponent : OmniMovementComponent
    {
        /// <summary>
        /// You need to point this to the [CameraRig] prefab in your SteamVR Prefabs folder.
        /// </summary>
        [Header("-----Make sure to point to [CameraRig]------")]
        public GameObject viveCameraRigPrefab;

        public Transform viveParentObject;
        public Transform viveRootObject;


        public override void OmniInitialize()
        {
            if(viveCameraRigPrefab == null && cameraReference == null)
            {
                Debug.LogError("SteamVROmniMovementComponent(OmniInitialize) - Prefab needs to be set to the SteamVR [CameraRig] from the SteamVR Prefabs folder");
                return;
            }

            if(cameraReference == null)
            {
                GameObject viveRig = Instantiate(viveCameraRigPrefab, viveRootObject.transform.position, viveRootObject.transform.rotation) as GameObject;
                viveRig.transform.parent = viveRootObject;
            }


            cameraReference = GameObject.Find("Camera (eye)").transform;

            if (developerMode)
            {
                if (cameraReference == null)
                {
                    Debug.LogError(System.DateTime.Now.ToLongTimeString() + ": SteamVROmniMovementComponent(OmniInitialize) - Attempted to Initialize the Omni - developer mode is true and Camera Reference not set in prefab.");
                    return;
                }

                cameraReference.gameObject.AddComponent<SmoothMouseLook>();

                viveRootObject.localPosition = new Vector3(0, 1, 0);
                return;
            }

            // Create a new OmniManager and see if we can find the Omni
            omniManager = new OmniManager();

            if (omniManager.FindOmni())
            {
                Debug.Log(System.DateTime.Now.ToLongTimeString() + ": SteamVROmniMovementComponent(OmniInitialize) - Successfully found the Omni.");
                byte[] logMotionData = OmniCommon.OmniPacketBuilder.buildPacket((byte)OmniCommon.Command.SET_MOTION_DATA_MODE, new byte[] { 0x7F }, 0x00);
                omniManager.SendData(logMotionData);
                omniFound = true;
            }
            else
            {
                Debug.LogError(System.DateTime.Now.ToLongTimeString() + ": SteamVROmniMovementComponent(OmniInitialize) - Attempted to Initialize the Omni, but Omni not found.");
                omniFound = false;
                return;
            }
            
        }


        public override void AlignOmni()
        {
            if (!hasAligned)
            {
                if (motionData != null)
                {
                    if ((cameraReference.transform.position.x != 0) && (cameraReference.transform.position.y != 0) && (cameraReference.transform.position.z != 0) &&
                            (cameraReference.rotation.eulerAngles.x != 0) && (cameraReference.rotation.eulerAngles.y != 0) && (cameraReference.rotation.eulerAngles.z != 0))
                    {
                        if (hasFullyInitialized)
                        {
                            initialRotation = dummyObject.transform.rotation.eulerAngles.y;
                            transform.rotation = dummyObject.transform.rotation;
                        }
                        //locally store the offset value
                        //cameraOffset = HVSDK.HvGetOmniYawOffset();
                        cameraOffset = motionData.RingAngle;
                        omniOffset = motionData.RingAngle;
                        viveParentObject.rotation = Quaternion.Euler(0, cameraOffset - motionData.RingAngle + initialRotation, 0);
                        hasAligned = true;
                        }
                }

                if (!hasFullyInitialized && hasAligned)
                {
                    //grab initial step count here
                    if (motionData != null)
                    {
                        ResetStepCount();
                        hasFullyInitialized = true;
                        Debug.Log(System.DateTime.Now.ToLongTimeString() + ": SteamVROmniMovementComponent(AlignOmni) - Successfully aligned Omni. Omni has been fully initialized");
                    }
                }
            }
        }
    }
}
