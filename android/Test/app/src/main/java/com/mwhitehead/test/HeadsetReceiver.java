package com.mwhitehead.test;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.media.AudioManager;
import android.util.Log;
import android.widget.Toast;

/**
 * Created by Matthew on 09/01/2015.
 */
public class HeadsetReceiver extends BroadcastReceiver {

    private int ringerMode = -1;

    public HeadsetReceiver() {
        Log.i("com.mwhitehead.test", "Headset broadcast receiver instantiated");
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.i("com.mwhitehead.test", "Headset changed");
        Log.i("com.mwhitehead.test", "Intent: " + intent.toString());

        boolean isConnected = intent.getIntExtra("state", 0) == 1;

        if (isConnected) {
            Toast toast = Toast.makeText(context, "Headset connected - enabling audio", Toast.LENGTH_LONG);
            toast.show();

            AudioManager audioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);

            // Save previous audio mode
            ringerMode = audioManager.getRingerMode();

            Log.i("com.mwhitehead.test", "Saving ringer mode before enabling volume: " + ringerMode);

            // Then set the ringer mode to non-silent
            audioManager.setRingerMode(AudioManager.RINGER_MODE_NORMAL);
            Log.i("com.mwhitehead.test", "Disabling silent");
        } else {
            // This is the case on first start. Remember, broadcasts can be repeated to newly-started apps,
            // so we don't want to do anything until the headphones have been plugged in and unplugged at least once
            if (ringerMode != -1) {
                Toast toast = Toast.makeText(context, "Headset disconnected - returning to normal", Toast.LENGTH_LONG);
                toast.show();

                AudioManager audioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);

                // Resume the previous ringer mode
                audioManager.setRingerMode(ringerMode);

                Log.i("com.mwhitehead.test", "Resuming previous audio mode " + ringerMode);
            }
        }
    }
}
