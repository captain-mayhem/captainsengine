package de.captain.online;

import android.opengl.GLSurfaceView;
import android.content.Context;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.opengles.GL10;

class AdventureView extends GLSurfaceView{
	private static String TAG = "AdventureView";
	private static final boolean DEBUG = false;
	
	private static class ContextFactory implements GLSurfaceView.EGLContextFactory {
		private static int EGL_CONTEXT_CLIENT_VERSION = 0x3098;
        public EGLContext createContext(EGL10 egl, EGLDisplay display, EGLConfig eglConfig) {
            Log.w(TAG, "creating OpenGL ES 2.0 context");
            checkEglError("Before eglCreateContext", egl);
            int[] attrib_list = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL10.EGL_NONE };
            EGLContext context = egl.eglCreateContext(display, eglConfig, EGL10.EGL_NO_CONTEXT, attrib_list);
            checkEglError("After eglCreateContext", egl);
            return context;
        }

        public void destroyContext(EGL10 egl, EGLDisplay display, EGLContext context) {
            egl.eglDestroyContext(display, context);
        }
	}
	
	private static class ConfigChooser implements GLSurfaceView.EGLConfigChooser {

        public ConfigChooser(int r, int g, int b, int a, int depth, int stencil) {
            mRedSize = r;
            mGreenSize = g;
            mBlueSize = b;
            mAlphaSize = a;
            mDepthSize = depth;
            mStencilSize = stencil;
        }

        /* This EGL config specification is used to specify 2.0 rendering.
         * We use a minimum size of 4 bits for red/green/blue, but will
         * perform actual matching in chooseConfig() below.
         */
        private static int EGL_OPENGL_ES2_BIT = 4;
        private static int[] s_configAttribs2 =
        {
            EGL10.EGL_RED_SIZE, 4,
            EGL10.EGL_GREEN_SIZE, 4,
            EGL10.EGL_BLUE_SIZE, 4,
            EGL10.EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL10.EGL_NONE
        };

        public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display) {

            /* Get the number of minimally matching EGL configurations
             */
            int[] num_config = new int[1];
            egl.eglChooseConfig(display, s_configAttribs2, null, 0, num_config);

            int numConfigs = num_config[0];

            if (numConfigs <= 0) {
                throw new IllegalArgumentException("No configs match configSpec");
            }

            /* Allocate then read the array of minimally matching EGL configs
             */
            EGLConfig[] configs = new EGLConfig[numConfigs];
            egl.eglChooseConfig(display, s_configAttribs2, configs, numConfigs, num_config);

            if (DEBUG) {
                 printConfigs(egl, display, configs);
            }
            /* Now return the "best" one
             */
            return chooseConfig(egl, display, configs);
        }

        public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display,
                EGLConfig[] configs) {
            for(EGLConfig config : configs) {
                int d = findConfigAttrib(egl, display, config,
                        EGL10.EGL_DEPTH_SIZE, 0);
                int s = findConfigAttrib(egl, display, config,
                        EGL10.EGL_STENCIL_SIZE, 0);

                // We need at least mDepthSize and mStencilSize bits
                if (d < mDepthSize || s < mStencilSize)
                    continue;

                // We want an *exact* match for red/green/blue/alpha
                int r = findConfigAttrib(egl, display, config,
                        EGL10.EGL_RED_SIZE, 0);
                int g = findConfigAttrib(egl, display, config,
                            EGL10.EGL_GREEN_SIZE, 0);
                int b = findConfigAttrib(egl, display, config,
                            EGL10.EGL_BLUE_SIZE, 0);
                int a = findConfigAttrib(egl, display, config,
                        EGL10.EGL_ALPHA_SIZE, 0);

                if (r == mRedSize && g == mGreenSize && b == mBlueSize && a == mAlphaSize)
                    return config;
            }
            return null;
        }

        private int findConfigAttrib(EGL10 egl, EGLDisplay display,
                EGLConfig config, int attribute, int defaultValue) {

            if (egl.eglGetConfigAttrib(display, config, attribute, mValue)) {
                return mValue[0];
            }
            return defaultValue;
        }

        private void printConfigs(EGL10 egl, EGLDisplay display,
            EGLConfig[] configs) {
            int numConfigs = configs.length;
            Log.w(TAG, String.format("%d configurations", numConfigs));
            for (int i = 0; i < numConfigs; i++) {
                Log.w(TAG, String.format("Configuration %d:\n", i));
                printConfig(egl, display, configs[i]);
            }
        }

        private void printConfig(EGL10 egl, EGLDisplay display,
                EGLConfig config) {
            int[] attributes = {
                    EGL10.EGL_BUFFER_SIZE,
                    EGL10.EGL_ALPHA_SIZE,
                    EGL10.EGL_BLUE_SIZE,
                    EGL10.EGL_GREEN_SIZE,
                    EGL10.EGL_RED_SIZE,
                    EGL10.EGL_DEPTH_SIZE,
                    EGL10.EGL_STENCIL_SIZE,
                    EGL10.EGL_CONFIG_CAVEAT,
                    EGL10.EGL_CONFIG_ID,
                    EGL10.EGL_LEVEL,
                    EGL10.EGL_MAX_PBUFFER_HEIGHT,
                    EGL10.EGL_MAX_PBUFFER_PIXELS,
                    EGL10.EGL_MAX_PBUFFER_WIDTH,
                    EGL10.EGL_NATIVE_RENDERABLE,
                    EGL10.EGL_NATIVE_VISUAL_ID,
                    EGL10.EGL_NATIVE_VISUAL_TYPE,
                    0x3030, // EGL10.EGL_PRESERVED_RESOURCES,
                    EGL10.EGL_SAMPLES,
                    EGL10.EGL_SAMPLE_BUFFERS,
                    EGL10.EGL_SURFACE_TYPE,
                    EGL10.EGL_TRANSPARENT_TYPE,
                    EGL10.EGL_TRANSPARENT_RED_VALUE,
                    EGL10.EGL_TRANSPARENT_GREEN_VALUE,
                    EGL10.EGL_TRANSPARENT_BLUE_VALUE,
                    0x3039, // EGL10.EGL_BIND_TO_TEXTURE_RGB,
                    0x303A, // EGL10.EGL_BIND_TO_TEXTURE_RGBA,
                    0x303B, // EGL10.EGL_MIN_SWAP_INTERVAL,
                    0x303C, // EGL10.EGL_MAX_SWAP_INTERVAL,
                    EGL10.EGL_LUMINANCE_SIZE,
                    EGL10.EGL_ALPHA_MASK_SIZE,
                    EGL10.EGL_COLOR_BUFFER_TYPE,
                    EGL10.EGL_RENDERABLE_TYPE,
                    0x3042 // EGL10.EGL_CONFORMANT
            };
            String[] names = {
                    "EGL_BUFFER_SIZE",
                    "EGL_ALPHA_SIZE",
                    "EGL_BLUE_SIZE",
                    "EGL_GREEN_SIZE",
                    "EGL_RED_SIZE",
                    "EGL_DEPTH_SIZE",
                    "EGL_STENCIL_SIZE",
                    "EGL_CONFIG_CAVEAT",
                    "EGL_CONFIG_ID",
                    "EGL_LEVEL",
                    "EGL_MAX_PBUFFER_HEIGHT",
                    "EGL_MAX_PBUFFER_PIXELS",
                    "EGL_MAX_PBUFFER_WIDTH",
                    "EGL_NATIVE_RENDERABLE",
                    "EGL_NATIVE_VISUAL_ID",
                    "EGL_NATIVE_VISUAL_TYPE",
                    "EGL_PRESERVED_RESOURCES",
                    "EGL_SAMPLES",
                    "EGL_SAMPLE_BUFFERS",
                    "EGL_SURFACE_TYPE",
                    "EGL_TRANSPARENT_TYPE",
                    "EGL_TRANSPARENT_RED_VALUE",
                    "EGL_TRANSPARENT_GREEN_VALUE",
                    "EGL_TRANSPARENT_BLUE_VALUE",
                    "EGL_BIND_TO_TEXTURE_RGB",
                    "EGL_BIND_TO_TEXTURE_RGBA",
                    "EGL_MIN_SWAP_INTERVAL",
                    "EGL_MAX_SWAP_INTERVAL",
                    "EGL_LUMINANCE_SIZE",
                    "EGL_ALPHA_MASK_SIZE",
                    "EGL_COLOR_BUFFER_TYPE",
                    "EGL_RENDERABLE_TYPE",
                    "EGL_CONFORMANT"
            };
            int[] value = new int[1];
            for (int i = 0; i < attributes.length; i++) {
                int attribute = attributes[i];
                String name = names[i];
                if ( egl.eglGetConfigAttrib(display, config, attribute, value)) {
                    Log.w(TAG, String.format("  %s: %d\n", name, value[0]));
                } else {
                    // Log.w(TAG, String.format("  %s: failed\n", name));
                    while (egl.eglGetError() != EGL10.EGL_SUCCESS);
                }
            }
        }

        // Subclasses can adjust these values:
        protected int mRedSize;
        protected int mGreenSize;
        protected int mBlueSize;
        protected int mAlphaSize;
        protected int mDepthSize;
        protected int mStencilSize;
        private int[] mValue = new int[1];
    }
	
	/*private static*/ class Renderer implements GLSurfaceView.Renderer {
		public Renderer(String adventure){
			mAdventure = adventure;
			AdventureLib.setView(this);
		}
		
        public void onDrawFrame(GL10 gl) {
        	long newtime = System.currentTimeMillis();
        	gl.glClear(GL10.GL_COLOR_BUFFER_BIT);
            AdventureLib.render((int)(newtime-mCurrentTime));
            mCurrentTime = newtime;
        }

        public void onSurfaceChanged(GL10 gl, int width, int height) {
            //AdventureLib.init(width, height);
        	AdventureLib.setWindowDims(width, height);
        	if (mInitialized)
        		return;
        	mWindowWidth = width;
        	mWindowHeight = height;
        	mInitialized = true;
        	String extdir = android.os.Environment.getExternalStorageDirectory().getAbsolutePath();
        	String dir = extdir+"/adventure/"+mAdventure;
        	Log.i(TAG, "Trying to load adventure from "+ dir);
			AdventureLib.init(dir+"/data/game.dat");
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            // Do nothing.
        	mCurrentTime = System.currentTimeMillis();
        }
        
        public void handleMotionEvent(MotionEvent event){
        	int realx = (int)(event.getX()/mRealWidth*mAdvWidth);
        	int realy = (int)(event.getY()/mRealHeight*mAdvHeight);
        	if (event.getAction() == MotionEvent.ACTION_MOVE){
        		AdventureLib.move(realx, realy);
        	}
        	else if (event.getAction() == MotionEvent.ACTION_DOWN){
        		AdventureLib.move(realx, realy);
        		
        	}
        	else if (event.getAction() == MotionEvent.ACTION_UP){
        		//if (event.getDownTime() < 500){
        		//	AdventureLib.leftclick(mXDown, mYDown);
        		//}
        		AdventureLib.move(realx, realy);
        		AdventureLib.leftclick(realx, realy);
        		AdventureLib.leftrelease(realx, realy);
        	}
        }
        
        public void handleKeyDown(int keycode){
        	Log.i(TAG, "Key pressed:"+keycode);
        	AdventureLib.keydown(0);
        }
        
        public void setAdventureDims(int x, int y){
        	mAdvWidth = x;
        	mAdvHeight = y;
        	mRealWidth = mWindowWidth;
        	mRealHeight = mWindowHeight;
        	float aspect = mAdvWidth/(float)mAdvHeight;
        	if (aspect > 1.0f){
        		mRealHeight = (int)(mWindowWidth/aspect);
        		if (mRealHeight > mWindowHeight){
        			mRealHeight = mWindowHeight;
        			mRealWidth = (int)(mWindowHeight*aspect);
        		}
        	}
        	else if (aspect < 1.0f){
        		mRealWidth = (int)(mWindowHeight*aspect);
        		if (mRealWidth > mWindowWidth){
        			mRealWidth = mWindowWidth;
        			mRealHeight = (int)(mWindowWidth*aspect);
        		}
        	}
        }
        
        private long mCurrentTime;
        boolean mInitialized;
        private int mAdvWidth = 640;
    	private int mAdvHeight = 480;
    	private int mWindowWidth;
    	private int mWindowHeight;
    	private int mRealWidth;
    	private int mRealHeight;
    	private String mAdventure;
    	private int mXDown;
    	private int mYDown;
    }
	
	public AdventureView(Context context, String adventure){
		super(context);
		Log.w(TAG, adventure);
		setEGLContextFactory(new ContextFactory());
		setEGLConfigChooser(new ConfigChooser(5, 6, 5, 0, 16, 0));
		Renderer rend = new Renderer(adventure);
		setRenderer(rend);
		mRenderer = rend;
	}
	
	private static void checkEglError(String prompt, EGL10 egl) {
        int error;
        while ((error = egl.eglGetError()) != EGL10.EGL_SUCCESS) {
            Log.e(TAG, String.format("%s: EGL error: 0x%x", prompt, error));
        }
    }
	
	public boolean onTouchEvent(final MotionEvent event){
		if (mRenderer == null)
			return false;
		queueEvent(new Runnable(){
			public void run(){
				mRenderer.handleMotionEvent(event);
			}
		}
		);
		return true;		
	}
	
	public boolean onKeyDown(final int keyCode, KeyEvent event){
		if (mRenderer != null && keyCode == KeyEvent.KEYCODE_MENU){
			queueEvent(new Runnable(){
				public void run(){
					mRenderer.handleKeyDown(keyCode);
				}
			});
			return true;
		}
		return super.onKeyDown(keyCode, event);
	}
	
	private Renderer mRenderer;
}
