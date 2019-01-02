#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QElapsedTimer>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>
#include <thread>

MainWindow::MainWindow(QWidget *parent)
    :
    QMainWindow(parent),
    ui(new Ui::MainWindow), worker(new expensiveComputingWorker()), computingThread(new QThread()), newImages(false), compute_disparity(false), is_computing_disparity(false)
{


    ui->setupUi(this);

    qRegisterMetaType<Stereo*>("Stereo*");
    qRegisterMetaType<std::vector<std::string>>("std::vector<std::string>");

    worker->moveToThread(this->computingThread);
    computingThread->start();




    ui->slider_minDisparity->setRange(-320, 320);
    ui->slider_minDisparity->setValue(-32);
    ui->slider_maxDisparity->setRange(-320, 320);
    ui->slider_maxDisparity->setValue(64);
    ui->slider_blockSize->setRange(1, 51);
    ui->slider_P1->setRange(0, 10000);
    ui->slider_P2->setRange(0, 10000);
    ui->slider_disp12MaxDiff->setRange(0, 320);
    ui->slider_preFilterCap->setRange(0, 300);
    ui->slider_uniquenessRatio->setRange(0, 100);
    ui->slider_speckleWindowSize->setRange(0, 51);
    ui->slider_speckleRange->setRange(0, 300);


    QListIterator < QObject * > i(ui->frame_disparity->children());

    //int a=0;
    while (i.hasNext())
    {
        QSlider *slider;
        if ((slider = qobject_cast<QSlider *>(i.next())))
        {
            connect(slider, SIGNAL(valueChanged(int)), this, SLOT(on_slider_moved(int)));
            //a++;
        }
    }


    init_calibration_tab();
    init_feature_matching_tab();
    init_rectification_tab();

}

MainWindow::~MainWindow()
{
    delete ui;
    computingThread->exit();

    if(!computingThread->wait(5000)) //Wait until it actually has terminated (max. 5 sec)
    {
    qWarning("Thread deadlock detected, bad things may happen !!!");
    computingThread->terminate(); //Thread didn't exit in time, probably deadlocked, terminate it!
    computingThread->wait(); //Note: We have to wait again here!
    }
    delete computingThread;
    delete worker;

}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    // Your code here.

    //init_disparity_tab();

    this->newImages = true;
    this->show_calibration_image(-1, ui->checkBox_showUndistorted->isChecked());
    this->show_matchedFeatures();
}

void MainWindow::on_actionLeft_image_triggered()
{
    QString imageFile = QFileDialog::getOpenFileName(this,
                                                     tr("Load left image"), "",
                                                     tr("Image files (*.png *.PNG *.jpeg *.jpg *.JPG *.jpe *.bmp *.dib);;All files (*.*)"));

    //QString cameraParamsFile = QFileDialog::getOpenFileName(this,
    //                                                        tr("Load left camera params"), "",
    //                                                        tr("opencv XML/YAML files (*.xml *.yml);;All Files (*)"));

    if(!imageFile.isEmpty())
    {
        this->stereo.loadLeftImage(imageFile.toUtf8().constData());
        this->show_matchedFeatures();
        this->show_rectified_images();
    }
}

void MainWindow::on_actionRight_image_triggered()
{
    QString imageFile = QFileDialog::getOpenFileName(this,
                                                     tr("Load right image"), "",
                                                     tr("Image files (*.png *.PNG *.jpeg *.jpg *.JPG *.jpe *.bmp *.dib);;All files (*.*)"));

    //QString cameraParamsFile = QFileDialog::getOpenFileName(this,
    //                                                        tr("Load right camera params"), "",
    //                                                        tr("opencv XML/YAML files (*.xml *.yml);;All Files (*)"));

    if(!imageFile.isEmpty())
    {
        this->stereo.loadRightImage(imageFile.toUtf8().constData());
        this->show_matchedFeatures();
        this->show_rectified_images();
    }
}

//###################################################### calibration tab ######################################################

