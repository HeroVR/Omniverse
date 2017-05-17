//Uncomment the following line to support SteamVR and point to the [CameraRig] from SteamVR in the Vive prefab.
#define STEAMVR_OMNI
using UnityEngine;
using System.Collections;


namespace Virtuix
{
    public class SteamVROmniController : OmniController
    {
        /// <summary>
        /// You need to point this to the [CameraRig] prefab in your SteamVR Prefabs folder.
        /// </summary>
        [Header("Make sure to point to [CameraRig]")]
        public GameObject viveCameraRigPrefab;

        public Transform viveParentObject;
        public Transform viveRootObject;



#if !STEAMVR_OMNI
        public override void OmniInitialize()
        {
            Debug.LogError("SteamVROmniController(OmniInitialize) - Failed to Initialize Omni. You need to add the SteamVR SDK, set the viveCameraRigPrefab to [CameraRig] from the SteamVR prefab folder, and uncomment the STEAMVR_OMNI define in SteamVROmniController.");
        }
#else
        
        public SteamVR_TrackedObject[] steamVR_TrackedObjects;

        public override void OmniInitialize()
        {
            if(viveCameraRigPrefab == null && cameraReference == null)
            {
                Debug.Log("SteamVROmniController(OmniInitialize) - Prefab needs to be set to the SteamVR [CameraRig] from the SteamVR Prefabs folder");
                return;
            }

            if(cameraReference == null)
            {
                GameObject viveRig = Instantiate(viveCameraRigPrefab, viveRootObject.transform.position, viveRootObject.transform.rotation) as GameObject;
                viveRig.transform.parent = viveRootObject;
            }


            cameraReference = GameObject.Find("Camera (eye)").transform;

            debugTextUI.gameObject.transform.parent.parent = cameraReference;

            if (developerMode)
            {
                if (cameraReference == null)
                {
                    Debug.Log(System.DateTime.Now.ToLongTimeString() + ": SteamVROmniController(OmniInitialize) - Attempted to Initialize the Omni - developer mode is true and Camera Reference not set in prefab.");
                    return;
                }

                if(SteamVR.instance == null)
                {
                    cameraReference.gameObject.AddComponent<SmoothMouseLook>();
                }
                viveRootObject.localPosition = new Vector3(0, this.GetComponent<CharacterController>().height/2, 0);
                return;
            }

            // Create a new OmniManager and see if we can find the Omni

            omniManager = new OmniManager();

            if (omniManager.FindOmni())
            {
                Debug.Log(System.DateTime.Now.ToLongTimeString() + ": SteamVROmniController(OmniInitialize) - Successfully found the Omni.");
                byte[] logMotionData = OmniCommon.OmniPacketBuilder.buildPacket((byte)OmniCommon.Command.SET_MOTION_DATA_MODE, new byte[] { 0x7F }, 0x00);
                omniManager.SendData(logMotionData);
                omniFound = true;
            }
            else
            {
                Debug.Log(System.DateTime.Now.ToLongTimeString() + ": SteamVROmniController(OmniInitialize) - Attempted to Initialize the Omni, but Omni not found.");
                omniFound = false;
                return;
            }
            
        }

        public override bool CheckInputToCalibrate()
        {
            if (steamVR_TrackedObjects.Length > 0)
            {
                foreach (SteamVR_TrackedObject tObject in steamVR_TrackedObjects)
                {
                    if (tObject.isActiveAndEnabled)
                    {
                        if (SteamVR_Controller.Input((int)tObject.index).GetPressDown(SteamVR_Controller.ButtonMask.Touchpad) || (Input.GetKeyDown(KeyCode.U)) || (Input.GetKeyDown(KeyCode.JoystickButton3)))
                        {
                            Debug.Log("SteamVROmniController(CheckInputToCalibrate) - Calibration Input Received: " + tObject.index.ToString());
                            return true;
                        }
                    }
                }
            }
            return false;
        }

