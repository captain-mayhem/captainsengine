package de.captain_online.adventure;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ContentResolver;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.provider.DocumentsContract;
import android.provider.OpenableColumns;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import de.captain.cge.CgeActivity;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

/**
 * Starts the Adventure (de.captain.cge.CgeActivity with libAdventure.so)
 * with a game: its game.dat is the Adventure's first argument.
 *
 * The Adventure reads a game's files (game.dat, gfx.dat, sfx.dat ...) as
 * plain files, so they are copied into the app's storage, in the folder that
 * isn't backed up (they come from the APK or the user's files):
 * - the flavor "game" has its game in the APK's assets (game/). It is copied
 *   to game/ after the app was installed or updated, and started at once;
 *   back from the game ends the app.
 * - the flavor "player" lists the games in games/&lt;name&gt;/. A game is added
 *   from a folder (the Storage Access Framework's ACTION_OPEN_DOCUMENT_TREE)
 *   or from a .zip (ACTION_OPEN_DOCUMENT) with a game.dat, also in a
 *   subfolder (e.g. data/); the folder with the game.dat is copied. A long
 *   press deletes a game.
 * The saved games are in the app's files folder (CGE::UserDirs), named after
 * the game's project, so they stay when a game is deleted.
 */
public class LauncherActivity extends Activity {
  private static final int REQUEST_FOLDER = 1;
  private static final int REQUEST_ZIP = 2;
  private static final String GAME_FILE = "game.dat";

