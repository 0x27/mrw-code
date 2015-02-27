package com.mwhitehead.test;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.Uri;
import android.util.Log;
import android.widget.Toast;

import java.util.Calendar;

public class BootReceiver extends BroadcastReceiver {

    private static boolean alarmForDimmingSet = false;

    private static boolean alarmForWeekdayBrighteningSet = false;

    private static boolean alarmForWeekendBrighteningSet = false;

    private PendingIntent pendingIntent, pendingIntent2;

    public BootReceiver() {
        Log.i("com.mwhitehead.test", "BootReceiver instantiated");
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.i("com.mwhitehead.test", "Boot received by Matt's app");

        CharSequence text = "My App Started";
        int duration = Toast.LENGTH_LONG;

        Toast toast = Toast.makeText(context, text, duration);
        toast.show();

        // Start a background service which has the ability to create our headset receiver
        context.startService(new Intent(context, BackgroundService.class));

        // Get a single instance of a calendar object
        Calendar calendar = Calendar.getInstance();

        if (!alarmForDimmingSet) {
            // Tell the OS to set an alarm off at 11.15pm to dim the screen
            calendar.set(Calendar.HOUR_OF_DAY, 23);
            calendar.set(Calendar.MINUTE, 15);
            calendar.set(Calendar.SECOND, 0);

            Intent myIntent = new Intent("DIM_BRIGHTNESS_FOR_NIGHT", Uri.EMPTY, context, AlarmReceiver.class);
            pendingIntent = PendingIntent.getBroadcast(context, 1, myIntent, PendingIntent.FLAG_UPDATE_CURRENT);

            AlarmManager alarmManager = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
            alarmManager.setRepeating(AlarmManager.RTC_WAKEUP, calendar.getTimeInMillis(), AlarmManager.INTERVAL_DAY, pendingIntent);

            Log.i("com.mwhitehead.test", "Set broadcast alarm");
            alarmForDimmingSet = true;
        }

        if (!alarmForWeekdayBrighteningSet) {
            // Tell the OS to set an alarm off at 7.00am to brighten the screen.
            calendar.set(Calendar.HOUR_OF_DAY, 7);
            calendar.set(Calendar.MINUTE, 00);
            calendar.set(Calendar.SECOND, 0);

            Intent myIntent = new Intent("INCREASE_BRIGHTNESS_FOR_DAY", Uri.EMPTY, context, AlarmReceiver.class);
            pendingIntent2 = PendingIntent.getBroadcast(context, 2, myIntent, PendingIntent.FLAG_UPDATE_CURRENT);

            AlarmManager alarmManager = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
            alarmManager.setRepeating(AlarmManager.RTC_WAKEUP, calendar.getTimeInMillis(), AlarmManager.INTERVAL_DAY, pendingIntent2);

            Log.i("com.mwhitehead.test", "Set broadcast alarm 2");
            alarmForWeekdayBrighteningSet = true;
        }

        if (!alarmForWeekendBrighteningSet) {
            // Tell the OS to set an alarm off at 8.30am to brighten the screen.
            calendar.set(Calendar.HOUR_OF_DAY, 8);
            calendar.set(Calendar.MINUTE, 30);
            calendar.set(Calendar.SECOND, 0);

            Intent myIntent = new Intent("INCREASE_BRIGHTNESS_FOR_WEEKEND", Uri.EMPTY, context, AlarmReceiver.class);
            pendingIntent2 = PendingIntent.getBroadcast(context, 3, myIntent, PendingIntent.FLAG_UPDATE_CURRENT);

            AlarmManager alarmManager = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
            alarmManager.setRepeating(AlarmManager.RTC_WAKEUP, calendar.getTimeInMillis(), AlarmManager.INTERVAL_DAY, pendingIntent2);

            Log.i("com.mwhitehead.test", "Set broadcast alarm 2");
            alarmForWeekendBrighteningSet = true;
        }
    }
}
