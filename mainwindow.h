﻿#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "c_gradation.h"
#include "opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "deque"
#include "ui_hist_hdr.h"
#include "ui_local_hdr.h"
#include "ui_imageinfo.h"
#include "ui_contrast.h"
#include "ui_denoise.h"
#include "ui_about.h"
#include "QListWidget"
#include "processing.h"
#include <QtConcurrent/QtConcurrent>
#include "qscrollbar.h"
#include "QWheelEvent"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    void settext(QString arg);
    explicit MainWindow(QWidget *parent = 0);
    void initialize();
    ~MainWindow();

private slots:

    void r_imageshort(cv::Mat_<unsigned short> img);

    void r_imagechar(cv::Mat_<unsigned char> img);

//    void on_showimgscale_sliderMoved(int position);

    void r_cancel();

    void r_ok(cv::Mat_<unsigned short> a);

    void r_lhdr_dst(cv::Mat a);

    void on_openfile_triggered();

    void on_hist_triggered();

    void on_rate_editingFinished();

    void on_invert_triggered();

    void on_back_triggered();

    void on_hist_hdr_triggered();

    void on_localadaptive_hdr_triggered();

    void on_resave_triggered();

    void on_info_triggered();

    void on_showimgscale_valueChanged(int value);

    void on_turn_horizontal_triggered();

    void on_turn_vertical_triggered();

    void on_contrast_triggered();

    void on_denoise_triggered();

    void on_about_triggered();



    void on_exit_triggered();

    void on_minus_clicked();

    void on_plus_clicked();

    void on_imagelist_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

signals:
    void s_imageshort(cv::Mat_<unsigned short>);
    void s_hist(cv::Mat_<unsigned char>);
    void s_number(int);

private:
    Ui::MainWindow *ui;
    QImage showimg;
    QImage tshowimg;
    c_gradation* w1;
    ui_hist_hdr* w2;
    ui_local_hdr* w3;
    ui_imageinfo* w4;
    processing* w_process;
    ui_about* w5;
    ui_contrast* w_contrast;
    ui_denoise* w_denoise;

    cv::Mat_<unsigned char> srcimgchar;
    cv::Mat_<unsigned short> srcimgshort;
    std::deque<cv::Mat_<unsigned short>> backup;
    void enableaction();
    void disableaction();
    void showimage(cv::Mat_<unsigned short> s);

    QPoint scrollpos;
    bool scrollclicked;

    QPoint cpos;
    bool cclicked;
    int ingray;

    int ow;
    int oh;


    void openfile(QString filename, int type);

    void reset();

protected:
    bool eventFilter(QObject *target, QEvent *e);


};

#endif // MAINWINDOW_H
