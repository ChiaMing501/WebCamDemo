#ifndef TESTOPENCV_H
#define TESTOPENCV_H

#include <QMainWindow>
#include <QMessageBox>
#include <QDebug>
#include <QImage>
#include <QString>
#include <QDateTime>
#include <QTimer>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio/legacy/constants_c.h>

#include <sys/utsname.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

QT_BEGIN_NAMESPACE
namespace Ui { class TestOpencv; }
QT_END_NAMESPACE

typedef enum
{
    CHANNEL_B = 0,
    CHANNEL_G,
    CHANNEL_R

} ColorChannel;

class TestOpencv : public QMainWindow
{
    Q_OBJECT

    public:
        TestOpencv(QWidget *parent = nullptr);
        ~TestOpencv();

    signals:
        void sig_changeUI(QString str);

    private slots:
        void slot_changeUI(QString atr);

        void on_showButton_clicked();
        void on_modifyRgbButton_clicked();
        void on_outputMsgButton_clicked();
        void displayLiveView();
        void stopLiveView();
        void nextFrame();

    private:
        Ui::TestOpencv *ui;

        cv::VideoCapture *capturePtr;
        cv::Mat          *framePtr;
        QTimer           *timerPtr;
        QImage           qImage;
        QPixmap          qPixmap;

        pthread_t m_tid;

        static void *threadFun(void *arg);

        QImage cvMat2QImage(const cv::Mat& mat);
        void toGrayImage(const cv::Mat &sourceImage, cv::Mat &outputImage, const int imageHeight, const int imageWidth);
        void toNegativeImage(const cv::Mat &sourceImage, cv::Mat &outputImage, const int imageHeight, const int imageWidth);

}; //end of class TestOpencv

#endif // TESTOPENCV_H
