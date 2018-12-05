#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlTableModel>
#include "../src/Stereo.h"

#include <vector>
#include <string>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    static void compute_disp(MainWindow *window);

    static void load_calibration_images(MainWindow *window, std::vector<std::string> imagesList);

    static void run_calibration(MainWindow *window);

private slots:

    void on_button_loadImages_clicked();

    void on_slider_moved(int position);

    void on_button_calibrate_clicked();

    void on_tableWidget_imagesList_cellDoubleClicked(int row, int column);

    void on_checkBox_showUndistorted_stateChanged(int arg1);

    void on_actionSave_camera_params_triggered();

    void on_actionLoad_camera_params_triggered();

private:
    Ui::MainWindow *ui;

    Stereo stereo;


    void resizeEvent(QResizeEvent *event) override;

    //########################### calibration tab ###########################

    bool newImages;

     void init_calibration_tab();

     void load_calibration_images_init();

     void show_calibration_image(int i, bool undistorted);


     //########################### disparity tab ###########################

     bool compute_disparity;
     bool is_computing_disparity;

    void compute_disp_init();
    //void compute_disp();
    void show_disp();

    void init_disparity_tab();


};

#endif // MAINWINDOW_H
