package de.captain.cge;

import android.content.Context;
import android.text.InputType;
import android.view.KeyEvent;
import android.view.View;
import android.view.inputmethod.BaseInputConnection;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputMethodManager;

import java.nio.charset.StandardCharsets;

/**
 * An invisible view that takes the on-screen keyboard's input for the engine
 * (Platform::startTextInput).
 *
 * A NativeActivity's own view has no input connection, so a keyboard could
 * only send key events, which many keyboards don't do for text. This view
 * has one: committed text goes to the engine as characters, deleting as
 * backspace and delete keys, the keyboard's action (enter, done) as the enter
 * key. A word the keyboard is still composing is typed at once and replaced
 * with backspaces when it changes. The keyboard is asked for plain text
 * without suggestions, so most keyboards commit every character right away.
 *
 * The engine keeps the text, not this view, so the keyboard sees an empty
 * field and sends a backspace key event when there is nothing for it to
 * delete; key events of the keyboard go to the native activity's input queue
 * like a hardware keyboard's.
 */
class TextInputView extends View {
  /** the composing text the engine has typed */
  private String composing = "";

  TextInputView(Context context) {
    super(context);
    setFocusable(true);
    setFocusableInTouchMode(true);
  }

  @Override
  public boolean onCheckIsTextEditor() {
    return true;
  }

  @Override
  public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
    outAttrs.inputType = InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_VISIBLE_PASSWORD
        | InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS;
    // the game stays visible: no full-screen editor in landscape
    outAttrs.imeOptions = EditorInfo.IME_FLAG_NO_FULLSCREEN | EditorInfo.IME_FLAG_NO_EXTRACT_UI
        | EditorInfo.IME_ACTION_DONE;
    composing = "";
    return new Connection();
  }

  /** shows the keyboard (again, if the user closed it) or hides it; on the UI thread */
  void setActive(boolean active) {
    InputMethodManager imm = (InputMethodManager) getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
    if (imm == null)
      return;
    if (active) {
      if (!hasFocus()) {
        requestFocus();
        imm.restartInput(this);
      }
      // after the focus change has reached the input method
      post(() -> imm.showSoftInput(this, 0));
    } else {
      imm.hideSoftInputFromWindow(getWindowToken(), 0);
      clearFocus();
      composing = "";
    }
  }

  private static void key(int keyCode) {
    try {
      CgeActivity.nativeKey(keyCode);
    } catch (UnsatisfiedLinkError e) {
      // the engine hasn't started yet
    }
  }

  /** types text: line breaks are the enter key */
  private static void type(CharSequence text) {
    String s = text.toString();
    int start = 0;
    for (int i = 0; i <= s.length(); ++i) {
      if (i < s.length() && s.charAt(i) != '\n')
        continue;
      if (i > start) {
        try {
          CgeActivity.nativeText(s.substring(start, i).getBytes(StandardCharsets.UTF_8));
        } catch (UnsatisfiedLinkError e) {
          return;
        }
      }
      if (i < s.length())
        key(KeyEvent.KEYCODE_ENTER);
      start = i + 1;
    }
  }

  private class Connection extends BaseInputConnection {
    Connection() {
      super(TextInputView.this, false);
    }

    /** replaces the composing text the engine has typed with text: deletes
     *  and types what comes after their common beginning */
    private void replaceComposing(CharSequence text) {
      String s = text.toString();
      int same = 0;
      while (same < composing.length() && same < s.length() && composing.charAt(same) == s.charAt(same))
        ++same;
      // not in the middle of a surrogate pair
      if (same > 0 && Character.isHighSurrogate(s.charAt(same - 1)))
        --same;
      for (int i = composing.codePointCount(same, composing.length()); i > 0; --i)
        key(KeyEvent.KEYCODE_DEL);
      type(s.substring(same));
    }

    @Override
    public boolean commitText(CharSequence text, int newCursorPosition) {
      replaceComposing(text);
      composing = "";
      return true;
    }

    @Override
    public boolean setComposingText(CharSequence text, int newCursorPosition) {
      replaceComposing(text);
      composing = text.toString();
      return true;
    }

    @Override
    public boolean finishComposingText() {
      composing = "";
      return true;
    }

    @Override
    public boolean deleteSurroundingText(int beforeLength, int afterLength) {
      // the engine's text fields delete whole characters
      for (int i = 0; i < beforeLength; ++i)
        key(KeyEvent.KEYCODE_DEL);
      for (int i = 0; i < afterLength; ++i)
        key(KeyEvent.KEYCODE_FORWARD_DEL);
      return true;
    }

    @Override
    public boolean deleteSurroundingTextInCodePoints(int beforeLength, int afterLength) {
      return deleteSurroundingText(beforeLength, afterLength);
    }

    @Override
    public boolean performEditorAction(int actionCode) {
      key(KeyEvent.KEYCODE_ENTER);
      return true;
    }
  }
}
