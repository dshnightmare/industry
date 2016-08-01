#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QTextCodec"
#include "QString"
#include "QFileDialog"
#include "opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "hist.h"
#include "c_gradation.h"
#include "string"
#include "cv.h"
#include "highgui.h"
#include "iostream"
#include "tiffio.h"
#include "queue"
#include "fstream"
#include "QFile"
#include "QFileDialog"
#include "QTextStream"
#include "QGraphicsDropShadowEffect"
#include "QDirModel"
#include "vector"

using namespace std;

extern int maxback;

extern int inout_interval;

extern unsigned short indark, inwhite, outdark, outwhite;

extern void getLevelMap(unsigned short map[65536], unsigned short inputDark,unsigned short inputGray, unsigned short inputLight,  unsigned short outDark, unsigned short outLight);

extern void curve(Mat_<unsigned short>  input, Mat_<unsigned short> & output,unsigned short bMap[65536]);

extern void divcurve(int istart,int iend,int jstart,int jend, Mat_<unsigned short>  input, Mat_<unsigned short> & output,unsigned short bMap[65536]);

extern void levelAdjustment(Mat_<unsigned short>  input, Mat_<unsigned short> & output, unsigned short inputDark, unsigned short inputGray,unsigned short inputLight,  unsigned short outDark, unsigned short outLight);




#define max(a,b) (a>b)?a:b
#define min(a,b) (a>b)?b:a



void reshowimg();
static QString filename;
static vector<QString> filelist;
static int ptr;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);

}

MainWindow::~MainWindow()
{

    delete ui;
}
void MainWindow::initialize()
{
    ui->imagelist->setResizeMode(QListView::Adjust);
    ui->imagelist->setViewMode(QListView::IconMode);
    ui->imagelist->setMovement(QListView::Static);
    ui->imagelist->setSpacing(0);
    ptr=0;


    ui->scrollshowimg->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollshowimg->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollclicked=false;
    ui->winshowimg->installEventFilter(this);
    double rate=log(0.5) / log(((double)(32767) - (double)0) / ((double)65535 - (double)0));
    ingray=indark+(int)((double)(inwhite-indark)*pow(double(2.718),log(0.5)/rate));
}

void MainWindow::showimage(cv::Mat_<unsigned short> s)
{
    for (int i=0;i<srcimgshort.rows;i++)
    {
        for (int j=0;j<srcimgshort.cols;j++)
        {
            srcimgchar(i,j)=unsigned char(double(s(i,j))/65535*255);
        }
    }
    showimg=mat2qimage(srcimgchar);
    tshowimg=showimg.scaled(ow*ui->showimgscale->value()/1000,oh*ui->showimgscale->value()/1000,Qt::KeepAspectRatio,Qt::SmoothTransformation);
    ui->winshowimg->setPixmap(QPixmap::fromImage(tshowimg));
    ui->winshowimg->setAlignment(Qt::AlignVCenter|Qt::AlignHCenter);
}

