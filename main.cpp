#include "mainwindow.h"
#include <QApplication>
#include "QString"
#include "QTextCodec"
#include "QTextStream"
#include "qfile.h"
#include "QDebug"

using namespace std;
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));
    w.setWindowTitle(QString::fromLocal8Bit("工业探伤"));
    w.initialize();
    w.show();
    QFile file(":/style-back.qss");
    file.open(QFile::ReadOnly);
    QString ss=file.readAll();
    file.close();
    a.setStyleSheet(ss);

    return a.exec();
}
