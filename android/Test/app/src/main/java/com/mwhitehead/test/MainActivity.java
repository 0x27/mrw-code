package com.mwhitehead.test;

import android.app.Activity;
import android.app.AlarmManager;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.net.Uri;
import android.os.Bundle;
import android.support.v4.app.NotificationCompat;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.EditText;
import android.content.Intent;
import android.widget.Toast;

import java.util.Calendar;


public class MainActivity extends Activity {

    public static final String EXTRA_MESSAGE = "com.mwhitehead.test.app.MESSAGE";

    private static boolean headphones = true;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Log.i("com.mwhitehead.test", "Main activity created");

    }

    protected void onStart() {
        super.onStart();
        Log.i("com.mwhitehead.test", "Main activity started");
    }

    protected void onRestart() {
        super.onRestart();
        Log.i("com.mwhitehead.test", "Main activity restarted");
    }

    protected void onResume() {
        super.onResume();
        Log.i("com.mwhitehead.test", "Main activity resumed");
    }

    protected void onPause() {
        super.onPause();
        Log.i("com.mwhitehead.test", "Main activity paused");
    }

    protected void onStop() {
        super.onStop();
        Log.i("com.mwhitehead.test", "Main activity stopped");
    }

    protected void onDestroy() {
        super.onDestroy();
        Log.i("com.mwhitehead.test", "Main activity destroyed");
    }

    public void sendMessage(View view) {
        Intent intent = new Intent(this, MainActivity3.class);
        EditText editText = (EditText) findViewById(R.id.inputText);
        String editTextString = editText.getText().toString();
        intent.putExtra(EXTRA_MESSAGE, editTextString);
        startActivity(intent);
    }

    public void showSettings(MenuItem menuItem) {
        Intent intent = new Intent(this, SettingsActivity.class);
        startActivity(intent);
    }

    public void enableNotifications(MenuItem menuItem) {
        Intent intent = new Intent("android.settings.ACTION_NOTIFICATION_LISTENER_SETTINGS");
        startActivity(intent);
    }

    public void sendNotification(MenuItem menuItem) {
        // TODO - Send a sample notification
        Log.i("com.mwhitehead.test", "Send example notification");
        NotificationManager nManager = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
        NotificationCompat.Builder ncomp = new NotificationCompat.Builder(this);
        if (headphones) {
            ncomp.getExtras().putBoolean("HEADPHONE_STATUS", true);
            ncomp.setContentTitle("Headphones plugged in");
            ncomp.setContentText("Example notification showing headphones being plugged in");
            ncomp.setTicker("Example notification showing headphones being plugged in");
            headphones = false;
        } else {
            ncomp.getExtras().putBoolean("HEADPHONE_STATUS", false);
            ncomp.setContentTitle("Headphones removed");
            ncomp.setContentText("Example notification showing headphones being removed");
            ncomp.setTicker("Example notification showing headphones being removed");
            headphones = true;
        }
        ncomp.setSmallIcon(R.drawable.ic_launcher);
        ncomp.setAutoCancel(true);
        nManager.notify((int)System.currentTimeMillis(),ncomp.build());
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }
}
