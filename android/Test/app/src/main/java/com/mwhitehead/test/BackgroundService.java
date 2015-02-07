package com.mwhitehead.test;

import android.app.IntentService;
import android.app.Service;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.IBinder;
import android.util.Log;
import android.widget.Toast;

/**
 * Created by Matthew on 09/01/2015.
 */
public class BackgroundService extends Service {
    public BackgroundService() {
        super();
        Log.i("com.mwhitehead.test", "Background Service Instantiated");
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Toast.makeText(this, "Service created...", Toast.LENGTH_LONG).show();
        Log.i("com.mwhitehead.test", "Service created...");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.i("com.mwhitehead.test", "Service started...");

        IntentFilter receiverFilter = new IntentFilter(Intent.ACTION_HEADSET_PLUG);
        HeadsetReceiver receiver = new HeadsetReceiver();
        registerReceiver(receiver, receiverFilter);

        return START_STICKY;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Toast.makeText(this, "Service destroyed...", Toast.LENGTH_LONG).show();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
}