bool MainWindow::eventFilter(QObject *target, QEvent *e)
{
    if (target == ui->winshowimg)
    {
        if (e->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent* ev = static_cast<QMouseEvent*>(e);
            if (ev->button()==Qt::LeftButton)
            {
                scrollpos=ev->globalPos();
                scrollclicked=true;
                return true;
            }
            else if (ev->button()==Qt::RightButton)
            {
                cpos=ev->globalPos();
                cclicked=true;
                return true;
            }
        }
        else if(e->type() == QEvent::MouseMove)
        {

            QMouseEvent* ev = static_cast<QMouseEvent*>(e);
            if (scrollclicked == true)
            {
                QPoint temp = ev->globalPos();
                int xvalue = temp.x()-scrollpos.x();
                int yvalue = temp.y()-scrollpos.y();
                scrollpos=temp;
                QScrollBar* vbar=ui->scrollshowimg->verticalScrollBar();
                QScrollBar* hbar=ui->scrollshowimg->horizontalScrollBar();
                hbar->setValue(hbar->value()-xvalue);
                vbar->setValue(vbar->value()-yvalue);
                return true;
            }
            else if (cclicked == true && !srcimgshort.empty())
            {

                QPoint temp = ev->globalPos();
                int xvalue = temp.x()-cpos.x();
                int yvalue = temp.y()-cpos.y();
                cpos=temp;

                int h_inc=inout_interval*xvalue;
                int tindark = indark-h_inc;
                int tinwhite = inwhite + h_inc;
                if (tindark<0) tindark=0;
                if (tinwhite>65535) tinwhite = 65535;
                if (tindark>=tinwhite-1) tindark = tinwhite - 2;
//                if (indark-h_inc<0)
//                {
//                    indark = h;
//                }
//                if (inwhite+h_inc>65535)
//                {
//                    h_inc = 65535-inwhite;
//                }
//                if ( (indark-h_inc) >= (inwhite+h_inc-2))
//                {
//                    int mid=(indark+inwhite)/2;
//                    h_inc = -(mid - indark - 1);
//                }
//                indark -= h_inc;
//                inwhite +=h_inc;

                int v_inc=inout_interval*yvalue;
                tindark = tindark - v_inc;
                tinwhite = tinwhite - v_inc;
                if (tindark<0) tindark=0;
                if (tinwhite>65535) tinwhite = 65535;
                if (tindark>=tinwhite-1) tindark = tinwhite - 2;

//                if (indark-v_inc<0)
//                {
//                    v_inc = indark;
//                }
//                if (inwhite-v_inc>65535)
//                {
//                    v_inc = inwhite - 65535;
//                }
//                indark-=v_inc;
//                inwhite-=v_inc;

                indark = tindark;
                inwhite = tinwhite;
                double rate=log(0.5) / log(((double)(32767) - (double)0) / ((double)65535 - (double)0));
                ingray=indark+(int)((double)(inwhite-indark)*pow(double(2.718),log(0.5)/rate));
//                levelAdjustment(srcimgshort,timg,indark,ingray,inwhite,outdark,outwhite);
                cv::Mat_<unsigned short> timg=cv::Mat_<unsigned short>(srcimgshort.rows, srcimgshort.cols, CV_16UC1);
                levelAdjustment(srcimgshort,timg,indark,ingray,inwhite,outdark,outwhite);
                showimage(timg);

//                //inb加减
//                if ((float)abs(yvalue)/abs(xvalue)<0.2)
//                {

//                    if (indark+inout_interval*xvalue>=inwhite) indark=inwhite-2;
//                    else if (indark+inout_interval*xvalue<0)
//                    {
//                        indark=0;
//                    }
//                    else
//                    {

//                        indark+=inout_interval*xvalue;
//                    }

//                    double rate=log(0.5) / log(((double)(32767) - (double)0) / ((double)65535 - (double)0));
//                    ingray=indark+(int)((double)(inwhite-indark)*pow(double(2.718),log(0.5)/rate));
//                    if (ingray>=inwhite) ingray = inwhite-1;
//                    levelAdjustment(srcimgshort,timg,indark,ingray,inwhite,outdark,outwhite);
//                    showimage(timg);
//                }
//                //inw加减
//                if ((float)abs(xvalue)/abs(yvalue)<0.2)
//                {

//                    if (inwhite-inout_interval*yvalue<=indark) inwhite=indark+2;
//                    else if (inwhite-inout_interval*yvalue>65535) inwhite=65535;
//                    else    inwhite-=inout_interval*yvalue;

//                    double rate=log(0.5) / log(((double)(32767) - (double)0) / ((double)65535 - (double)0));
//                    ingray=indark+(int)((double)(inwhite-indark)*pow(double(2.718),log(0.5)/rate));
//                    if (ingray>=inwhite) ingray = inwhite-1;
//                    levelAdjustment(srcimgshort,timg,indark,ingray,inwhite,outdark,outwhite);
//                    showimage(timg);
//                }

//                //outb加减
//                if (xvalue*yvalue<0&&(float)abs(yvalue)/abs(xvalue)>0.2&&(float)abs(xvalue)/abs(yvalue)>0.2)
//                {

//                    if (outdark+inout_interval*xvalue>=outwhite) outdark=outwhite-1;
//                    else if (outdark+inout_interval*xvalue<0) outdark=0;
//                    else outdark+=inout_interval*xvalue;
//                    levelAdjustment(srcimgshort,timg,indark,ingray,inwhite,outdark,outwhite);
//                    showimage(timg);

//                }
//                //outw加减
//                if (xvalue*yvalue>0&&(float)abs(yvalue)/abs(xvalue)>0.2&&(float)abs(xvalue)/abs(yvalue)>0.2)
//                {

//                    if (outwhite+inout_interval*xvalue<=outdark) outwhite=outdark+1;
//                    else if (outwhite+inout_interval*xvalue>65535) outwhite=65535;
//                    else outwhite+=inout_interval*xvalue;
//                    levelAdjustment(srcimgshort,timg,indark,ingray,inwhite,outdark,outwhite);
//                    showimage(timg);
//                }
                qDebug("%d %d %d %d %d %d %d\n", xvalue,yvalue,indark,ingray,inwhite,outdark,outwhite);
                return true;
            }

        }
        else if(e->type() == QEvent::MouseButtonRelease)
        {
            QMouseEvent* ev = static_cast<QMouseEvent*>(e);
            if (ev->button()==Qt::LeftButton)
            {
                scrollclicked=false;
            }
            else if (ev->button()==Qt::RightButton)
            {
                cclicked=false;
            }
            return true;
        }
        else if(e->type()==QEvent::Wheel)
        {
            QWheelEvent* ev = static_cast<QWheelEvent*>(e);
            int num=ev->delta()/10;
//            qDebug("%d\n",num);
            int min=ui->showimgscale->minimum();
            int max=ui->showimgscale->maximum();
            if (ui->showimgscale->value()+num<min) ui->showimgscale->setValue(min);
            else if (ui->showimgscale->value()+num>max) ui->showimgscale->setValue(max);
            else ui->showimgscale->setValue(ui->showimgscale->value()+num);
            return true;
        }
    }

    return QMainWindow::eventFilter(target, e);
}


