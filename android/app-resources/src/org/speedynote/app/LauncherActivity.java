package org.speedynote.app;

import android.content.Intent;
import android.content.res.Configuration;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.SearchView;
import androidx.appcompat.widget.Toolbar;
import androidx.core.graphics.Insets;
import androidx.core.view.OnApplyWindowInsetsListener;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.core.view.WindowInsetsControllerCompat;
import androidx.recyclerview.widget.GridLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.google.android.material.appbar.AppBarLayout;
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
 * Fully edge-to-edge (system bars are handled via insets), Material You
 * dynamic color, large top app bar, and localized strings.
 */
public class LauncherActivity extends AppCompatActivity implements NoteCardAdapter.Listener {

    private static final String TAG = "LauncherActivity";

    private RecyclerView mRecyclerView;
    private View mEmptyView;
    private NoteCardAdapter mAdapter;
    private AppBarLayout mAppBar;
    private ExtendedFloatingActionButton mFab;
    private List<NoteItem> mAllNotes = new ArrayList<>();
    private String mQuery = "";
    private int mRecyclerBaseBottomPadding = 0;
    private int mFabBaseBottomMargin = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        CrashLogger.install(this);
        DynamicColors.applyToActivityIfAvailable(this);
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_launcher);

        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        if (getSupportActionBar() != null) {
            // The visible title comes from the CollapsingToolbarLayout.
            getSupportActionBar().setDisplayShowTitleEnabled(false);
        }

        mAppBar = findViewById(R.id.appbar);
        mRecyclerView = findViewById(R.id.note_grid);
        mEmptyView = findViewById(R.id.empty_view);
        mFab = findViewById(R.id.fab_new);

        int columns = getResources().getConfiguration().smallestScreenWidthDp >= 600 ? 3 : 2;
        mRecyclerView.setLayoutManager(new GridLayoutManager(this, columns));
        mAdapter = new NoteCardAdapter(this);
        mRecyclerView.setAdapter(mAdapter);

        mRecyclerBaseBottomPadding = mRecyclerView.getPaddingBottom();
        ViewGroup.MarginLayoutParams fabParams =
                (ViewGroup.MarginLayoutParams) mFab.getLayoutParams();
        mFabBaseBottomMargin = fabParams.bottomMargin;

        // Edge-to-edge: draw behind the system bars and keep the content
        // clear of them via window insets (status bar above the app bar,
        // navigation bar below the list and the FAB).
        WindowCompat.setDecorFitsSystemWindows(getWindow(), false);
        View root = findViewById(R.id.root);
        ViewCompat.setOnApplyWindowInsetsListener(root, new OnApplyWindowInsetsListener() {
            @Override
            public WindowInsetsCompat onApplyWindowInsets(View v, WindowInsetsCompat windowInsets) {
                Insets bars = windowInsets.getInsets(
                        WindowInsetsCompat.Type.systemBars()
                                | WindowInsetsCompat.Type.displayCutout());
                mAppBar.setPadding(0, bars.top, 0, 0);
                mRecyclerView.setPadding(mRecyclerView.getPaddingLeft(),
                        mRecyclerView.getPaddingTop(),
                        mRecyclerView.getPaddingRight(),
                        mRecyclerBaseBottomPadding + bars.bottom);
                ViewGroup.MarginLayoutParams lp =
                        (ViewGroup.MarginLayoutParams) mFab.getLayoutParams();
                lp.bottomMargin = mFabBaseBottomMargin + bars.bottom;
                mFab.setLayoutParams(lp);
                return windowInsets;
            }
        });

        mFab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                showNewNotebookSheet();
            }
        });

        applySystemBarAppearance();
    }

    @Override
    protected void onResume() {
        super.onResume();
        reloadNotes();
        applySystemBarAppearance();
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        applySystemBarAppearance();
    }

    private void applySystemBarAppearance() {
        boolean dark = (getResources().getConfiguration().uiMode
                & Configuration.UI_MODE_NIGHT_MASK) == Configuration.UI_MODE_NIGHT_YES;
        WindowInsetsControllerCompat controller =
                WindowCompat.getInsetsController(getWindow(), getWindow().getDecorView());
        if (controller != null) {
            controller.setAppearanceLightStatusBars(!dark);
            controller.setAppearanceLightNavigationBars(!dark);
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.menu_launcher, menu);
        MenuItem searchItem = menu.findItem(R.id.action_search);
        SearchView searchView = (SearchView) searchItem.getActionView();
        if (searchView != null) {
            searchView.setQueryHint(getString(R.string.search_hint));
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
        List<NoteItem> loaded;
        try {
            loaded = NoteLibraryReader.load(this);
        } catch (Throwable t) {
            Log.w(TAG, "Failed to load the notebook library", t);
            loaded = new ArrayList<>();
        }
        mAllNotes = loaded;
        applyFilter(mQuery);
    }

    private void applyFilter(String query) {
        mQuery = query == null ? "" : query;
        String q = mQuery.trim().toLowerCase(Locale.ROOT);
        List<NoteItem> filtered = new ArrayList<>();
        for (NoteItem n : mAllNotes) {
            if (q.isEmpty()
                    || (n.name != null && n.name.toLowerCase(Locale.ROOT).contains(q))) {
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
