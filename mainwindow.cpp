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
#include <QThread>
#include <downloadstate.h>
#include <renamedialog.h>
#include <settingsdialog.h>
#include <settingmanager.h>


#define DOWNLOAD_PATTERN QString("%(title)s.%(ext)s")

// check if which download command is used
#define USE_YT_DLP

#ifdef USE_YT_DLP
#define YT_DOWNLOAD_CMD "/opt/homebrew/bin/yt-dlp"
#else
#define YT_DOWNLOAD_CMD "/usr/local/bin/youtube-dl"
#endif


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->m_renameDialog = new RenameDialog;
    this->m_settingDialog = new SettingsDialog;
    this->m_setting = &SettingManager::getInstance();

    init();
}

void MainWindow::onFocusChanged(QWidget*, QWidget*)
{
    QClipboard* clipboard = QApplication::clipboard();
    QString cilpBoardText = clipboard->text();
    if(cilpBoardText.toLower().startsWith("https://")) {
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
    m_isSingleMusic = true;
    ui->playListOptionGroup->setVisible(false);
    ui->startEdit->setText("");
    ui->endEdit->setText("");

    connectSignals();
    injectEnvironmentVar();
    initUI();
}

void MainWindow::connectSignals() {

    connect(ui->cancelButton, &QPushButton::released, this, &MainWindow::handleCancelButton);
    connect(ui->startButton, &QPushButton::released, this, &MainWindow::handleStartButton);
    connect(ui->uploadButton, &QPushButton::released, this, &MainWindow::handleUploadButton);
    connect(ui->settingsButton, &QPushButton::released, this, &MainWindow::handleSettingsButton);
    connect(ui->singleMusicRadioButton, &QRadioButton::clicked, this, &MainWindow::handleTypeSelected);
    connect(ui->playlistRadioButton, &QRadioButton::clicked, this, &MainWindow::handleTypeSelected);
    connect(ui->autoUploadCheck, &QCheckBox::stateChanged, this, &MainWindow::autoUploadStateChanged);
    connect(qApp, SIGNAL(focusChanged(QWidget*, QWidget*)), this, SLOT(onFocusChanged(QWidget*, QWidget*)));

    // download process
    connect(&m_downloadProcess, SIGNAL(readyReadStandardOutput()),
            this, SLOT(onDownloadProgress()));
    connect(&m_downloadProcess, SIGNAL(readyReadStandardError()),
            this, SLOT(onDownloadProgress()));
    connect(&m_downloadProcess, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(onDownloadFinish(int, QProcess::ExitStatus)));

    // upload process
    connect(&m_uploadProcess, SIGNAL(readyReadStandardOutput()),
            this, SLOT(onUploadProgress()));
    connect(&m_uploadProcess, SIGNAL(readyReadStandardError()),
            this, SLOT(onUploadProgress()));
    connect(&m_uploadProcess, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(onUploadFinish(int, QProcess::ExitStatus)));

    // publish process
    connect(&m_publishProcess, SIGNAL(readyReadStandardError()),
            this, SLOT(onPublishProgress()));
    connect(&m_publishProcess, SIGNAL(readyReadStandardOutput()),
            this, SLOT(onPublishProgress()));
    connect(&m_publishProcess, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(onPublishFinish(int, QProcess::ExitStatus)));

    connect(m_renameDialog, SIGNAL(accepted()), this, SLOT(onFileRenameAccepted()));
    connect(m_renameDialog, SIGNAL(rejected()), this, SLOT(onFileRenameRejected()));

    // user selection
    connect(ui->remoteUserComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onRemotePathComboBoxCurrentIndexChanged(int)));

    // refresh user selection when setting dialog closes
    connect(m_settingDialog, SIGNAL(finished(int)), this, SLOT(onSettingDialogFinished()));
}

