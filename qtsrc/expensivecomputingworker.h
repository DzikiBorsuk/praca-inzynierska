#ifndef EXPENSIVECOMPUTINGWORKER_H
#define EXPENSIVECOMPUTINGWORKER_H

#include <QObject>
#include <QElapsedTimer>
#include <vector>
#include "../src/Stereo.h"

class expensiveComputingWorker : public QObject
{
    Q_OBJECT
public:


signals:
    void finish_loadImageList(const QString& msg);
    void finish_runCalibration(const QString& msg);
    void finish_featureMatching(const QString& msg);
    void finish_imageRectification(const QString& msg);

public slots:

   void loadImagesList(Stereo *stereo, std::vector<std::string> imagesList)
   {
       QElapsedTimer timer;
       timer.start();

       stereo->calib.loadImagesList(imagesList);

       auto elapsed = timer.elapsed() / 1000.0;

       QString msg = "Images Loaded successfully. Time: " + QString::number(elapsed) + "s";
       emit finish_loadImageList(msg);
   }

   void runCalibration(Stereo *stereo, int width, int height)
   {

       QElapsedTimer timer;
       timer.start();

       stereo->calib.findCalibrationPoints({width, height});
       stereo->calib.calibrateCamera();
       stereo->calib.calculateReprojectionError();
       stereo->calib.undistortCalibrationImages();
       //window->stereo.calib.drawPattern();

       auto elapsed = timer.elapsed() / 1000.0;

       QString msg = "Calibration finished. Time: " + QString::number(elapsed) + "s";
       emit finish_runCalibration(msg);
   }

   void runFeatureMatching(Stereo *stereo)
   {
       QElapsedTimer timer;
       timer.start();

       stereo->featureMatching.detect2Keypoints();
       stereo->featureMatching.extract2Descriptor();
       stereo->featureMatching.match2Keypoints();

       auto elapsed = timer.elapsed() / 1000.0;

       QString msg = "Feature matching finished. Time: " + QString::number(elapsed) + "s";

       emit finish_featureMatching(msg);
   }

   void runImageRectification(Stereo *stereo, int rectificationMethod, int rejectionMethod, double threshold, double confidence, int iterations)
   {
       QElapsedTimer timer;
       timer.start();

       stereo->rect.rejectOutliers(rejectionMethod,threshold,confidence,iterations);
       stereo->rect.rectifyImages(rectificationMethod,stereo->getOrgLeft(),
                                         stereo->calib.getCameraMatrix(), stereo->calib.getDistortionCoefficient(),
                                         stereo->getOrgRight(),
                                         stereo->calib.getCameraMatrix(), stereo->calib.getDistortionCoefficient());

       auto elapsed = timer.elapsed() / 1000.0;

       QString msg = "Rectification finished. Time " + QString::number(elapsed) + "s";

       emit finish_imageRectification(msg);
   }
};

#endif // EXPENSIVECOMPUTINGWORKER_H
