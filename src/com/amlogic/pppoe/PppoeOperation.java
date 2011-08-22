package com.amlogic.pppoe;

public class PppoeOperation 
{
	public static final int PPP_STATUS_CONNECTED = 0x10;
	public static final int PPP_STATUS_DISCONNECTED = 0x20;
	public static final int PPP_STATUS_CONNECTING = 0x40;

	public native boolean connect(String account, String passwd);
	public native boolean disconnect();
	public native int status();
	static {
        System.loadLibrary("pppoejni");
   	}
}
