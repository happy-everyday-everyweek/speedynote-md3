package org.speedynote.app;

import android.content.Context;
import android.util.Log;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.Locale;

/**
 * Reads the Qt notebook library from the app-private data directory.
 *
 * The Qt side stores everything in two places:
 *  - &lt;AppDataLocation&gt;/notebook_library.json - library metadata
 *  - &lt;CacheLocation&gt;/thumbnails/&lt;documentId&gt;.png - cached page previews
 *
 * AppDataLocation resolves to the app's files dir on Android; a couple of
 * candidate locations are probed to stay robust against Qt path changes.
 */
public final class NoteLibraryReader {

    private static final String TAG = "NoteLibraryReader";
    private static final String LIBRARY_FILE = "notebook_library.json";
    private static final String THUMB_DIR = "thumbnails";

    private NoteLibraryReader() {
    }

    public static List<NoteItem> load(Context context) {
        List<NoteItem> result = new ArrayList<>();
        File libraryFile = findLibraryFile(context);
        if (libraryFile == null) {
            Log.i(TAG, "notebook_library.json not found yet");
            return result;
        }

        File thumbDir = findThumbnailDir(context);

        try (BufferedReader reader = new BufferedReader(new InputStreamReader(
                new FileInputStream(libraryFile), StandardCharsets.UTF_8))) {
            StringBuilder sb = new StringBuilder();
            String line;
            while ((line = reader.readLine()) != null) {
                sb.append(line);
            }
            JSONObject root = new JSONObject(sb.toString());
            JSONArray notebooks = root.optJSONArray("notebooks");
            if (notebooks == null) {
                return result;
            }
            for (int i = 0; i < notebooks.length(); i++) {
                JSONObject obj = notebooks.optJSONObject(i);
                if (obj == null) {
                    continue;
                }
                NoteItem item = new NoteItem();
                item.bundlePath = obj.optString("path", "");
                if (item.bundlePath.isEmpty()) {
                    continue;
                }
                item.name = obj.optString("name", "");
                if (item.name.isEmpty()) {
                    item.name = deriveName(item.bundlePath);
                }
                item.documentId = obj.optString("documentId", "");
                item.starredFolder = obj.optString("starredFolder", "");
                item.lastModified = parseIsoDate(obj.optString("lastModified", ""));
                item.lastAccessed = parseIsoDate(obj.optString("lastAccessed", ""));
                item.isStarred = obj.optBoolean("isStarred", false);
                item.isPdfBased = obj.optBoolean("isPdfBased", false);
                item.isEdgeless = obj.optBoolean("isEdgeless", false);

                if (thumbDir != null && !item.documentId.isEmpty()) {
                    File thumb = new File(thumbDir, item.documentId + ".png");
                    if (thumb.isFile()) {
                        item.thumbnailPath = thumb.getAbsolutePath();
                    }
                }
                result.add(item);
            }
        } catch (Exception e) {
            Log.w(TAG, "Failed to read notebook library", e);
        }

        Collections.sort(result, new Comparator<NoteItem>() {
            @Override
            public int compare(NoteItem a, NoteItem b) {
                return Long.compare(b.lastAccessed, a.lastAccessed);
            }
        });
        return result;
    }

    private static String deriveName(String bundlePath) {
        String name = bundlePath;
        int slash = Math.max(name.lastIndexOf('/'), name.lastIndexOf('\\'));
        if (slash >= 0 && slash < name.length() - 1) {
            name = name.substring(slash + 1);
        }
        String lower = name.toLowerCase(Locale.ROOT);
        if (lower.endsWith(".snbx")) {
            name = name.substring(0, name.length() - 5);
        } else if (lower.endsWith(".snb")) {
            name = name.substring(0, name.length() - 4);
        }
        return name;
    }

    private static long parseIsoDate(String value) {
        if (value == null || value.isEmpty()) {
            return 0L;
        }
        String[] patterns = {
                "yyyy-MM-dd'T'HH:mm:ss",
                "yyyy-MM-dd'T'HH:mm:ss'Z'",
                "yyyy-MM-dd'T'HH:mm:ss.SSS",
                "yyyy-MM-dd'T'HH:mm:ss.SSS'Z'",
                "yyyy-MM-dd"
        };
        for (String p : patterns) {
            try {
                SimpleDateFormat fmt = new SimpleDateFormat(p, Locale.US);
                fmt.setLenient(true);
                java.util.Date d = fmt.parse(value);
                if (d != null) {
                    return d.getTime();
                }
            } catch (Exception ignored) {
                // try the next pattern
            }
        }
        return 0L;
    }

    private static File findLibraryFile(Context context) {
        File direct = new File(context.getFilesDir(), LIBRARY_FILE);
        if (direct.isFile()) {
            return direct;
        }
        File found = searchForFile(context.getFilesDir(), LIBRARY_FILE, 2);
        if (found != null) {
            return found;
        }
        File dataDir = context.getFilesDir().getParentFile();
        if (dataDir != null) {
            return searchForFile(dataDir, LIBRARY_FILE, 2);
        }
        return null;
    }

    private static File findThumbnailDir(Context context) {
        File direct = new File(context.getCacheDir(), THUMB_DIR);
        if (direct.isDirectory()) {
            return direct;
        }
        File found = searchForDir(context.getCacheDir(), THUMB_DIR, 2);
        if (found != null) {
            return found;
        }
        File dataDir = context.getFilesDir().getParentFile();
        if (dataDir != null) {
            File cache = new File(dataDir, "cache");
            File nested = new File(cache, THUMB_DIR);
            if (nested.isDirectory()) {
                return nested;
            }
            return searchForDir(cache, THUMB_DIR, 2);
        }
        return null;
    }

    private static File searchForFile(File base, String name, int depth) {
        if (base == null || depth < 0 || !base.isDirectory()) {
            return null;
        }
        File[] children = base.listFiles();
        if (children == null) {
            return null;
        }
        for (File c : children) {
            if (c.isFile() && name.equals(c.getName())) {
                return c;
            }
        }
        for (File c : children) {
            if (c.isDirectory()) {
                File r = searchForFile(c, name, depth - 1);
                if (r != null) {
                    return r;
                }
            }
        }
        return null;
    }

    private static File searchForDir(File base, String name, int depth) {
        if (base == null || depth < 0 || !base.isDirectory()) {
            return null;
        }
        File[] children = base.listFiles();
        if (children == null) {
            return null;
        }
        for (File c : children) {
            if (c.isDirectory() && name.equals(c.getName())) {
                return c;
            }
        }
        for (File c : children) {
            if (c.isDirectory()) {
                File r = searchForDir(c, name, depth - 1);
                if (r != null) {
                    return r;
                }
            }
        }
        return null;
    }
}