void MainWindow::initUI() {

    qInfo() << "intUI";

    if (ui->autoUploadCheck->checkState() == Qt::CheckState::Checked) {
        ui->uploadButton->hide();
    } else {
        ui->uploadButton->show();
    }

    SettingManager& settings = SettingManager::getInstance();
    int remoteUser = settings.getValue(SettingManager::KEY_REMOTE_PATH_OPTION).toInt();
    ui->remoteUserComboBox->setCurrentIndex(remoteUser);
//    onRemotePathComboBoxCurrentIndexChanged(remoteUser);
}

void MainWindow::handleCancelButton()
{
    ui->logEdit->clear();
}


void MainWindow::handleSettingsButton()
{
    this->m_settingDialog->show();
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

    if(!ui->logEdit->toPlainText().isEmpty()) {
        printToOutput("\n", false);
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
        this->m_isSingleMusic = false;
        ui->playListOptionGroup->setVisible(true);
    }
}

void MainWindow::handleUploadButton()
{
    //list a folder
    //curl -l -u pi:hallo ftp://192.168.1.21:21/music/mp3_yuan

    uploadToFtp(m_downloadFileFullPath);
    ui->uploadButton->setEnabled(false);
    ui->uploadButton->setText("Uploading");
}

void MainWindow::uploadToFtp(QString fileName) {

    if (fileName.isEmpty()) return;

    qInfo() << "uploading" << fileName;

    printToOutput("Uploading " + fileName);

    // read ftp parameters
    QString ftpPassword = m_setting->getValue(SettingManager::KEY_FTP_PASSWORD);
    QString ftpUser = m_setting->getValue(SettingManager::KEY_FTP_USER);
    QString ftpServer = m_setting->getValue(SettingManager::KEY_FTP_SERVER);
    QString ftpRemotePath = m_setting->getValue(SettingManager::KEY_FTP_REMOTE_PATH);
    QString ftpPort = m_setting->getValue(SettingManager::KEY_FTP_PORT);

    QStringList arguments{"-T", fileName,
                "-g",
                "-u", ftpUser + ":" + ftpPassword,
                "ftp://" + ftpServer + ":" + ftpPort + "/" + ftpRemotePath + "/"};
    m_uploadProcess.start("/usr/bin/curl", arguments);
}

/**
 * TODO not completed yet
 *
 * @brief MainWindow::downloadPlayList
 * @param url
 * @param startPos
 * @param endPos
 */
void MainWindow::downloadPlayList(QString &url, unsigned int startPos, unsigned int endPos)
{
    if (startPos > endPos) {
        showMessageBox("Playlist start position cannot be larger than end position.");
        return;
    }

    QStringList arguments{"-icw", "-x","--extract-audio",
                          "--playlist-start",
                          QString::number(startPos),
                                  "--playlist-end",
                                  QString::number(endPos) ,
                                  "--audio-format", "mp3",
                                  "--output",getDownloadFolder() + QDir::separator() + DOWNLOAD_PATTERN,
                                  url};

    m_downloadProcess.start(YT_DOWNLOAD_CMD, arguments);
    ui->startButton->setEnabled(false);
    ui->startButton->setText("Downloading");
}

void MainWindow::injectEnvironmentVar()
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PATH", env.value("PATH") + ":/usr/local/opt/openjdk/bin:/opt/homebrew/bin:/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/share/dotnet:~/.dotnet/tools:/Library/Apple/usr/bin:/Users/robinshi/Library/Android/sdk/platform-tools/");
    m_downloadProcess.setProcessEnvironment(env);
    m_uploadProcess.setProcessEnvironment(env);
}

void MainWindow::downloadSingle(QString &url)
{
    m_downloadFileFullPath = "";
    QStringList arguments{"-icw", "--extract-audio",  "--audio-format", "mp3", "--output", getDownloadFolder() + DOWNLOAD_PATTERN, url};

    printToOutput("Starting.....");
    m_downloadProcess.start(YT_DOWNLOAD_CMD, arguments);
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
        QString formattedTime = date.toString("MM.dd hh:mm:ss");
        ui->logEdit->appendPlainText(formattedTime + "   " + str);
    } else {
        ui->logEdit->appendPlainText(str);
    }
}

void MainWindow::printToStatusBar(QString str)
{
    ui->statusbar->showMessage(str);
}


