package com.piotrmeller.camerasync.activity;

import android.Manifest;
import android.app.ActivityManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CameraMetadata;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.TotalCaptureResult;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.Image;
import android.media.ImageReader;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.util.Log;
import android.util.Size;
import android.util.SparseIntArray;
import android.view.Surface;
import android.view.TextureView;
import android.view.WindowManager;
import android.widget.Button;
import android.support.design.widget.FloatingActionButton;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.Toast;

import com.piotrmeller.camerasync.camera.AutoFitTextureView;
import com.piotrmeller.camerasync.camera.CameraFragment;
import com.piotrmeller.camerasync.service.NetworkService;
import com.piotrmeller.camerasync.R;
import com.piotrmeller.camerasync.service.ServiceCallbacks;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.List;

public class MainActivity extends AppCompatActivity implements ServiceCallbacks {
    private static final String TAG = "CameraSync";


    protected NetworkService mService;
    protected boolean mBound = false;

    private CameraFragment cameraFragment;

    private Button takePictureButton;
    private AutoFitTextureView textureView;
    private static final SparseIntArray ORIENTATIONS = new SparseIntArray();

    static {
        ORIENTATIONS.append(Surface.ROTATION_0, 90);
        ORIENTATIONS.append(Surface.ROTATION_90, 0);
        ORIENTATIONS.append(Surface.ROTATION_180, 270);
        ORIENTATIONS.append(Surface.ROTATION_270, 180);
    }

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);


        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        cameraFragment = CameraFragment.newInstance();

        if (null == savedInstanceState) {
            getSupportFragmentManager().beginTransaction()
                    .replace(R.id.container, cameraFragment)
                    .commit();
        }

//        textureView = (AutoFitTextureView) findViewById(R.id.texture);
//        assert textureView != null;
//        textureView.setSurfaceTextureListener(textureListener);


        FloatingActionButton fab = (FloatingActionButton) findViewById(R.id.fab);
        fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                //takePicture();
                long targetTimestamp = System.nanoTime()+600000000;
                mService.sendTakePictureReqsuest(targetTimestamp);
                //new Handler().postDelayed(() -> cameraFragment.takePictureImmediately(), 150);
                //takePictureRequest((targetTimestamp-System.nanoTime())/1000000);
                takePictureRequest(targetTimestamp+200000000);
            }
        });


        // Example of a call to a native method
        //TextView tv = (TextView) findViewById(R.id.sample_text);
        //tv.setText(stringFromJNI());
    }


    @Override
    protected void onStart() {
        super.onStart();
        Intent intent = new Intent(this, NetworkService.class);
        if (!isMyServiceRunning(NetworkService.class)) {
            startService(intent);
        }
        bindService(intent, mConnection, Context.BIND_AUTO_CREATE);


//        if(!isMyServiceRunning(NetworkService.class)) {
//            startService(new Intent(getBaseContext(), NetworkService.class));
//        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.e(TAG, "onResume");
//        startBackgroundThread();
//        if (textureView.isAvailable()) {
//            openCamera();
//        } else {
//            textureView.setSurfaceTextureListener(textureListener);
//        }
    }

    @Override
    protected void onStop() {
        super.onStop();
        if (mBound) {
            mService.registerCallbacks(null);
            unbindService(mConnection);
            mBound = false;
        }
    }

    @Override
    protected void onPause() {
        Log.e(TAG, "onPause");
        //closeCamera();
        //stopBackgroundThread();
        super.onPause();
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
        if (id == R.id.menu_server) {
            Intent intent = new Intent(this, ServerActivity.class);
            startActivity(intent);
            return true;
        } else if (id == R.id.menu_connect) {
            Intent intent = new Intent(this, ClientActivity.class);
            startActivity(intent);
            return true;
        } else if (id == R.id.action_settings) {
            return true;
        } else if (id == R.id.cameraParams) {
            Intent intent = new Intent(this, CameraParams.class);
            startActivity(intent);
        }


        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Intent intent = new Intent(this, NetworkService.class);
        stopService(intent);
        //serverOld.onDestroy();
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    private ServiceConnection mConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName className,
                                       IBinder service) {

            NetworkService.LocalBinder binder = (NetworkService.LocalBinder) service;
            mService = binder.getService();
            mBound = true;
            mService.registerCallbacks(MainActivity.this);
        }

        @Override
        public void onServiceDisconnected(ComponentName arg0) {
            mBound = false;
        }
    };


    private boolean isMyServiceRunning(Class<?> serviceClass) {
        ActivityManager manager = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
        for (ActivityManager.RunningServiceInfo service : manager.getRunningServices(Integer.MAX_VALUE)) {
            if (serviceClass.getName().equals(service.service.getClassName())) {
                return true;
            }
        }
        return false;
    }


    @Override
    public void takePictureRequest(long target) {
        //new Handler().postDelayed(() -> cameraFragment.takePictureImmediately(), delay);
        //cameraFragment.takePictureImmediately();
        //new Handler(Looper.getMainLooper()).postDelayed(() -> cameraFragment.takePictureImmediately(),delay);
        new Thread(() -> {
            boolean loop=true;
            while (loop)
            {
                if(target-System.nanoTime()>100000000)
                {
                    try {
                        Thread.sleep(50);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
                else
                {
                    loop = false;
                    while (target-System.nanoTime()>0)
                    {

                    }
                    runOnUiThread(() -> cameraFragment.takePictureImmediately());
                }
            }

        }).start();
    }
}
