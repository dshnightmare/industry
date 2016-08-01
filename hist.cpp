#include "hist.h"


QImage mat2qimage(cv::Mat_<unsigned char>& img)
{
    QImage img1;
    if (img.channels()==3)
    {
        cvtColor(img,img,CV_BGR2RGB);
        img1 = QImage((const unsigned char*)(img.data),img.cols,img.rows,
                      img.cols*img.channels(),QImage::Format_RGB888);
    }
    else if(img.channels()==1)
    {
        QVector<QRgb>  colorTable;

            for(int k=0;k<256;++k)
            {

                   colorTable.push_back( qRgb(k,k,k) );

            }


        img1 = QImage((const unsigned char*)(img.data),img.cols,img.rows,
                    img.cols*img.channels(),QImage::Format_Indexed8);
         img1.setColorTable(colorTable);
    }
    else
    {
        img1 = QImage((const unsigned char *)(img.data), img.cols, img.rows,
                        img.cols*img.channels(), QImage::Format_RGB888);
    }
    return img1;
}


