#include "TestOpencv.h"
#include "ui_TestOpencv.h"

using namespace cv;

TestOpencv::TestOpencv(QWidget *parent) : QMainWindow(parent), ui(new Ui::TestOpencv)
{
    ui->setupUi(this);

    connect(this, SIGNAL(sig_changeUI(QString)), this, SLOT(slot_changeUI(QString)));

    connect(ui->displayButton, SIGNAL(clicked()), this, SLOT(displayLiveView()));
    connect(ui->stopButton, SIGNAL(clicked()), this, SLOT(stopLiveView()));

    m_tid = 0;
    pthread_create(&m_tid, NULL, threadFun, (void *)this);
    pthread_detach(m_tid);

    capturePtr = nullptr;
    framePtr   = nullptr;
    timerPtr   = nullptr;

    capturePtr = new VideoCapture(0);
    framePtr   = new Mat();
    timerPtr   = new QTimer(this);

    ui->lineEdit->setReadOnly(true);
    ui->liveViewLineEdit->setReadOnly(true);
    ui->liveViewLineEdit->setText("Live View Status ==> Ready");

} //end of constructor

TestOpencv::~TestOpencv()
{
    delete ui;

    if(capturePtr != nullptr)
    {
        delete capturePtr;
        capturePtr = nullptr;
    }

    if(framePtr != nullptr)
    {
        delete framePtr;
        framePtr = nullptr;
    }

    if(timerPtr != nullptr)
    {
        timerPtr->stop();
        capturePtr->release();

        delete timerPtr;
        timerPtr = nullptr;
    }

} //end of destructor

void TestOpencv::displayLiveView()
{
    /*capturePtr = new VideoCapture(0);
    framePtr   = new Mat();*/

    capturePtr->open(0);

    //Disable continuous auto focus (on: 1, off: 0)
    capturePtr->set(CAP_PROP_AUTOFOCUS, 0);

    //Set manual focus step (0 - 255)
    capturePtr->set(CAP_PROP_FOCUS, 200);

    //640×360 (16:9)
    capturePtr->set(CAP_PROP_FRAME_WIDTH, 640);
    capturePtr->set(CAP_PROP_FRAME_HEIGHT, 360);

    //Frame rate
    capturePtr->set(CAP_PROP_FPS, 30);

    if(capturePtr->isOpened())
    {
        double frameRate = capturePtr->get(CV_CAP_PROP_FPS);

        *capturePtr >> *framePtr;

        if(!framePtr->empty())
        {
            QImage qImage = cvMat2QImage(*framePtr);

            ui->liveViewLabel->setPixmap(QPixmap::fromImage(qImage));

            //timerPtr = new QTimer(this);
            timerPtr->setInterval(1000 / frameRate);
            connect(timerPtr, SIGNAL(timeout()), this, SLOT(nextFrame()));
            timerPtr->start();
        }

        //QMessageBox::information(this, "Live View", "Start");
        ui->liveViewLineEdit->setText("Live View Status ==> Started");
    }

} //end of function displayLiveView

void TestOpencv::stopLiveView()
{
    timerPtr->stop();

    //QMessageBox::information(this, "Live View", "Stop");
    ui->liveViewLineEdit->setText("Live View Status ==> Stopped");

    disconnect(timerPtr, SIGNAL(timeout()), this, SLOT(nextFrame()));
    capturePtr->release();

} //end of function stopLiveView

