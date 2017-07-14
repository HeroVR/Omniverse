using UnityEngine;
using System;
using System.IO;
using System.Threading;
using System.Runtime.InteropServices;
using System.Text;
using UnityEngine.VR;
using UnityEngine.UI;

public delegate IntPtr WndProcDelegate(IntPtr hWnd, uint msg, IntPtr wParam, IntPtr lParam);

public class OmniMovementComponent : MonoBehaviour {

    [Header("-----Movement Options-----")]


    [Tooltip("Affects overall speed forward, back, left, right.")]
    public float maxSpeed = 10;

    [Range(0.0f, 1.0f)]
    [Tooltip("Multiplier to reduce strafing speed (Strafing at full speed can feel too fast).")]
    public float strafeSpeedMultiplier = 1.0f;

    [Range(0.0f, 1.0f)]
    [Tooltip("Multiplier to reduce backwards speed (Moving backwards at full speed can feel too fast).")]
    public float backwardsSpeedMultiplier = 1.0f;

    [Tooltip("Multiplier for gravity for character controller.")]
    public float gravityMultiplier = 1;

    [HideInInspector]
    [Range(0.0f, 1.0f)]
    [Tooltip("Fully coupled to camera = 100%, Fully decoupled (follows torso/ring angle = 0%.")]
    public float couplingPercentage = 1.0f;

    [Tooltip("Set to True if you want to use a joystick or WASD for testing instead of the Omni and no HMD. Please uncheck when you do a full build for release.")]
    public bool developerMode = false;

    [Header("Output and Calculated Variables. Can be used to debug or poll during game.")]
    [HideInInspector]
    public Vector2 hidInput; // holds x and y values for movement from the controller
    [HideInInspector]
    public float currentOmniYaw;
    [HideInInspector]
    public int currentStepCount;
    [HideInInspector]
    public float omniOffset = 0f;
    [HideInInspector]
    public float angleBetweenOmniAndCamera;


    //Has the Omni been found
    [HideInInspector]
    public bool omniFound = false;


    [Header("-----GameObject References-----")]
    public Transform cameraReference;
    public Transform dummyObject;

    protected Vector3 dummyForward;

    protected CharacterController characterController;
    protected bool hasCalibrated = false;
    protected int startingStepCount;

    protected OmniManager omniManager;

    private const int SOP = (int)OmniCommon.packet_start_end.OMNI_SOP;
    private const int EOP = (int)OmniCommon.packet_start_end.OMNI_EOP;

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
    private float joystickDeadzone = 0.05f;

    private Vector3 forwardMovement;
    private Vector3 strafeMovement;


    #region reconnecting logic
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
        Debug.Log(System.DateTime.Now.ToLongTimeString() + ": OmniMovementComponent- Omniverse SDK Version 1.0.01");

        OmniInitialize();

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

    public Vector3 GetForwardMovement()
    {
        return forwardMovement;
    }

    public Vector3 GetStrafeMovement()
    {
        return strafeMovement;
    }

    //angle difference between camera and omni angle, used for decoupled effect
    protected float ComputeAngleBetweenControllerAndCamera()
    {
        float cameraYaw = cameraReference.rotation.eulerAngles.y;
        float adjustedOmniYaw = currentOmniYaw - omniOffset + transform.rotation.eulerAngles.y;
        float angleBetweenControllerAndCamera = 0f;
        angleBetweenControllerAndCamera = Mathf.Abs(cameraYaw - adjustedOmniYaw) % 360;
        angleBetweenControllerAndCamera = angleBetweenControllerAndCamera > 180 ? 360 - angleBetweenControllerAndCamera : angleBetweenControllerAndCamera;

        //calculate sign
        float sign = (cameraYaw - adjustedOmniYaw >= 0 && cameraYaw - adjustedOmniYaw <= 180) ||
            (cameraYaw - adjustedOmniYaw <= -180 && cameraYaw - adjustedOmniYaw >= -360) ? 1 : -1;

        angleBetweenControllerAndCamera *= sign;

        return angleBetweenControllerAndCamera;
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
        Debug.Log(System.DateTime.Now.ToLongTimeString() + ": OmniMovementComponent(OnDisable) - Uninstall Hook");
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
                Debug.LogError(System.DateTime.Now.ToLongTimeString() + ": OmniMovementComponent(OmniInitialize) - Attempted to Initialize the Omni - developer mode is true and Camera Reference not set in prefab.");
                return;
            }

