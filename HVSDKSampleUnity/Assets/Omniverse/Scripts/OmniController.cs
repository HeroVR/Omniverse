using UnityEngine;
using System;
using System.IO;
using System.Threading;
using System.Runtime.InteropServices;
using System.Text;
using UnityEngine.VR;
using UnityEngine.UI;

/* This is the basic version of the Omni Controller. 
 * Tested and working with consumer Oculus HMD and Oculus SDK 1.3.
 * In order to use this with other HMDs or for your own project you can derive from this class and override the specific parts that are needed.
 * Check out the SteamVROmniController as an example. 
 */

public delegate IntPtr WndProcDelegate(IntPtr hWnd, uint msg, IntPtr wParam, IntPtr lParam);

namespace Virtuix
{
    public class OmniController : MonoBehaviour
    {
        [Header("Movement options.")]
        [Tooltip("Check if you want to use a joystick or WASD for testing instead of the Omni and no HMD. Please uncheck when you do a full build for release.")]
        public bool developerMode = false;
        [Tooltip("Adds arrow to show heading for debugging.")]
        public bool showForwardArrowDebug = false;
        [Tooltip("Adds text for debugging.")]
        public bool showDebugText = false;
        [Tooltip("Joystick deadzone value.")]
        [Range(0.05f, 1.0f)]
        public float joystickDeadzone = 0.05f;
        [Range(0.0f, 1.0f)]
        [Tooltip("Fully coupled to camera = 100%, Fully decoupled (follows torso/ring angle = 0%.")]
        public float couplingPercentage = 1.0f;
        [Tooltip("Affects overall speed forward,back,left,right.")]
        public float speed = 10;
        [Range(0.0f, 1.0f)]
        [Tooltip("Multiplier to reduce strafing distance (pure, 100% input strafe can feel too fast).")]
        public float strafeSpeedMultiplier = 0.8f;
        [Tooltip("Multiplier for gravity for character controller.")]
        public float gravityMultiplier = 1;
        [Header("Output and Calculated Variables. Can be used to debug or poll during game.")]
        public Vector2 hidInput; // holds x and y values for movement from the controller
        public float currentOmniYaw;
        public int currentStepCount;
        public float cameraOffset = 0f;
        public float omniOffset = 0f;
        public float initialRotation = 0f;
        public float currentCameraRotation;
        public float angleBetweenOmniAndCamera;
        [Header("Has Omni been found?")]
        public bool omniFound = false;
        public bool hasCalibrated;
        public Text debugTextUI;


        [Header("GameObject References")]
        public Transform cameraReference;
        public Transform dummyObject;

        protected Vector3 dummyForward;

        protected CharacterController characterController;
        protected bool hasAligned = false;
        protected int startingStepCount;

        protected OmniManager omniManager;
  
        private const int SOP = (int)OmniCommon.packet_start_end.OMNI_SOP;
        private const int EOP = (int)OmniCommon.packet_start_end.OMNI_EOP;

        private static OmniCommon.Messages.OmniBaseMessage obm;
        private static OmniCommon.Messages.OmniRawDataMessage ordm;
        private static OmniCommon.Messages.OmniMotionDataMessage omdm;

        /* Disable unused variable warnings */
#pragma warning disable 0414
        protected static OmniCommon.Messages.OmniRawData rawData;
        protected static OmniCommon.Messages.OmniMotionData motionData;
#pragma warning restore 0414

        protected byte rawX = 0;
        protected byte rawY = 0;
        protected float omniX = 0;
        protected float omniY = 0;

        protected bool hasFullyInitialized = false;

        private int debugCounterForDataMessages = 0;

        //Input delay for re-firing input for calibration

        private float inputDeltaTime = 0f;
        private float inputRefreshDelay = 0f;

        #region reconnecting stuff
        IntPtr hMainWindow;
        IntPtr oldWndProcPtr;
        IntPtr newWndProcPtr;
        WndProcDelegate newWndProc;
        bool isrunning = false;

        [DllImport("user32.dll", CharSet = CharSet.Auto, ExactSpelling = true)]
        public static extern System.IntPtr GetForegroundWindow();

        [DllImport("user32.dll")]
        static extern IntPtr SetWindowLongPtr(IntPtr hWnd, int nIndex, IntPtr dwNewLong);

        [DllImport("user32.dll")]
        static extern IntPtr CallWindowProc(IntPtr lpPrevWndFunc, IntPtr hWnd, uint Msg, IntPtr wParam, IntPtr lParam);

