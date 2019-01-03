package com.piotrmeller.camerasync.activity;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.view.WindowManager;
import android.widget.EditText;

import com.piotrmeller.camerasync.service.NetworkService;
import com.piotrmeller.camerasync.R;

import java.net.InetSocketAddress;

public class ClientActivity extends AppCompatActivity {

    protected NetworkService mService;
    protected boolean mBound = false;

    EditText ip;
    EditText port;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_client);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        ip = (EditText) findViewById(R.id.ipInfo);
        port = (EditText) findViewById(R.id.portInfo);
    }

    @Override
    protected void onStart() {
        super.onStart();
        Intent intent = new Intent(this, NetworkService.class);
        bindService(intent,mConnection,Context.BIND_AUTO_CREATE);



//        if(!isMyServiceRunning(NetworkService.class)) {
//            startService(new Intent(getBaseContext(), NetworkService.class));
//        }
    }

    @Override
    protected void onStop() {
        super.onStop();
        if (mBound) {
            unbindService(mConnection);
            mBound = false;
        }
    }

    private ServiceConnection mConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName className,
                                       IBinder service) {

            NetworkService.LocalBinder binder = (NetworkService.LocalBinder) service;
            mService = binder.getService();
            mBound = true;
        }

        @Override
        public void onServiceDisconnected(ComponentName arg0) {
            mBound = false;
        }
    };

    public void onClick_connectToServer(View v) {
        String ip_address = ip.getText().toString();
        String portstr = port.getText().toString();
        int port_number = Integer.parseInt(port.getText().toString());
        InetSocketAddress address = new InetSocketAddress(ip_address,port_number);
        mService.client.connectToServer(address);
    }

}