void MainWindow::handleYtDlpCommandOutput(QString& stdout) {
    int newState = DownloadState::STATE_NONE;


    if (stdout.contains("Deleting original file")) {
        return;
    } else if (stdout.contains("[ExtractAudio] Destination:")) {
        m_downloadFileFullPath = stdout.mid(QString("[ExtractAudio] Destination: ").size());
        m_downloadFileFullPath = m_downloadFileFullPath.mid(0, m_downloadFileFullPath.size() - 1);
        qInfo() << "downloaded file is:" << m_downloadFileFullPath;
        newState = DownloadState::STATE_CONVERTING;
    } else if (stdout.contains("[download]")) {
        newState = DownloadState::STATE_DOWNLOADING;
    } else if (stdout.contains("[youtube] ")) {
        newState = DownloadState::STATE_PREPARING;
    }

    if (this->m_state != newState) {
        this->m_state = newState;
        printToOutput(DownloadState::stateToString(newState));
    }

    //    printToOutput(stdout);
}

void MainWindow::handleYoutubeDlCommandOutput(QString& stdout) {
    int newState = DownloadState::STATE_NONE;

    if (stdout.contains("Deleting original file")) {
        return;
    } else if (stdout.contains("[ExtractAudio] Destination: ")) {
        m_downloadFileFullPath = stdout.mid(QString("[ExtractAudio] Destination: ").size());
        m_downloadFileFullPath = m_downloadFileFullPath.mid(0, m_downloadFileFullPath.size() - 1);
        qInfo() << "downloaded file is:" << m_downloadFileFullPath;
        newState = DownloadState::STATE_CONVERTING;
    } else if (stdout.contains("[ffmpeg]")) {
        newState = DownloadState::STATE_CONVERTING;
    } else if (stdout.contains("[download]")) {
        newState = DownloadState::STATE_DOWNLOADING;
    } else if (stdout.contains("[youtube] ")) {
        newState = DownloadState::STATE_PREPARING;
    }

    if (this->m_state != newState) {
        this->m_state = newState;
        printToOutput(DownloadState::stateToString(newState));
    }
}

/**
 * Receive and parse command output
 * @brief MainWindow::onDownloadProgress
 */
