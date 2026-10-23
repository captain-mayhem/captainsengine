package de.captain.cge;

import android.app.NativeActivity;
import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.view.WindowInsets;
import android.view.WindowInsetsController;
import android.window.OnBackInvokedCallback;
import android.window.OnBackInvokedDispatcher;

/**
 * The activity of Captain's Game Engine applications on Android.
 *
 * A NativeActivity whose native library (android.app.lib_name in the
 * manifest) defines android_main with CGE_MAIN. It adds what a
 * NativeActivity can't do from native code, because it has to happen on the
 * UI thread:
 * - immersive full screen: the status and navigation bars are hidden and
 *   come back for a moment with a swipe from the edge;
 * - back: from Android 13 on, back gestures and the back button reach an app
 *   through an OnBackInvokedCallback instead of as a key event. They are
 *   passed to the engine (KEY_ESCAPE), so the application decides what back
 *   does, e.g. opening its pause menu. Before Android 13 back arrives as the
 *   key AKEYCODE_BACK, which the engine also turns into KEY_ESCAPE.
 */
public class CgeActivity extends NativeActivity {
  private OnBackInvokedCallback backCallback;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    if (Build.VERSION.SDK_INT >= 33) {
      backCallback = () -> {
        try {
          nativeBack();
        } catch (UnsatisfiedLinkError e) {
          // the engine hasn't started yet
        }
      };
      getOnBackInvokedDispatcher().registerOnBackInvokedCallback(
          OnBackInvokedDispatcher.PRIORITY_DEFAULT, backCallback);
    }
    hideSystemBars();
  }

  @Override
  protected void onDestroy() {
    if (backCallback != null) {
      getOnBackInvokedDispatcher().unregisterOnBackInvokedCallback(backCallback);
      backCallback = null;
    }
    super.onDestroy();
  }

  @Override
  public void onWindowFocusChanged(boolean hasFocus) {
    super.onWindowFocusChanged(hasFocus);
    // the bars may have come back, e.g. after a dialog of the system
    if (hasFocus)
      hideSystemBars();
  }

  @SuppressWarnings("deprecation")
  private void hideSystemBars() {
    if (Build.VERSION.SDK_INT >= 30) {
      WindowInsetsController controller = getWindow().getInsetsController();
      if (controller != null) {
        controller.hide(WindowInsets.Type.systemBars());
        controller.setSystemBarsBehavior(WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
      }
    } else {
      getWindow().getDecorView().setSystemUiVisibility(
          View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
          | View.SYSTEM_UI_FLAG_FULLSCREEN
          | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
          | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
          | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
          | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION);
    }
  }

  /** a back gesture; registered by the engine (AndroidWindow::registerBackCallback) */
  private static native void nativeBack();
}
