package de.captain.cge;

import android.app.NativeActivity;
import android.content.ActivityNotFoundException;
import android.content.ClipData;
import android.content.ClipDescription;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.provider.OpenableColumns;
import android.view.PointerIcon;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowInsets;
import android.view.WindowInsetsController;
import android.view.WindowManager;
import android.webkit.MimeTypeMap;
import android.window.OnBackInvokedCallback;
import android.window.OnBackInvokedDispatcher;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

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
 * - the mouse's pointer icon (Platform::setCursor);
 * - importing and exporting files (Platform::importFile, exportFile) with
 *   the system's document picker (ACTION_OPEN_DOCUMENT and
 *   ACTION_CREATE_DOCUMENT), which needs no permissions. The file is copied
 *   on a thread of its own; the engine polls the outcome (pollTransfer).
 * The engine also reads and writes the clipboard through it, and takes the
 * application's arguments from the starting intent: the String array extra
 * EXTRA_ARGUMENTS becomes argv[1] and on (IApplication::init), e.g. the game
 * that a launcher activity of the application picked. The native code calls
 * its methods on the engine's thread.
 */
public class CgeActivity extends NativeActivity {
  /** the intent's extra with the application's arguments, a String[] */
  public static final String EXTRA_ARGUMENTS = "de.captain.cge.ARGUMENTS";

  private OnBackInvokedCallback backCallback;
  private TextInputView textInput;
  /** created on the UI thread, used on the engine's */
  private ClipboardManager clipboard;

  // the file transfer (Platform::TransferStatus's values)
  private static final int TRANSFER_NONE = 0;
  private static final int TRANSFER_RUNNING = 1;
  private static final int TRANSFER_DONE = 2;
  private static final int TRANSFER_CANCELLED = 3;
  private static final int TRANSFER_FAILED = 4;
  private static final int REQUEST_IMPORT = 0x4347;
  private static final int REQUEST_EXPORT = 0x4348;
  private final Object transferLock = new Object();
  private int transferStatus = TRANSFER_NONE;
  private String transferName = "";
  /** an import's folder and whether it replaces a file; an export's file */
  private String transferFolder;
  private boolean transferReplace;
  private String transferPath;

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

  /** the application's arguments from the starting intent (EXTRA_ARGUMENTS), each as UTF-8 */
  public byte[][] getArguments() {
    String[] args = getIntent() != null ? getIntent().getStringArrayExtra(EXTRA_ARGUMENTS) : null;
    if (args == null)
      return new byte[0][];
    byte[][] utf8 = new byte[args.length][];
    for (int i = 0; i < args.length; ++i)
      utf8[i] = (args[i] != null ? args[i] : "").getBytes(StandardCharsets.UTF_8);
    return utf8;
  }

  /** the mouse's pointer icon over the window, a PointerIcon.TYPE_ (0 hides it) */
  public void setPointerIcon(int type) {
    runOnUiThread(() -> getWindow().getDecorView().setPointerIcon(PointerIcon.getSystemIcon(this, type)));
  }

  // --- importing and exporting files ---

  /** starts a transfer; false if one runs */
  private boolean startTransfer() {
    synchronized (transferLock) {
      if (transferStatus == TRANSFER_RUNNING)
        return false;
      transferStatus = TRANSFER_RUNNING;
      transferName = "";
      return true;
    }
  }

  private void finishTransfer(int status, String name) {
    synchronized (transferLock) {
      transferStatus = status;
      transferName = name != null ? name : "";
    }
  }

  /** the transfer's status in the first byte and the name (UTF-8) after it;
   * an ended transfer is told once (Platform::pollTransfer) */
  public byte[] pollTransfer() {
    synchronized (transferLock) {
      byte[] name = transferName.getBytes(StandardCharsets.UTF_8);
      byte[] result = new byte[1 + name.length];
      result[0] = (byte) transferStatus;
      System.arraycopy(name, 0, result, 1, name.length);
      if (transferStatus != TRANSFER_RUNNING)
        transferStatus = TRANSFER_NONE;
      return result;
    }
  }

  /** the MIME types of extensions (".zip,.txt"); none (any file) for none or an unknown one */
  private static String[] mimeTypes(String accept) {
    List<String> types = new ArrayList<>();
    for (String ext : accept.split(",")) {
      ext = ext.trim();
      if (ext.startsWith("."))
        ext = ext.substring(1);
      if (ext.isEmpty())
        continue;
      String type = MimeTypeMap.getSingleton().getMimeTypeFromExtension(ext.toLowerCase());
      if (type == null)
        return new String[0];
      types.add(type);
      // what some providers call a zip
      if (type.equals("application/zip"))
        types.add("application/x-zip-compressed");
    }
    // files whose type a provider doesn't know
    if (!types.isEmpty())
      types.add("application/octet-stream");
    return types.toArray(new String[0]);
  }