void MainWindow::enableaction()
{
    ui->hist->setEnabled(true);
    ui->rate->setEnabled(true);
    ui->showimgscale->setEnabled(true);
    ui->invert->setEnabled(true);
    ui->back->setEnabled(true);
    ui->hdr->setEnabled(true);
    ui->localadaptive_hdr->setEnabled(true);
    ui->hist_hdr->setEnabled(true);
    ui->resave->setEnabled(true);
    ui->info->setEnabled(true);
    ui->turn->setEnabled(true);
    ui->contrast->setEnabled(true);
    ui->denoise->setEnabled(true);
    ui->plus->setEnabled(true);
    ui->minus->setEnabled(true);
}
void MainWindow::disableaction()
{
    ui->hist->setEnabled(false);
    ui->rate->setEnabled(false);
    ui->showimgscale->setEnabled(false);
    ui->invert->setEnabled(false);
    ui->back->setEnabled(false);
    ui->hdr->setEnabled(false);
    ui->localadaptive_hdr->setEnabled(false);
    ui->hist_hdr->setEnabled(false);
    ui->resave->setEnabled(false);
    ui->info->setEnabled(false);
    ui->turn->setEnabled(false);
    ui->contrast->setEnabled(false);
    ui->denoise->setEnabled(false);
    ui->plus->setEnabled(false);
    ui->plus->setEnabled(false);
}

void MainWindow::reset()
{
    indark=0;
    inwhite=65535;
    outdark=0;
    outwhite=65535;
    inout_interval=10;
    double rate=log(0.5) / log(((double)(32767) - (double)0) / ((double)65535 - (double)0));
    ingray=indark+(int)((double)(inwhite-indark)*pow(double(2.718),log(0.5)/rate));

}

//更改放缩显示数字
void MainWindow::settext(QString arg)
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));
    ui->rate->setText(arg);
}
//另存文件
void MainWindow::on_resave_triggered()
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));
    QString filename1 = QFileDialog::getSaveFileName(this,QString::fromLocal8Bit("另存为"),"","Images(*.tiff)");
    if (filename1.isEmpty()) return;
    QTextCodec *code = QTextCodec::codecForName("gb18030");
    TIFF* tif = TIFFOpen(code->fromUnicode(filename1).data(), "w");
//    TIFF* ttif = TIFFOpen(code->fromUnicode(filename).data(), "r");
        if (tif)
        {
            int h=srcimgshort.rows, w=srcimgshort.cols;
             uint32* temp;

//             TIFFGetField(ttif,TIFFTAG_IMAGEWIDTH,temp);
             TIFFSetField(tif, TIFFTAG_IMAGEWIDTH,w);
//             TIFFGetField(ttif,TIFFTAG_IMAGELENGTH,temp);
             TIFFSetField(tif, TIFFTAG_IMAGELENGTH,h);
//             TIFFGetField(ttif,TIFFTAG_BITSPERSAMPLE,temp);
             TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE,16);
//             TIFFGetField(ttif,TIFFTAG_SAMPLESPERPIXEL,temp);
             TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL,1);
//             TIFFGetField(ttif,TIFFTAG_ROWSPERSTRIP,temp);
//             TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP,*temp);
//             TIFFGetField(ttif,TIFFTAG_COMPRESSION,temp);
//             TIFFSetField(tif, TIFFTAG_COMPRESSION,*temp);
//             TIFFGetField(ttif,TIFFTAG_PHOTOMETRIC,temp);
//             TIFFSetField(tif, TIFFTAG_PHOTOMETRIC,*temp);
//             TIFFGetField(ttif,TIFFTAG_FILLORDER,temp);
//             TIFFSetField(tif, TIFFTAG_FILLORDER,*temp);
//             TIFFGetField(ttif,TIFFTAG_PLANARCONFIG,temp);
//             TIFFSetField(tif, TIFFTAG_PLANARCONFIG,*temp);
//             TIFFGetField(ttif,TIFFTAG_XRESOLUTION,temp);
//             TIFFSetField(tif, TIFFTAG_XRESOLUTION,*temp);
//             TIFFGetField(ttif,TIFFTAG_YRESOLUTION,temp);
//             TIFFSetField(tif, TIFFTAG_YRESOLUTION,*temp);
//             TIFFGetField(ttif,TIFFTAG_RESOLUTIONUNIT,temp);
//             TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT,*temp);

             tsize_t aa=TIFFScanlineSize(tif);
             tdata_t buf=_TIFFmalloc(TIFFScanlineSize(tif));
             uint16* data;
             data=(uint16*)buf;
             for (int row = 0; row < h; row++)
             {

                 for (int j=0;j<w;j++)
                     data[j]=(uint16)(65535-srcimgshort(row,j));

                 TIFFWriteScanline(tif,buf,(uint32)row, 0);
             }
//            TIFFClose(ttif);
            TIFFClose(tif);
        }
}

