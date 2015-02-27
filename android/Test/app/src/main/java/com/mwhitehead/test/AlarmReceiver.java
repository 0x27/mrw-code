package com.mwhitehead.test;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.provider.Settings;
import android.util.Log;
import android.view.WindowManager;
import android.widget.Toast;

import java.util.Calendar;

public class AlarmReceiver extends BroadcastReceiver {
    public AlarmReceiver() {
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.i("com.mwhitehead.test", "Alarm notification received");

        Toast toast = Toast.makeText(context, "Alarm manager went off", Toast.LENGTH_LONG);
        toast.show();

        Log.i("com.mwhitehead.test", intent.toString());

        if (intent.getAction() != null) {

            Log.i("com.mwhitehead.test", intent.getAction().toString());

            if (intent.getAction().equals("DIM_BRIGHTNESS_FOR_NIGHT")) {
                try {
                    Log.i("com.mwhitehead.test", "Screen brightness: " + Settings.System.getInt(context.getContentResolver(), Settings.System.SCREEN_BRIGHTNESS));

                    Log.i("com.mwhitehead.test", "Setting brightness level manually");
                    android.provider.Settings.System.putInt(context.getContentResolver(), Settings.System.SCREEN_BRIGHTNESS_MODE, 0);
                    android.provider.Settings.System.putInt(context.getContentResolver(), Settings.System.SCREEN_BRIGHTNESS, 15);
                } catch (Settings.SettingNotFoundException e) {
                    Log.i("com.mwhitehead.test", "Apparently screen brightness setting doesn't exist???");
                }
            } else if (intent.getAction().equals("INCREASE_BRIGHTNESS_FOR_DAY")) {
                try {
                    Log.i("com.mwhitehead.test", "Screen brightness: " + Settings.System.getInt(context.getContentResolver(), Settings.System.SCREEN_BRIGHTNESS));

                    Log.i("com.mwhitehead.test", "Setting brightness level back to automatic");

                    // Get a single instance of a calendar object
                    Calendar calendar = Calendar.getInstance();

                    if (calendar.get(Calendar.DAY_OF_WEEK) != Calendar.SATURDAY && calendar.get(Calendar.DAY_OF_WEEK) != Calendar.SUNDAY) {
                        android.provider.Settings.System.putInt(context.getContentResolver(), Settings.System.SCREEN_BRIGHTNESS_MODE, 0);
                        android.provider.Settings.System.putInt(context.getContentResolver(), Settings.System.SCREEN_BRIGHTNESS, 105);
                        android.provider.Settings.System.putInt(context.getContentResolver(), Settings.System.SCREEN_BRIGHTNESS_MODE, 1);
                    }
                } catch (Settings.SettingNotFoundException e) {
                    Log.i("com.mwhitehead.test", "Apparently screen brightness setting doesn't exist???");
                }
            } else if (intent.getAction().equals("INCREASE_BRIGHTNESS_FOR_WEEKEND")) {
                try {
                    Log.i("com.mwhitehead.test", "Screen brightness: " + Settings.System.getInt(context.getContentResolver(), Settings.System.SCREEN_BRIGHTNESS));

                    Log.i("com.mwhitehead.test", "Setting brightness level back to automatic");

                    // Get a single instance of a calendar object
                    Calendar calendar = Calendar.getInstance();

                    if (calendar.get(Calendar.DAY_OF_WEEK) == Calendar.SATURDAY || calendar.get(Calendar.DAY_OF_WEEK) == Calendar.SUNDAY) {
                        android.provider.Settings.System.putInt(context.getContentResolver(), Settings.System.SCREEN_BRIGHTNESS_MODE, 0);
                        android.provider.Settings.System.putInt(context.getContentResolver(), Settings.System.SCREEN_BRIGHTNESS, 105);
                        android.provider.Settings.System.putInt(context.getContentResolver(), Settings.System.SCREEN_BRIGHTNESS_MODE, 1);
                    }
                } catch (Settings.SettingNotFoundException e) {
                    Log.i("com.mwhitehead.test", "Apparently screen brightness setting doesn't exist???");
                }
            }
        } else {
            Log.i("com.mwhitehead.test", "Action NULL");
        }

        //WindowManager.LayoutParams layoutParams = getWindow().getAttributes();
        //layoutParams.screenBrightness = brightness / 100.0f;
        //getWindow().setAttributes(layoutParams);
    }
}
