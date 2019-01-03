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
import android.widget.TextView;

import com.piotrmeller.camerasync.service.NetworkService;
import com.piotrmeller.camerasync.R;

import java.net.InetSocketAddress;

public class ServerActivity extends AppCompatActivity {

    protected NetworkService mService;
    protected boolean mBound = false;
    TextView text;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_server);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        text = (TextView) findViewById(R.id.textView);
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (mBound) {
            if (mService.server.isServerOpen()) {
                InetSocketAddress address = mService.server.getServerAddress();
                text.setText(address.getAddress().getHostAddress() + ":" + address.getPort());
            }
        }
    }

    @Override
    protected void onStart() {
        super.onStart();
        Intent intent = new Intent(this, NetworkService.class);
        bindService(intent, mConnection, Context.BIND_AUTO_CREATE);


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

            if (mService.server.isServerOpen()) {
                InetSocketAddress address = mService.server.getServerAddress();
                text.setText(address.getAddress().getHostAddress() + ":" + address.getPort());
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName arg0) {
            mBound = false;
        }
    };

    public void onClick_createServer(View v) {
        mService.server.startServer();
        InetSocketAddress address = mService.server.getServerAddress();
        text.setText(address.getAddress().getHostAddress() + ":" + address.getPort());
    }
}
