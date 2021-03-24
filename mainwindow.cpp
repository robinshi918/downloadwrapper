#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QProcess>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->cancelButton, &QPushButton::released, this, &MainWindow::handleCancelButton);
    connect(ui->startButton, &QPushButton::released, this, &MainWindow::handleStartButton);
    connect(ui->singleMusicRadioButton, &QRadioButton::clicked, this, &MainWindow::handleTypeSelected);
    connect(ui->playlistRadioButton, &QRadioButton::clicked, this, &MainWindow::handleTypeSelected);

    init();
}

MainWindow::~MainWindow()
{
    QProcess p;
    delete ui;
}

void MainWindow::handleCancelButton()
{
    QString url = ui->urlEdit->text();
    QMessageBox msgBox;
    msgBox.setText("url: " + url);
    msgBox.exec();
}

void MainWindow::handleStartButton()
{
    QString url = ui->urlEdit->text();
    QMessageBox msgBox;
    msgBox.setText("Start");
    msgBox.exec();
}

void MainWindow::handleTypeSelected()
{
    if (ui->singleMusicRadioButton->isChecked()) {
        init();
    } else {
        this->isSingleMusic = false;
        ui->playListOptionGroup->setVisible(true);
    }
}

void MainWindow::init()
{
    isSingleMusic = true;
    ui->playListOptionGroup->setVisible(false);
    ui->startEdit->setText("");
    ui->endEdit->setText("");

    qInfo() << "MainWindow::init()";
}

void MainWindow::downloadPlayList(QString &url, unsigned int startPos, unsigned int endPos)
{

}

void MainWindow::downloadSingle(QString &url)
{

}



