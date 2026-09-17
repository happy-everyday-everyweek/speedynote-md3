package org.speedynote.app;

import android.content.Context;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.nio.charset.StandardCharsets;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

/**
 * Installs a process-wide uncaught-exception handler that persists the last
 * crash to a file, then delegates to the previous handler (the default one
 * kills the process). Purely diagnostic: it never swallows a crash.
 *
 * The report is written twice:
 *   - files/crash_last.txt (app-private)
 *   - Android/data/org.speedynote.app/files/crash_last.txt (user-visible
 *     through any file manager, no root needed)
 */
public final class CrashLogger {

    private static final String TAG = "CrashLogger";
    private static final String FILE_NAME = "crash_last.txt";
    private static volatile boolean sInstalled = false;

    private CrashLogger() {
    }

    public static void install(final Context context) {
        if (sInstalled) {
            return;
        }
        sInstalled = true;

        final Context appContext = context.getApplicationContext();
        final Thread.UncaughtExceptionHandler previous =
                Thread.getDefaultUncaughtExceptionHandler();

        Thread.setDefaultUncaughtExceptionHandler(new Thread.UncaughtExceptionHandler() {
            @Override
            public void uncaughtException(Thread thread, Throwable throwable) {
                try {
                    writeReport(appContext, thread, throwable);
                } catch (Throwable ignored) {
                    // Never mask the original crash.
                }
                if (previous != null) {
                    previous.uncaughtException(thread, throwable);
                }
            }
        });
    }

    private static void writeReport(Context appContext, Thread thread, Throwable throwable) {
        StringWriter sw = new StringWriter();
        PrintWriter pw = new PrintWriter(sw);
        pw.println("time: " + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss", Locale.US)
                .format(new Date()));
        pw.println("thread: " + thread.getName());
        pw.println("package: " + appContext.getPackageName());
        pw.println("---");
        throwable.printStackTrace(pw);
        pw.flush();
        byte[] data = sw.toString().getBytes(StandardCharsets.UTF_8);

        // 1) App-private copy.
        try {
            FileOutputStream out = new FileOutputStream(
                    new File(appContext.getFilesDir(), FILE_NAME));
            out.write(data);
            out.close();
        } catch (Throwable ignored) {
        }

        // 2) User-visible copy (app-specific external storage).
        try {
            File ext = appContext.getExternalFilesDir(null);
            if (ext != null) {
                FileOutputStream out = new FileOutputStream(new File(ext, FILE_NAME));
                out.write(data);
                out.close();
            }
        } catch (Throwable ignored) {
        }

        Log.e(TAG, "Crash recorded to " + FILE_NAME, throwable);
    }
}
