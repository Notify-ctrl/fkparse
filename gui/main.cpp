#include "mainwindow.h"
#include "fkparse.h"
#include <QApplication>

// Simple test of library
int main(int argc, char **argv) {
  QApplication app(argc, argv);

  MainWindow w;
  w.show();

  return app.exec();
}
