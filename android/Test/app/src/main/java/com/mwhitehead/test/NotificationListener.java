package com.mwhitehead.test;

import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.media.AudioManager;
import android.service.notification.NotificationListenerService;
import android.service.notification.StatusBarNotification;
import android.util.Log;
import android.widget.Toast;

/**
 * Created by Matthew on 24/12/2014.
 */
public class NotificationListener extends NotificationListenerService {

    private int ringerMode = AudioManager.RINGER_MODE_VIBRATE;

    public NotificationListener() {
        super();
        Log.i("com.mwhitehead.test", "NotificationListener class instantiated");
    }

    @Override
    public void onCreate() {
        super.onCreate();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    @Override
    public void onNotificationPosted(StatusBarNotification sbn) {
        Log.i("com.mwhitehead.test", "Notification received: " + sbn.getTag() + " " + sbn.getNotification().toString() + " ID: " + sbn.getId());

        Toast toast = Toast.makeText(this, "Notification:", Toast.LENGTH_LONG);
        toast.show();
        toast = Toast.makeText(this, "" + sbn.getNotification().tickerText, Toast.LENGTH_LONG);
        toast.show();

        Log.i("com.mwhitehead.test", "Notification extras: " + sbn.getNotification().extras.toString());

        Log.i("com.mwhitehead.test", "Notification text: " + sbn.getNotification().tickerText);

        if (sbn.getNotification().tickerText != null) {
            String tickerText = sbn.getNotification().tickerText.toString().trim();

            if (tickerText.equals("Headphones connected")) {
                //AudioManager audioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);

                // Save previous audio mode
                //ringerMode = audioManager.getRingerMode();

                Log.i("com.mwhitehead.test", "Saving ringer mode before enabling volume: " + ringerMode);

                // Then set the ringer mode to non-silent
                //audioManager.setRingerMode(AudioManager.RINGER_MODE_NORMAL);
                Log.i("com.mwhitehead.test", "Disabling silent");
            }
        }
    }

    @Override
    public void onNotificationRemoved(StatusBarNotification sbn) {
        Log.i("com.mwhitehead.test", "Notification received: " + sbn.getTag() + " " + sbn.getNotification().toString() + " ID: " + sbn.getId());

        Log.i("com.mwhitehead.test", "Notification extras: " + sbn.getNotification().extras.toString());

        Log.i("com.mwhitehead.test", "Notification text: " + sbn.getNotification().tickerText);

        if (sbn.getNotification().tickerText != null) {
            String tickerText = sbn.getNotification().tickerText.toString().trim();

            if (tickerText.equals("Headphones connected")) {
                //AudioManager audioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);

                // Resume the previous ringer mode
                //audioManager.setRingerMode(ringerMode);

                Log.i("com.mwhitehead.test", "Resuming previous audio mode " + ringerMode);
            }
        }
    }
}
