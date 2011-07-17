package de.captain.online;

public class AdventureLib{
	static{
		System.loadLibrary("Adventure");
	}
	
	public static native void init(String filename);
	public static native void render(int time);
	public static native void move(int x, int y);
	public static native void leftclick(int x, int y);
}
