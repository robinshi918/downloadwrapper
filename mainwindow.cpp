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
#include <QDateTime>
#include <downloadstate.h>
#include <renamedialog.h>
#include <settingsdialog.h>


#define DEFAULT_DOWNLOAD_FOLDER QDir::home().path() +  QDir::separator() + "Desktop/mp3/download/"
#define DOWNLOAD_PATTERN QString("%(title)s.%(ext)s")

#define DEFAULT_FTP_USER_NAME QString("pi")
#define DEFAULT_FTP_PASSWORD QString("hallo")
#define DEFAULT_FTP_SERVER QString("192.168.1.21")
#define DEFAULT_FTP_REMOTE_PATH QString("upload")
#define DEFAULT_FTP_PORT QString("21")


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->renameDialog = new RenameDialog;
    this->settingsDialog = new SettingsDialog;
    connect(ui->cancelButton, &QPushButton::released, this, &MainWindow::handleCancelButton);
    connect(ui->startButton, &QPushButton::released, this, &MainWindow::handleStartButton);
    connect(ui->uploadButton, &QPushButton::released, this, &MainWindow::handleUploadButton);
    connect(ui->settingsButton, &QPushButton::released, this, &MainWindow::handleSettingsButton);
    connect(ui->singleMusicRadioButton, &QRadioButton::clicked, this, &MainWindow::handleTypeSelected);
    connect(ui->playlistRadioButton, &QRadioButton::clicked, this, &MainWindow::handleTypeSelected);
    connect(ui->autoUploadCheck, &QCheckBox::stateChanged, this, &MainWindow::autoUploadStateChanged);
    connect(qApp, SIGNAL(focusChanged(QWidget*, QWidget*)), this, SLOT(onFocusChanged(QWidget*, QWidget*)));
    connect(ui->pathSelectionButton, &QPushButton::released, this, &MainWindow::onSelectDownloadPath);
    init();
}

void MainWindow::initSettings()
{
    QString settingFile = QApplication::applicationDirPath().left(1) + ":/setting.ini";
    if (settings == NULL) {
        settings = new QSettings(settingFile, QSettings::NativeFormat);
    }

    this->ftpPassword = settings->value("ftp_password", DEFAULT_FTP_PASSWORD).toString();
    this->ftpUser = settings->value("ftp_user", DEFAULT_FTP_USER_NAME).toString();
    this->ftpServer = settings->value("ftp_server", DEFAULT_FTP_SERVER).toString();
    this->ftpRemotePath = settings->value("ftp_remote_path", DEFAULT_FTP_REMOTE_PATH).toString();
    this->ftpPort = settings->value("ftp_port", DEFAULT_FTP_PORT).toString();
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

    initSettings();
    connectSignals();
    injectEnvironmentVar();
    initUI();
}

void MainWindow::connectSignals() {
    // download process
    connect(&downloadProcess, SIGNAL(readyReadStandardOutput()),
            this, SLOT(onDownloadProgress()));
    connect(&downloadProcess, SIGNAL(readyReadStandardError()),
            this, SLOT(onDownloadProgress()));
    connect(&downloadProcess, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(onDownloadFinish(int, QProcess::ExitStatus)));

    // upload process
    connect(&uploadProcess, SIGNAL(readyReadStandardOutput()),
            this, SLOT(onUploadProgress()));
    connect(&uploadProcess, SIGNAL(readyReadStandardError()),
            this, SLOT(onUploadProgress()));
    connect(&uploadProcess, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(onUploadFinish(int, QProcess::ExitStatus)));

    // publish process
    connect(&publishProcess, SIGNAL(readyReadStandardError()),
            this, SLOT(onPublishProgress()));
    connect(&publishProcess, SIGNAL(readyReadStandardOutput()),
            this, SLOT(onPublishProgress()));
    connect(&publishProcess, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(onPublishFinish(int, QProcess::ExitStatus)));


    connect(renameDialog, SIGNAL(accepted()), this, SLOT(onFileRenameAccepted()));
    connect(renameDialog, SIGNAL(rejected()), this, SLOT(onFileRenameRejected()));


}