void MainWindow::on_imagelist_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    int num=current->text().toInt()-1;
    QString filename=filelist[num];
    openfile(filename,2);
}
//type=1,列表第一个，打开并显示，type=0，只在列表里显示，type=2，列表切换显示大图
void MainWindow::openfile(QString filename, int type)
{
    typedef vector<QString> vq;
    if (type!=2)
    {
        for ( vector<QString>::iterator it = filelist.begin();it<filelist.end();it++)
        {
            if (*it == filename) return;
        }
    }


    if (filename.isEmpty()) return;



//    if ((int)backup.size() < maxback)
//    {
//        backup.push_back(srcimgshort);
//    }
//    else
//    {
//        backup.pop_front();
//        backup.push_back(srcimgshort);
//    }

    QTextCodec *code = QTextCodec::codecForName("gb18030");

    cv::Mat_<unsigned short> srcimgshort_temp;
//    srcimgshort=cvLoadImage(code->fromUnicode(filename).data(),CV_LOAD_IMAGE_ANYDEPTH);
    TIFF* tif = TIFFOpen(code->fromUnicode(filename).data(), "r");
        if (tif) {
            uint32 w, h;
            tdata_t buf;
            uint32 row;
            uint32 config;

            TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
            TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
            srcimgshort_temp=cv::Mat(h,w,CV_16UC1);
            TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &config);
//            tsize_t aa=TIFFScanlineSize(tif);
            buf = _TIFFmalloc(TIFFScanlineSize(tif));



            uint16  nsamples;
            uint16* data;
            TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &nsamples);

                for (row = 0; row < h; row++)
                {
                    TIFFReadScanline(tif, buf, row, 0);
                    data = (uint16*)buf;
                    for (uint32 j=0;j < w;j++)
                    {
                        srcimgshort_temp(row,j)=65535-data[j];
                    }

                    //printArray(data, imagelength);
                }
                // printArray(data,imagelength,height);


            _TIFFfree(buf);
            TIFFClose(tif);
        }
    enableaction();
    if (srcimgshort_temp.rows>srcimgshort_temp.cols)
    {
        srcimgshort=cv::Mat(srcimgshort_temp.cols,srcimgshort_temp.rows,CV_16UC1);
        for (int i=0;i<srcimgshort_temp.rows;i++)
        {
            for (int j=0;j<srcimgshort_temp.cols;j++)
            {
                srcimgshort(srcimgshort_temp.cols-1-j,i)=srcimgshort_temp(i,j);
            }
        }
        srcimgshort_temp.release();
    }
    else srcimgshort = srcimgshort_temp;
    srcimgchar=cv::Mat(srcimgshort.rows,srcimgshort.cols,CV_8UC1);
    for (int i=0;i<srcimgshort.rows;i++)
    {
        for (int j=0;j<srcimgshort.cols;j++)
        {
            srcimgchar(i,j)=unsigned char(double(srcimgshort(i,j))/65535*255);
        }
    }
    if (type==0||type==1)
    {
        QImage tshowimage=mat2qimage(srcimgchar);
        QImage timage=tshowimage.scaled(ui->imagelist->width()-5,ui->imagelist->height(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
        filelist.push_back(filename);
        QListWidgetItem* Item = new QListWidgetItem( QIcon(QPixmap::fromImage(timage)),QString::number(++ptr ));
        ui->imagelist->setIconSize(QSize(timage.width(),timage.height()));
        ui->imagelist->addItem(Item);
    }


    if (type==2||type==1)
    {
        emit s_imageshort(srcimgshort);
        showimg=mat2qimage(srcimgchar);

        int slidermax,slidermin,sliderpos;

        int winw=ui->scrollshowimg->width();
        int winh=ui->scrollshowimg->height();
        ow=showimg.width();
        oh=showimg.height();
        double wrate=double(winw)/ow;
        double hrate=double(winh)/oh;

        double rate=min(wrate,hrate);
        settext(QString::number(rate*100,'f',1));

        if(rate>1)
        {
            sliderpos=1000;
            slidermin=500;
            slidermax=4000;
        }
        else
        {
            sliderpos=1000*rate;
            slidermin=0.5*sliderpos;
            slidermax=2000;
        }
        ui->showimgscale->setMaximum(slidermax);
        ui->showimgscale->setMinimum( slidermin);
        ui->showimgscale->setValue(sliderpos);

        tshowimg=showimg.scaled(ow*sliderpos/1000,oh*sliderpos/1000,Qt::KeepAspectRatio,Qt::SmoothTransformation);
        ui->winshowimg->setPixmap(QPixmap::fromImage(tshowimg));
        ui->winshowimg->setAlignment(Qt::AlignVCenter|Qt::AlignHCenter);
        QGraphicsDropShadowEffect *e1=new QGraphicsDropShadowEffect;
        e1->setColor(QColor(0,0,0));
        e1->setBlurRadius(10);
        e1->setOffset(1,1);
        ui->winshowimg->setGraphicsEffect(e1);
    }

    reset();
    backup.clear();
    ui->back->setEnabled(false);


}

//打开文件
void MainWindow::on_openfile_triggered()
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));
    QStringList filelist = QFileDialog::getOpenFileNames(this,QString::fromLocal8Bit("打开"),"","Images (*.tif *.tiff)");
     for(QStringList::Iterator it=filelist.begin();it!=filelist.end();it++)
    {
        
        if (it==filelist.begin())
            openfile(*it,1);
        else
            openfile(*it,0);
    }

}
//更改放缩大小文本框
void MainWindow::on_rate_editingFinished()
{
    int position=ui->rate->text().toFloat()*10;
    if (position==ui->showimgscale->value()) return;
    int min=ui->showimgscale->minimum();
    int max=ui->showimgscale->maximum();
    position = position>min?position:min;
    position = position<max?position:max;
    ui->showimgscale->setValue(position);
}
//放缩大小滑块
void MainWindow::on_showimgscale_valueChanged(int value)
{

    settext(QString::number(float(value)/10,'f',1));
    tshowimg=showimg.scaled(ow*value/1000,oh*value/1000,Qt::KeepAspectRatio,Qt::SmoothTransformation);
    ui->winshowimg->setPixmap(QPixmap::fromImage(tshowimg));
    ui->winshowimg->setAlignment(Qt::AlignVCenter|Qt::AlignHCenter);
}

