package io.github.mh4u.shell;

import android.app.Activity;
import android.os.Bundle;
import android.os.Process;
import android.view.Gravity;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;

import java.io.File;

public final class MainActivity extends Activity {
    private TextView status;
    private TextView lifecycle;
    private int resumeCount = 0;
    private long createdAt;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        createdAt = System.currentTimeMillis();
        buildUi();
        lifecycle.setText("Lifecycle: onCreate");
        refreshStatus();
    }

    private void buildUi() {
        int pad = dp(22);
        LinearLayout page = new LinearLayout(this);
        page.setOrientation(LinearLayout.VERTICAL);
        page.setPadding(pad, pad, pad, pad);
        page.setGravity(Gravity.TOP);
        page.setBackgroundColor(0xFF10141D);

        TextView title = new TextView(this);
        title.setText("MONSTER HUNTER 4 ULTIMATE");
        title.setTextColor(0xFFF3F5FA);
        title.setTextSize(22);
        title.setGravity(Gravity.CENTER_VERTICAL);
        page.addView(title, matchWrap());

        TextView subtitle = new TextView(this);
        subtitle.setText("Android Native Shell • Phase 1");
        subtitle.setTextColor(0xFF9BAAC3);
        subtitle.setTextSize(14);
        LinearLayout.LayoutParams subParams = matchWrap();
        subParams.topMargin = dp(8);
        page.addView(subtitle, subParams);

        status = new TextView(this);
        status.setTextColor(0xFFE2E8F0);
        status.setTextSize(15);
        status.setLineSpacing(dp(5), 1.0f);
        status.setPadding(dp(14), dp(14), dp(14), dp(14));
        status.setBackgroundColor(0xFF1B2433);
        LinearLayout.LayoutParams statusParams = matchWrap();
        statusParams.topMargin = dp(24);
        page.addView(status, statusParams);

        lifecycle = new TextView(this);
        lifecycle.setTextColor(0xFF8EE6B0);
        lifecycle.setTextSize(14);
        LinearLayout.LayoutParams lifeParams = matchWrap();
        lifeParams.topMargin = dp(14);
        page.addView(lifecycle, lifeParams);

        Button check = new Button(this);
        check.setText("Refresh device status");
        check.setOnClickListener(v -> refreshStatus());
        LinearLayout.LayoutParams buttonParams = matchWrap();
        buttonParams.topMargin = dp(18);
        page.addView(check, buttonParams);

        TextView note = new TextView(this);
        note.setText("This build validates the custom Android Activity, app-private storage, "
                + "touch input, rotation and pause/resume. It does not boot the game or render "
                + "the 3DS scene yet. The existing Azahar-based APK remains a separate baseline.");
        note.setTextColor(0xFF9BAAC3);
        note.setTextSize(13);
        note.setLineSpacing(dp(4), 1.0f);
        LinearLayout.LayoutParams noteParams = matchWrap();
        noteParams.topMargin = dp(22);
        page.addView(note, noteParams);

        ScrollView scroll = new ScrollView(this);
        scroll.addView(page);
        setContentView(scroll);
    }

    private void refreshStatus() {
        File files = getFilesDir();
        File codeDir = new File(getApplicationInfo().nativeLibraryDir);
        status.setText("SHELL STATUS: RUNNING\n\n"
                + "ABI: " + android.os.Build.SUPPORTED_ABIS[0] + "\n"
                + "Android: " + android.os.Build.VERSION.RELEASE + " (API "
                + android.os.Build.VERSION.SDK_INT + ")\n"
                + "Process ID: " + Process.myPid() + "\n"
                + "App-private data: " + files.getAbsolutePath() + "\n"
                + "Native library dir: " + codeDir.getAbsolutePath() + "\n"
                + "Game/core: not connected");
    }

    @Override
    protected void onResume() {
        super.onResume();
        resumeCount++;
        if (lifecycle != null) {
            lifecycle.setText("Lifecycle: RESUMED • resume count " + resumeCount);
        }
    }

    @Override
    protected void onPause() {
        if (lifecycle != null) {
            lifecycle.setText("Lifecycle: PAUSED");
        }
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        if (lifecycle != null) {
            lifecycle.setText("Lifecycle: DESTROYED");
        }
        super.onDestroy();
    }

    private LinearLayout.LayoutParams matchWrap() {
        return new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT);
    }

    private int dp(int value) {
        return Math.round(value * getResources().getDisplayMetrics().density);
    }
}
