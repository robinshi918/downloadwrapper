#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QProcess>
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QClipboard>
#include <QApplication>

#define DEFAULT_DOWNLOAD_FOLDER QDir::home().path() +  QDir::separator() + "Desktop/mp3/download/"
#define DOWNLOAD_PATTERN QString("%(title)s.%(ext)s")

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
    connect(ui->autoUploadCheck, &QCheckBox::stateChanged, this, &MainWindow::autoUploadStateChanged);
    connect(qApp, SIGNAL(focusChanged(QWidget*, QWidget*)), this, SLOT(onFocusChanged(QWidget*, QWidget*)));
    connect(ui->pathSelectionButton, &QPushButton::released, this, &MainWindow::onSelectDownloadPath);
    init();
}

void MainWindow::onFocusChanged(QWidget* old, QWidget* newWidget)
{

    QClipboard* clipboard = QApplication::clipboard();
    //qInfo() << "onFocusChanged() : clipBoard text = " << clipboard->text();
    QString cilpBoardText = clipboard->text();
    if(!cilpBoardText.isEmpty()) {
        ui->urlEdit->setText(cilpBoardText);
    }
}

MainWindow::~MainWindow()
{
    QProcess p;
    delete ui;
}

void MainWindow::init()
{
    isSingleMusic = true;
    downloadFolder = DEFAULT_DOWNLOAD_FOLDER;
    ui->playListOptionGroup->setVisible(false);
    ui->startEdit->setText("");
    ui->endEdit->setText("");
    ui->savedPathEdit->setText(QString(DEFAULT_DOWNLOAD_FOLDER));

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

    if (ui->autoUploadCheck->checkState() == Qt::CheckState::Checked) {
        ui->uploadButton->hide();
    } else {
        ui->uploadButton->show();
    }
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
    printToOutput("Uploading " + fileName);
    QStringList arguments{"-T", fileName, "-g", "-u", "pi:hallo","ftp://192.168.1.21:21/upload/"};
    uploadProcess.start("/usr/bin/curl", arguments);
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
                                  "--output",getDownloadFolder() + QDir::separator() + DOWNLOAD_PATTERN,
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
    QStringList arguments{"-icw", "--extract-audio",  "--audio-format", "mp3", "--output",getDownloadFolder() + DOWNLOAD_PATTERN, url};

    downloadProcess.start("/usr/local/bin/youtube-dl", arguments);
    ui->startButton->setEnabled(false);
    ui->startButton->setText("Downloading");
}

void MainWindow::printToOutput(QString str, bool replaceLastLine)
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
        qInfo() << stdout;
        ui->statusbar->showMessage(stdout);

        if (stdout.contains("Deleting original file")) {return;}
        else if (stdout.contains("[ffmpeg] Destination: ")) {
            downloadFileName = stdout.mid(QString("[ffmpeg] Destination: ").size());
            downloadFileName = downloadFileName.mid(0, downloadFileName.size() - 1);
            qInfo() << "downloaded file is:" << downloadFileName;
            return;
        } else if (stdout.contains("[ffmpeg]")) {
            printToOutput("converting to mp3 file. \n\n", true);
            return;
        } else if (stdout.contains("[download]")) {
            //printToOutput("downloading from Youtube", true);
            return;
        } else if (stdout.contains("[youtube] ")) {
            printToOutput("Start downloading....\n\n");
            return;
        }

        bool isDownloadingInfo = stdout.contains("[download]") && !stdout.contains("[download] Destination:");
        printToOutput(stdout, isDownloadingInfo);
    }

    if (stderr.size() > 0) {
        qInfo() << stderr;
        ui->statusbar->showMessage(stderr);
        printToOutput(stderr);
    }
}

void MainWindow::downloadCommandFinished(int exitCode, QProcess::ExitStatus) {
    qInfo() << "download finished " << exitCode;

    if (exitCode == 0) {
        QFileInfo info(downloadFileName);
        QString fileName(info.fileName());

        printToOutput("\n###### Download Successful!!! #####\nFile Name: " + downloadFileName + "\n\n");
        ui->statusbar->showMessage("[Download Successful] -> " + fileName);
        if (ui->autoUploadCheck->checkState() == Qt::CheckState::Checked) {
            uploadToFtp(downloadFileName);
        }
    } else {
        printToOutput("###### Download Failed!!! #####\n\n");
        ui->statusbar->showMessage("[Download failed] " + QString("").setNum(exitCode));
    }
    ui->startButton->setEnabled(true);
    ui->startButton->setText("Download");

}

void MainWindow::readUploadProcessOutput(void){
    QString stdout = uploadProcess.readAllStandardOutput();
    QString stderr = uploadProcess.readAllStandardError();

    if (stdout.size() > 0) {
        qInfo() << "[upload:stdout]"  << stdout;
        ui->statusbar->showMessage(stdout);
        // addToOutput(stdout);
    }

    if (stderr.size() > 0) {
        qInfo() << "[upload:stderr]" << stderr;
        // addToOutput(stderr);
        ui->statusbar->showMessage(stderr);
    }
}

void MainWindow::uploadCommandFinished(int exitCode, QProcess::ExitStatus) {
    qInfo() << "upload finished " << exitCode;

    QFileInfo info(downloadFileName);
    QString fileName(info.fileName());

    if (exitCode == 0) {
        ui->statusbar->showMessage("[Upload Successful] ->" + fileName);
        printToOutput("\n###### Upload Successful!!! #####\nFile Name: " + fileName + "\n\n");
    } else {
        ui->statusbar->showMessage("[Upload failed] " + QString("").setNum(exitCode));
        printToOutput("###### Upload Failed!!! #####\nFile Name: " + fileName + "\n\n");
    }
    ui->uploadButton->setEnabled(true);
    ui->uploadButton->setText("Upload");
}

void MainWindow::autoUploadStateChanged(int state) {
    if (state == Qt::CheckState::Checked) {
        ui->uploadButton->hide();
    } else {
        ui->uploadButton->show();
    }
}

void MainWindow::onSelectDownloadPath()
{
    QString path = QFileDialog::getExistingDirectory(0,
                                                     ("Select Output Folder"),
                                                     QDir::home().path() +  QDir::separator() + "Desktop/mp3/");
    qInfo() << "selected path =" << path;
    if (!path.isEmpty()) {
        ui->savedPathEdit->setText(path);
        downloadFolder = path;
    }
}

QString MainWindow::getDownloadFolder()
{
    return downloadFolder;
}

// subsonic commands
// 1. force scan media library
// 2. get scan status to check if scan has finished.

// refer to http://www.subsonic.org/pages/api.jsp#startScan
// HOW-TO: call startScan and poll the result of getScanStatus
//startScan
//curl "http://192.168.1.21:4040/rest/startScan?u=admin&t=1d20736c96f8b965488b23b3edee8b14&s=c19b2d&v=1.16.1&c=robinapp&f=json"

//getScanStatus
//curl "http://192.168.1.21:4040/rest/getScanStatus?u=admin&t=1d20736c96f8b965488b23b3edee8b14&s=c19b2d&v=1.16.1&c=robinapp&f=json"






