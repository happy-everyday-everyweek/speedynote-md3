package org.speedynote.app;

/**
 * One notebook entry, parsed from the Qt-side notebook library
 * (notebook_library.json) by {@link NoteLibraryReader}.
 */
public class NoteItem {
    public String bundlePath = "";
    public String name = "";
    public String documentId = "";
    public String starredFolder = "";
    public long lastModified = 0L;
    public long lastAccessed = 0L;
    public boolean isStarred = false;
    public boolean isPdfBased = false;
    public boolean isEdgeless = false;
    /** Absolute path of the cached thumbnail PNG, or null when absent. */
    public String thumbnailPath = null;
}
