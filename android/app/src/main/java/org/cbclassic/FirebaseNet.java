package org.cbclassic;

import android.util.Log;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.StandardCharsets;

/**
 * Сеть для нативной части игры: запросы к Firebase Realtime Database.
 *
 * Готовая библиотека Firebase (com.google.firebase:firebase-database) сюда не
 * подключена осознанно: она требует Gradle-плагина google-services и файла
 * google-services.json с ключом проекта, а игра собирается без них. Realtime
 * Database при этом полностью доступна по официальному REST-протоколу — им и
 * ходим: те же правила базы, тот же адрес, никаких ключей.
 *
 * Метод вызывается из C++ (src/cpp/fbnet.cpp) на отдельном потоке, поэтому
 * внутри обычный блокирующий HttpURLConnection с таймаутами.
 */
public final class FirebaseNet {
    private static final String TAG = "cbclassic";
    private static final int TIMEOUT_MS = 5000;

    private FirebaseNet() {
    }

    /** method = GET/PUT/PATCH/DELETE, url — полный адрес узла с ".json". */
    public static String request(String method, String url, String body) {
        HttpURLConnection connection = null;
        try {
            connection = (HttpURLConnection) new URL(url).openConnection();
            connection.setRequestMethod(method);
            connection.setConnectTimeout(TIMEOUT_MS);
            connection.setReadTimeout(TIMEOUT_MS);
            connection.setRequestProperty("Content-Type", "application/json");
            connection.setRequestProperty("Accept", "application/json");
            if (body != null && !body.isEmpty() && !"GET".equals(method)) {
                byte[] data = body.getBytes(StandardCharsets.UTF_8);
                connection.setDoOutput(true);
                connection.setFixedLengthStreamingMode(data.length);
                try (OutputStream out = connection.getOutputStream()) {
                    out.write(data);
                }
            }
            final int code = connection.getResponseCode();
            InputStream in = code >= 400 ? connection.getErrorStream() : connection.getInputStream();
            final String text = readAll(in);
            if (code >= 200 && code < 300) {
                return text;
            }
            Log.w(TAG, "Firebase " + method + " " + url + " -> HTTP " + code + ": " + text);
            return null;
        } catch (Exception e) {
            Log.w(TAG, "Firebase " + method + " " + url + " не удался: " + e);
            return null;
        } finally {
            if (connection != null) {
                connection.disconnect();
            }
        }
    }

    private static String readAll(InputStream in) throws IOException {
        if (in == null) {
            return "";
        }
        try (InputStream stream = in) {
            ByteArrayOutputStream buffer = new ByteArrayOutputStream();
            byte[] chunk = new byte[4096];
            int n;
            while ((n = stream.read(chunk)) > 0) {
                buffer.write(chunk, 0, n);
            }
            return buffer.toString("UTF-8");
        }
    }
}
