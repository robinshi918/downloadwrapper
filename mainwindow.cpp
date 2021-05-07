#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QProcess>
#include <QMessageBox>
#include <QDebug>
#include <QFile>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->cancelButton, &QPushButton::released, this, &MainWindow::handleCancelButton);
    connect(ui->startButton, &QPushButton::released, this, &MainWindow::handleStartButton);
    connect(ui->uploadButton, &QPushButton::released, this, &MainWindow::handleUploadButton);
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
        return;
    }

    if (ui->singleMusicRadioButton->isChecked()) {
        downloadSingle(url);
    } else {
        downloadPlayList(url, ui->startEdit->text().toUInt(), ui->endEdit->text().toUInt());
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

void MainWindow::handleUploadButton()
{
    // upload a file
    //curl -T upload_test.mp3 -u pi:hallo ftp://192.168.1.21:21/video/

    //list a folder
    //curl -l -u pi:hallo ftp://192.168.1.21:21/music/mp3_yuan
}

void MainWindow::init()
{
    isSingleMusic = true;
    ui->playListOptionGroup->setVisible(false);
    ui->startEdit->setText("");
    ui->endEdit->setText("");
    ui->urlEdit->setText("https://www.youtube.com/watch?v=l11mUSu7aeA");

    connect(&p, SIGNAL(readyReadStandardOutput()),
            this, SLOT(readSubProcess()));
    connect(&p, SIGNAL(readyReadStandardError()),
            this, SLOT(readSubProcess()));
    connect(&p, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(commandFinished(int, QProcess::ExitStatus)));

    injectEnvironmentVar();
}

void MainWindow::downloadPlayList(QString &url, unsigned int startPos, unsigned int endPos)
{
    if (startPos > endPos) {
        showMessageBox("Playlist start position cannot be larger than end position.");
        return;
    }

    QStringList arguments{"-icw", "--extract-audio",
                          "--playlist-start",
                          QString::number(startPos),
                          "--playlist-end",
                                  QString::number(endPos) ,
                                  "--audio-format", "mp3",
                                  "--output","/Users/robinshi/Desktop/QtTest/mp3/%(title)s.%(ext)s",
                                  url};

    p.start("/usr/local/bin/youtube-dl", arguments);
    ui->startButton->setEnabled(false);
    ui->startButton->setText("Downloading");
}

void MainWindow::injectEnvironmentVar()
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("PATH", env.value("PATH") + ":/usr/local/opt/openjdk/bin:/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/share/dotnet:~/.dotnet/tools:/Library/Apple/usr/bin:/Users/robinshi/Library/Android/sdk/platform-tools/");
        p.setProcessEnvironment(env);
}

void MainWindow::downloadSingle(QString &url)
{
    QStringList arguments{"-icw", "--extract-audio",  "--audio-format", "mp3", "--output","/Users/robinshi/Desktop/QtTest/mp3/%(title)s.%(ext)s",  url};


    p.start("/usr/local/bin/youtube-dl", arguments);
    ui->startButton->setEnabled(false);
    ui->startButton->setText("Downloading");
}

void MainWindow::addToOutput(QString str, bool replaceLastLine)
{
    if (str.isEmpty()) {
        return;
    }

    QString labelContent = ui->outputLabel->text();
    if (!replaceLastLine) {
        ui->outputLabel->setText(str  + labelContent);
    } else {
        QStringList lines = labelContent.split("\n");
        if (lines.size() > 0) {
            lines[0] = str;
        }
        labelContent = lines.join("\n");
        ui->outputLabel->setText(labelContent);
    }
}

void MainWindow::readSubProcess() {
    QString stdout = p.readAllStandardOutput();
    QString stderr = p.readAllStandardError();

    if (stdout.size() > 0) {

        if (stdout.contains("Deleting original file")) return;

        qInfo() << stdout;
        bool isDownloadingInfo = stdout.contains("[download]") && !stdout.contains("[download] Destination:");
        addToOutput(stdout, isDownloadingInfo);
    }

    if (stderr.size() > 0) {
        qInfo() << stderr;
        addToOutput(stderr);
    }
}

void MainWindow::commandFinished(int exitCode, QProcess::ExitStatus) {
    qInfo() << "finished " << exitCode;
    if (exitCode == 0) {
        addToOutput("\n###### Download Successful!!! #####\n\n");
    } else {
        addToOutput("###### Download Failed!!! #####\n\n");
    }
    ui->startButton->setEnabled(true);
    ui->startButton->setText("Start");

}





