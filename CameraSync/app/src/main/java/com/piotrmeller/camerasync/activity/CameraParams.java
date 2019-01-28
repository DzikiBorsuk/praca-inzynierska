package com.piotrmeller.camerasync.activity;

import android.content.Context;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraManager;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Size;
import android.util.SizeF;

import com.piotrmeller.camerasync.R;

public class CameraParams extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_camera_params);

        CameraManager manager = (CameraManager) this.getSystemService(Context.CAMERA_SERVICE);

        try
        {
            for(String cameraID: manager.getCameraIdList())
            {
                CameraCharacteristics cameraCharacteristics = manager.getCameraCharacteristics(cameraID);
                float[] intrinsic = cameraCharacteristics.get(CameraCharacteristics.LENS_INTRINSIC_CALIBRATION);
                float[] distortion = cameraCharacteristics.get(CameraCharacteristics.LENS_RADIAL_DISTORTION);
                float[] rotation = cameraCharacteristics.get(CameraCharacteristics.LENS_POSE_ROTATION);
                float[] translation = cameraCharacteristics.get(CameraCharacteristics.LENS_POSE_TRANSLATION);
                float[] focal = cameraCharacteristics.get(CameraCharacteristics.LENS_INFO_AVAILABLE_FOCAL_LENGTHS);
                Size resolution = cameraCharacteristics.get(CameraCharacteristics.SENSOR_INFO_PIXEL_ARRAY_SIZE);
                SizeF sensor = cameraCharacteristics.get(CameraCharacteristics.SENSOR_INFO_PHYSICAL_SIZE);
                String aaaa= "fdfdssdf";


            }

        }
        catch (Exception e)
        {

        }
    }
}
