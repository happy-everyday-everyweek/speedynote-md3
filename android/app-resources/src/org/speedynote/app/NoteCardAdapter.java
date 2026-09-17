package org.speedynote.app;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.text.format.DateUtils;
import android.util.LruCache;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import java.util.ArrayList;
import java.util.List;

/**
 * RecyclerView adapter rendering notebook cards from NoteItem data.
 * Cards use the standard MaterialCardView so shape, ripple, container
 * colors and dark mode all come from the active Material 3 theme.
 *
 * Thumbnails are decoded with a bounded sample size (never full-size) and
 * kept in an LruCache: decoding full-size captures on the UI thread used to
 * OOM and crash the app on large libraries.
 */
public class NoteCardAdapter extends RecyclerView.Adapter<NoteCardAdapter.NoteViewHolder> {

    public interface Listener {
        void onNoteClicked(NoteItem note);
    }

    /** Target size (px) of the longer edge of decoded thumbnails. */
    private static final int THUMB_TARGET_PX = 512;

    private final Listener mListener;
    private final List<NoteItem> mNotes = new ArrayList<>();
    private final LruCache<String, Bitmap> mThumbCache;

    public NoteCardAdapter(Listener listener) {
        mListener = listener;
        // Use at most 1/8 of the app heap for thumbnail bitmaps.
        int maxKb = (int) (Runtime.getRuntime().maxMemory() / 1024 / 8);
        mThumbCache = new LruCache<String, Bitmap>(maxKb) {
            @Override
            protected int sizeOf(String key, Bitmap value) {
                return value.getByteCount() / 1024;
            }
        };
    }

    public void setNotes(List<NoteItem> notes) {
        mNotes.clear();
        if (notes != null) {
            mNotes.addAll(notes);
        }
        notifyDataSetChanged();
    }

    @NonNull
    @Override
    public NoteViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View v = LayoutInflater.from(parent.getContext())
                .inflate(R.layout.item_note_card, parent, false);
        return new NoteViewHolder(v);
    }

    @Override
    public void onBindViewHolder(@NonNull NoteViewHolder holder, int position) {
        holder.bind(mNotes.get(position), mListener, mThumbCache);
    }

    @Override
    public int getItemCount() {
        return mNotes.size();
    }

    /** Decode a thumbnail at a bounded size, with caching and hard failure fallbacks. */
    private static Bitmap loadThumb(String path, LruCache<String, Bitmap> cache) {
        if (path == null || path.isEmpty()) {
            return null;
        }
        Bitmap cached = cache.get(path);
        if (cached != null) {
            return cached;
        }
        try {
            BitmapFactory.Options bounds = new BitmapFactory.Options();
            bounds.inJustDecodeBounds = true;
            BitmapFactory.decodeFile(path, bounds);
            if (bounds.outWidth <= 0 || bounds.outHeight <= 0) {
                return null;
            }
            int sample = 1;
            while (bounds.outWidth / (sample * 2) >= THUMB_TARGET_PX
                    || bounds.outHeight / (sample * 2) >= THUMB_TARGET_PX) {
                sample *= 2;
            }
            BitmapFactory.Options opts = new BitmapFactory.Options();
            opts.inSampleSize = sample;
            Bitmap bmp = BitmapFactory.decodeFile(path, opts);
            if (bmp != null) {
                cache.put(path, bmp);
            }
            return bmp;
        } catch (Throwable t) {
            // Corrupt or unreadable thumbnail: fall back to the placeholder.
            return null;
        }
    }

    static class NoteViewHolder extends RecyclerView.ViewHolder {
        private final ImageView mThumb;
        private final ImageView mStar;
        private final TextView mName;
        private final TextView mDate;
        private final TextView mBadge;

        NoteViewHolder(@NonNull View itemView) {
            super(itemView);
            mThumb = itemView.findViewById(R.id.note_thumb);
            mStar = itemView.findViewById(R.id.note_star);
            mName = itemView.findViewById(R.id.note_name);
            mDate = itemView.findViewById(R.id.note_date);
            mBadge = itemView.findViewById(R.id.note_badge);
        }

        void bind(final NoteItem note, final Listener listener,
                  LruCache<String, Bitmap> cache) {
            mName.setText(note.name);

            long when = note.lastAccessed > 0 ? note.lastAccessed : note.lastModified;
            if (when > 0) {
                CharSequence rel = DateUtils.getRelativeTimeSpanString(
                        when, System.currentTimeMillis(), DateUtils.MINUTE_IN_MILLIS);
                mDate.setText(rel);
            } else {
                mDate.setText("");
            }

            mStar.setVisibility(note.isStarred ? View.VISIBLE : View.GONE);

            if (note.isPdfBased) {
                mBadge.setVisibility(View.VISIBLE);
                mBadge.setText(R.string.badge_pdf);
            } else if (note.isEdgeless) {
                mBadge.setVisibility(View.VISIBLE);
                mBadge.setText(R.string.badge_edgeless);
            } else {
                mBadge.setVisibility(View.GONE);
            }

            Bitmap thumb = loadThumb(note.thumbnailPath, cache);
            if (thumb != null) {
                mThumb.setScaleType(ImageView.ScaleType.CENTER_CROP);
                mThumb.setImageBitmap(thumb);
            } else {
                mThumb.setScaleType(ImageView.ScaleType.CENTER);
                mThumb.setImageResource(R.drawable.ic_note_placeholder);
            }

            itemView.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    if (listener != null) {
                        listener.onNoteClicked(note);
                    }
                }
            });
        }
    }
}