void MainWindow::onDownloadProgress() {
    QString stdout = m_downloadProcess.readAllStandardOutput();
    QString stderr = m_downloadProcess.readAllStandardError();

    if (stdout.size() > 0) {
        qInfo() << stdout;
        // always print to status bar
        printToStatusBar(stdout);


#ifdef USE_YT_DLP
        handleYtDlpCommandOutput(stdout);
#else
        handleYoutubeDlCommandOutput((stdout);
        #endif


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
        QFileInfo info(m_downloadFileFullPath);
        QString fileName(info.fileName());

        printToOutput(DownloadState::stateToString(DownloadState::STATE_COMPLETE));
        printToOutput(QString("----> ") + fileName);
        this->m_state = DownloadState::STATE_IDLE;

        printToStatusBar("[Download Successful] -> " + fileName);

        if (ui->autoRenameCheck->checkState() == Qt::CheckState::Checked) {
            m_renameDialog->setFileName(fileName);
            m_renameDialog->show();
        }
    } else {
        printToOutput("###### Download Failed!!! #####\n");
        printToStatusBar("[Download failed] " + QString("").setNum(exitCode));
    }
    ui->startButton->setEnabled(true);
    ui->startButton->setText("Download");
}

void MainWindow::onUploadProgress(void){
    QString stdout = m_uploadProcess.readAllStandardOutput();
    QString stderr = m_uploadProcess.readAllStandardError();

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

    QFileInfo info(m_downloadFileFullPath);
    QString fileName(info.fileName());

    if (exitCode == 0) {
        printToStatusBar("[Upload Successful] ->" + fileName);
        printToOutput("Upload Successful!!! ftp folder =  " +
                      m_setting->getValue(SettingManager::KEY_FTP_REMOTE_PATH)
                      + "/" + fileName);
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

QString MainWindow::getDownloadFolder()
{
    return m_setting->getValue(SettingManager::KEY_DOWNLOAD_FOLDER_PATH) + QDir::separator();
}

void MainWindow::onFileRenameAccepted() {
    QString newFileName = m_renameDialog->getFileName();
    qInfo() << "file rename accepted: " << newFileName;

    QFileInfo oldFileInfo(m_downloadFileFullPath);
    QString oldFileName(oldFileInfo.fileName());

    bool hasNewName = newFileName != oldFileName;

    QString newFileFullPath = getDownloadFolder() + QDir::separator() + newFileName;
    qInfo() << "newFileFullPath" << newFileFullPath;
    qInfo() << "downloadedFullPath" << m_downloadFileFullPath;
    if (!hasNewName || QFile::rename(m_downloadFileFullPath, newFileFullPath)) {
        printToOutput("File renamed to: " + newFileName);
        this->m_downloadFileFullPath = newFileFullPath;

        if (ui->autoUploadCheck->checkState() == Qt::CheckState::Checked) {
            uploadToFtp(m_downloadFileFullPath);
        } else {
            qInfo() << "skip uploading to ftp.....";
        }
    } else {
        printToOutput("File renamed failed: " + newFileName);
    }
}

void MainWindow::onFileRenameRejected() {
    qInfo() << "file rename rejected!";
    // if user click cancel button in rename dialog, do nothing.
    printToOutput("Task Aborted!!");
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

    QStringList arg{
        "http://" + m_setting->getValue(SettingManager::KEY_SUBSONIC_SERVER)
                + ":" + m_setting->getValue(SettingManager::KEY_SUBSONIC_PORT)
                + "/rest/startScan?u="
                + m_setting->getValue(SettingManager::KEY_SUBSONIC_USER)
                + "&t=" + m_setting->getValue(SettingManager::KEY_SUBSONIC_PASSWORD)
                + "&s=" + m_setting->getValue(SettingManager::KEY_SUBSONIC_SALT)
                + "&v=1.16.1&c=robinapp&f=json"
    };
    qInfo() << "subsonic param = " << arg.at(0);

    //    QStringList arguments{
    //        "http://192.168.1.21:4040/rest/startScan?u=admin&t=1d20736c96f8b965488b23b3edee8b14&s=c19b2d&v=1.16.1&c=robinapp&f=json"
    //    };
    m_publishProcess.start("/usr/bin/curl", arg);
}

void MainWindow::onPublishProgress()
{
    QString stdout = m_publishProcess.readAllStandardOutput();
    QString stderr = m_publishProcess.readAllStandardError();

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

void MainWindow::onRemotePathComboBoxCurrentIndexChanged(int index)
{
    SettingManager& settings = SettingManager::getInstance();

    settings.setValue(SettingManager::KEY_REMOTE_PATH_OPTION, QString::number(index));

    switch (index) {
        case 0:
            settings.setValue(SettingManager::KEY_FTP_REMOTE_PATH, "music/mp3_Dora");
            qInfo() << "set to Dora path";
            printToOutput("change to Dora");
            break;
        case 1:
            settings.setValue(SettingManager::KEY_FTP_REMOTE_PATH, "music/mp3_yuan");
            qInfo() << "set to Yuan path";
            printToOutput("change to Yuan");
            break;
        case 2:
            settings.setValue(SettingManager::KEY_FTP_REMOTE_PATH, "music/JulieSong");
            qInfo() << "set to Julie path";
            printToOutput("change to Julie");
            break;
        default:
            qInfo() << "set to other paths";
            printToOutput("change to Other person, Please go to Settings to set exact remote path!!");
            break;
    }

}

void MainWindow::onSettingDialogFinished() {

    QThread::msleep(400);

    SettingManager& settings = SettingManager::getInstance();
    int remoteUser = settings.getValue(SettingManager::KEY_REMOTE_PATH_OPTION).toInt();
    qInfo() << "setting dialog closed" << remoteUser;
    ui->remoteUserComboBox->setCurrentIndex(remoteUser);

}





