#-------------------------------------------------
#
# Project created by QtCreator 2018-11-20T14:38:34
#
#-------------------------------------------------

QT       += core gui
QT       += sql
QT       += widgets


CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = praca-inzynierska
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#INCLUDEPATH += /usr/local/include
INCLUDEPATH += $$PWD/../../openCV/3.4.4/BUILD-3.4.4/install/include

unix:!macx: LIBS += -L/usr/local/lib -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_highgui -lopencv_flann -lopencv_features2d -lopencv_calib3d -lopencv_ximgproc -lopencv_xfeatures2d

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../openCV/3.4.4/BUILD-3.4.4/install/x64/vc15/lib/ -lopencv_core344 -lopencv_imgproc344 -lopencv_imgcodecs344 -lopencv_highgui344 -lopencv_flann344 -lopencv_features2d344 -lopencv_calib3d344 -lopencv_ximgproc344 -lopencv_xfeatures2d344
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../openCV/3.4.4/BUILD-3.4.4/install/x64/vc15/lib/ -lopencv_core344d -lopencv_imgproc344d -lopencv_imgcodecs344d -lopencv_highgui344d -lopencv_flann344d -lopencv_features2d344d -lopencv_calib3d344d -lopencv_ximgproc344d -lopencv_xfeatures2d344d

SOURCES += \
    qtsrc/mainwindow.cpp \
    qtsrc/mainqt.cpp \
    src/Calibration.cpp \
    src/Disparity.cpp \
    src/FeatureMatching.cpp \
    src/Rectification.cpp \
    src/Stereo.cpp

HEADERS += \
    qtsrc/mainwindow.h \
    qtsrc/expensivecomputingworker.h \
    src/Calibration.h \
    src/Disparity.h \
    src/FeatureMatching.h \
    src/Rectification.h \
    src/Stereo.h

FORMS += \
    qtsrc/mainwindow.ui

DISTFILES +=