        IntPtr wndProc(IntPtr hWnd, uint msg, IntPtr wParam, IntPtr lParam)
        {
            if (msg == 0x0219)
            {
                //Debug.LogError("Event Triggered; " + wParam.ToInt32());// + "; " + lParam.ToInt32());
                switch (wParam.ToInt32())
                {
                    case 0x8000: // DBT_DEVICEARRIVAL
                                 //case 0x8004: // DBT_DEVICEREMOVECOMPLETE
                                 //case 0x0007: // DBT_DEVNODES_CHANGED:
                        AttemptToReconnectTheOmni();
                        break;
                }
            }
            return CallWindowProc(oldWndProcPtr, hWnd, msg, wParam, lParam);
        }
        #endregion
        
        void Start()
        {
            
            OmniInitialize();
                
            //Setup character controller
            CharacterControllerInit();

            if (!showForwardArrowDebug)
                HideForwardDebug();

            Start_TryingtoReconnect();
        }

        void Start_TryingtoReconnect()
        {
            if (isrunning) return;

            hMainWindow = GetForegroundWindow();
            newWndProc = new WndProcDelegate(wndProc);
            newWndProcPtr = Marshal.GetFunctionPointerForDelegate(newWndProc);
            oldWndProcPtr = SetWindowLongPtr(hMainWindow, -4, newWndProcPtr);
            isrunning = true;
        }

        void Update()
        {
            //Own OmniUpdate call so we can override if needed.

            if (developerMode)
                DeveloperModeUpdate();
            else
            {
                if (!omniManager.omniDisconnected)
                    OmniUpdate();
            }
        }


        void DebugText()
        {
            if(!debugTextUI.transform.parent.gameObject.activeInHierarchy)
            {
                debugTextUI.transform.parent.gameObject.SetActive(true);
            }

            debugTextUI.text = "Omni X: " + omniX + "\nOmni Y: " + omniY + "\nOmni Yaw: " + currentOmniYaw + "\nStep Count: " + currentStepCount + "\nCalibrated: " + hasCalibrated;

        }

        void HideForwardDebug()
        {
            GameObject forward = GameObject.Find("ForwardIndicator");
            forward.SetActive(false);
        }

        //angle difference between camera and omni angle, used for decoupled effect
        protected float ComputeAngleBetweenControllerAndCamera()
        {
            float retVal = 0f;
            float forwardRotation = currentOmniYaw - omniOffset;
            Vector3 v = cameraReference.forward;
            Vector3 d = transform.forward;
            v.Normalize();
            //rotate forward vector around omni yaw
            d = Quaternion.Euler(0, forwardRotation, 0) * d;
            d.Normalize();

            //compute angle difference
            retVal = Mathf.Acos(Vector3.Dot(v, d));
            retVal = retVal * Mathf.Rad2Deg;
            float direction = Vector3.Dot(Vector3.Cross(v, d), Vector3.up);

            if (direction > 0)
                retVal = -retVal;
            if (direction < 0)
                retVal = Mathf.Abs(retVal);
            return retVal;
        }

        //called on exit
        void OnApplicationQuit()
        {
            StopOmni();
        }

        //called on scene change
        void OnDisable()
        {
            StopOmni();

            // Disable the reconnect attempting
            Debug.Log(System.DateTime.Now.ToLongTimeString() + ": OmniController(OnDisable) - Uninstall Hook");
            if (!isrunning) return;
            SetWindowLongPtr(hMainWindow, -4, oldWndProcPtr);
            hMainWindow = IntPtr.Zero;
            oldWndProcPtr = IntPtr.Zero;
            newWndProcPtr = IntPtr.Zero;
            newWndProc = null;
            isrunning = false;
        }

        void StopOmni()
        {
            if (omniManager != null)
            {
                omniManager.Cleanup();
                omniManager = null; 
            }
        }

        /// <summary>
        /// Finds the Omni so we can start reading data.
        /// </summary>
        public virtual void OmniInitialize()
        {
            if (developerMode)
            {
                if (cameraReference == null)
                {
                    Debug.Log(System.DateTime.Now.ToLongTimeString() + ": OmniController(OmniInitialize) - Attempted to Initialize the Omni - developer mode is true and Camera Reference not set in prefab.");
                    return;
                }

                if(!VRDevice.isPresent)
                {
                    cameraReference.gameObject.AddComponent<SmoothMouseLook>();
                }

                return;
            }

            // Create a new OmniManager and see if we can find the Omni

            omniManager = new OmniManager();

            if (omniManager.FindOmni())
            {
                Debug.Log(System.DateTime.Now.ToLongTimeString() + ": OmniController(OmniInitialize) - Successfully found the Omni.");
                byte[] logMotionData = OmniCommon.OmniPacketBuilder.buildPacket((byte)OmniCommon.Command.SET_MOTION_DATA_MODE, new byte[] { 0x7F }, 0x00);
                omniManager.SendData(logMotionData);
                omniFound = true;
            }
            else
            {
                Debug.Log(System.DateTime.Now.ToLongTimeString() + ": OmniController(OmniInitialize) - Attempted to Initialize the Omni, but Omni not found.");
                omniFound = false;
                return;
            }
        }



