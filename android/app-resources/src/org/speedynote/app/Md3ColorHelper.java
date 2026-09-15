package org.speedynote.app;

import android.content.res.Resources;
import android.os.Build;

/**
 * Material You integration helpers for the MD3 theme.
 *
 * <p>Exposes the platform dynamic color palette (Android 12+) so the C++
 * side can derive a matching Material Design 3 color scheme from the
 * user's wallpaper. Falls back gracefully on older devices.</p>
 */
public final class Md3ColorHelper {

    private Md3ColorHelper() {
    }

    /**
     * Returns the system accent color as 0xAARRGGBB, or 0 when dynamic
     * color is unavailable (Android < 12 or resource lookup failure).
     */
    public static int getSystemAccentColor() {
        if (Build.VERSION.SDK_INT < 31) {
            return 0;
        }
        try {
            Resources res = Resources.getSystem();
            int id = res.getIdentifier("system_accent1_600", "color", "android");
            if (id == 0) {
                id = res.getIdentifier("system_accent1_500", "color", "android");
            }
            if (id == 0) {
                return 0;
            }
            return res.getColor(id, null);
        } catch (Throwable t) {
            return 0;
        }
    }
}