            if (!VRDevice.isPresent)
            {
                cameraReference.gameObject.AddComponent<SmoothMouseLook>();
                Vector3 adjustedCameraPosition = cameraReference.localPosition;
                adjustedCameraPosition.y = 1.5f;
                cameraReference.position = adjustedCameraPosition;
            }
            return;
        }

        // Create a new OmniManager and see if we can find the Omni
        omniManager = new OmniManager();

        if (omniManager.FindOmni())
        {
            Debug.LogError(System.DateTime.Now.ToLongTimeString() + ": OmniMovementComponent(OmniInitialize) - Successfully found the Omni.");
            omniFound = true;
        }
        else
        {
            Debug.LogError(System.DateTime.Now.ToLongTimeString() + ": OmniMovementComponent(OmniInitialize) - Attempted to Initialize the Omni, but Omni not found.");
            omniFound = false;
            return;
        }
    }



    void AttemptToReconnectTheOmni()
    {
        if (omniManager == null)
            omniManager = new OmniManager();

        if (!omniManager.omniDisconnected)
            return;

        if (omniManager.FindOmni())
        {
            Debug.LogError(System.DateTime.Now.ToLongTimeString() + ": OmniMovementComponent(AttemptToReconnectTheOmni) - Successfully found the Omni for reconnect.");
            omniFound = true;
        }
        else
        {
            Debug.LogError(System.DateTime.Now.ToLongTimeString() + ": OmniMovementComponent(AttemptToReconnectTheOmni) - Attempted to Reconnect the Omni, but Omni not found.");
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
            OmniCommon.Messages.OmniBaseMessage obm = OmniCommon.OmniPacketBuilder.decodePacket(packet);

            if (obm != null)
            {
                switch (obm.MsgType)
                {
                    case OmniCommon.Messages.MessageType.OmniMotionAndRawDataMessage:
                        OmniCommon.Messages.OmniMotionAndRawDataMessage OmniMotionAndRawData = new OmniCommon.Messages.OmniMotionAndRawDataMessage(obm);
                        motionData = OmniMotionAndRawData.GetMotionData();
                        break;
                    default:
                        break;
                }
            }
        }
        else
        {
            motionData = null;
            Debug.LogError(System.DateTime.Now.ToLongTimeString() + ": OmniMovementComponent(ReadOmniData) - During this frame, failed to read the Omni data packet.");
        }

        if (motionData != null)
        {
            /*
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
            }*/

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
    /// Checks joystick or WASD for input if in developer mode
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
            inputX = 0;
        }

        hidInput.y = inputY;
        hidInput.x = inputX;
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
    /// Calibration Function. Sets up proper alignment between Omni and HMD based on Omni input data and HMD orientation.
    /// </summary>
    public virtual void CalibrateOmni()
    {
        if (!hasCalibrated)
        {
            //set offset to be current ring angle
            if (motionData != null)
            {
                if ((cameraReference.transform.position.x != 0) && (cameraReference.transform.position.y != 0) && (cameraReference.transform.position.z != 0) &&
                            (cameraReference.rotation.eulerAngles.x != 0) && (cameraReference.rotation.eulerAngles.y != 0) && (cameraReference.rotation.eulerAngles.z != 0))
                {
                    omniOffset = OVSDK.GetOmniYawOffset();
                    hasCalibrated = true;
                    Debug.Log(System.DateTime.Now.ToLongTimeString() + ": OmniMovementComponent(CalibrateOmni) - Successfully calibrated Omni.");
                }
                if (!hasFullyInitialized)
                {
                    //grab initial step count here
                    ResetStepCount();
                    hasFullyInitialized = true;

                }
            }
        }
    }


    public void GetOmniInputForCharacterMovement()
    {
        if (cameraReference == null)
        {
            Debug.LogError("OmniMovementComponent(GetOmniInputForCharacterMovement) - Camera Reference not set in prefab.");
            return;
        }

        forwardMovement = new Vector3(0.0f, 0.0f, 0.0f);
        strafeMovement = forwardMovement;

        //calculate angle between camera and omni
        angleBetweenOmniAndCamera = ComputeAngleBetweenControllerAndCamera();

        float forwardRotation = 0f;

        //keep within bounds (0, 360)
        if (currentOmniYaw > 360f) currentOmniYaw -= 360f;
        if (currentOmniYaw < 0f) currentOmniYaw += 360f;

        //Get the coupling percentage from Omniverse
        couplingPercentage = developerMode ? 1.0f : OVSDK.GetOmniCoupleRate();


        //calculate forward rotation
        forwardRotation = (currentOmniYaw - omniOffset + transform.rotation.eulerAngles.y) + (angleBetweenOmniAndCamera * couplingPercentage);

        //display forward rotation defined by our coupling percentage
        dummyObject.rotation = Quaternion.Euler(0, forwardRotation, 0);
        dummyForward = dummyObject.forward;
        
        //calculate forward movement
        Vector3 movementInputVector = new Vector3(hidInput.y * dummyForward.x, 0, hidInput.y * dummyForward.z);

        //apply multiplier to reduce backwards movement speed by a given percentage
        if (hidInput.y < 0) { movementInputVector *= backwardsSpeedMultiplier; }
         
        //display rounded values
        if (dummyForward.x != 0)
            dummyForward.x = Mathf.Round(dummyForward.x * 100) / 100;
        if (dummyForward.z != 0)
            dummyForward.z = Mathf.Round(dummyForward.z * 100) / 100;

        //apply gravity
        movementInputVector += Physics.gravity * gravityMultiplier;
        //perform forward movement
        forwardMovement = (movementInputVector * maxSpeed * Time.deltaTime);


        //check if there is strafe input
        if (hidInput.x != 0)
        {
            //calculate strafe movement
            movementInputVector = new Vector3(hidInput.x * dummyObject.right.x, 0, hidInput.x * dummyObject.right.z);
            //apply modifier to reduce strafe movement by a given percentage
            movementInputVector *= strafeSpeedMultiplier;
            //apply gravity
            movementInputVector += Physics.gravity * gravityMultiplier;
            //perform strafe movement
            strafeMovement = (movementInputVector * maxSpeed * Time.deltaTime);
        }
    }

    void Update()
    {
        if (!developerMode)
        {
            ReadOmniData();
            CalibrateOmni();
        }
    }

    public virtual void DeveloperModeUpdate()
    {
        CheckInputForMovementTesting();
        GetOmniInputForCharacterMovement();
    }
}