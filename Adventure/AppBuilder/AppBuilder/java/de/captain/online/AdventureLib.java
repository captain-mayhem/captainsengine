package de.captain.online;

public class AdventureLib{
	static{
		System.loadLibrary("adventure");
	}
	
	public static native void init(String filename);
}