void MainWindow::on_minus_clicked()
{
    int min=ui->showimgscale->minimum();
    int max=ui->showimgscale->maximum();
    if (ui->showimgscale->value()-1>=min&&ui->showimgscale->value()-1<=max)
        ui->showimgscale->setValue(ui->showimgscale->value()-1);
}

void MainWindow::on_plus_clicked()
{
    int min=ui->showimgscale->minimum();
    int max=ui->showimgscale->maximum();
    if (ui->showimgscale->value()+1>=min&&ui->showimgscale->value()+1<=max)
        ui->showimgscale->setValue(ui->showimgscale->value()+1);
}

//触发调整色阶按钮
void MainWindow::on_hist_triggered()
{
    if (srcimgshort.empty()) return;
    w1=new c_gradation();
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));
    QObject::connect(this,SIGNAL(s_imageshort(cv::Mat_<unsigned short>)),w1,SLOT(r_imageshort(cv::Mat_<unsigned short>)));
    QObject::connect(w1,SIGNAL(s_imagechar(cv::Mat_<unsigned char>)),this,SLOT(r_imagechar(cv::Mat_<unsigned char>)));
    QObject::connect(w1,SIGNAL(s_cancel()),this,SLOT(r_cancel()));
    QObject::connect(w1,SIGNAL(s_ok(cv::Mat_<unsigned short>)),this,SLOT(r_ok(cv::Mat_<unsigned short>)));
    QObject::connect(this,SIGNAL(s_hist(cv::Mat_<unsigned char>)),w1,SLOT(r_hist(cv::Mat_<unsigned char>)));
    QObject::connect(w1,SIGNAL(s_imageshort(cv::Mat_<unsigned short>)),this,SLOT(r_imageshort(cv::Mat_<unsigned short>)));

    cv::Mat_<unsigned short> timg=cv::Mat_<unsigned short>(srcimgshort.rows, srcimgshort.cols, CV_16UC1);
    levelAdjustment(srcimgshort,timg,indark,ingray,inwhite,outdark,outwhite);

    emit s_imageshort(timg);
    emit s_hist(srcimgchar);
    w1->setWindowTitle(QString::fromLocal8Bit("色阶调整"));
    w1->setWindowFlags(Qt::WindowCloseButtonHint);
    w1->setGeometry(x()+100,y()+100,491,389);
    w1->setFixedWidth(491);
    w1->setFixedHeight(389);
    w1->exec();
}