        void AttemptToReconnectTheOmni()
        {            
            if(omniManager == null)
                omniManager = new OmniManager();

            if (!omniManager.omniDisconnected)
                return;

            if (omniManager.FindOmni())
            {
                Debug.Log(System.DateTime.Now.ToLongTimeString() + ": OmniController(AttemptToReconnectTheOmni) - Successfully found the Omni for reconnect.");
                byte[] logMotionData = OmniCommon.OmniPacketBuilder.buildPacket((byte)OmniCommon.Command.SET_MOTION_DATA_MODE, new byte[] { 0x7F }, 0x00);
                omniManager.SendData(logMotionData);
                omniFound = true;
            }
            else
            {
                Debug.Log(System.DateTime.Now.ToLongTimeString() + ": OmniController(AttemptToReconnectTheOmni) - Attempted to Reconnect the Omni, but Omni not found.");
                omniFound = false;
                return;
            }
        }





        /// <summary>
        /// Pulls packets from the Omni and decodes them to populate the class variables and motiondata.
        /// </summary>
        void ReadOmniData()
        {
            byte[] packet = null;

            if (omniManager.ReadData(ref packet) > 0)
            {
                obm = OmniCommon.OmniPacketBuilder.decodePacket(packet);

                if (obm != null)
                {
                    switch (obm.MsgType)
                    {
                        case OmniCommon.Messages.MessageType.OmniRawDataMessage:
                            
                            ordm = new OmniCommon.Messages.OmniRawDataMessage(obm);
                            rawData = ordm.GetRawData();
                            break;
                        case OmniCommon.Messages.MessageType.OmniMotionDataMessage:
                            omdm = new OmniCommon.Messages.OmniMotionDataMessage(obm);
                            motionData = omdm.GetMotionData();
                            break;
                        default:
                            break;
                    }
                }
            }
            else
            {
                motionData = null;
                Debug.Log(System.DateTime.Now.ToLongTimeString() + ": OmniController(ReadOmniData) - During this frame, failed to read the Omni data packet.");
            }

            if (motionData != null)
            {
                debugCounterForDataMessages++;
                if (debugCounterForDataMessages > 30)
                {
                    Debug.Log(System.DateTime.Now.ToLongTimeString() + 
                        ": Timestamp = " + motionData.Timestamp + 
                        "; Step count = " + motionData.StepCount + 
                        "; Ring Angle = " + motionData.RingAngle + 
                        "; Ring Delta = " + motionData.RingDelta + 
                        "; joy x = " + motionData.GamePad_X + 
                        "; joy y = " + motionData.GamePad_Y);
                    debugCounterForDataMessages = 0;
                }

                currentOmniYaw = motionData.RingAngle;

                rawX = motionData.GamePad_X;
                rawY = motionData.GamePad_Y;
                omniX = motionData.GamePad_X;
                omniY = motionData.GamePad_Y;

                if (omniX > 0)
                {
                    omniX = (omniX / 255.0f) * 2f - 1f;
                }
                else
                    omniX = -1f;

                if (omniY > 0)
                {
                    omniY = (omniY / 255.0f) * 2f - 1f;
                }
                else
                    omniY = -1;

                //clamp '0'
                if (rawX == 127)
                {
                    omniX = 0f;
                }
                if (rawY == 127)
                {
                    omniY = 0f;
                }
                //@TODO - may need to change this for fully coupled mode...
                omniY *= -1f;
                hidInput = new Vector2(omniX, omniY);

                if (hasFullyInitialized) UpdateStepCountFromMotionData();
            }
            else
            {
                hidInput = Vector2.zero;
            }
        }


        /// <summary>
        /// Checks to see if the key or button to calibrate has been pressed.
        /// Pulled out so we can override what keys to check based on HMD/controller combination.
        /// </summary>
        /// <returns>Returns true if pressed</returns>

        public virtual bool CheckInputToCalibrate()
        {
            return (Input.GetKey(KeyCode.U));
        }