void MainWindow::initUI() {

    qInfo() << "intUI";

    if (ui->autoUploadCheck->checkState() == Qt::CheckState::Checked) {
        ui->uploadButton->hide();
    } else {
        ui->uploadButton->show();
    }
}

void MainWindow::handleCancelButton()
{
    ui->logEdit->clear();
}


void MainWindow::handleSettingsButton()
{
    this->settingsDialog->show();
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

    printToOutput("\n", false);
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

    uploadToFtp(downloadFileFullPath);
    ui->uploadButton->setEnabled(false);
    ui->uploadButton->setText("Uploading");
}

void MainWindow::uploadToFtp(QString fileName) {

    if (fileName.isEmpty()) return;

    qInfo() << "uploading" << fileName;

    printToOutput("Uploading " + fileName);
    QStringList arguments{"-T", fileName,
                "-g",
                "-u", ftpUser + ":" + ftpPassword,
                "ftp://" + ftpServer + ":" + ftpPort + "/" + ftpRemotePath + "/"};
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
    downloadFileFullPath = "";
    QStringList arguments{"-icw", "--extract-audio",  "--audio-format", "mp3", "--output",getDownloadFolder() + DOWNLOAD_PATTERN, url};

    downloadProcess.start("/usr/local/bin/youtube-dl", arguments);
    ui->startButton->setEnabled(false);
    ui->startButton->setText("Downloading");
}

void MainWindow::printToOutput(QString str, bool printTimestamp)
{
    if (str.isEmpty()) {
        return;
    }

    if (printTimestamp) {
        QDateTime date = QDateTime::currentDateTime();
        QString formattedTime = date.toString("yyyy.MM.dd hh:mm:ss");
        ui->logEdit->appendPlainText(formattedTime + "   " + str);
    } else {
        ui->logEdit->appendPlainText(str);
    }
}

void MainWindow::printToStatusBar(QString str)
{
    ui->statusbar->showMessage(str);
}

void MainWindow::onDownloadProgress() {
    QString stdout = downloadProcess.readAllStandardOutput();
    QString stderr = downloadProcess.readAllStandardError();

    if (stdout.size() > 0) {
        qInfo() << stdout;
        // always print to status bar
        printToStatusBar(stdout);

        int newState = DownloadState::STATE_NONE;

        if (stdout.contains("Deleting original file")) {
            return;
        } else if (stdout.contains("[ffmpeg] Destination: ")) {
            downloadFileFullPath = stdout.mid(QString("[ffmpeg] Destination: ").size());
            downloadFileFullPath = downloadFileFullPath.mid(0, downloadFileFullPath.size() - 1);
            qInfo() << "downloaded file is:" << downloadFileFullPath;
            newState = DownloadState::STATE_CONVERTING;
        } else if (stdout.contains("[ffmpeg]")) {
            newState = DownloadState::STATE_CONVERTING;
        } else if (stdout.contains("[download]")) {
            newState = DownloadState::STATE_DOWNLOADING;
        } else if (stdout.contains("[youtube] ")) {
            newState = DownloadState::STATE_PREPARING;
        }

        if (this->state != newState) {
            this->state = newState;
            printToOutput(DownloadState::stateToString(newState));
        }
    }

    if (stderr.size() > 0) {
        qInfo() << stderr;
        printToStatusBar(stderr);
        printToOutput(stderr);
    }
}

void MainWindow::onDownloadFinish(int exitCode, QProcess::ExitStatus) {
    qInfo() << "download finished " << exitCode;

    if (exitCode == 0) {
        QFileInfo info(downloadFileFullPath);
        QString fileName(info.fileName());

        printToOutput(DownloadState::stateToString(DownloadState::STATE_COMPLETE));
        printToOutput(QString("----> ") + fileName);
        this->state = DownloadState::STATE_IDLE;

        printToStatusBar("[Download Successful] -> " + fileName);

        if (ui->autoRenameCheck->checkState() == Qt::CheckState::Checked) {
            renameDialog->setFileName(fileName);
            renameDialog->show();
        }

        //        if (ui->autoUploadCheck->checkState() == Qt::CheckState::Checked) {
        //            uploadToFtp(downloadFileName);
        //        }
    } else {
        printToOutput("###### Download Failed!!! #####\n");
        printToStatusBar("[Download failed] " + QString("").setNum(exitCode));
    }
    ui->startButton->setEnabled(true);
    ui->startButton->setText("Download");
}