//触发直方图均衡化hdr
void MainWindow::on_hist_hdr_triggered()
{
    if (srcimgshort.empty()) return;
    w2=new ui_hist_hdr();
    QObject::connect(this,SIGNAL(s_imageshort(cv::Mat_<unsigned short>)),w2,SLOT(r_imageshort(cv::Mat_<unsigned short>)));
    QObject::connect(w2,SIGNAL(s_imagechar(cv::Mat_<unsigned char>)),this,SLOT(r_imagechar(cv::Mat_<unsigned char>)));
    QObject::connect(w2,SIGNAL(s_cancel()),this,SLOT(r_cancel()));
    QObject::connect(w2,SIGNAL(s_ok(cv::Mat_<unsigned short>)),this,SLOT(r_ok(cv::Mat_<unsigned short>)));

    cv::Mat_<unsigned short> timg=cv::Mat_<unsigned short>(srcimgshort.rows, srcimgshort.cols, CV_16UC1);
    levelAdjustment(srcimgshort,timg,indark,ingray,inwhite,outdark,outwhite);

    emit s_imageshort(timg);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));
    w2->setWindowTitle(QString::fromLocal8Bit("直方图均衡化HDR调整"));
    w2->setWindowFlags(Qt::WindowCloseButtonHint);
    w2->setGeometry(x()+100,y()+100,332,100);
    w2->setFixedWidth(332);
    w2->setFixedHeight(100);
    w2->exec();
}
//局部调整hdr
void MainWindow::on_localadaptive_hdr_triggered()
{
    if (srcimgshort.empty()) return;
    w3=new ui_local_hdr();
    QObject::connect(this,SIGNAL(s_imageshort(cv::Mat_<unsigned short>)),w3,SLOT(r_imageshort(cv::Mat_<unsigned short>)));
    QObject::connect(w3,SIGNAL(s_imagechar(cv::Mat_<unsigned char>)),this,SLOT(r_imagechar(cv::Mat_<unsigned char>)));
    QObject::connect(w3,SIGNAL(s_cancel()),this,SLOT(r_cancel()));
    QObject::connect(w3,SIGNAL(s_ok(cv::Mat_<unsigned short>)),this,SLOT(r_ok(cv::Mat_<unsigned short>)));
    QObject::connect(w3,SIGNAL(s_dst(cv::Mat)),this,SLOT(r_lhdr_dst(cv::Mat)));

    cv::Mat_<unsigned short> timg=cv::Mat_<unsigned short>(srcimgshort.rows, srcimgshort.cols, CV_16UC1);
    levelAdjustment(srcimgshort,timg,indark,ingray,inwhite,outdark,outwhite);

    emit s_imageshort(timg);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));
    w3->setWindowTitle(QString::fromLocal8Bit("局部适应HDR调整"));
    w3->setWindowFlags(Qt::WindowCloseButtonHint);
    w3->setGeometry(x()+100,y()+100,316,152);
    w3->setFixedWidth(316);
    w3->setFixedHeight(152);
    w3->exec();
}
//对比度
void MainWindow::on_contrast_triggered()
{
    if (srcimgshort.empty()) return;
    w_contrast=new ui_contrast();
    QObject::connect(this,SIGNAL(s_imageshort(cv::Mat_<unsigned short>)),w_contrast,SLOT(r_imageshort(cv::Mat_<unsigned short>)));
    QObject::connect(w_contrast,SIGNAL(s_imagechar(cv::Mat_<unsigned char>)),this,SLOT(r_imagechar(cv::Mat_<unsigned char>)));
    QObject::connect(w_contrast,SIGNAL(s_cancel()),this,SLOT(r_cancel()));
    QObject::connect(w_contrast,SIGNAL(s_ok(cv::Mat_<unsigned short>)),this,SLOT(r_ok(cv::Mat_<unsigned short>)));
    QObject::connect(w_contrast,SIGNAL(s_imageshort(cv::Mat_<unsigned short>)),this,SLOT(r_imageshort(cv::Mat_<unsigned short>)));

    cv::Mat_<unsigned short> timg=cv::Mat_<unsigned short>(srcimgshort.rows, srcimgshort.cols, CV_16UC1);
    levelAdjustment(srcimgshort,timg,indark,ingray,inwhite,outdark,outwhite);

    emit s_imageshort(timg);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));
    w_contrast->setWindowTitle(QString::fromLocal8Bit("对比度调整"));
    w_contrast->setWindowFlags(Qt::WindowCloseButtonHint);
    w_contrast->setGeometry(x()+100,y()+100,369,105);
    w_contrast->setFixedWidth(369);
    w_contrast->setFixedHeight(105);
    w_contrast->exec();
}
//去噪
void MainWindow::on_denoise_triggered()
{
    w_process = new processing();
    QObject::connect(this,SIGNAL(s_number(int)),w_process,SLOT(r_number(int)));
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));
    w_process->setWindowTitle(QString::fromLocal8Bit("去噪处理中"));
    w_process->setWindowFlags(windowFlags() &~ Qt::WindowCloseButtonHint);
    w_process->setGeometry(x()+100,y()+100,276,68);
    w_process->setFixedWidth(276);
    w_process->setFixedHeight(68);
    w_process->show();

    QTime time;
    time.start();
    while(time.elapsed()<500)
    QCoreApplication::processEvents();

    emit(s_number(0));
    ui->back->setEnabled(true);
    if ((int)backup.size()<maxback)
    {
        backup.push_back(srcimgshort-0);
    }
    else
    {
        backup.pop_front();
        backup.push_back(srcimgshort-0);
    }


    levelAdjustment(srcimgshort,srcimgshort,indark,ingray,inwhite,outdark,outwhite);

    cv::Mat temp=cv::Mat(srcimgshort.size(),CV_32F);
    for (int i=0;i<srcimgshort.rows;i++)
    {
        for (int j=0;j<srcimgshort.cols;j++)
        {
            float t=float(srcimgshort(i,j));
            temp.at<float>(i,j)=float(srcimgshort(i,j));
            t=temp.at<float>(i,j);
        }
    }
    emit(s_number(10));
    cv::Mat temp1;

    cv::bilateralFilter(temp, temp1, 10, 50.0f, 50.0f);
    emit(s_number(80));
    double d1,d2;
    cv::Mat div=temp-temp1;
    cv::minMaxIdx(div,&d1,&d2);
    float t=temp1.at<float>(0,0);
    temp1.convertTo(srcimgshort,CV_16UC1);
    for (int i=0;i<srcimgshort.rows;i++)
    {
        for (int j=0;j<srcimgshort.cols;j++)
        {
            srcimgchar(i,j)=unsigned char(double(srcimgshort(i,j))/65535*255);
        }
    }
    emit(s_number(90));
    showimg=mat2qimage(srcimgchar);
    tshowimg=showimg.scaled(ow*ui->showimgscale->value()/1000,oh*ui->showimgscale->value()/1000,Qt::KeepAspectRatio,Qt::SmoothTransformation);
    ui->winshowimg->setPixmap(QPixmap::fromImage(tshowimg));
    ui->winshowimg->setAlignment(Qt::AlignVCenter|Qt::AlignHCenter);
    emit(s_number(100));

    reset();


