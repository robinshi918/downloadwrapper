#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QProcess>
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QFileInfo>

#define DOWNLOAD_FOLDER "/Users/shiyun/Desktop/mp3/download/%(title)s.%(ext)s"

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
    ui->outputLabel->clear();
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

    uploadToFtp(downloadFileName);
    ui->uploadButton->setEnabled(false);
    ui->uploadButton->setText("Uploading");
}

void MainWindow::uploadToFtp(QString fileName) {

    if (fileName.isEmpty()) return;

    qInfo() << "uploading" << fileName;
    QStringList arguments{"-T", fileName, "-g", "-u", "pi:hallo","ftp://192.168.1.21:21/upload/"};
    uploadProcess.start("/usr/bin/curl", arguments);
}

void MainWindow::init()
{
    isSingleMusic = true;
    ui->playListOptionGroup->setVisible(false);
    ui->startEdit->setText("");
    ui->endEdit->setText("");
    ui->urlEdit->setText("https://www.youtube.com/watch?v=l11mUSu7aeA");

    connect(&downloadProcess, SIGNAL(readyReadStandardOutput()),
            this, SLOT(readDownloadProcessOutput()));
    connect(&downloadProcess, SIGNAL(readyReadStandardError()),
            this, SLOT(readDownloadProcessOutput()));
    connect(&downloadProcess, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(downloadCommandFinished(int, QProcess::ExitStatus)));


    connect(&uploadProcess, SIGNAL(readyReadStandardOutput()),
            this, SLOT(readUploadProcessOutput()));
    connect(&uploadProcess, SIGNAL(readyReadStandardError()),
            this, SLOT(readUploadProcessOutput()));
    connect(&uploadProcess, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(uploadCommandFinished(int, QProcess::ExitStatus)));

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
                                  "--output",DOWNLOAD_FOLDER,
                                  url};

    downloadProcess.start("/usr/local/bin/youtube-dl", arguments);
    ui->startButton->setEnabled(false);
    ui->startButton->setText("Downloading");
}

void MainWindow::injectEnvironmentVar()
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PATH", env.value("PATH") + ":/usr/local/opt/openjdk/bin:/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/share/dotnet:~/.dotnet/tools:/Library/Apple/usr/bin:/Users/robinshi/Library/Android/sdk/platform-tools/");
    downloadProcess.setProcessEnvironment(env);
    uploadProcess.setProcessEnvironment(env);
}

void MainWindow::downloadSingle(QString &url)
{
    downloadFileName = "";
    QStringList arguments{"-icw", "--extract-audio",  "--audio-format", "mp3", "--output",DOWNLOAD_FOLDER, url};

    downloadProcess.start("/usr/local/bin/youtube-dl", arguments);
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

void MainWindow::readDownloadProcessOutput() {
    QString stdout = downloadProcess.readAllStandardOutput();
    QString stderr = downloadProcess.readAllStandardError();

    if (stdout.size() > 0) {

        if (stdout.contains("Deleting original file")) return;
        if (stdout.contains("[ffmpeg] Destination: ")) {
            downloadFileName = stdout.mid(QString("[ffmpeg] Destination: ").size());
            downloadFileName = downloadFileName.mid(0, downloadFileName.size() - 1);
            qInfo() << "downloaded file is:" << downloadFileName;
        }

        qInfo() << stdout;
        bool isDownloadingInfo = stdout.contains("[download]") && !stdout.contains("[download] Destination:");
        addToOutput(stdout, isDownloadingInfo);
    }

    if (stderr.size() > 0) {
        qInfo() << stderr;
        addToOutput(stderr);
    }
}

void MainWindow::downloadCommandFinished(int exitCode, QProcess::ExitStatus) {
    qInfo() << "download finished " << exitCode;
    if (exitCode == 0) {
        addToOutput("\n###### Download Successful!!! #####\n\n");
    } else {
        addToOutput("###### Download Failed!!! #####\n\n");
    }
    ui->startButton->setEnabled(true);
    ui->startButton->setText("Download");
}

void MainWindow::readUploadProcessOutput(void){
    QString stdout = uploadProcess.readAllStandardOutput();
    QString stderr = uploadProcess.readAllStandardError();

    if (stdout.size() > 0) {
        qInfo() << "[upload:stdout]"  << stdout;
        addToOutput(stdout);
    }

    if (stderr.size() > 0) {
        qInfo() << "[upload:stderr]" << stderr;
        addToOutput(stderr);
    }
}

void MainWindow::uploadCommandFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    qInfo() << "upload finished " << exitCode;

    QFileInfo info(downloadFileName);
    QString fileName(info.fileName());

    if (exitCode == 0) {
        addToOutput("\n###### Upload Successful!!! #####\nFile Name: " + fileName + "\n\n");
    } else {
        addToOutput("###### Upload Failed!!! #####\nFile Name: " + fileName + "\n\n");
    }
    ui->uploadButton->setEnabled(true);
    ui->uploadButton->setText("Upload");
}

QString MainWindow::normalizeUploadParameter(QString file) {
    return "";
}





