package org.cbclassic;

import android.content.res.AssetManager;
import android.os.Bundle;
import android.util.Log;

import org.libsdl.app.SDLActivity;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.Writer;
import java.util.zip.ZipEntry;

/**
 * Точка входа Android-версии.
 *
 * Ассеты (картинки, звуки, project.lua, сгенерированные Lua-модули) лежат внутри APK
 * и при первом запуске распаковываются в filesDir/root — дальше движок работает
 * с обычными файлами, как на десктопе.
 */
public class MainActivity extends SDLActivity {
    private static final String TAG = "cbclassic";
    // Версия распакованных ассетов: увеличивайте, когда меняются файлы внутри APK
    // (новый шрифт, правленые картинки) — иначе старая установка их не подхватит.
    private static final int ASSETS_VERSION = 11;

    @Override
    protected String[] getLibraries() {
        return new String[]{"SDL2", "main"};
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        extractAssets();
        super.onCreate(savedInstanceState);
        applyImmersiveMode();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            applyImmersiveMode();
        }
    }

    /** Полноэкранный режим: сцена занимает весь экран без системных полос. */
    private void applyImmersiveMode() {
        if (android.os.Build.VERSION.SDK_INT >= 19) {
            getWindow().getDecorView().setSystemUiVisibility(
                    android.view.View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                            | android.view.View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                            | android.view.View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                            | android.view.View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                            | android.view.View.SYSTEM_UI_FLAG_FULLSCREEN
                            | android.view.View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
        }
    }

    private void extractAssets() {
        // Маркеры: проект, шрифт Roboto и номер версии ассетов. Версия нужна,
        // чтобы после обновления APK новые файлы (например, Roboto) доехали
        // до уже установленного приложения, а не остались старые.
        File root = new File(getFilesDir(), "root");
        File marker = new File(root, "assets/project.lua");
        File font = new File(root, "fonts/Roboto-Regular.ttf");
        File version = new File(root, ".assets-version");
        if (marker.exists() && font.exists() &&
            Integer.toString(ASSETS_VERSION).equals(readText(version))) {
            Log.i(TAG, "ассеты уже распакованы (версия " + ASSETS_VERSION + ")");
            return;
        }
        try {
            long started = System.currentTimeMillis();
            AssetManager am = getAssets();
            copyTree(am, "assets", new File(root, "assets"));
            copyTree(am, "src/lua", new File(root, "src/lua"));
            copyTree(am, "generated/lua", new File(root, "generated/lua"));
            copyTree(am, "fonts", new File(root, "fonts"));
            writeText(version, Integer.toString(ASSETS_VERSION));
            Log.i(TAG, "ассеты распакованы за " + (System.currentTimeMillis() - started) + " мс");
        } catch (IOException e) {
            Log.e(TAG, "не удалось распаковать ассеты", e);
        }
    }

    private static String readText(File file) {
        try (BufferedReader reader = new BufferedReader(new FileReader(file))) {
            String line = reader.readLine();
            return line == null ? "" : line.trim();
        } catch (Exception e) {
            return "";
        }
    }

    private static void writeText(File file, String text) {
        try (Writer writer = new FileWriter(file)) {
            writer.write(text);
        } catch (Exception e) {
            Log.w(TAG, "не удалось записать " + file.getName(), e);
        }
    }

    private void copyTree(AssetManager am, String path, File target) throws IOException {
        String[] children = am.list(path);
        if (children == null || children.length == 0) {
            copyFile(am, path, target);
            return;
        }
        if (!target.exists() && !target.mkdirs()) {
            throw new IOException("не удалось создать " + target);
        }
        for (String child : children) {
            copyTree(am, path + "/" + child, new File(target, child));
        }
    }

    private void copyFile(AssetManager am, String path, File target) throws IOException {
        File parent = target.getParentFile();
        if (parent != null && !parent.exists() && !parent.mkdirs()) {
            throw new IOException("не удалось создать " + parent);
        }
        try (InputStream in = am.open(path);
             OutputStream out = new FileOutputStream(target)) {
            byte[] buffer = new byte[1 << 16];
            int read;
            while ((read = in.read(buffer)) > 0) {
                out.write(buffer, 0, read);
            }
        }
    }
}
