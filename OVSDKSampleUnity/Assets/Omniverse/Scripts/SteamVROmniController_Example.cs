using System.Collections;
using System.Collections.Generic;
using UnityEngine;



/*

    To add Omni Movement to an existing Character, follow the steps outlined below:
        1. Add the SteamVROmniMovementComponent Script to your Character.
        2. Inside of your Character Controller Script, create an SteamVROmniMovementComponent variable and get the Reference to the Component on Start().
        3. Copy the UseOmniInputToMovePlayer() function to your Character Controller Script.
        4. Call the UseOmniInputToMovePlayer() function in your Character Controller Script's Update() function.
        5. Uncomment Line 2 in the SteamVROmniMovementComponent Script.
            b. Line 2 is a #define statement for SteamVR. If you don't uncomment this, then the component will not work.

*/



using Virtuix;


public class SteamVROmniController_Example : MonoBehaviour {

    protected SteamVROmniMovementComponent MovementComponent;
    private CharacterController m_CharacterController;

    // Use this for initialization
    void Start()
    {
        MovementComponent = GetComponent<SteamVROmniMovementComponent>();
        m_CharacterController = GetComponent<CharacterController>();
    }


    // Update is called once per frame
    void Update()
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
