#include "mainwindow.h"
#include <QtWidgets>

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent) {
  parser = fkp_new_parser();
  m_error = false;
  setMinimumSize(400, 300);

  QWidget *centralWidget = new QWidget(this);
  setCentralWidget(centralWidget);

  QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
  QHBoxLayout *fileLayout = new QHBoxLayout;
  fileNameEdit = new QLineEdit;
  fileNameEdit->setEnabled(false);
  connect(fileNameEdit, &QLineEdit::textChanged, [this]{
    packBtn->setEnabled(false);
  });
  fileLayout->addWidget(fileNameEdit);
  QPushButton *chooseFileBtn = new QPushButton(tr("Choose File..."));
  connect(chooseFileBtn, &QPushButton::clicked, [this]{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
      QDir::currentPath(), tr("Text (*.txt)"));
    this->fileNameEdit->setText(fileName);
  });
  fileLayout->addWidget(chooseFileBtn);
  mainLayout->addLayout(fileLayout);

  QHBoxLayout *btnLayout = new QHBoxLayout;
  compileTypeBox = new QComboBox;
  compileTypeBox->addItem(tr("QSanguosha (*.lua)"), FKP_QSAN_LUA);
  compileTypeBox->addItem(tr("QSanguosha-Heg (*.lua)"), FKP_QSAN_HEG_LUA);
  compileTypeBox->addItem(tr("Noname (*.js)"), FKP_NONAME_JS);
  compileTypeBox->addItem(tr("DSanguosha (*.ts)"), FKP_DSGS_TS);
  compileTypeBox->addItem(tr("FreeKill (*.lua)"), FKP_FK_LUA);
  connect(compileTypeBox, &QComboBox::currentTextChanged, [this]{
    packBtn->setEnabled(false);
  });
  compileBtn = new QPushButton(tr("Compile"));
  packBtn = new QPushButton(tr("Pack"));
  packBtn->setEnabled(false);
  connect(compileBtn, &QPushButton::clicked, this, &MainWindow::compile);
  connect(packBtn, &QPushButton::clicked, this, &MainWindow::pack);
  connect(this, &MainWindow::errorChanged, packBtn, &QPushButton::setEnabled);
  btnLayout->addWidget(compileTypeBox);
  btnLayout->addWidget(compileBtn);
  btnLayout->addWidget(packBtn);
  mainLayout->addLayout(btnLayout);

  errorEdit = new QTextEdit;
  errorEdit->setFontFamily("Noto Sans Mono CJK SC");
  mainLayout->addWidget(errorEdit);
}

MainWindow::~MainWindow() {
  fkp_close(parser);
}

bool MainWindow::isError() const {
  return m_error;
}

void MainWindow::setError(bool error) {
  m_error = error;
  emit errorChanged(!error);
}

void MainWindow::compile() {
  QString fileName = fileNameEdit->text();
  if (!QFile::exists(fileName)) {
    errorEdit->setText(tr("File does not exist!"));
    return;
  }
  QString cwd = QDir::currentPath();

  QStringList strlist = fileName.split('/');
  QString shortFileName = strlist.last();
  strlist.removeLast();
  QString path = strlist.join('/');
  QDir::setCurrent(path);

  bool error = fkp_parse(
    parser,
    shortFileName.toUtf8().data(),
    (fkp_analyze_type)compileTypeBox->currentData().toInt()
  );
  setError(error);

  if (error) {
    QStringList tmplist = shortFileName.split('.');
    tmplist.removeLast();
    QString fName = tmplist.join('.') + "-error.txt";
    if (!QFile::exists(fName)) {
      errorEdit->setText(tr("Unknown compile error."));
    } else {
      QFile f(fName);
      f.open(QIODevice::ReadOnly);
      errorEdit->setText(f.readAll());
      f.remove();
    }
  } else {
    errorEdit->setText(tr("Successfully compiled chosen file."));
  }
  QDir::setCurrent(cwd);
}

static void copyFkpHash2QHash(QHash<QString, QString> &dst, fkp_hash *from) {
  dst.clear();
  for (size_t i = 0; i < from->capacity; i++) {
    if (from->entries[i].key != NULL) {
      dst[from->entries[i].key] = QString((const char *)from->entries[i].value);
    }
  }
}

void MainWindow::readHashFromParser() {
  copyFkpHash2QHash(generals, parser->generals);
  copyFkpHash2QHash(skills, parser->skills);
  copyFkpHash2QHash(marks, parser->marks);
}

void MainWindow::pack() {
  readHashFromParser();
  auto packType = (fkp_analyze_type)compileTypeBox->currentData().toInt();
  switch (packType) {
  case FKP_QSAN_LUA:
    packQSan();
    break;
  default:
    break;
  }
}

void MainWindow::packQSan() {
  QString fileName = fileNameEdit->text();
  if (!QFile::exists(fileName)) {
    errorEdit->setText(tr("File does not exist!"));
    return;
  }
  QString cwd = QDir::currentPath();

  QStringList strlist = fileName.split('/');
  QString shortFileName = strlist.last();
  QStringList tmplist = shortFileName.split('.');
  tmplist.removeLast();
  shortFileName = tmplist.join('.');
  strlist.removeLast();
  QString path = strlist.join('/');
  QDir::setCurrent(path);

  QDir dir(path);
  QString outDirName = shortFileName + "_out";
  if (!dir.mkdir(outDirName)) {
    dir.cd(outDirName);
    dir.removeRecursively();
    dir.cdUp();
    dir.mkdir(outDirName);
  }
  dir.cd(outDirName);
  dir.mkdir("extensions");
  QFile::copy(path + "/" + shortFileName + ".lua",
              dir.absolutePath() + "/extensions/" + shortFileName + ".lua");
  QFile::remove(path + "/" + shortFileName + ".lua");

  dir.mkpath("lua/lib");
  QFile::copy(cwd + "/lua/fkparser.lua", dir.absolutePath() + "/lua/lib/fkparser.lua");

  dir.mkpath("image/fullskin/generals/full");
  dir.mkpath("image/generals/avatar");
  dir.mkpath("image/generals/card");
  dir.mkpath("audio/death");
  dir.mkpath("audio/skill");

  QHashIterator<QString, QString> gi(generals);
  while (gi.hasNext()) {
    gi.next();
    QString id = gi.value();
    QString name = gi.key();
    QString card = name + "_卡图.jpg";
    QString full = name + "_全身图.png";
    QString avatar = name + "_头图.png";
    QString death = name + ".ogg";
    QFile::copy(card, dir.absolutePath() + "/image/generals/card/" + id + ".jpg");
    QFile::copy(full, dir.absolutePath() + "/image/fullskin/generals/full/" + id + ".png");
    QFile::copy(avatar, dir.absolutePath() + "/image/generals/avatar/" + id + ".png");
    QFile::copy(death, dir.absolutePath() + "/audio/death/" + id + ".ogg");
  }

  QHashIterator<QString, QString> si(skills);
  while (si.hasNext()) {
    si.next();
    QString id = si.value();
    QString name = si.key();
    QString au1 = name + "1.ogg";
    QString au2 = name + "2.ogg";
    QFile::copy(au1, dir.absolutePath() + "/audio/skill/" + id + "1.ogg");
    QFile::copy(au2, dir.absolutePath() + "/audio/skill/" + id + "2.ogg");
  }

  QHashIterator<QString, QString> mi(marks);
  while (mi.hasNext()) {
    mi.next();
    QString id = mi.value();
    QString name = mi.key();
    QFile::copy(name + ".png", dir.absolutePath() + "/image/mark/" + id + ".png");
  }

  QDir::setCurrent(cwd);
}

