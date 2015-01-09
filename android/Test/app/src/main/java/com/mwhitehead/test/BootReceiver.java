package com.mwhitehead.test;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.util.Log;
import android.widget.Toast;

import java.util.Calendar;

public class BootReceiver extends BroadcastReceiver {

    private static boolean alarmForDimmingSet = false;

    private static boolean alarmForBrighteningSet = false;

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

        Calendar dimmingCalendar = Calendar.getInstance();
        dimmingCalendar.set(Calendar.HOUR_OF_DAY, 23);
        dimmingCalendar.set(Calendar.MINUTE, 00);
        dimmingCalendar.set(Calendar.SECOND, 0);

        Calendar brighteningCalendar = Calendar.getInstance();
        Log.i("com.mwhitehead.test", "Current time: " + brighteningCalendar.toString());
        Log.i("com.mwhitehead.test", "Current time: " + brighteningCalendar.getTime());

        brighteningCalendar.set(Calendar.HOUR_OF_DAY, 07);
        brighteningCalendar.set(Calendar.MINUTE, 00);
        brighteningCalendar.set(Calendar.SECOND, 0);

        if (!alarmForDimmingSet) {

            Intent myIntent = new Intent("DIM_BRIGHTNESS_FOR_NIGHT", Uri.EMPTY, context, AlarmReceiver.class);
            pendingIntent = PendingIntent.getBroadcast(context, 1, myIntent, PendingIntent.FLAG_UPDATE_CURRENT);

            AlarmManager alarmManager = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
            //alarmManager.set(AlarmManager.RTC, calendar.getTimeInMillis(), pendingIntent);
            alarmManager.setRepeating(AlarmManager.RTC_WAKEUP, dimmingCalendar.getTimeInMillis(), AlarmManager.INTERVAL_DAY, pendingIntent);

            //alarmManager.set(AlarmManager.RTC, calendar.getTimeInMillis(), pendingIntent);
            //alarmManager.setExact(AlarmManager.RTC_WAKEUP, dimmingCalendar.getTimeInMillis(), pendingIntent);

            Log.i("com.mwhitehead.test", "Set broadcast alarm");
            alarmForDimmingSet = true;

            //Toast toast = Toast.makeText(AlarmService.this, "Set alarm for dimming", Toast.LENGTH_LONG);
            //toast.show();
        }

        if (!alarmForBrighteningSet) {

            Intent myIntent = new Intent("INCREASE_BRIGHTNESS_FOR_DAY", Uri.EMPTY, context, AlarmReceiver.class);
            pendingIntent2 = PendingIntent.getBroadcast(context, 2, myIntent, PendingIntent.FLAG_UPDATE_CURRENT);

            AlarmManager alarmManager = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
            //alarmManager.set(AlarmManager.RTC, calendar.getTimeInMillis(), pendingIntent);
            alarmManager.setRepeating(AlarmManager.RTC_WAKEUP, brighteningCalendar.getTimeInMillis(), AlarmManager.INTERVAL_DAY, pendingIntent2);

            Log.i("com.mwhitehead.test", "Set broadcast alarm 2");
            alarmForBrighteningSet = true;

            //Toast toast = Toast.makeText(AlarmService.this, "Set alarm for resetting", Toast.LENGTH_LONG);
            //toast.show();
        }
    }
}