void MainWindow::init_calibration_tab()
{
    qRegisterMetaType < QList < QPersistentModelIndex >> ("QList<QPersistentModelIndex>");
    qRegisterMetaType<QAbstractItemModel::LayoutChangeHint>("QAbstractItemModel::LayoutChangeHint");

    ui->tableWidget_imagesList->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget_imagesList->setColumnCount(2);
    QStringList header;
    header << "Image" << "Reprojection Error";
    ui->tableWidget_imagesList->setHorizontalHeaderLabels(header);
    ui->tableWidget_imagesList->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget_imagesList->verticalHeader()->setVisible(false);

    ui->label_CalibrationImagePreviev->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    QObject::connect(worker, SIGNAL(finish_loadImageList(const QString&)), this, SLOT(load_calibration_images_finished(const QString&)));
    QObject::connect(worker, SIGNAL(finish_runCalibration(const QString&)), this, SLOT(calibration_finished(const QString&)));
}

void MainWindow::on_actionLoad_camera_params_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Save camera params"), "",
                                                    tr("opencv XML/YAML files (*.xml *.yml);;All Files (*)"));

    if(!fileName.isEmpty())
    {
        this->stereo.calib.loadParams(fileName.toUtf8().constData());
    }
}

void MainWindow::on_actionSave_camera_params_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save camera params"), "",
                                                    tr("opencv XML/YAML files (*.xml *yml *.yaml);;All Files (*)"));

    if(!fileName.isEmpty())
    {
        this->stereo.calib.saveParams(fileName.toUtf8().constData());
    }
}

void MainWindow::on_button_loadImages_clicked()
{
    QStringList filenames = QFileDialog::getOpenFileNames(this,
                                                          tr("Image calibration list"),
                                                          QDir::currentPath(),
                                                          tr("Image files (*.png *.PNG *.jpeg *.jpg *.JPG *.jpe *.bmp *.dib);;All files (*.*)"));
    if (!filenames.isEmpty())
    {
        std::vector<std::string> imagesList;
        imagesList.reserve(filenames.size());
        for (int i = 0; i < filenames.size(); i++)
        {
            imagesList.push_back(filenames.at(i).toUtf8().constData());
        }

        //ui->tableWidget_imagesList->clear();
        ui->tableWidget_imagesList->setRowCount(0);
        //std::thread thread(load_calibration_images, this, imagesList);

        QMetaObject::invokeMethod( worker, "loadImagesList",Q_ARG(Stereo*, &stereo) ,Q_ARG( std::vector<std::string>, imagesList));

        newImages = true;

        ui->button_loadImages->setEnabled(false);
        ui->button_calibrate->setEnabled(false);
        ui->label_status->setText("Loading images");
    }
}

void MainWindow::on_button_calibrate_clicked()
{
    //TODO: sprawdzić poprawność width i height
    int width = this->ui->lineEdit_patternWidth->text().toInt();
    int height = this->ui->lineEdit_patternHeight->text().toInt();

    QMetaObject::invokeMethod( worker, "runCalibration",Q_ARG(Stereo*, &stereo) ,Q_ARG( int, width), Q_ARG(int, height));

    ui->button_loadImages->setEnabled(false);
    ui->button_calibrate->setEnabled(false);
    ui->label_status->setText("Calibrating");
}

void MainWindow::on_tableWidget_imagesList_cellDoubleClicked(int row, int column)
{
    int i = ui->tableWidget_imagesList->item(row, 0)->text().toInt();
    show_calibration_image(i, ui->checkBox_showUndistorted->isChecked());
}

void MainWindow::on_checkBox_showUndistorted_stateChanged(int arg1)
{
    auto item = ui->tableWidget_imagesList->selectedItems();
    int row = 0;
    if (!item.isEmpty())
    {
        row = item.at(0)->row();

    }
    row = ui->tableWidget_imagesList->item(row, 0)->text().toInt();
    show_calibration_image(row, ui->checkBox_showUndistorted->isChecked());
}

void MainWindow::load_calibration_images_finished(const QString& msg)
{

    this->ui->button_loadImages->setEnabled(true);
    this->ui->button_calibrate->setEnabled(true);
    this->ui->label_status->setText(msg);

    this->show_calibration_image(0, false);
}