void MainWindow::onUploadProgress(void){
    QString stdout = uploadProcess.readAllStandardOutput();
    QString stderr = uploadProcess.readAllStandardError();

    if (stdout.size() > 0) {
        qInfo() << "[upload:stdout]"  << stdout;
        printToStatusBar(stdout);
    }

    if (stderr.size() > 0) {
        qInfo() << "[upload:stderr]" << stderr;
        printToStatusBar(stderr);
    }
}

void MainWindow::onUploadFinish(int exitCode, QProcess::ExitStatus) {
    qInfo() << "upload finished " << exitCode;

    QFileInfo info(downloadFileFullPath);
    QString fileName(info.fileName());

    if (exitCode == 0) {
        printToStatusBar("[Upload Successful] ->" + fileName);
        printToOutput("Upload Successful!!! ftp folder =  " + ftpRemotePath);
        publishSubsonic();
    } else {
        printToStatusBar("[Upload failed] " + QString("").setNum(exitCode));
        printToOutput("Upload Failed!!! File Name: " + fileName);
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

void MainWindow::onFileRenameAccepted() {
    QString newFileName = renameDialog->getFileName();
    qInfo() << "file rename accepted: " << newFileName;

    QFileInfo oldFileInfo(downloadFileFullPath);
    QString oldFileName(oldFileInfo.fileName());

    bool hasNewName = newFileName != oldFileName;

    QString newFileFullPath = downloadFolder + QDir::separator() + newFileName;
    qInfo() << "newFileFullPath" << newFileFullPath;
    if (!hasNewName || QFile::rename(downloadFileFullPath, newFileFullPath)) {
        printToOutput("File renamed to: " + newFileName);
        this->downloadFileFullPath = newFileFullPath;

        if (ui->autoUploadCheck->checkState() == Qt::CheckState::Checked) {
            uploadToFtp(downloadFileFullPath);
        } else {
            qInfo() << "skip uploading to ftp.....";
        }
    } else {
        printToOutput("File renamed failed: " + newFileName);
    }
}

void MainWindow::onFileRenameRejected() {
    qInfo() << "file rename rejected!";
    if (ui->autoUploadCheck->checkState() == Qt::CheckState::Checked) {
        uploadToFtp(downloadFileFullPath);
    } else {
        qInfo() << "skip uploading to ftp.....";
    }
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

void MainWindow::publishSubsonic()
{
    if (ui->autoPublishCheck->checkState() == Qt::CheckState::Unchecked) {
        qInfo() <<"ignore publish to subsonic server...";
        printToOutput("ignore publish to subsonic server...");
        return;
    }

    qInfo() << "publish to subsonic server....";

    printToOutput("publishing to subsonic server....");

    // s(salt): 6 hex digits
    // t = md5(passwd + salt)
    QStringList arguments{
        "http://192.168.1.21:4040/rest/startScan?u=admin&t=1d20736c96f8b965488b23b3edee8b14&s=c19b2d&v=1.16.1&c=robinapp&f=json"
    };
    publishProcess.start("/usr/bin/curl", arguments);
}

void MainWindow::onPublishProgress()
{
    QString stdout = publishProcess.readAllStandardOutput();
    QString stderr = publishProcess.readAllStandardError();

    if (stdout.size() > 0) {
        qInfo() << stdout;
        printToStatusBar(stdout);
    }

    if (stderr.size() > 0) {
        qInfo() << stderr;
        printToStatusBar(stderr);
    }
}

void MainWindow::onPublishFinish(int exitCode, QProcess::ExitStatus exitStatus)
{
    qInfo() << "onPublishFinish() exitCode = " << exitCode << ", exitStatus = " << exitStatus;
    if (exitCode == 0) {
        printToOutput("Published to Subsonic server successfully!");
        printToStatusBar("Published to Subsonic server successfully!");
    } else {
        printToOutput("Publish to Subsonic server failed.");
        printToStatusBar("Publish to Subsonic server failed");
    }
}