//    if (srcimgshort.empty()) return;
//    w_denoise=new ui_denoise();
//    QObject::connect(this,SIGNAL(s_imageshort(cv::Mat_<unsigned short>)),w_denoise,SLOT(r_imageshort(cv::Mat_<unsigned short>)));
//    QObject::connect(w_denoise,SIGNAL(s_imagechar(cv::Mat_<unsigned char>)),this,SLOT(r_imagechar(cv::Mat_<unsigned char>)));
//    QObject::connect(w_denoise,SIGNAL(s_cancel()),this,SLOT(r_cancel()));
//    QObject::connect(w_denoise,SIGNAL(s_ok(cv::Mat_<unsigned short>)),this,SLOT(r_ok(cv::Mat_<unsigned short>)));
//    QObject::connect(w_denoise,SIGNAL(s_imageshort(cv::Mat_<unsigned short>)),this,SLOT(r_imageshort(cv::Mat_<unsigned short>)));
//    emit s_imageshort(srcimgshort);
//    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));
//    w_denoise->setWindowTitle(QString::fromLocal8Bit("对比度调整"));
//    w_denoise->setWindowFlags(Qt::WindowCloseButtonHint);
//    w_denoise->setGeometry(x()+100,y()+100,331,138);
//    w_denoise->setFixedWidth(381);
//    w_denoise->setFixedHeight(138);
//    w_denoise->exec();

}
//图片信息
void MainWindow::on_info_triggered()
{
    if (srcimgshort.empty()) return;
    w4=new ui_imageinfo();
    QObject::connect(this,SIGNAL(s_imageshort(cv::Mat_<unsigned short>)),w4,SLOT(r_imageshort(cv::Mat_<unsigned short>)));
    emit s_imageshort(srcimgshort);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));
    w4->setWindowTitle(QString::fromLocal8Bit("文件信息"));
    w4->setWindowFlags(Qt::WindowCloseButtonHint);
    w4->setGeometry(x()+100,y()+100,220,80);
    w4->setFixedWidth(220);
    w4->setFixedHeight(80);
    w4->exec();
}
//关于
void MainWindow::on_about_triggered()
{
    w5=new ui_about();
    w5->setWindowTitle(QString::fromLocal8Bit("关于本软件"));
    w5->setWindowFlags(Qt::WindowCloseButtonHint);
    w5->setGeometry(x()+100,y()+100,310,130);
    w5->setFixedWidth(310);
    w5->setFixedHeight(130);
    w5->exec();
}

//触发正负片按钮
void MainWindow::on_invert_triggered()
{
    ui->back->setEnabled(true);
    if ((int)backup.size()<maxback)
    {
        backup.push_back(srcimgshort-0);
    }
    else
    {
        backup.pop_front();
        backup.push_back(srcimgshort-0);
    }

    cv::Mat_<unsigned short> timg=cv::Mat_<unsigned short>(srcimgshort.rows, srcimgshort.cols, CV_16UC1);
    levelAdjustment(srcimgshort,timg,indark,ingray,inwhite,outdark,outwhite);

    srcimgshort=65535-timg;

    for (int i=0;i<srcimgshort.rows;i++)
    {
        for (int j=0;j<srcimgshort.cols;j++)
        {
            srcimgchar(i,j)=unsigned char(double(srcimgshort(i,j))/65535*255);
        }
    }

    showimg=mat2qimage(srcimgchar);
    tshowimg=showimg.scaled(ow*ui->showimgscale->value()/1000,oh*ui->showimgscale->value()/1000,Qt::KeepAspectRatio,Qt::SmoothTransformation);
    ui->winshowimg->setPixmap(QPixmap::fromImage(tshowimg));
    ui->winshowimg->setAlignment(Qt::AlignVCenter|Qt::AlignHCenter);

    reset();
}
//触发水平翻转按钮
void MainWindow::on_turn_horizontal_triggered()
{
    ui->back->setEnabled(true);
    if ((int)backup.size()<maxback)
    {
        backup.push_back(srcimgshort-0);
    }
    else
    {
        backup.pop_front();
        backup.push_back(srcimgshort-0);
    }
    int w=srcimgshort.cols;
    int h=srcimgshort.rows;
    cv::Mat_<unsigned short> timg=cv::Mat_<unsigned short>(srcimgshort.rows, srcimgshort.cols, CV_16UC1);
    levelAdjustment(srcimgshort,timg,indark,ingray,inwhite,outdark,outwhite);
    for (int i=0;i<timg.rows;i++)
    {
        for (int j=0;j<timg.cols;j++)
        {
            srcimgshort(i,j)=timg(i,w-1-j);
        }
    }



    for (int i=0;i<srcimgshort.rows;i++)
    {
        for (int j=0;j<srcimgshort.cols;j++)
        {
            srcimgchar(i,j)=unsigned char(double(srcimgshort(i,j))/65535*255);
        }
    }
    showimg=mat2qimage(srcimgchar);
    tshowimg=showimg.scaled(ow*ui->showimgscale->value()/1000,oh*ui->showimgscale->value()/1000,Qt::KeepAspectRatio,Qt::SmoothTransformation);
    ui->winshowimg->setPixmap(QPixmap::fromImage(tshowimg));
    ui->winshowimg->setAlignment(Qt::AlignVCenter|Qt::AlignHCenter);

    reset();
}
//触发竖直翻转按钮
void MainWindow::on_turn_vertical_triggered()
{
    ui->back->setEnabled(true);
    if ((int)backup.size()<maxback)
    {
        backup.push_back(srcimgshort-0);
    }
    else
    {
        backup.pop_front();
        backup.push_back(srcimgshort-0);
    }
    int w=srcimgshort.cols;
    int h=srcimgshort.rows;
    cv::Mat_<unsigned short> timg=cv::Mat_<unsigned short>(srcimgshort.rows, srcimgshort.cols, CV_16UC1);
    levelAdjustment(srcimgshort,timg,indark,ingray,inwhite,outdark,outwhite);
    for (int i=0;i<timg.rows;i++)
    {
        for (int j=0;j<timg.cols;j++)
        {
            srcimgshort(i,j)=timg(h-1-i,j);
        }
    }

    for (int i=0;i<srcimgshort.rows;i++)
    {
        for (int j=0;j<srcimgshort.cols;j++)
        {
            srcimgchar(i,j)=unsigned char(double(srcimgshort(i,j))/65535*255);
        }
    }
    showimg=mat2qimage(srcimgchar);
    tshowimg=showimg.scaled(ow*ui->showimgscale->value()/1000,oh*ui->showimgscale->value()/1000,Qt::KeepAspectRatio,Qt::SmoothTransformation);
    ui->winshowimg->setPixmap(QPixmap::fromImage(tshowimg));
    ui->winshowimg->setAlignment(Qt::AlignVCenter|Qt::AlignHCenter);

    reset();
}
//接收到处理完图片的槽函数
void MainWindow::r_imagechar(cv::Mat_<unsigned char> img)
{
    showimg=mat2qimage(img);
    tshowimg=showimg.scaled(ow*ui->showimgscale->value()/1000,oh*ui->showimgscale->value()/1000,Qt::KeepAspectRatio,Qt::SmoothTransformation);
    ui->winshowimg->setPixmap(QPixmap::fromImage(tshowimg));
    ui->winshowimg->setAlignment(Qt::AlignVCenter|Qt::AlignHCenter);
}
void MainWindow::r_lhdr_dst(cv::Mat a)
{
    cv::Mat_<unsigned char> img;
    a.convertTo(img, CV_8UC1, 255);
    showimg=mat2qimage(img);
    tshowimg=showimg.scaled(ow*ui->showimgscale->value()/1000,oh*ui->showimgscale->value()/1000,Qt::KeepAspectRatio,Qt::SmoothTransformation);
    ui->winshowimg->setPixmap(QPixmap::fromImage(tshowimg));
    ui->winshowimg->setAlignment(Qt::AlignVCenter|Qt::AlignHCenter);
}

