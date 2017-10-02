using UnityEngine;
using System.Collections;

public class OVInitialization : MonoBehaviour {

    private void Awake()
    {
        DontDestroyOnLoad(transform.gameObject);
    }

    // Use this for initialization
    void Start()
    {
        if (!OVSDK.HasInitialized())
        {
            //Replace the GameID and GameKey below with the values provided to you for this game by Virtuix.
            //If you have not received a unique GameID or GameKey, please contact Virtuix to register your game.
            OVSDK.Init(0000, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", "omni=1");
        }
    }
}