        public override void Calibrate()
        {
            Debug.Log("SteamVROmniController(Calibrate) - MotionData during calibration: " + motionData);
            if (motionData != null)
            {
                Debug.Log("SteamVROmniController(Calibrate) - Calibrating player's forward direction");
                //assume the player's torso and head are in the same direction
                //calculate angle delta between omni and vive HMD
                //store this angle to be applied to Vive Parent
                //float value = 265;
                float value = motionData.RingAngle - cameraReference.localRotation.eulerAngles.y;

                hasAligned = false;
                Debug.Log("SteamVROmniController(Calibrate) - Saving offset value as " + value);
                PlayerPrefs.SetFloat("Vive_Offset", value);
                PlayerPrefs.SetFloat("Vive_OffsetX", cameraReference.transform.localPosition.x);
                PlayerPrefs.SetFloat("Vive_OffsetZ", cameraReference.transform.localPosition.z);
                PlayerPrefs.Save();
            }

            hasCalibrated = true;
        }

        public override void AlignOmni()
        {
            if (!hasAligned)
            {
                //check if has been calibrated
                if (PlayerPrefs.HasKey("Vive_Offset"))
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
                            cameraOffset = PlayerPrefs.GetFloat("Vive_Offset");
                            //offset the vive so that it's coordinates are in-line with the Omni
                            viveRootObject.localPosition = new Vector3(-PlayerPrefs.GetFloat("Vive_OffsetX", 0f), (-1), -PlayerPrefs.GetFloat("Vive_OffsetZ", 0f));
                            viveParentObject.rotation = Quaternion.Euler(0, cameraOffset - motionData.RingAngle + initialRotation, 0);
                            omniOffset = motionData.RingAngle;
                            hasAligned = true;
                        }
                    }
                }
                else
                {
                    return;
                }

                if (!hasFullyInitialized && hasAligned)
                {
                    //grab initial step count here
                    if (motionData != null)
                    {
                        ResetStepCount();
                        hasFullyInitialized = true;
                        Debug.Log(System.DateTime.Now.ToLongTimeString() + ": SteamVROmniController(AlignOmni) - Successfully aligned Omni. Omni has been fully initialized");
                    }
                }
            }
        }

        public override void CharacterControllerMovement()
        {
            if (cameraReference == null)
            {
                Debug.LogError("SteamVROmniController(CharacterControllerMovement) - Camera Reference not set in prefab.");
                return;
            }

            currentCameraRotation = cameraReference.rotation.eulerAngles.y;
            //keep within bounds (0, 360)
            if (currentCameraRotation < 360f) currentCameraRotation += 360f;
            if (currentCameraRotation > 360f) currentCameraRotation -= 360f;

            //calculate angle between camera and omni
            angleBetweenOmniAndCamera = ComputeAngleBetweenControllerAndCamera();
            //examine camera vs omni yaws and determine correct forward yaw as defined by chosen coupling percentage
            float forwardRotation = 0f;
            forwardRotation = currentOmniYaw - omniOffset + initialRotation;
            //keep within bounds (0, 360)
            if (currentOmniYaw > 360f) currentOmniYaw -= 360f;
            if (currentOmniYaw < 0f) currentOmniYaw += 360f;
            //calculate forward rotation

            if (couplingPercentage < 1)
                forwardRotation += ((angleBetweenOmniAndCamera /* - initialRotation*/) * couplingPercentage);
            else
            {               
                forwardRotation = currentCameraRotation;// omniOffset;
            }

            //display forward rotation defined by our coupling percentage
            dummyObject.rotation = Quaternion.Euler(0, forwardRotation, 0);
            dummyForward = dummyObject.forward;
            //calculate forward movement
            Vector3 moveMagnitude = new Vector3(hidInput.y * dummyForward.x, 0, hidInput.y * dummyForward.z);
            //display rounded values
            if (dummyForward.x != 0)
                dummyForward.x = Mathf.Round(dummyForward.x * 100) / 100;
            if (dummyForward.z != 0)
                dummyForward.z = Mathf.Round(dummyForward.z * 100) / 100;
            //perform forward movement
            //apply gravity
            moveMagnitude += Physics.gravity * gravityMultiplier;
            //perform forward movement
            characterController.Move(moveMagnitude * speed * Time.deltaTime);

            //check if there is strafe input
            if (hidInput.x != 0)
            {
                //calculate strafe movement
                moveMagnitude = new Vector3(hidInput.x * dummyObject.right.x, 0, hidInput.x * dummyObject.right.z);
                //apply modifier to reduce strafe movement by a given percentage
                moveMagnitude *= strafeSpeedMultiplier;
                //apply gravity
                moveMagnitude += Physics.gravity * gravityMultiplier;
                //perform strafe movement
                characterController.Move(moveMagnitude * speed * Time.deltaTime);
            }
        }
#endif
    }
}
