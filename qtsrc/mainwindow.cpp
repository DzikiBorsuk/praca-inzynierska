#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>
#include <thread>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), stereo("img/2.JPG","img/3.JPG","img/out_camera_data.xml"), compute_disparity(false),is_computing_disparity(false)
{


    ui->setupUi(this);

    ui->slider_minDisparity->setRange(-320,320);ui->slider_minDisparity->setValue(-32);
    ui->slider_maxDisparity->setRange(-320,320);ui->slider_maxDisparity->setValue(64);
    ui->slider_blockSize->setRange(1,51);
    ui->slider_P1->setRange(0,10000);
    ui->slider_P2->setRange(0,10000);
    ui->slider_disp12MaxDiff->setRange(0,320);
    ui->slider_preFilterCap->setRange(0,300);
    ui->slider_uniquenessRatio->setRange(0,100);
    ui->slider_speckleWindowSize->setRange(0,51);
    ui->slider_speckleRange->setRange(0,300);


        QListIterator<QObject *> i(ui->frame_disparity->children());

        //int a=0;
        while (i.hasNext()) {
            QSlider* slider;
            if(slider = qobject_cast<QSlider*>(i.next()))
            {
                connect(slider, SIGNAL(sliderMoved(int)), this, SLOT(on_slider_moved(int)));
                //a++;
            }
        }

        //cv::namedWindow(std::to_string(a));

        //connect(ui->slider_blockSize, SIGNAL(sliderMoved(int)), this, SLOT(on_slider_moved(int)));

        //ui->label_disparity->setPixmap(disparity_pixmap);

        init_disparity_tab();


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_slider_moved(int position)
{
    QObject* obj = sender();

   // std::string name = obj->objectName();

    if(obj==ui->slider_minDisparity)
    {
        int value = abs(position);

        value = value - value%16;

        if(position<0)
        {
            value=-value;
        }

        stereo.disp.setMinDisparity(value);

        ui->label_minDisparity_value->setNum(value);

    }else if(obj==ui->slider_maxDisparity)
    {
        int value = abs(position);

        value = value - value%16;

        if(position<0)
        {
            value=-value;
        }


        stereo.disp.setMaxDisparity(value);

         ui->label_maxDisparity_value->setNum(value);

    }
    else if(obj==ui->slider_blockSize)
        {
            int value = position;

            value = value - (value%2-1);

            stereo.disp.setBlockSize(value);

            ui->label_blockSize_value->setNum(value);

        }
    else if(obj==ui->slider_P1)
        {
            int value = position;

            stereo.disp.setP1(value);

            ui->label_P1_value->setNum(value);

        }
    else if(obj==ui->slider_P2)
        {
            int value = position;

            stereo.disp.setP2(value);

            ui->label_P2_value->setNum(value);

        }
    else if(obj==ui->slider_disp12MaxDiff)
        {
            int value = position;

            stereo.disp.setDisp12MaxDiff(value);

            ui->label_disp12MaxDiff_value->setNum(value);

        }
    else if(obj==ui->slider_preFilterCap)
        {
            int value = position;

            stereo.disp.setPreFilterCap(value);

            ui->label_preFilterCap_value->setNum(value);

        }
    else if(obj==ui->slider_uniquenessRatio)
        {
            int value = position;

            stereo.disp.setUniquenessRatio(value);

            ui->label_uniquenessRatio_value->setNum(value);

        }
    else if(obj==ui->slider_speckleWindowSize)
        {
        int value = position;

        if(value != 0)
        {
        value = value - (value%2-1);
        }

        stereo.disp.setSpeckleWindowSize(value);

        ui->label_speckleWindowSize_value->setNum(value);

        }
    else if(obj==ui->slider_speckleRange)
        {
        int value = position;

        stereo.disp.setSpeckleRange(value);

        ui->label_speckleRange_value->setNum(value);

        }

    compute_disp_init();
}

void MainWindow::compute_disp_init()
{
    if(is_computing_disparity==false)
    {
        is_computing_disparity=true;
        compute_disparity=false;
        std::thread disparityThread(compute_disp,this);
        //disparityThread.join();
        disparityThread.detach();
    }
    else
    {
        compute_disparity=true;
    }

}

void MainWindow::compute_disp(MainWindow *window)
{
    //stereo.computeDisp();
    window->stereo.computeDisp();
    window->show_disp();
}

void MainWindow::show_disp()
{
    is_computing_disparity=false;

    cv::Mat disp = stereo.disp.vis_filter;

    //cv::Mat disp_color;
    //cv::cvtColor(disp, disp_color, CV_GRAY2RGB);

    // we finally can convert the image to a QPixmap and display it
    QImage disparity_image = QImage((unsigned char*) disp.data, disp.cols, disp.rows, QImage::Format_Grayscale8);
    QPixmap disparity_pixmap = QPixmap::fromImage(disparity_image);


    // some computation to resize the image if it is too big to fit in the GUI
    int max_width  = std::min(ui->label_disparity->minimumWidth(),  disparity_image.width());
    int max_height = std::min(ui->label_disparity->minimumHeight(), disparity_image.height());
    ui->label_disparity->setPixmap(disparity_pixmap.scaled(max_width, max_height, Qt::KeepAspectRatio));

    if(compute_disparity==true)
    {
        compute_disp_init();

    }
}

void MainWindow::init_disparity_tab()
{
    stereo.match_feautures();
    stereo.rectifyImage();
    stereo.disp.initSGBM();
    stereo.computeDisp();

    cv::Mat disp = stereo.disp.vis_filter;

    //cv::Mat disp_color;
    //cv::cvtColor(disp, disp_color, CV_GRAY2RGB);

    // we finally can convert the image to a QPixmap and display it
    QImage disparity_image = QImage((unsigned char*) disp.data, disp.cols, disp.rows, QImage::Format_Grayscale8);
    QPixmap disparity_pixmap = QPixmap::fromImage(disparity_image);


    // some computation to resize the image if it is too big to fit in the GUI
    int max_width  = std::min(ui->label_disparity->minimumWidth(),  disparity_image.width());
    int max_height = std::min(ui->label_disparity->minimumHeight(), disparity_image.height());
    ui->label_disparity->setPixmap(disparity_pixmap.scaled(max_width, max_height, Qt::KeepAspectRatio));



    cv::Mat left_bgr = stereo.rect.getLeft();
    cv::Mat right_bgr = stereo.rect.getRight();

    cv::Mat left, right;
    cv::cvtColor(left_bgr, left, CV_BGR2RGB);
    cv::cvtColor(right_bgr, right, CV_BGR2RGB);

    // we finally can convert the image to a QPixmap and display it
    QImage left_image = QImage((unsigned char*) left.data, left.cols, left.rows, QImage::Format_RGB888);
    QPixmap left_pixmap = QPixmap::fromImage(left_image);
    QImage right_image = QImage((unsigned char*) right.data, right.cols, right.rows, QImage::Format_RGB888);
    QPixmap right_pixmap = QPixmap::fromImage(right_image);


    // some computation to resize the image if it is too big to fit in the GUI
    int max_width_left  = std::min(ui->label_leftImage->minimumWidth(),  left_image.width());
    int max_height_left = std::min(ui->label_leftImage->minimumHeight(), left_image.height());
    ui->label_leftImage->setPixmap(left_pixmap.scaled(max_width_left, max_height_left, Qt::KeepAspectRatio));

    int max_width_right  = std::min(ui->label_rightImage->minimumWidth(),  right_image.width());
    int max_height_right = std::min(ui->label_rightImage->minimumHeight(), right_image.height());
    ui->label_rightImage->setPixmap(right_pixmap.scaled(max_width_right, max_height_right, Qt::KeepAspectRatio));
}