void MainWindow::calibration_finished(const QString& msg)
{

    double error = this->stereo.calib.getAvgError2();
    auto goodImages = this->stereo.calib.getGoodImages();
    auto reprojectionErrors = this->stereo.calib.getReprojectionErrorsArray();
    this->ui->tableWidget_imagesList->setRowCount(goodImages.size());

    for (int i = 0; i < goodImages.size(); ++i)
    {
        this->ui->tableWidget_imagesList->setItem(i, 0, new QTableWidgetItem(QString::number(i)));
        if (goodImages[i])
        {
            std::cout<<i<<std::endl;
            this->ui->tableWidget_imagesList
                ->setItem(i, 1, new QTableWidgetItem(QString::number(reprojectionErrors[i])));
        }
        else
        {
            this->ui->tableWidget_imagesList->setItem(i, 1, new QTableWidgetItem("not found"));
        }
    }
    this->ui->tableWidget_imagesList->sortItems(1);
    this->ui->label_avgRepError_value->setText(QString::number(error));


    this->ui->button_loadImages->setEnabled(true);
    this->ui->button_calibrate->setEnabled(true);
    this->ui->label_status->setText(msg);
}

void MainWindow::show_calibration_image(int i, bool undistorted)
{

    static int last_i = 0;
    static bool last_undistorted = false;

    if (i == -1)
    {
        i = last_i;
    }

    if (last_i != i || last_undistorted != undistorted || this->newImages)
    {
        last_i = i;
        last_undistorted = undistorted;
        this->newImages = false;

        cv::Mat image, src;

        if (undistorted)
        {
            src = this->stereo.calib.getUndistortedImage(i);
        }
        else
        {
            src = this->stereo.calib.getImage(i);
        }

        if (!src.empty())
        {
            cv::cvtColor(src, image, CV_BGR2RGB);
        }
        //cv::Mat disp_color;
        //cv::cvtColor(disp, disp_color, CV_GRAY2RGB);

        // we finally can convert the image to a QPixmap and display it
        QImage qt_image = QImage((unsigned char *) image.data, image.cols, image.rows, QImage::Format_RGB888);
        QPixmap qt_pixmap = QPixmap::fromImage(qt_image);


        // some computation to resize the image if it is too big to fit in the GUI
        //int max_width  = std::min(ui->label_disparity->minimumWidth(),  disparity_image.width());
        //int max_height = std::min(ui->label_disparity->minimumHeight(), disparity_image.height());
        int max_width = ui->label_CalibrationImagePreviev->width();
        int max_height = ui->label_CalibrationImagePreviev->height();
        ui->label_CalibrationImagePreviev->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui->label_CalibrationImagePreviev->setPixmap(qt_pixmap.scaled(max_width, max_height, Qt::KeepAspectRatio));

    }
}



//###################################################### feature matching tab ######################################################


void MainWindow::init_feature_matching_tab()
{
    QObject::connect(worker, SIGNAL(finish_featureMatching(const QString&)), this, SLOT(feature_matching_finished(const QString&)));

    QStringList detectorsList = (QStringList() << "SIFT" << "SURF" << "ORB" << "AKAZE" << "KAZE" << "BRISK");
    ui->comboBox_detector->addItems(detectorsList);

    QStringList descriptorsList = (QStringList() << "SIFT" << "SURF" << "ORB" << "AKAZE" << "KAZE" << "BRISK");
    ui->comboBox_descriptor->addItems(descriptorsList);

    QStringList matchersList = (QStringList() << "FlannBased" << "BruteForce" << "BrueForce-L1" << "BruteForce-Hamming"
                                              << "BruteForce-Hamming(2)");
    ui->comboBox_matcher->addItems(matchersList);


    ui->label_matchedFeatures->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

}

