package com.amlogic.pppoe;

public class PppoeOperation 
{
	public static final int PPP_STATUS_CONNECTED = 0x10;
	public static final int PPP_STATUS_DISCONNECTED = 0x20;
	public static final int PPP_STATUS_CONNECTING = 0x40;

	public native boolean connect(String ifname, String account, String passwd);
	public native boolean disconnect();
	public native boolean terminate();
	public native int status(String ifname);
	static {
        System.loadLibrary("pppoejni");
   	}
}
