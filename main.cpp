#include "mainwindow.h"
#include "openglwidget.h"
#include <QApplication>
#include <QString>
#include <omp.h>

#include "geometry.hpp"

#include <iostream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