  private File gamesDir;
  private ArrayAdapter<String> games;
  private TextView status;
  private Button addFolder;
  private Button addZip;
  private boolean busy;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    if (hasPackedGame()) {
      preparePackedGame();
      return;
    }
    gamesDir = new File(getNoBackupFilesDir(), "games");
    gamesDir.mkdirs();
    showGameList();
  }

  @Override
  protected void onResume() {
    super.onResume();
    if (games != null)
      updateGameList();
  }

  /** starts the Adventure with the game in dir */
  private void startGame(File dir) {
    Intent intent = new Intent(this, CgeActivity.class);
    intent.putExtra(CgeActivity.EXTRA_ARGUMENTS, new String[] {new File(dir, GAME_FILE).getAbsolutePath()});
    startActivity(intent);
  }

  private int dp(float value) {
    return (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, value, getResources().getDisplayMetrics());
  }

  // The flavor "game"

  private boolean hasPackedGame() {
    try (InputStream in = getAssets().open("game/" + GAME_FILE)) {
      return true;
    } catch (IOException e) {
      return false;
    }
  }

  /** copies the packed game, if this installation hasn't yet, and starts it */
  private void preparePackedGame() {
    File dir = new File(getNoBackupFilesDir(), "game");
    SharedPreferences prefs = getSharedPreferences("launcher", MODE_PRIVATE);
    long installed;
    try {
      installed = getPackageManager().getPackageInfo(getPackageName(), 0).lastUpdateTime;
    } catch (PackageManager.NameNotFoundException e) {
      installed = -1;
    }
    if (prefs.getLong("packedGame", 0) == installed && new File(dir, GAME_FILE).isFile()) {
      startGame(dir);
      finish();
      return;
    }
    LinearLayout layout = new LinearLayout(this);
    layout.setOrientation(LinearLayout.VERTICAL);
    layout.setGravity(Gravity.CENTER);
    layout.addView(new ProgressBar(this));
    TextView text = new TextView(this);
    text.setText(R.string.preparing);
    text.setGravity(Gravity.CENTER);
    text.setPadding(dp(16), dp(16), dp(16), dp(16));
    layout.addView(text);
    setContentView(layout);
    final long stamp = installed;
    new Thread(() -> {
      try {
        deleteRecursively(dir);
        copyAsset("game", dir);
        // only now, so an interrupted copy is done again
        prefs.edit().putLong("packedGame", stamp).commit();
        runOnUiThread(() -> {
          startGame(dir);
          finish();
        });
      } catch (IOException e) {
        runOnUiThread(() -> text.setText(getString(R.string.prepare_failed, e.getMessage())));
      }
    }).start();
  }

  /** copies an asset file or folder */
  private void copyAsset(String path, File target) throws IOException {
    String[] names = getAssets().list(path);
    if (names != null && names.length > 0) {
      if (!target.isDirectory() && !target.mkdirs())
        throw new IOException("can't create " + target);
      for (String name : names)
        copyAsset(path + "/" + name, new File(target, name));
      return;
    }
    try (InputStream in = getAssets().open(path)) {
      copy(in, target);
    } catch (FileNotFoundException e) {
      // an empty folder
      target.mkdirs();
    }
  }

  // The flavor "player"

  private void showGameList() {
    LinearLayout layout = new LinearLayout(this);
    layout.setOrientation(LinearLayout.VERTICAL);
    layout.setPadding(dp(16), dp(16), dp(16), dp(16));

    TextView title = new TextView(this);
    title.setText(R.string.games_title);
    title.setTextSize(TypedValue.COMPLEX_UNIT_SP, 24);
    layout.addView(title);

    status = new TextView(this);
    status.setPadding(0, dp(8), 0, dp(8));
    layout.addView(status);

    ListView list = new ListView(this);
    games = new ArrayAdapter<>(this, android.R.layout.simple_list_item_1, new ArrayList<>());
    list.setAdapter(games);
    list.setOnItemClickListener((parent, view, position, id) -> {
      if (!busy)
        startGame(new File(gamesDir, games.getItem(position)));
    });
    list.setOnItemLongClickListener((parent, view, position, id) -> {
      if (!busy)
        confirmDelete(games.getItem(position));
      return true;
    });
    layout.addView(list, new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, 0, 1));

    LinearLayout buttons = new LinearLayout(this);
    buttons.setOrientation(LinearLayout.HORIZONTAL);
    addFolder = new Button(this);
    addFolder.setText(R.string.add_folder);
    addFolder.setOnClickListener(v -> {
      Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
      startActivityForResult(intent, REQUEST_FOLDER);
    });
    buttons.addView(addFolder, new LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1));
    addZip = new Button(this);
    addZip.setText(R.string.add_zip);
    addZip.setOnClickListener(v -> {
      Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
      intent.addCategory(Intent.CATEGORY_OPENABLE);
      intent.setType("*/*");
      intent.putExtra(Intent.EXTRA_MIME_TYPES, new String[] {"application/zip", "application/x-zip-compressed"});
      startActivityForResult(intent, REQUEST_ZIP);
    });
    buttons.addView(addZip, new LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1));
    layout.addView(buttons);

    // the frame keeps the list out of the system bars (fitting them replaces a
    // view's padding)
    FrameLayout frame = new FrameLayout(this);
    frame.setFitsSystemWindows(true);
    frame.addView(layout);
    setContentView(frame);
    updateGameList();
  }

  /** the games: the folders in games/ with a game.dat */
  private void updateGameList() {
    List<String> names = new ArrayList<>();
    File[] dirs = gamesDir.listFiles();
    if (dirs != null) {
      for (File dir : dirs) {
        if (!dir.getName().startsWith(".") && new File(dir, GAME_FILE).isFile())
          names.add(dir.getName());
      }
    }
    names.sort(String.CASE_INSENSITIVE_ORDER);
    games.clear();
    games.addAll(names);
    if (!busy)
      status.setText(names.isEmpty() ? getString(R.string.no_games) : "");
  }

  private void setBusy(boolean busy, String text) {
    this.busy = busy;
    addFolder.setEnabled(!busy);
    addZip.setEnabled(!busy);
    status.setText(text);
  }

  private void confirmDelete(String name) {
    new AlertDialog.Builder(this)
        .setTitle(getString(R.string.delete_title, name))
        .setMessage(R.string.delete_message)
        .setPositiveButton(R.string.delete, (dialog, which) -> {
          deleteRecursively(new File(gamesDir, name));
          updateGameList();
        })
        .setNegativeButton(R.string.cancel, null)
        .show();
  }

  @Override
  protected void onActivityResult(int requestCode, int resultCode, Intent data) {
    super.onActivityResult(requestCode, resultCode, data);
    if (resultCode != RESULT_OK || data == null || data.getData() == null || games == null)
      return;
    Uri uri = data.getData();
    boolean folder = requestCode == REQUEST_FOLDER;
    String name = folder ? treeName(uri) : zipName(uri);
    setBusy(true, getString(R.string.adding, name));
    // copied into a folder of its own first, so a game is only listed when complete
    File temp = new File(gamesDir, ".adding");
    new Thread(() -> {
      String error = null;
      try {
        deleteRecursively(temp);
        boolean found = folder ? copyTree(uri, temp) : extractZip(uri, temp);
        if (!found)
          error = getString(R.string.no_game_dat);
        else if (!temp.renameTo(uniqueGameDir(name)))
          error = "can't rename " + temp;
      } catch (IOException | RuntimeException e) {
        error = e.getMessage() != null ? e.getMessage() : e.toString();
      }
      if (error != null)
        deleteRecursively(temp);
      final String failure = error;
      runOnUiThread(() -> {
        setBusy(false, "");
        updateGameList();
        Toast.makeText(this, failure == null ? getString(R.string.added, name)
            : getString(R.string.add_failed, name, failure), Toast.LENGTH_LONG).show();
      });
    }).start();
  }

  /** games/name, or name (2) ... if there is one already */
  private File uniqueGameDir(String name) {
    String base = name.replaceAll("[/\\\\:*?\"<>|]", "_").trim();
    if (base.isEmpty() || base.startsWith("."))
      base = "game" + base;
    File dir = new File(gamesDir, base);
    for (int i = 2; dir.exists(); ++i)
      dir = new File(gamesDir, base + " (" + i + ")");
    return dir;
  }

  // a game from a folder (a document tree)

  private static class Document {
    String id;
    String name;
    boolean isFolder;
  }

  private String treeName(Uri tree) {
    Uri root = DocumentsContract.buildDocumentUriUsingTree(tree, DocumentsContract.getTreeDocumentId(tree));
    String name = queryName(root, DocumentsContract.Document.COLUMN_DISPLAY_NAME);
    return name != null ? name : "game";
  }

  private List<Document> children(Uri tree, String id) {
    List<Document> ret = new ArrayList<>();
    Uri uri = DocumentsContract.buildChildDocumentsUriUsingTree(tree, id);
    String[] columns = {DocumentsContract.Document.COLUMN_DOCUMENT_ID,
        DocumentsContract.Document.COLUMN_DISPLAY_NAME, DocumentsContract.Document.COLUMN_MIME_TYPE};
    try (Cursor cursor = getContentResolver().query(uri, columns, null, null, null)) {
      while (cursor != null && cursor.moveToNext()) {
        Document doc = new Document();
        doc.id = cursor.getString(0);
        doc.name = cursor.getString(1);
        doc.isFolder = DocumentsContract.Document.MIME_TYPE_DIR.equals(cursor.getString(2));
        ret.add(doc);
      }
    }
    return ret;
  }

  private static boolean hasGameFile(List<Document> docs) {
    for (Document doc : docs) {
      if (!doc.isFolder && GAME_FILE.equals(doc.name))
        return true;
    }
    return false;
  }

  /** copies the tree's folder with the game.dat (itself or a subfolder) to target; false if there is none */
  private boolean copyTree(Uri tree, File target) throws IOException {
    String id = DocumentsContract.getTreeDocumentId(tree);
    List<Document> docs = children(tree, id);
    if (!hasGameFile(docs)) {
      String found = null;
      for (Document doc : docs) {
        if (doc.isFolder && hasGameFile(children(tree, doc.id))) {
          found = doc.id;
          break;
        }
      }
      if (found == null)
        return false;
      id = found;
    }
    copyDocuments(tree, id, target);
    return true;
  }

  private void copyDocuments(Uri tree, String id, File target) throws IOException {
    if (!target.mkdirs())
      throw new IOException("can't create " + target);
    ContentResolver resolver = getContentResolver();
    for (Document doc : children(tree, id)) {
      File file = new File(target, doc.name);
      if (doc.isFolder) {
        copyDocuments(tree, doc.id, file);
        continue;
      }
      try (InputStream in = resolver.openInputStream(DocumentsContract.buildDocumentUriUsingTree(tree, doc.id))) {
        if (in == null)
          throw new IOException("can't read " + doc.name);
        copy(in, file);
      }
    }
  }

  // a game from a .zip

  private String zipName(Uri uri) {
    String name = queryName(uri, OpenableColumns.DISPLAY_NAME);
    if (name == null)
      return "game";
    int dot = name.lastIndexOf('.');
    return dot > 0 ? name.substring(0, dot) : name;
  }

  /** extracts the zip's folder with the game.dat to target; false if there is none */
  private boolean extractZip(Uri uri, File target) throws IOException {
    // the folder of the game.dat closest to the root
    String prefix = null;
    try (ZipInputStream zip = new ZipInputStream(openUri(uri))) {
      for (ZipEntry entry; (entry = zip.getNextEntry()) != null; ) {
        String name = entry.getName().replace('\\', '/');
        if (entry.isDirectory() || !(name.equals(GAME_FILE) || name.endsWith("/" + GAME_FILE)))
          continue;
        String folder = name.substring(0, name.length() - GAME_FILE.length());
        if (prefix == null || folder.split("/").length < prefix.split("/").length)
          prefix = folder;
      }
    }
    if (prefix == null)
      return false;
    if (!target.mkdirs())
      throw new IOException("can't create " + target);
    String root = target.getCanonicalPath() + File.separator;
    try (ZipInputStream zip = new ZipInputStream(openUri(uri))) {
      for (ZipEntry entry; (entry = zip.getNextEntry()) != null; ) {
        String name = entry.getName().replace('\\', '/');
        if (!name.startsWith(prefix) || name.length() == prefix.length())
          continue;
        File file = new File(target, name.substring(prefix.length()));
        // no entries outside the target ("../")
        if (!file.getCanonicalPath().startsWith(root))
          throw new IOException("bad entry " + name);
        if (entry.isDirectory()) {
          file.mkdirs();
          continue;
        }
        File parent = file.getParentFile();
        if (parent != null)
          parent.mkdirs();
        copy(zip, file);
      }
    }
    return true;
  }

  // helpers

  private InputStream openUri(Uri uri) throws IOException {
    InputStream in = getContentResolver().openInputStream(uri);
    if (in == null)
      throw new IOException("can't read " + uri);
    return in;
  }

  private String queryName(Uri uri, String column) {
    try (Cursor cursor = getContentResolver().query(uri, new String[] {column}, null, null, null)) {
      if (cursor != null && cursor.moveToFirst())
        return cursor.getString(0);
    } catch (RuntimeException e) {
      // no name
    }
    return null;
  }

  /** copies a stream (not closing it) to a file */
  private static void copy(InputStream in, File file) throws IOException {
    byte[] buffer = new byte[64 * 1024];
    try (OutputStream out = new FileOutputStream(file)) {
      for (int read; (read = in.read(buffer)) > 0; )
        out.write(buffer, 0, read);
    }
  }

  private static void deleteRecursively(File file) {
    File[] files = file.listFiles();
    if (files != null) {
      for (File child : files)
        deleteRecursively(child);
    }
    file.delete();
  }
}