        /// <summary>
        /// Calls CheckInputToCalibrate to see if we need to calibrate the HMD to the Omni this frame.
        /// </summary>
        void CheckInputForCalibration()
        {
            inputDeltaTime += Time.deltaTime;

            if (CheckInputToCalibrate())
            {
                if (inputDeltaTime >= inputRefreshDelay)
                {
                    Calibrate();
                    inputDeltaTime = 0f;
                }
            }
        }

        /// <summary>
        /// Checks joystick or WASD for input if 
        /// </summary>
        void CheckInputForMovementTesting()
        {
            //Get values from joystick or WASD.
            float inputX = Input.GetAxis("Horizontal");
            float inputY = Input.GetAxis("Vertical");

            //Clamp to deadzone

            if (inputY < joystickDeadzone && inputY > 0 || inputY > -joystickDeadzone && inputY < 0)
            {
                inputY = 0;
            }

            if (inputX < joystickDeadzone && inputX > 0 || inputX > -joystickDeadzone && inputX < 0)
            {
                inputY = 0;
            }

            hidInput.y = inputY;
            hidInput.x = inputX;
            
        }

        /// <summary>
        /// This is called when user hits calibate key.
        /// This is to setup the orientation of the Omni to match the HMD.
        /// In basic version we just clear the alignment and then the next
        /// loop it is aligned by the Align function.
        /// </summary>
        public virtual void Calibrate()
        {
            if (motionData != null)
            {
                Debug.Log(System.DateTime.Now.ToLongTimeString() + ": OmniController(Calibrate) - Calibrating player's forward direction");
                //Will align to correct calibration next time it hits align;
                hasAligned = false;
                hasCalibrated = true;
            }
        }

        /// <summary>
        /// Updates step count from motion data after getting the initial step count in the align.
        /// </summary>
        protected void UpdateStepCountFromMotionData()
        {
            if (motionData == null) return;
            currentStepCount = (int)motionData.StepCount - startingStepCount;
        }

        /// <summary>
        /// Resets step counter to 0. Call at anytime you want to reset the step count.
        /// </summary>
        public void ResetStepCount()
        {
            if (motionData == null) return;
            startingStepCount = (int)motionData.StepCount;
            currentStepCount = 0;
        }

        /// <summary>
        /// Second half of Calibration. Sets up proper alignment between Omni and HMD based on Omni input data and HMD orientation.
        /// </summary>
        public virtual void AlignOmni()
        {
            if (!hasAligned)
            {
                //rift enters with 0 rotation
                //set offset to be current ring angle
                if (motionData != null)
                {
                    if (hasFullyInitialized)
                    {
                        initialRotation = dummyObject.transform.rotation.eulerAngles.y;
                        transform.rotation = dummyObject.transform.rotation;
                    }
                    cameraOffset = motionData.RingAngle;
                    omniOffset = motionData.RingAngle;
                    InputTracking.Recenter();
                    hasAligned = true;
                }
                if (!hasFullyInitialized)
                {
                    //grab initial step count here
                    if (motionData != null)
                    {
                        ResetStepCount();
                        hasFullyInitialized = true;
                        Debug.Log(System.DateTime.Now.ToLongTimeString() + ": OmniController(AlignOmni) - Successfully aligned Omni. Omni has been fully initialized");
                    }
                }
            }
        }


        /// <summary>
        /// Init Character controller here.
        /// </summary>
        public virtual void CharacterControllerInit()
        {
            characterController = GetComponent<CharacterController>();
            initialRotation = transform.rotation.eulerAngles.y;
        }

        /// <summary>
        /// This moves our character controller based on input data. The HMD setup is parented on top of the controller.
        /// </summary>
        public virtual void CharacterControllerMovement()
        {
            if (cameraReference == null)
            {
                Debug.Log("OmniController(CharacterControllerMovement) - Camera Reference not set in prefab.");
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
                forwardRotation = currentCameraRotation;// cameraOffset;
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

        /// <summary>
        /// Main update loop for Omni.
        /// </summary>
        public virtual void OmniUpdate()
        {
            if (!omniFound)
            {
                Debug.Log(System.DateTime.Now.ToLongTimeString() + ": OmniController(OmniUpdate) - Omni Not found.");
                return;
            }

            CheckInputForCalibration();
            ReadOmniData();
            AlignOmni();
            CharacterControllerMovement();

            if (showDebugText)
            {
                DebugText();
            }
            else
            {
                if (debugTextUI.transform.parent.gameObject.activeInHierarchy)
                {
                    debugTextUI.transform.parent.gameObject.SetActive(false);
                }
            }


        }

        public virtual void DeveloperModeUpdate()
        {
            CheckInputForMovementTesting();
            CharacterControllerMovement();
        }


    }
}