  /** lets the user choose a file, which is copied into folder (Platform::importFile) */
  public boolean importFile(byte[] folder, byte[] accept, boolean replace) {
    if (!startTransfer())
      return false;
    transferFolder = new String(folder, StandardCharsets.UTF_8);
    transferReplace = replace;
    String[] types = mimeTypes(new String(accept, StandardCharsets.UTF_8));
    runOnUiThread(() -> {
      Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
      intent.addCategory(Intent.CATEGORY_OPENABLE);
      intent.setType("*/*");
      if (types.length > 0)
        intent.putExtra(Intent.EXTRA_MIME_TYPES, types);
      try {
        startActivityForResult(intent, REQUEST_IMPORT);
      } catch (ActivityNotFoundException e) {
        finishTransfer(TRANSFER_FAILED, null);
      }
    });
    return true;
  }

  /** lets the user choose where the file goes (Platform::exportFile) */
  public boolean exportFile(byte[] path) {
    if (!startTransfer())
      return false;
    transferPath = new String(path, StandardCharsets.UTF_8);
    String name = new File(transferPath).getName();
    int dot = name.lastIndexOf('.');
    String type = dot > 0 ? MimeTypeMap.getSingleton().getMimeTypeFromExtension(name.substring(dot + 1).toLowerCase()) : null;
    String mime = type != null ? type : "application/octet-stream";
    runOnUiThread(() -> {
      Intent intent = new Intent(Intent.ACTION_CREATE_DOCUMENT);
      intent.addCategory(Intent.CATEGORY_OPENABLE);
      intent.setType(mime);
      intent.putExtra(Intent.EXTRA_TITLE, name);
      try {
        startActivityForResult(intent, REQUEST_EXPORT);
      } catch (ActivityNotFoundException e) {
        finishTransfer(TRANSFER_FAILED, null);
      }
    });
    return true;
  }

  @Override
  protected void onActivityResult(int requestCode, int resultCode, Intent data) {
    if (requestCode != REQUEST_IMPORT && requestCode != REQUEST_EXPORT) {
      super.onActivityResult(requestCode, resultCode, data);
      return;
    }
    Uri uri = data != null ? data.getData() : null;
    if (resultCode != RESULT_OK || uri == null) {
      finishTransfer(TRANSFER_CANCELLED, null);
      return;
    }
    // copied away from the UI thread
    new Thread(() -> {
      if (requestCode == REQUEST_IMPORT)
        importFrom(uri);
      else
        exportTo(uri);
    }).start();
  }

  /** the document's name, or null */
  private String displayName(Uri uri) {
    try (Cursor cursor = getContentResolver().query(uri, new String[] {OpenableColumns.DISPLAY_NAME}, null, null, null)) {
      if (cursor != null && cursor.moveToFirst())
        return cursor.getString(0);
    } catch (RuntimeException e) {
      // no name
    }
    return null;
  }

  /** the name a file chosen as name gets in folder (Platform::importedName) */
  private static String importedName(File folder, String name, boolean replace) {
    StringBuilder clean = new StringBuilder();
    for (int i = 0; name != null && i < name.length(); ++i) {
      char c = name.charAt(i);
      clean.append(c == '/' || c == '\\' || c < 0x20 || c == 0x7f ? '_' : c);
    }
    String result = clean.toString();
    if (result.isEmpty() || result.equals(".") || result.equals(".."))
      result = "upload";
    if (replace)
      return result;
    int dot = result.lastIndexOf('.');
    String stem = dot > 0 ? result.substring(0, dot) : result;
    String ext = dot > 0 ? result.substring(dot) : "";
    String free = result;
    for (int n = 2; new File(folder, free).exists(); ++n)
      free = stem + " (" + n + ")" + ext;
    return free;
  }

  private static void copy(InputStream in, OutputStream out) throws IOException {
    byte[] buffer = new byte[65536];
    int n;
    while ((n = in.read(buffer)) > 0)
      out.write(buffer, 0, n);
  }

  private void importFrom(Uri uri) {
    File folder = new File(transferFolder);
    folder.mkdirs();
    String name = importedName(folder, displayName(uri), transferReplace);
    // a replaced file only goes once the new one is complete
    File temporary = new File(folder, ".import.tmp");
    try (InputStream in = getContentResolver().openInputStream(uri);
         OutputStream out = new FileOutputStream(temporary)) {
      if (in == null)
        throw new IOException("no stream");
      copy(in, out);
    } catch (IOException | SecurityException e) {
      temporary.delete();
      finishTransfer(TRANSFER_FAILED, null);
      return;
    }
    if (!temporary.renameTo(new File(folder, name))) {
      temporary.delete();
      finishTransfer(TRANSFER_FAILED, null);
      return;
    }
    finishTransfer(TRANSFER_DONE, name);
  }

  private void exportTo(Uri uri) {
    try (InputStream in = new FileInputStream(transferPath);
         OutputStream out = getContentResolver().openOutputStream(uri, "wt")) {
      if (out == null)
        throw new IOException("no stream");
      copy(in, out);
    } catch (IOException | SecurityException e) {
      finishTransfer(TRANSFER_FAILED, null);
      return;
    }
    String name = displayName(uri);
    finishTransfer(TRANSFER_DONE, name != null ? name : new File(transferPath).getName());
  }

  // registered by the engine (AndroidWindow::registerNatives)
  /** a back gesture */
  private static native void nativeBack();
  /** a key of the on-screen keyboard (a KeyEvent.KEYCODE_), pressed and released */
  static native void nativeKey(int keyCode);
  /** text of the on-screen keyboard */
  static native void nativeText(byte[] utf8);
}
