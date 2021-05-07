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

}

void MainWindow::showMessageBox(QString text)
{
    QMessageBox msgBox;
    msgBox.setText(text);
    msgBox.exec();
}

void MainWindow::handleStartButton()
{
    QString url = ui->urlEdit->text();
    if (url.isEmpty()) {
        showMessageBox("download does not start because url is empty.");
    } else {
        downloadSingle(url);
    }
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

    ui->urlEdit->setText("https://www.youtube.com/watch?v=l11mUSu7aeA");

//    qInfo() << "MainWindow::init()";
}

void MainWindow::downloadPlayList(QString &url, unsigned int startPos, unsigned int endPos)
{

}

void MainWindow::downloadSingle(QString &url)
{
    QStringList arguments{"-icw", "--extract-audio",  "--audio-format", "mp3", "--output","'/Users/robinshi/Desktop/QtTest/mp3/%(title)s.%(ext)s'",  url};
//    qInfo() << arguments;

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("PATH", env.value("PATH") + ":/usr/local/opt/openjdk/bin:/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/share/dotnet:~/.dotnet/tools:/Library/Apple/usr/bin:/Users/robinshi/Library/Android/sdk/platform-tools/");
//        env.insert("PATH", env.value("PATH") + ":/opt/local/sbin");
        p.setProcessEnvironment(env);



    p.start("/usr/local/bin/youtube-dl", arguments);

    connect(&p, SIGNAL(readyReadStandardOutput()),
            this, SLOT(readSubProcess()));
    connect(&p, SIGNAL(readyReadStandardError()),
            this, SLOT(readSubProcess()));


}

void MainWindow::addToOutput(QString str)
{
    if (!str.isEmpty()) {
        QString labelContent = ui->outputLabel->text();
        ui->outputLabel->setText(labelContent + str);
    }
}

void MainWindow::readSubProcess() {
    QString stdout = p.readAllStandardOutput();
    QString stderr = p.readAllStandardError();

    qInfo() << stdout;
    qInfo() << stderr;
    addToOutput(stdout);
    addToOutput(stderr);
}





