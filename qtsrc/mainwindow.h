#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "../src/Stereo.h"

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

private slots:

    void on_slider_moved(int position);

private:
    Ui::MainWindow *ui;

    Stereo stereo;

    bool compute_disparity;
    bool is_computing_disparity;

    void compute_disp_init();
    //void compute_disp();
    void show_disp();

    void init_disparity_tab();


};

#endif // MAINWINDOW_H
