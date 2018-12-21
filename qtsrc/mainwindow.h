#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlTableModel>
#include <QThread>
#include "../src/Stereo.h"
#include "expensivecomputingworker.h"

#include <vector>
#include <string>

namespace Ui
{
class MainWindow;
}

class MainWindow: public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


private slots:

    void on_button_loadImages_clicked();

    void on_slider_moved(int position);

    void on_button_calibrate_clicked();

    void on_tableWidget_imagesList_cellDoubleClicked(int row, int column);

    void on_checkBox_showUndistorted_stateChanged(int arg1);

    void on_actionSave_camera_params_triggered();

    void on_actionLoad_camera_params_triggered();

    void on_actionLeft_image_triggered();

    void on_actionRight_image_triggered();

    void on_button_setDetector_clicked();

    void on_button_setDescriptor_clicked();

    void on_button_setMatcher_clicked();

    void on_button_matchFeatures_clicked();

    void on_comboBox_detector_currentIndexChanged(int index);

    void on_comboBox_descriptor_currentIndexChanged(int index);

    void on_comboBox_matcher_currentIndexChanged(int index);

    void on_pushButton_runRectification_clicked();

    void on_checkBox_showRectified_stateChanged(int arg1);


     //########################### calibration tab ###########################

    void load_calibration_images_finished(const QString& msg);

    void calibration_finished(const QString& msg);

    //########################### feature matching tab ###########################

    void feature_matching_finished(const QString& msg);

    //########################### rectification tab ###########################

    void image_rectification_finished(const QString& msg);

private:
    Ui::MainWindow *ui;

    Stereo stereo;

    expensiveComputingWorker *worker;

    QThread *computingThread;

    void resizeEvent(QResizeEvent *event) override;

    //########################### calibration tab ###########################

    bool newImages;

    void init_calibration_tab();

    void load_calibration_images_init();

    void show_calibration_image(int i, bool undistorted);

    //########################### feature matching tab ###########################

    void init_feature_matching_tab();

    void show_matchedFeatures();


    //########################### rectification tab ###########################

    void init_rectification_tab();

    void show_rectified_images();

    //########################### disparity tab ###########################


    static void compute_disp(MainWindow *window);

    bool compute_disparity;
    bool is_computing_disparity;

    void compute_disp_init();
    //void compute_disp();
    void show_disp();

    void init_disparity_tab();

};

#endif // MAINWINDOW_H
