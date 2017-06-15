using UnityEngine;
using System.Collections;
using System;
using System.Text;

public class SetOmniConnectGamepadMode : MonoBehaviour {


    private string coroutineUrl;
    private string serverGetResponse;
    private string serverPostResponse;
    private string postString;
    private string json;
    private byte[] pData;

    private void Start()
    {
        OmniConnectMode omniConnectMode = new OmniConnectMode();
        omniConnectMode.Data = "Gamepad";
        json = JsonUtility.ToJson(omniConnectMode);
        coroutineUrl = "http://localhost:8085/gamemode";
        pData = Encoding.ASCII.GetBytes(json.ToCharArray());
        StartCoroutine(CoroutineGetRequest(coroutineUrl));
    }

    private IEnumerator CoroutinePostRequest(string url, byte[] postRequest)
    {
        WWW web = new WWW(url, postRequest);
        while (!web.isDone)
        {
            yield return null;
        }

        if (web.error != null)
        {
            serverPostResponse = "Server Error: " + web.error;
            Debug.Log(serverPostResponse);
        }
        else
        {
            serverPostResponse = web.text;
        }
    }

    private IEnumerator CoroutineGetRequest(string url)
    {
        WWW web = new WWW(url);
        while (!web.isDone)
        {
            yield return null;
        }

        if (web.error != null)
        {
            serverGetResponse = "Server Error: " + web.error;
        }
        else
        {
            serverGetResponse = web.text;
            Debug.Log(serverGetResponse);
            CheckGetResponse(serverGetResponse);
        }
    }

    void CheckGetResponse(string getResponse)
    {
        if (getResponse != "Gamepad")
        {
            StartCoroutine(CoroutinePostRequest(coroutineUrl, pData));
        }
    }

}

[Serializable]
public class OmniConnectMode
{
    public string Data;
}
