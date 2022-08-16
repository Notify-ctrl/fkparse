#ifndef _MAINWINDOW_H
#define _MAINWINDOW_H

#include "fkparse.h"
#include <QMainWindow>

class QComboBox;
class QPushButton;
class QLineEdit;
class QTextEdit;

class MainWindow : public QMainWindow {
  Q_OBJECT
  Q_PROPERTY(bool m_error READ isError WRITE setError NOTIFY errorChanged)

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  bool isError() const;
  void setError(bool error);

signals:
  void errorChanged(bool);

private:
  QLineEdit *fileNameEdit;
  QComboBox *compileTypeBox;
  QPushButton *compileBtn;
  QPushButton *packBtn;
  QTextEdit *errorEdit;

  fkp_parser *parser;
  bool m_error;

  QHash<QString, QString> generals;
  QHash<QString, QString> skills;
  QHash<QString, QString> marks;

  void compile();
  void readHashFromParser();
  void pack();
  void packQSan();
};

#endif
