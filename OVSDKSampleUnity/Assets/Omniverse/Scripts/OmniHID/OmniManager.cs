using UnityEngine;
using System.Collections.Generic;
using System;
using System.Runtime.InteropServices;

public class OmniManager
{
    private const ushort vendor_id_omni = 0x29eb;
    private const ushort product_id_omni = 0x00ff;

    private IntPtr hidapi_handle;

    private byte[] inputBuf = new byte[65];
    private byte[] outputBuf = new byte[65];

    public bool omniDisconnected = true;


    public bool FindOmni()
    {
        Debug.Log(System.DateTime.Now.ToLongTimeString() + ": OmniManager(FindOmni) - Trying to find the Omni");
        ushort vendor = 0;
        ushort product = 0;

        vendor = vendor_id_omni;
        product = product_id_omni;

        IntPtr ptr = HIDapi.hid_enumerate(vendor, product);
        IntPtr cur_ptr = ptr;

        if (ptr == IntPtr.Zero)
            return false;

        hid_device_info enumerate = (hid_device_info)Marshal.PtrToStructure(ptr, typeof(hid_device_info));

        bool found = false;

        while (cur_ptr != IntPtr.Zero)
        {
            IntPtr handle = HIDapi.hid_open_path(enumerate.path);

            if (enumerate.path.Contains("mi_04"))
            {
                hidapi_handle = handle;
                HIDapi.hid_set_nonblocking(hidapi_handle, 1);
                found = true;
                omniDisconnected = false;
                break;
            }

            if (enumerate.path.Contains("mi_00"))
            {
                hidapi_handle = handle;
                HIDapi.hid_set_nonblocking(hidapi_handle, 1);
                found = true;
                omniDisconnected = false;
                break;
            }

            cur_ptr = enumerate.next;
            if (cur_ptr != IntPtr.Zero)
                enumerate = (hid_device_info)Marshal.PtrToStructure(cur_ptr, typeof(hid_device_info));
        }
        HIDapi.hid_free_enumeration(ptr);

        Debug.Log(System.DateTime.Now.ToLongTimeString() + ": OmniManager(FindOmni) - Result of trying to find the Omni = " + found);
        return found;
    }

    public void Cleanup()
    {
        if (hidapi_handle != IntPtr.Zero)
		{
            HIDapi.hid_close(hidapi_handle);
			hidapi_handle = IntPtr.Zero;
		}            
    }

    public int ReadData(ref byte[] packet) 
    {
        if (omniDisconnected)   //if disconnected, don't do anything
            return -2;

        if (hidapi_handle == IntPtr.Zero) return -2;

        Array.Clear(inputBuf, 0, 65);

        HIDapi.hid_read(hidapi_handle, inputBuf, new UIntPtr(Convert.ToUInt32(inputBuf.Length)));


        if(inputBuf[0] != 0xEF) //Checking for invalid message
        {
            Debug.LogError(System.DateTime.Now.ToLongTimeString() + ": OmniManager(ReadData) - The Omni has been diconnected and is no longer being detected.");
            omniDisconnected = true;
            Cleanup();
        }

        if (inputBuf[2] == 169) // hex A9
        {
            packet = new byte[inputBuf[1]];
            Buffer.BlockCopy(inputBuf, 0, packet, 0, inputBuf[1]);
        } else
        {
            return -2; //pods off or write data
        }

        return inputBuf[1];
    }
}