//16位
void MainWindow::r_imageshort(cv::Mat_<unsigned short> img)
{
    cv::Mat_<unsigned char> timg=cv::Mat(img.rows,img.cols,CV_8UC1);
    for (int i=0;i<img.rows;i++)
    {
        for (int j=0;j<img.cols;j++)
        {
            timg(i,j)=unsigned char(double(img(i,j))/65535*255);
        }
    }
    showimg=mat2qimage(timg);
    tshowimg=showimg.scaled(ow*ui->showimgscale->value()/1000,oh*ui->showimgscale->value()/1000,Qt::KeepAspectRatio,Qt::SmoothTransformation);
    ui->winshowimg->setPixmap(QPixmap::fromImage(tshowimg));
    ui->winshowimg->setAlignment(Qt::AlignVCenter|Qt::AlignHCenter);
}

//处理取消，还原
void MainWindow::r_cancel()
{
    showimg=mat2qimage(srcimgchar);
    tshowimg=showimg.scaled(ow*ui->showimgscale->value()/1000,oh*ui->showimgscale->value()/1000,Qt::KeepAspectRatio,Qt::SmoothTransformation);
    ui->winshowimg->setPixmap(QPixmap::fromImage(tshowimg));
    ui->winshowimg->setAlignment(Qt::AlignVCenter|Qt::AlignHCenter);

}
//处理确认，变更
void MainWindow::r_ok(cv::Mat_<unsigned short> a)
{
    ui->back->setEnabled(true);
    if ((int)backup.size() < maxback)
    {
        backup.push_back(srcimgshort);
    }
    else
    {
        backup.pop_front();
        backup.push_back(srcimgshort);
    }
    srcimgshort=a;
    for (int i=0;i<srcimgshort.rows;i++)
    {
        for (int j=0;j<srcimgshort.cols;j++)
        {
            srcimgchar(i,j)=unsigned char(double(srcimgshort(i,j))/65535*255);
        }
    }

    reset();
}
//回退申请
void MainWindow::on_back_triggered()
{
    if (backup.empty()) return;

    srcimgshort=backup.back();
    backup.pop_back();
    if (backup.empty())
    {
        ui->back->setEnabled(false);
    }
    if (srcimgshort.empty())
    {
        ui->winshowimg->clear();
    }
    else
    {

        for (int i=0;i<srcimgshort.rows;i++)
        {
            for (int j=0;j<srcimgshort.cols;j++)
            {
                srcimgchar(i,j)=unsigned char(double(srcimgshort(i,j))/65535*255);
             }
        }
        showimg=mat2qimage(srcimgchar);
        tshowimg=showimg.scaled(ow*ui->showimgscale->value()/1000,oh*ui->showimgscale->value()/1000,Qt::KeepAspectRatio,Qt::SmoothTransformation);
        ui->winshowimg->setPixmap(QPixmap::fromImage(tshowimg));
        ui->winshowimg->setAlignment(Qt::AlignVCenter|Qt::AlignHCenter);

        reset();
    }

}
void MainWindow::on_exit_triggered()
{
    this->close();
}




