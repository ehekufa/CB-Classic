package org.cbclassic;

import android.content.res.AssetManager;
import android.os.Bundle;
import android.util.Log;

import org.libsdl.app.SDLActivity;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
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

    @Override
    protected String[] getLibraries() {
        return new String[]{"SDL2", "main"};
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        extractAssets();
        super.onCreate(savedInstanceState);
    }

    private void extractAssets() {
        File marker = new File(getFilesDir(), "root/assets/project.lua");
        if (marker.exists()) {
            Log.i(TAG, "ассеты уже распакованы");
            return;
        }
        File root = new File(getFilesDir(), "root");
        try {
            long started = System.currentTimeMillis();
            AssetManager am = getAssets();
            copyTree(am, "assets", new File(root, "assets"));
            copyTree(am, "src/lua", new File(root, "src/lua"));
            copyTree(am, "generated/lua", new File(root, "generated/lua"));
            Log.i(TAG, "ассеты распакованы за " + (System.currentTimeMillis() - started) + " мс");
        } catch (IOException e) {
            Log.e(TAG, "не удалось распаковать ассеты", e);
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
