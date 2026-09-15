package org.speedynote.app;

import android.content.Intent;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.SearchView;
import androidx.recyclerview.widget.GridLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.google.android.material.bottomsheet.BottomSheetDialog;
import com.google.android.material.color.DynamicColors;
import com.google.android.material.floatingactionbutton.ExtendedFloatingActionButton;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

/**
 * Native Material 3 home screen for SpeedyNote on Android.
 *
 * Shows the notebook library (read from the Qt-side library file), supports
 * searching, and hands off to the Qt editor activity:
 *  - tapping a notebook      -> action "open"          + path
 *  - new edgeless notebook   -> action "new-edgeless"
 *  - new paged notebook      -> action "new-paged"
 *  - new from PDF            -> action "open-pdf"
 *  - open existing notebook  -> action "open-notebook"
 *
 * Colors, dark mode and Material You dynamic color come from the system
 * Material 3 theme; this screen is 100% native Android UI.
 */
public class LauncherActivity extends AppCompatActivity implements NoteCardAdapter.Listener {

    private RecyclerView mRecyclerView;
    private View mEmptyView;
    private NoteCardAdapter mAdapter;
    private List<NoteItem> mAllNotes = new ArrayList<>();
    private String mQuery = "";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        DynamicColors.applyToActivityIfAvailable(this);
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_launcher);

        androidx.appcompat.widget.Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        if (getSupportActionBar() != null) {
            getSupportActionBar().setTitle("SpeedyNote");
        }

        mRecyclerView = findViewById(R.id.note_grid);
        mEmptyView = findViewById(R.id.empty_view);
        mRecyclerView.setLayoutManager(new GridLayoutManager(this, 2));
        mAdapter = new NoteCardAdapter(this);
        mRecyclerView.setAdapter(mAdapter);

        ExtendedFloatingActionButton fab = findViewById(R.id.fab_new);
        fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                showNewNotebookSheet();
            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
        reloadNotes();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.menu_launcher, menu);
        MenuItem searchItem = menu.findItem(R.id.action_search);
        SearchView searchView = (SearchView) searchItem.getActionView();
        if (searchView != null) {
            searchView.setQueryHint("Search notebooks");
            searchView.setOnQueryTextListener(new SearchView.OnQueryTextListener() {
                @Override
                public boolean onQueryTextSubmit(String query) {
                    applyFilter(query);
                    return true;
                }

                @Override
                public boolean onQueryTextChange(String newText) {
                    applyFilter(newText);
                    return true;
                }
            });
        }
        return true;
    }

    private void reloadNotes() {
        mAllNotes = NoteLibraryReader.load(this);
        applyFilter(mQuery);
    }

    private void applyFilter(String query) {
        mQuery = query == null ? "" : query;
        String q = mQuery.trim().toLowerCase(Locale.ROOT);
        List<NoteItem> filtered = new ArrayList<>();
        for (NoteItem n : mAllNotes) {
            if (q.isEmpty() || n.name.toLowerCase(Locale.ROOT).contains(q)) {
                filtered.add(n);
            }
        }
        mAdapter.setNotes(filtered);
        boolean empty = filtered.isEmpty();
        mRecyclerView.setVisibility(empty ? View.GONE : View.VISIBLE);
        mEmptyView.setVisibility(empty ? View.VISIBLE : View.GONE);
    }

    @Override
    public void onNoteClicked(NoteItem note) {
        launchEditor("open", note.bundlePath);
    }

    private void showNewNotebookSheet() {
        final BottomSheetDialog sheet = new BottomSheetDialog(this);
        View content = LayoutInflater.from(this).inflate(R.layout.sheet_new_notebook, null);

        content.findViewById(R.id.row_edgeless).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sheet.dismiss();
                launchEditor("new-edgeless", null);
            }
        });
        content.findViewById(R.id.row_paged).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sheet.dismiss();
                launchEditor("new-paged", null);
            }
        });
        content.findViewById(R.id.row_pdf).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sheet.dismiss();
                launchEditor("open-pdf", null);
            }
        });
        content.findViewById(R.id.row_open).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sheet.dismiss();
                launchEditor("open-notebook", null);
            }
        });

        sheet.setContentView(content);
        sheet.show();
    }

    private void launchEditor(String action, String path) {
        Intent intent = new Intent(this, SpeedyNoteActivity.class);
        intent.putExtra(SpeedyNoteActivity.EXTRA_ACTION, action);
        if (path != null) {
            intent.putExtra(SpeedyNoteActivity.EXTRA_PATH, path);
        }
        startActivity(intent);
    }
}