void MainWindow::on_comboBox_detector_currentIndexChanged(int index)
{
    //TODO QObject (list or vector)
    std::vector<QString> options;
    std::vector<QString> default_value;

    switch (index)
    {
        case 0:options = {"n features", "n octave layers", "contrast threshold", "edge threshold", "sigma"};
            default_value = {"0", "3", "0.04", "10", "1.6"};
            break;
        case 1:options = {"hessian threshold", "n octaes", "n octaves layer", "skip orientations"};
            default_value = {"100", "4", "3", "0"};
            break;
        case 2:options =
                   {"n features", "scale factor", " n levels", "edge threshold", "first level", "WTA_K", "score type",
                    "patch size", "fast threshold"};
            default_value = {"500", "1.2", "8", "31", "0", "2", "0", "31", "20"};
            break;
        case 3:options = {"descriptor type", "descriptor size", "descriptor channels", "threshold", "n octave",
                          "n octave layers", "diffusity"};
            default_value = {"5", "0", "3", "0.001", "4", "4", "1"};
            break;
        case 4:options = {"extended", "skip orientation", "threshold", "n octaves", "n octaves layers", "diffusity"};
            default_value = {"0", "0", "0.001", "4", "4", "1"};
            break;
        case 5:options = {"threshold", "n octave", "pattern scale"};
            default_value = {"30", "3", "1.0"};
            break;
        default:break;
    }

    ui->tableWidget_detectorParams->clear();
    ui->tableWidget_detectorParams->setColumnCount(2);
    ui->tableWidget_detectorParams->setRowCount(options.size());
    ui->tableWidget_detectorParams->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_detectorParams->verticalHeader()->setVisible(false);
    for (int i = 0; i < options.size(); ++i)
    {
        ui->tableWidget_detectorParams->setItem(i, 0, new QTableWidgetItem(options[i]));
        ui->tableWidget_detectorParams->setItem(i, 1, new QTableWidgetItem(default_value[i]));
    }
}

void MainWindow::on_comboBox_descriptor_currentIndexChanged(int index)
{
    std::vector<QString> options;
    std::vector<QString> default_value;

    switch (index)
    {
        case 0:options = {"n features", "n octave layers", "contrast threshold", "edge threshold", "sigma"};
            default_value = {"0", "3", "0.04", "10", "1.6"};
            break;
        case 1:options = {"hessian threshold", "n octaes", "n octaves layer", "skip orientations"};
            default_value = {"100", "4", "3", "0"};
            break;
        case 2:options =
                   {"n features", "scale factor", " n levels", "edge threshold", "first level", "WTA_K", "score type",
                    "patch size", "fast threshold"};
            default_value = {"500", "1.2", "8", "31", "0", "2", "0", "31", "20"};
            break;
        case 3:options = {"descriptor type", "descriptor size", "descriptor channels", "threshold", "n octave",
                          "n octave layers", "diffusity"};
            default_value = {"5", "0", "3", "0.001", "4", "4", "1"};
            break;
        case 4:options = {"extended", "skip orientation", "threshold", "n octaves", "n octaves layers", "diffusity"};
            default_value = {"0", "0", "0.001", "4", "4", "1"};
            break;
        case 5:options = {"threshold", "n octave", "pattern scale"};
            default_value = {"30", "3", "1.0"};
            break;
        default:break;
    }

    ui->tableWidget_descriptorParams->clear();
    ui->tableWidget_descriptorParams->setColumnCount(2);
    ui->tableWidget_descriptorParams->setRowCount(options.size());
    ui->tableWidget_descriptorParams->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_descriptorParams->verticalHeader()->setVisible(false);
    for (int i = 0; i < options.size(); ++i)
    {
        ui->tableWidget_descriptorParams->setItem(i, 0, new QTableWidgetItem(options[i]));
        ui->tableWidget_descriptorParams->setItem(i, 1, new QTableWidgetItem(default_value[i]));
    }
}

void MainWindow::on_comboBox_matcher_currentIndexChanged(int index)
{

}

void MainWindow::on_button_setDetector_clicked()
{
    //this->stereo.featureMatching.setDetector(0,std::vector<double>());
    std::vector<double> params;

    for (int i = 0; i < ui->tableWidget_detectorParams->rowCount(); ++i)
    {
        params.emplace_back(ui->tableWidget_detectorParams->item(i, 1)->text().toDouble());
    }
    int type = ui->comboBox_detector->currentIndex();

    this->stereo.featureMatching.setDetector(type, params);
}

void MainWindow::on_button_setDescriptor_clicked()
{
    std::vector<double> params;

    for (int i = 0; i < ui->tableWidget_descriptorParams->rowCount(); ++i)
    {
        params.emplace_back(ui->tableWidget_descriptorParams->item(i, 1)->text().toDouble());
    }
    int type = ui->comboBox_descriptor->currentIndex();

    this->stereo.featureMatching.setDescriptor(type, params);
}

