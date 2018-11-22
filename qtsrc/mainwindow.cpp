#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //QObjectList widgetList = ui->layout_disparity->children();


//    QSignalMapper *signalMapper = new QSignalMapper(this);
//    connect(signalMapper, SIGNAL(mapped(int)), this, SIGNAL(digitClicked(int)));

//    QListIterator<QObject *> i(ui->layout_disparity->children());

//    while (i.hasNext()) {
//        QSlider* slider;

//        if(slider = qobject_cast<QSlider*>(i.next()))
//        {

//        }
//    }

//    for (int i = 0; i < 10; ++i) {
//        QString text = QString::number(i);
//        buttons[i] = new QPushButton(text, this);
//        signalMapper->setMapping(buttons[i], i);
//        connect(buttons[i], SIGNAL(clicked()), signalMapper, SLOT(map()));
//    }

        QListIterator<QObject *> i(ui->frame_disparity->children());

        int a=0;
        while (i.hasNext()) {
            QSlider* slider;
            if(slider = qobject_cast<QSlider*>(i.next()))
            {
                connect(slider, SIGNAL(sliderMoved(int)), this, SLOT(on_slider_moved(int)));
                a++;
            }
        }

        cv::namedWindow(std::to_string(a));

        //connect(ui->slider_blockSize, SIGNAL(sliderMoved(int)), this, SLOT(on_slider_moved(int)));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_slider_moved(int position)
{

}

