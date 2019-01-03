package com.piotrmeller.camerasync.service;

import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;


public class NetworkService extends Service {

    public Server server;
    public Client client;

    ServiceCallbacks callbacks;


    @Override
    public void onCreate() {
        super.onCreate();
        server = new Server(this);
        client = new Client(this);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        super.onStartCommand(intent, flags, startId);
        return START_STICKY;
    }

    private Handler handler = new Handler();

    private final IBinder mBinder = new LocalBinder();

    public class LocalBinder extends Binder {
        public NetworkService getService() {
            //zwracamy instancje serwisu, przez nią odwołamy się następnie do metod.
            return NetworkService.this;
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    public void registerCallbacks(ServiceCallbacks callbacks) {
        this.callbacks = callbacks;
    }

    public void sendTakePictureReqsuest(){
        new Thread(() -> server.broadcast()).start();
    }


}
