package de.captain.online;

public class AdventureLib{
	static{
		System.loadLibrary("Adventure");
	}
	
	public static native boolean init(String filename);
	public static native void render(int time);
	public static native void setWindowDims(int x, int y);
	
	public static native void move(int x, int y);
	public static native void leftclick(int x, int y);
	public static native void leftrelease(int x, int y);
	public static native void keydown(int keycode);
	
	public static void setView(AdventureView.Renderer view){
		mView = view;
	}
	public static void adventureSize(int x, int y){
		mView.setAdventureDims(x, y);
	}
	
	private static AdventureView.Renderer mView;
}
