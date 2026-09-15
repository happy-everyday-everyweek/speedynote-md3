package org.speedynote.app;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.text.format.DateUtils;
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
 */
public class NoteCardAdapter extends RecyclerView.Adapter<NoteCardAdapter.NoteViewHolder> {

    public interface Listener {
        void onNoteClicked(NoteItem note);
    }

    private final Listener mListener;
    private final List<NoteItem> mNotes = new ArrayList<>();

    public NoteCardAdapter(Listener listener) {
        mListener = listener;
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
        holder.bind(mNotes.get(position), mListener);
    }

    @Override
    public int getItemCount() {
        return mNotes.size();
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

        void bind(NoteItem note, Listener listener) {
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
                mBadge.setText("PDF");
            } else if (note.isEdgeless) {
                mBadge.setVisibility(View.VISIBLE);
                mBadge.setText("Edgeless");
            } else {
                mBadge.setVisibility(View.GONE);
            }

            Bitmap thumb = null;
            if (note.thumbnailPath != null) {
                thumb = BitmapFactory.decodeFile(note.thumbnailPath);
            }
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
