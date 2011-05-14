package de.captain.online;

public class AdventureLib{
	static{
		System.loadLibrary("Adventure");
	}
	
	public static native void init(String filename);
	public static native void render(int time);
}
