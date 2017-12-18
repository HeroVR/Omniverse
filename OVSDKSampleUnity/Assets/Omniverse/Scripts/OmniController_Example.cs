using System.Collections;
using System.Collections.Generic;
using UnityEngine.VR;
using UnityEngine;

/*

    To add Omni Movement to an existing Character, follow the steps outlined below:
        1. Add the OmniMovementComponent Script to your Character.
        2. Inside of your Character Controller Script, create an OmniMovementComponent variable and get the Reference to the Component on Start().
        3. Copy the UseOmniInputToMovePlayer() function to your Character Controller Script.
        4. Call the UseOmniInputToMovePlayer() function in your Character Controller Script's Update() function.

*/

public class OmniController_Example : MonoBehaviour {

    protected OmniMovementComponent MovementComponent;
    private CharacterController m_CharacterController;
    private Camera cameraRef;

    private bool RotationCorrected = false;


    // Use this for initialization
    void Start ()
    {
        MovementComponent = GetComponent<OmniMovementComponent>();
        m_CharacterController = GetComponent<CharacterController>();
    }

    void CorrectSpawnForward()
    {
        Debug.Log(VRDevice.model + " connected.");

        RotationCorrected = true;

        Vector3 resultRotation = new Vector3(0.0f, 0.0f, 0.0f);
        Vector3 spawnRotation = transform.rotation.eulerAngles;
        cameraRef = GameObject.FindGameObjectWithTag("MainCamera").GetComponent<Camera>();
        Vector3 cameraRotation = cameraRef.transform.rotation.eulerAngles;

        float spawnRotationYaw = spawnRotation.y;
        float cameraYaw = cameraRotation.y;
        float difference = spawnRotationYaw - cameraYaw;

        resultRotation.y = difference;

        transform.Rotate(resultRotation);
    }


    // Update is called once per frame
    void Update ()
    {
        if(!RotationCorrected)
        {
            if(VRDevice.isPresent)
                CorrectSpawnForward();
        }

        UseOmniInputToMovePlayer();
    }


    // Gets Omni Movement Vectors from the OmniMovementComponent and uses them to Move the Player
    void UseOmniInputToMovePlayer()
    {
        if (MovementComponent.omniFound)
            MovementComponent.GetOmniInputForCharacterMovement();
       else if (MovementComponent.developerMode)
            MovementComponent.DeveloperModeUpdate();


        if (MovementComponent.GetForwardMovement() != Vector3.zero)
            m_CharacterController.Move(MovementComponent.GetForwardMovement());
        if (MovementComponent.GetStrafeMovement() != Vector3.zero)
            m_CharacterController.Move(MovementComponent.GetStrafeMovement());
    }
}
