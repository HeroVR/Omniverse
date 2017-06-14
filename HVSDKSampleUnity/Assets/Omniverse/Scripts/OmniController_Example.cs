using System.Collections;
using System.Collections.Generic;
using UnityEngine;




/*

    To add Omni Movement to an existing Character, follow the steps outlined below:
        1. Add the OmniMovementComponent Script to your Character.
        2. Inside of your Character Controller Script, create an OmniMovementComponent variable and get the Reference to the Component on Start().
        3. Copy the UseOmniInputToMovePlayer() function to your Character Controller Script.
        4. Call the UseOmniInputToMovePlayer() function in your Character Controller Script's Update() function.


    If you are using the Vive HMD then you will need to make a couple of changes:
        1. The OmniMovementComponent variable will need to be a SteamVROmniMovementComponent variable instead.
            a. Be sure to change the variable type in the GetComponent call on Start() as well.
        2. Uncomment Line 2 in the SteamVROmniMovementComponent Script.
            b. Line 2 is a #define statement for SteamVR. If you don't uncomment this, then the component will not work.

*/


public class OmniController_Example : MonoBehaviour {

    protected OmniMovementComponent MovementComponent;
    private CharacterController m_CharacterController;

    // Use this for initialization
    void Start ()
    {
        MovementComponent = GetComponent<OmniMovementComponent>();
        m_CharacterController = GetComponent<CharacterController>();
    }


    // Update is called once per frame
    void Update ()
    {
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
