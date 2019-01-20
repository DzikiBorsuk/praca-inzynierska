package com.piotrmeller.camerasync.activity;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.graphics.Bitmap;
import android.os.IBinder;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.view.WindowManager;
import android.widget.ImageView;
import android.widget.TextView;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.MultiFormatWriter;
import com.google.zxing.WriterException;
import com.google.zxing.common.BitMatrix;
import com.piotrmeller.camerasync.service.NetworkService;
import com.piotrmeller.camerasync.R;

import java.net.InetSocketAddress;

public class ServerActivity extends AppCompatActivity {

    protected NetworkService mService;
    protected boolean mBound = false;
    //TextView text;
    ImageView imgView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_server);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        //text = (TextView) findViewById(R.id.textView);
        imgView = (ImageView) findViewById(R.id.imageView);
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (mBound) {
            if (mService.server.isServerOpen()) {
                //InetSocketAddress address = mService.server.getServerAddress();
                //text.setText(address.getAddress().getHostAddress() + ":" + address.getPort());
                showQR();
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

    private void showQR()
    {
        InetSocketAddress address = mService.server.getServerAddress();
        String address_text = address.getAddress().getHostAddress() + ":" + address.getPort();
        try
        {
            Bitmap bmp =  TextToImageEncode(address_text);
            imgView.setImageBitmap(bmp);
        } catch (Exception e) {
            e.printStackTrace();
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
                showQR();
                //InetSocketAddress address = mService.server.getServerAddress();
                //text.setText(address.getAddress().getHostAddress() + ":" + address.getPort());
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName arg0) {
            mBound = false;
        }
    };

    public void onClick_createServer(View v) {
        mService.server.startServer();
        showQR();
        //InetSocketAddress address = mService.server.getServerAddress();
        //text.setText(address.getAddress().getHostAddress() + ":" + address.getPort());
    }

    static int QRcodeWidth = 500;

    private Bitmap TextToImageEncode(String Value) throws WriterException {
        BitMatrix bitMatrix;
        try {
            bitMatrix = new MultiFormatWriter().encode(
                    Value,
                    BarcodeFormat.QR_CODE,
                    QRcodeWidth, QRcodeWidth, null
            );

        } catch (IllegalArgumentException Illegalargumentexception) {

            return null;
        }
        int bitMatrixWidth = bitMatrix.getWidth();

        int bitMatrixHeight = bitMatrix.getHeight();

        int[] pixels = new int[bitMatrixWidth * bitMatrixHeight];

        for (int y = 0; y < bitMatrixHeight; y++) {
            int offset = y * bitMatrixWidth;

            for (int x = 0; x < bitMatrixWidth; x++) {

                pixels[offset + x] = bitMatrix.get(x, y) ?
                        0xFF000000:0xFFFFFFFF;
            }
        }
        Bitmap bitmap = Bitmap.createBitmap(bitMatrixWidth, bitMatrixHeight, Bitmap.Config.ARGB_4444);

        bitmap.setPixels(pixels, 0, 500, 0, 0, bitMatrixWidth, bitMatrixHeight);
        return bitmap;
    }
}