void MainWindow::on_button_setMatcher_clicked()
{
    int type = ui->comboBox_matcher->currentIndex();
    this->stereo.featureMatching.setMatcher(type + 1, std::vector<double>());
}

void MainWindow::on_button_matchFeatures_clicked()
{
    this->ui->label_status->setText("Matching");

    QMetaObject::invokeMethod( worker, "runFeatureMatching",Q_ARG(Stereo*, &stereo));
}


void MainWindow::feature_matching_finished(const QString& msg)
{
    this->stereo.rect.setPoints(this->stereo.featureMatching.getPoints_left(),this->stereo.featureMatching.getPoints_right());

    auto leftKeypoints = QString::number(this->stereo.featureMatching.getNumOfLeftKeyPoints());
    auto rightKeypoints = QString::number(this->stereo.featureMatching.getNumOfRightKeyPoints());
    auto matches = QString::number(this->stereo.featureMatching.getNumOfMatches());

    this->ui->label_status->setText(msg+"   Left keypoints: "+leftKeypoints+"   Right keypoints: "+rightKeypoints+"     Matches: "+matches);

    this->show_matchedFeatures();
}


void MainWindow::show_matchedFeatures()
{

    //TODO: clean
    cv::Mat src = this->stereo.featureMatching.getFeatures();

    cv::Mat img;

    if (!src.empty())
    {
        cv::cvtColor(src, img, CV_BGR2RGB);
    }

    // we finally can convert the image to a QPixmap and display it
    QImage image = QImage((unsigned char *) img.data, img.cols, img.rows, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(image);


    // some computation to resize the image if it is too big to fit in the GUI
    //int max_width  = std::min(ui->label_disparity->minimumWidth(),  disparity_image.width());
    //int max_height = std::min(ui->label_disparity->minimumHeight(), disparity_image.height());
    int max_width = ui->label_matchedFeatures->width();
    int max_height = ui->label_matchedFeatures->height();
    ui->label_matchedFeatures->setPixmap(pixmap.scaled(max_width, max_height, Qt::KeepAspectRatio));
}

//###################################################### rectification tab ######################################################

void MainWindow::init_rectification_tab()
{
    ui->label_rectificationLeftImage->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->label_rectificationRightImage->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    QStringList rectificationMethods = (QStringList() << "Loop-Zhang" << "Hartley" << "DSR"<<"Pose estimation");
    ui->comboBox_rectificationMethod->addItems(rectificationMethods);

    QStringList rejectionMethods = (QStringList() << "RANSAC" << "LMEDS" << "RHO" << "None");
    ui->comboBox_rectificationRejectionMethod->addItems(rejectionMethods);

    ui->lineEdit_rectificationRejectionThreshold->setText("1.5");
    ui->lineEdit_rectificationRejectionConfidence->setText("0.995");
    ui->lineEdit_rectificationIterations->setText("2000");

    QObject::connect(worker, SIGNAL(finish_imageRectification(const QString&)), this, SLOT(image_rectification_finished(const QString&)));


}

void MainWindow::on_actionSave_rectified_images_triggered()
{
    QString OutputFolder = QFileDialog::getExistingDirectory(this, ("Select Output Folder"), QDir::currentPath());

    if(!OutputFolder.isEmpty())
    {
        stereo.saveRectifiedImages(OutputFolder.toUtf8().constData());
    }
}

void MainWindow::on_pushButton_runRectification_clicked()
{
    ui->pushButton_runRectification->setDisabled(true);
    ui->label_status->setText("Rectification in progress.");

    //std::thread thread(run_image_rectification,this);
    //thread.detach();

    int rejectionMethod = this->ui->comboBox_rectificationRejectionMethod->currentIndex();
    double threshold = this->ui->lineEdit_rectificationRejectionThreshold->text().toDouble();
    double confidence = this->ui->lineEdit_rectificationRejectionConfidence->text().toDouble();
    int iterations = this->ui->lineEdit_rectificationIterations->text().toInt();
    int rectificationMethod = this->ui->comboBox_rectificationMethod->currentIndex();

    QMetaObject::invokeMethod( worker, "runImageRectification",Q_ARG(Stereo*, &stereo),Q_ARG(int, rectificationMethod),Q_ARG(int, rejectionMethod),Q_ARG(double, threshold),Q_ARG(double,confidence),Q_ARG(int,iterations));
}

void MainWindow::on_checkBox_showRectified_stateChanged(int arg1)
{
    this->show_rectified_images();
}

void MainWindow::show_rectified_images()
{
    this->newImages = false;

    cv::Mat leftImg, rightImg, leftSrc, rightSrc;

    if (ui->checkBox_showRectified->isChecked())
    {
        leftSrc = this->stereo.rect.getRectImageLeft();
        rightSrc = this->stereo.rect.getRectImageRight();
    }
    else
    {
        leftSrc = this->stereo.rect.getRectImageLeft();
        rightSrc = this->stereo.rect.getRectImageRight();
    }


    if (!leftSrc.empty())
    {
        cv::cvtColor(leftSrc, leftImg, CV_BGR2RGB);
    }
    if(!rightSrc.empty())
    {
        cv::cvtColor(rightSrc, rightImg, CV_BGR2RGB);
    }

    QImage left_image = QImage((unsigned char *) leftImg.data, leftImg.cols, leftImg.rows, QImage::Format_RGB888);
    QPixmap left_pixmap = QPixmap::fromImage(left_image);

    QImage right_image = QImage((unsigned char *) rightImg.data, rightImg.cols, rightImg.rows, QImage::Format_RGB888);
    QPixmap right_pixmap = QPixmap::fromImage(right_image);


    int max_left_width = ui->label_rectificationLeftImage->width();
    int max_left_height = ui->label_rectificationLeftImage->height();
    int max_right_width = ui->label_rectificationRightImage->width();
    int max_right_height = ui->label_rectificationRightImage->height();

    ui->label_rectificationLeftImage->setPixmap(left_pixmap.scaled(max_left_width, max_left_height, Qt::KeepAspectRatio));
    ui->label_rectificationRightImage->setPixmap(right_pixmap.scaled(max_left_width, max_left_height, Qt::KeepAspectRatio));

}

void MainWindow::image_rectification_finished(const QString &msg)
{
    this->ui->label_status->setText(msg);

    this->ui->pushButton_runRectification->setDisabled(false);
    this->ui->label_showRectified->setDisabled(false);
    this->ui->checkBox_showRectified->setDisabled(false);
    this->ui->checkBox_showRectified->setChecked(true);
    this->show_rectified_images();
}

//###################################################### disparity tab ######################################################

void MainWindow::on_slider_moved(int position)
{
    QObject *obj = sender();

    // std::string name = obj->objectName();

    if (obj == ui->slider_minDisparity)
    {
        int value = abs(position);

        value = value - value % 16;

        if (position < 0)
        {
            value = -value;
        }

        stereo.disp.setMinDisparity(value);

        ui->label_minDisparity_value->setNum(value);

    }
    else if (obj == ui->slider_maxDisparity)
    {
        int value = abs(position);

        value = value - value % 16;

        if (position < 0)
        {
            value = -value;
        }


        stereo.disp.setMaxDisparity(value);

        ui->label_maxDisparity_value->setNum(value);

    }
    else if (obj == ui->slider_blockSize)
    {
        int value = position;

        value = value - (value % 2 - 1);

        stereo.disp.setBlockSize(value);

        ui->label_blockSize_value->setNum(value);

    }
    else if (obj == ui->slider_P1)
    {
        int value = position;

        stereo.disp.setP1(value);

        ui->label_P1_value->setNum(value);

    }
    else if (obj == ui->slider_P2)
    {
        int value = position;

        stereo.disp.setP2(value);

        ui->label_P2_value->setNum(value);

    }
    else if (obj == ui->slider_disp12MaxDiff)
    {
        int value = position;

        stereo.disp.setDisp12MaxDiff(value);

        ui->label_disp12MaxDiff_value->setNum(value);

    }
    else if (obj == ui->slider_preFilterCap)
    {
        int value = position;

        stereo.disp.setPreFilterCap(value);

        ui->label_preFilterCap_value->setNum(value);

    }
    else if (obj == ui->slider_uniquenessRatio)
    {
        int value = position;

        stereo.disp.setUniquenessRatio(value);

        ui->label_uniquenessRatio_value->setNum(value);

    }
    else if (obj == ui->slider_speckleWindowSize)
    {
        int value = position;

        if (value != 0)
        {
            value = value - (value % 2 - 1);
        }

        stereo.disp.setSpeckleWindowSize(value);

        ui->label_speckleWindowSize_value->setNum(value);

    }
    else if (obj == ui->slider_speckleRange)
    {
        int value = position;

        stereo.disp.setSpeckleRange(value);

        ui->label_speckleRange_value->setNum(value);

    }

    compute_disp_init();
}

void MainWindow::compute_disp_init()
{
    if (is_computing_disparity == false)
    {
        is_computing_disparity = true;
        compute_disparity = false;
        std::thread disparityThread(compute_disp, this);
        //disparityThread.join();
        disparityThread.detach();
    }
    else
    {
        compute_disparity = true;
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
    is_computing_disparity = false;

    cv::Mat disp = stereo.disp.vis_filter;

    //cv::Mat disp_color;
    //cv::cvtColor(disp, disp_color, CV_GRAY2RGB);

    // we finally can convert the image to a QPixmap and display it
    QImage disparity_image = QImage((unsigned char *) disp.data, disp.cols, disp.rows, QImage::Format_Grayscale8);
    QPixmap disparity_pixmap = QPixmap::fromImage(disparity_image);


    // some computation to resize the image if it is too big to fit in the GUI
    //int max_width  = std::min(ui->label_disparity->minimumWidth(),  disparity_image.width());
    //int max_height = std::min(ui->label_disparity->minimumHeight(), disparity_image.height());
    int max_width = ui->label_disparity->width();
    int max_height = ui->label_disparity->height();
    ui->label_disparity->setPixmap(disparity_pixmap.scaled(max_width, max_height, Qt::KeepAspectRatio));

    if (compute_disparity == true)
    {
        compute_disp_init();

    }
}

void MainWindow::init_disparity_tab()
{
    static bool a = true;

    if (a)
    {
        a = false;
        stereo.match_feautures();
        //stereo.rectifyImage();
        stereo.disp.initSGBM();
        stereo.computeDisp();
    }

    cv::Mat disp = stereo.disp.vis_filter;

    //cv::Mat disp_color;
    //cv::cvtColor(disp, disp_color, CV_GRAY2RGB);

    // we finally can convert the image to a QPixmap and display it
    QImage disparity_image = QImage((unsigned char *) disp.data, disp.cols, disp.rows, QImage::Format_Grayscale8);
    QPixmap disparity_pixmap = QPixmap::fromImage(disparity_image);


    // some computation to resize the image if it is too big to fit in the GUI
    //int max_width  = std::min(ui->label_disparity->width(),  disparity_image.width());
    //int max_height = std::min(ui->label_disparity->height(), disparity_image.height());
    int max_width = ui->label_disparity->width();
    int max_height = ui->label_disparity->height();
    ui->label_disparity->setPixmap(disparity_pixmap.scaled(max_width, max_height, Qt::KeepAspectRatio));


    cv::Mat left_bgr = stereo.rect.getRectImageLeft();
    cv::Mat right_bgr = stereo.rect.getRectImageRight();

    cv::Mat left, right;
    cv::cvtColor(left_bgr, left, CV_BGR2RGB);
    cv::cvtColor(right_bgr, right, CV_BGR2RGB);

    // we finally can convert the image to a QPixmap and display it
    QImage left_image = QImage((unsigned char *) left.data, left.cols, left.rows, QImage::Format_RGB888);
    QPixmap left_pixmap = QPixmap::fromImage(left_image);
    QImage right_image = QImage((unsigned char *) right.data, right.cols, right.rows, QImage::Format_RGB888);
    QPixmap right_pixmap = QPixmap::fromImage(right_image);

    // some computation to resize the image if it is too big to fit in the GUI
    int max_width_left = ui->label_leftImage->width();
    int max_height_left = ui->label_leftImage->height();
    ui->label_leftImage->setPixmap(left_pixmap.scaled(max_width_left, max_height_left, Qt::KeepAspectRatio));

    int max_width_right = ui->label_rightImage->width();
    int max_height_right = ui->label_rightImage->height();
    ui->label_rightImage->setPixmap(right_pixmap.scaled(max_width_right, max_height_right, Qt::KeepAspectRatio));
}