void TestOpencv::nextFrame()
{
    *capturePtr >> *framePtr;

    if(capturePtr->isOpened())
    {
        int width  = 0;
        int height = 0;

        width  = saturate_cast<int>(capturePtr->get(CAP_PROP_FRAME_WIDTH));
        height = saturate_cast<int>(capturePtr->get(CAP_PROP_FRAME_HEIGHT));

        Mat outputFrame(height, width, CV_8UC3);

        //toGrayImage(*framePtr, outputFrame, height, width);
        toNegativeImage(*framePtr, outputFrame, height, width);

        //qImage  = cvMat2QImage(*framePtr);
        qImage  = cvMat2QImage(outputFrame);
        qPixmap = QPixmap::fromImage(qImage);

        qPixmap = qPixmap.scaled(ui->liveViewLabel->width(), ui->liveViewLabel->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        ui->liveViewLabel->setPixmap(qPixmap);
    }

} //end of function nextFrame

void TestOpencv::slot_changeUI(QString str)
{
    ui->lineEdit->setText(str);

} //end of function slot_changeUI

void *TestOpencv::threadFun(void *arg)
{
    //TestOpencv *testOpencv = (TestOpencv *)arg;
    TestOpencv *testOpencv = static_cast<TestOpencv *>(arg);

    while(true)
    {
        QDateTime currentDateTime = QDateTime::currentDateTime();
        QString currentDate       = currentDateTime.toString("yyyy.MM.dd hh:mm:ss.zzz ddd");

        testOpencv->sig_changeUI(currentDate);

        sleep(1);
    }

} //end of function threadFun

void TestOpencv::on_outputMsgButton_clicked()
{
    uid_t uid;
    gid_t gid;

    struct passwd *pw;

    uid = getuid();
    gid = getgid();

    qDebug() << endl;

    qDebug() << "User is " << getlogin() << endl;
    qDebug() << "User IDs: uid = " << uid << ", gid = " << gid << endl;

    pw = getpwuid(uid);
    qDebug() << "UID passwd entry:\n" << "name = " << pw->pw_name << ", uid = " << pw->pw_uid << ", gid = " << pw->pw_gid <<
        ", home = " << pw->pw_dir << ", shell = " << pw->pw_shell << endl;

    qDebug() << "============================================================" << endl;

    pw = getpwnam("root");
    qDebug() << "root passwd entry:\n" << "name = " << pw->pw_name << ", uid = " << pw->pw_uid << ", gid = " << pw->pw_gid <<
        ", home = " << pw->pw_dir << ", shell = " << pw->pw_shell << endl;

    /*char computer[256];
    struct utsname uts;

    if((gethostname(computer, 255) != 0) || (uname(&uts) < 0))
    {
        qDebug() << "Could not get host information !!" << endl;

        return;
    }

    qDebug() << endl;
    qDebug() << "Computer host name is " << computer << endl;
    qDebug() << "System is " << uts.sysname << " on " << uts.machine << " hardware" << endl;
    qDebug() << "Nodename is " << uts.nodename << endl;
    qDebug() << "Version is " << uts.release << ", " << uts.version << endl;*/

} //end of function on_outputMsgButton_clicked

QImage TestOpencv::cvMat2QImage(const cv::Mat &mat)
{
    // 8-bits unsigned, NO. OF CHANNELS = 1
    if(mat.type() == CV_8UC1)
    {
        QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
        // Set the color table (used to translate colour indexes to qRgb values)
        image.setColorCount(256);
        for(int i = 0; i < 256; i++)
        {
            image.setColor(i, qRgb(i, i, i));
        }
        // Copy input Mat
        uchar *pSrc = mat.data;
        for(int row = 0; row < mat.rows; row ++)
        {
            uchar *pDest = image.scanLine(row);
            memcpy(pDest, pSrc, mat.cols);
            pSrc += mat.step;
        }
        return image;
    }
    // 8-bits unsigned, NO. OF CHANNELS = 3
    else if(mat.type() == CV_8UC3)
    {
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return image.rgbSwapped();
    }
    else if(mat.type() == CV_8UC4)
    {
        qDebug() << "CV_8UC4";
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return image.copy();
    }
    else
    {
        qDebug() << "ERROR: Mat could not be converted to QImage.";
        return QImage();
    }

} //end of function cvMat2QImage

void TestOpencv::on_showButton_clicked()
{
    //QMessageBox::information(this, "Information", "Message Test");

    /*Mat img = imread("/home/stanleychang/stanleychangFiles/images/GFriend_1.bmp");

    imshow("GFriend Image", img);
    waitKey(0);*/

    VideoCapture capture(0);
    Mat          frame;
    Mat          gray;

    if(!capture.isOpened())
    {
        qDebug() << "Cannot open camera !!" << endl;

        return;
    }

    //Disable continuous auto focus (on: 1, off: 0)
    capture.set(CAP_PROP_AUTOFOCUS, 0);

    //Set manual focus step (0 - 255)
    capture.set(CAP_PROP_FOCUS, 200);

    //640×360 (16:9)
    capture.set(CAP_PROP_FRAME_WIDTH, 640);
    capture.set(CAP_PROP_FRAME_HEIGHT, 360);

    //Frame rate
    capture.set(CAP_PROP_FPS, 30);

    //capture.set(CAP_PROP_EXPOSURE, 0.0); //Manual --> Ex: -4 (means 2^-4 = 1/16 = 80 ms)

    while(true)
    {
        bool active = capture.read(frame);

        if(!active)
        {
            qDebug() << "Cannot receive frame !!" << endl;

            break;
        }

        cvtColor(frame, gray, COLOR_BGR2GRAY);

        //imshow("Live View", frame);
        //imshow("Live View (Gray)", gray);

        //QImage image = cvMat2QImage(frame);
        QImage image = cvMat2QImage(gray);
        ui->liveViewLabel->setPixmap(QPixmap::fromImage(image));
        ui->liveViewLabel->show();

        if(waitKey(1) == 'q')
        {
            break;
        }
    }

} //end of function on_showButton_clicked

void TestOpencv::on_modifyRgbButton_clicked()
{
    VideoCapture capture(0);
    VideoWriter  writer;
    Size         videoSize;
    Mat          frame;
    Mat          gray;

    QImage  image;
    QPixmap pixmap;

    int width  = 0;
    int height = 0;

    if(!capture.isOpened())
    {
        qDebug() << "Cannot open camera !!" << endl;

        return;
    }

    //QMessageBox::information(this, "Web Cam Display", "Modified Image");

    //Disable continuous auto focus (on: 1, off: 0)
    capture.set(CAP_PROP_AUTOFOCUS, 0);

    //Set manual focus step (0 - 255)
    capture.set(CAP_PROP_FOCUS, 150);

    //640×480 (4:3)
    //640×360 (16:9)
    capture.set(CAP_PROP_FRAME_WIDTH, 640);
    capture.set(CAP_PROP_FRAME_HEIGHT, 360);

    //Frame rate
    capture.set(CAP_PROP_FPS, 30);

    //capture.set(CAP_PROP_EXPOSURE, 0.0); //Manual --> Ex: -4 (means 2^-4 = 1/16 = 80 ms)

    width  = saturate_cast<int>(capture.get(CAP_PROP_FRAME_WIDTH));
    height = saturate_cast<int>(capture.get(CAP_PROP_FRAME_HEIGHT));

    /*videoSize = Size(width, height);
    writer.open("D:/OpenCV/Video/Cat.avi", CV_FOURCC('M', 'J', 'P', 'G'), 15, videoSize, true);*/

    while(true)
    {
        //bool active = capture.read(frame);
        Mat outputFrame(height, width, CV_8UC3);

        /*if(!active)
        {
            qDebug() << "Cannot receive frame !!" << endl;

            break;
        }*/

        capture >> frame;

        if(!capture.isOpened())
        {
            qDebug() << "Cannot receive frame !!" << endl;

            break;
        }

        //toGrayImage(frame, outputFrame, height, width);
        //toNegativeImage(frame, outputFrame, height, width);

        //image = cvMat2QImage(outputFrame);
        image = cvMat2QImage(frame);
        pixmap = QPixmap::fromImage(image);
        pixmap = pixmap.scaled(ui->liveViewLabel->width(), ui->liveViewLabel->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        ui->liveViewLabel->setPixmap(pixmap);
        //ui->liveViewLabel->setPixmap(QPixmap::fromImage(image));
        ui->liveViewLabel->show();

        //imshow("Live View", outputFrame);
        //writer.write(outputFrame);

        if(waitKey(1) == 'q')
        {
            break;
        }
    }

    capture.release();
    writer.release();

} //end of function on_modifyRgbButton_clicked

void TestOpencv::toNegativeImage(const Mat &sourceImage, Mat &outputImage, const int imageHeight, const int imageWidth)
{
    for(int y = 0; y < imageHeight; y++)
    {
        for(int x = 0; x < imageWidth; x++)
        {
            uchar pixel_B  = sourceImage.at<Vec3b>(y, x)[CHANNEL_B];
            uchar pixel_G  = sourceImage.at<Vec3b>(y, x)[CHANNEL_G];
            uchar pixel_R  = sourceImage.at<Vec3b>(y, x)[CHANNEL_R];

            outputImage.at<Vec3b>(y, x)[CHANNEL_B] = saturate_cast<uchar>(255 - pixel_B);
            outputImage.at<Vec3b>(y, x)[CHANNEL_G] = saturate_cast<uchar>(255 - pixel_G);
            outputImage.at<Vec3b>(y, x)[CHANNEL_R] = saturate_cast<uchar>(255 - pixel_R);
        }
    }

} //end of function toNegativeImage

void TestOpencv::toGrayImage(const Mat &sourceImage, Mat &outputImage, const int imageHeight, const int imageWidth)
{
    for(int y = 0; y < imageHeight; y++)
    {
        for(int x = 0; x < imageWidth; x++)
        {
            uchar pixel_B  = sourceImage.at<Vec3b>(y, x)[CHANNEL_B];
            uchar pixel_G  = sourceImage.at<Vec3b>(y, x)[CHANNEL_G];
            uchar pixel_R  = sourceImage.at<Vec3b>(y, x)[CHANNEL_R];
            float avgPixel = saturate_cast<float>((pixel_B + pixel_G + pixel_R) / 3.0);

            outputImage.at<Vec3b>(y, x)[CHANNEL_B] = saturate_cast<uchar>(avgPixel);
            outputImage.at<Vec3b>(y, x)[CHANNEL_G] = saturate_cast<uchar>(avgPixel);
            outputImage.at<Vec3b>(y, x)[CHANNEL_R] = saturate_cast<uchar>(avgPixel);
        }
    }

} //end of function toGrayImage
