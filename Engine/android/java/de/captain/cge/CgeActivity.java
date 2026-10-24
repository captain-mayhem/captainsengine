package de.captain.cge;

import android.app.NativeActivity;
import android.content.ClipData;
import android.content.ClipDescription;
import android.content.ClipboardManager;
import android.content.Context;
import android.os.Build;
import android.os.Bundle;
import android.view.PointerIcon;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowInsets;
import android.view.WindowInsetsController;
import android.view.WindowManager;
import android.window.OnBackInvokedCallback;
import android.window.OnBackInvokedDispatcher;

import java.nio.charset.StandardCharsets;

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
 *   key AKEYCODE_BACK, which the engine also turns into KEY_ESCAPE;
 * - the on-screen keyboard (Platform::startTextInput), through an invisible
 *   TextInputView. It covers the bottom of the screen; the window keeps its
 *   size (SOFT_INPUT_ADJUST_NOTHING);
 * - the mouse's pointer icon (Platform::setCursor).
 * The engine also reads and writes the clipboard through it. The native code
 * calls its methods on the engine's thread.
 */
public class CgeActivity extends NativeActivity {
  private OnBackInvokedCallback backCallback;
  private TextInputView textInput;
  /** created on the UI thread, used on the engine's */
  private ClipboardManager clipboard;

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
    textInput = new TextInputView(this);
    addContentView(textInput, new ViewGroup.LayoutParams(1, 1));
    getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_NOTHING);
    clipboard = (ClipboardManager) getSystemService(Context.CLIPBOARD_SERVICE);
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

  /** shows or hides the on-screen keyboard (Platform::startTextInput/stopTextInput) */
  public void setTextInput(boolean active) {
    runOnUiThread(() -> textInput.setActive(active));
  }

  /** replaces the clipboard's text; UTF-8, since JNI's strings are modified UTF-8 */
  public void setClipboardText(byte[] utf8) {
    if (clipboard != null)
      clipboard.setPrimaryClip(ClipData.newPlainText(null, new String(utf8, StandardCharsets.UTF_8)));
  }

  /** the clipboard's text as UTF-8, null if it holds none */
  public byte[] getClipboardText() {
    if (clipboard == null)
      return null;
    ClipData clip = clipboard.getPrimaryClip();
    if (clip == null || clip.getItemCount() == 0)
      return null;
    CharSequence text = clip.getItemAt(0).coerceToText(this);
    return text != null ? text.toString().getBytes(StandardCharsets.UTF_8) : null;
  }

  /** does the clipboard hold text? Without reading it, which Android 12 and later report to the user */
  public boolean hasClipboardText() {
    if (clipboard == null || !clipboard.hasPrimaryClip())
      return false;
    ClipDescription description = clipboard.getPrimaryClipDescription();
    return description != null && (description.hasMimeType(ClipDescription.MIMETYPE_TEXT_PLAIN)
        || description.hasMimeType(ClipDescription.MIMETYPE_TEXT_HTML));
  }

  /** the mouse's pointer icon over the window, a PointerIcon.TYPE_ (0 hides it) */
  public void setPointerIcon(int type) {
    runOnUiThread(() -> getWindow().getDecorView().setPointerIcon(PointerIcon.getSystemIcon(this, type)));
  }

  // registered by the engine (AndroidWindow::registerNatives)
  /** a back gesture */
  private static native void nativeBack();
  /** a key of the on-screen keyboard (a KeyEvent.KEYCODE_), pressed and released */
  static native void nativeKey(int keyCode);
  /** text of the on-screen keyboard */
  static native void nativeText(byte[] utf8);
}
