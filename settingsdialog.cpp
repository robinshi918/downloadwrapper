#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <settingmanager.h>
#include <QFileDialog>
#include <QDir>
#include <QCryptographicHash>
#include <QDebug>
#include <QMessageBox>
#include <QInputDialog>
#include "foldernamedialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog),
    m_ftpProcess(nullptr),
    m_directoryListProcess(nullptr)
{
    ui->setupUi(this);

    connectSetup();
}

SettingsDialog::~SettingsDialog()
{
    if (m_ftpProcess) {
        m_ftpProcess->kill();
        delete m_ftpProcess;
    }
    if (m_directoryListProcess) {
        m_directoryListProcess->kill();
        delete m_directoryListProcess;
    }
    delete ui;
}

void SettingsDialog::show()
{
    initUi();
    QDialog::show();
}

void SettingsDialog::connectSetup()
{
    connect(ui->pathSelectButton, &QPushButton::released, this, &SettingsDialog::onSelectDownloadPath);
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(onOkButton()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(onCancelButton()));
    connect(ui->remotePathComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onRemotePathComboBoxCurrentIndexChanged(int)));
    connect(ui->createFolderButton, &QPushButton::released, this, &SettingsDialog::onCreateFolderButton);
    connect(ui->loadDirectoriesButton, &QPushButton::released, this, &SettingsDialog::onLoadDirectoriesButton);
}

void SettingsDialog::initUi()
{
   SettingManager& settings = SettingManager::getInstance();

    ui->ftpServerIP->setText(settings.getValue(SettingManager::KEY_FTP_SERVER));
    ui->ftpServerPort->setText(settings.getValue(SettingManager::KEY_FTP_PORT));


    ui->ftpUser->setText(settings.getValue(SettingManager::KEY_FTP_USER));
    ui->ftpPasswd->setText(settings.getValue(SettingManager::KEY_FTP_PASSWORD));

    ui->subsonicServerIP->setText(settings.getValue(SettingManager::KEY_SUBSONIC_SERVER));
    ui->subsonicServerPort->setText(settings.getValue(SettingManager::KEY_SUBSONIC_PORT));
    ui->subsonicUser->setText(settings.getValue(SettingManager::KEY_SUBSONIC_USER));
    ui->subsonicPassword->setText(settings.getValue(SettingManager::KEY_SUBSONIC_PASSWORD));
    ui->subsonicSalt->setText(settings.getValue(SettingManager::KEY_SUBSONIC_SALT));

    ui->downloadPath->setText(settings.getValue(SettingManager::KEY_DOWNLOAD_FOLDER_PATH));

    ui->ftpRemotePath->setText(settings.getValue(SettingManager::KEY_FTP_REMOTE_PATH));
    int remotePathOption = settings.getValue(SettingManager::KEY_REMOTE_PATH_OPTION).toInt();
    qInfo() << "saved remote path option = " << remotePathOption;
    ui->remotePathComboBox->setCurrentIndex(remotePathOption);
    onRemotePathComboBoxCurrentIndexChanged(remotePathOption);
}

void SettingsDialog::onSelectDownloadPath()
{
    SettingManager setting = SettingManager::getInstance();
    QString oldPath = setting.getValue(SettingManager::KEY_DOWNLOAD_FOLDER_PATH);
    QString path = QFileDialog::getExistingDirectory(0,
                                                     ("Select Output Folder"),
                                                     oldPath);
    qInfo() << "selected path =" << path;
    if (!path.isEmpty()) {
        ui->downloadPath->setText(path);
        setting.setValue(SettingManager::KEY_DOWNLOAD_FOLDER_PATH, path);
    }
}

void SettingsDialog::onOkButton()
{
    SettingManager& settings = SettingManager::getInstance();

    settings.setValue(SettingManager::KEY_FTP_SERVER, ui->ftpServerIP->text());
    settings.setValue(SettingManager::KEY_FTP_PORT, ui->ftpServerPort->text());
    settings.setValue(SettingManager::KEY_FTP_REMOTE_PATH, ui->ftpRemotePath->text());
    settings.setValue(SettingManager::KEY_REMOTE_PATH_OPTION, QString::number(ui->remotePathComboBox->currentIndex()));

    settings.setValue(SettingManager::KEY_FTP_USER, ui->ftpUser->text());
    settings.setValue(SettingManager::KEY_FTP_PASSWORD, ui->ftpPasswd->text());

    settings.setValue(SettingManager::KEY_SUBSONIC_SERVER, ui->subsonicServerIP->text());
    settings.setValue(SettingManager::KEY_SUBSONIC_PORT, ui->subsonicServerPort->text());
    settings.setValue(SettingManager::KEY_SUBSONIC_USER, ui->subsonicUser->text());
    settings.setValue(SettingManager::KEY_SUBSONIC_PASSWORD, md5(ui->subsonicPassword->text() + ui->subsonicSalt->text()));
    settings.setValue(SettingManager::KEY_SUBSONIC_SALT, ui->subsonicSalt->text());

    settings.setValue(SettingManager::KEY_DOWNLOAD_FOLDER_PATH, ui->downloadPath->text());

    emit okButtonClicked();
}

QString SettingsDialog::md5(QString password)
{
    if (password.isEmpty()) return "";
    QString result = QString(QCryptographicHash::hash(QString("Hallo828c19b2d").toLocal8Bit(),QCryptographicHash::Md5).toHex());
    qInfo() << "md5 result of " + password << " = " << result;

    return result;
}

void SettingsDialog::onCancelButton()
{
    emit cancelButtonClicked();
}

void SettingsDialog::onRemotePathComboBoxCurrentIndexChanged(int /*index*/)
{
    QString currentText = ui->remotePathComboBox->currentText();
    
    if (currentText == "Other") {
        // Keep the current text in the line edit, don't change it
        qInfo() << "User selected Other, keeping current path";
        return;
    }
    
    // Set the selected directory path
    ui->ftpRemotePath->setText(currentText);
    qInfo() << "set to path:" << currentText;
}

void SettingsDialog::onCreateFolderButton()
{
    // Show dialog to get folder name
    FolderNameDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString folderName = dialog.getFolderName();
        if (!folderName.isEmpty()) {
            createFtpFolder(folderName);
        }
    }
}

void SettingsDialog::createFtpFolder(const QString &folderName)
{
    // Get FTP settings
    SettingManager& settings = SettingManager::getInstance();
    QString ftpPassword = settings.getValue(SettingManager::KEY_FTP_PASSWORD);
    QString ftpUser = settings.getValue(SettingManager::KEY_FTP_USER);
    QString ftpServer = settings.getValue(SettingManager::KEY_FTP_SERVER);
    QString ftpRemotePath = settings.getValue(SettingManager::KEY_FTP_REMOTE_PATH);
    QString ftpPort = settings.getValue(SettingManager::KEY_FTP_PORT);
    
    // Create the full path for the new folder
    QString fullPath = ftpRemotePath + "/" + folderName;
    
    // Disable the button while creating folder
    ui->createFolderButton->setEnabled(false);
    ui->createFolderButton->setText("Creating...");
    
    // Create FTP process to make directory
    if (m_ftpProcess) {
        m_ftpProcess->kill();
        delete m_ftpProcess;
    }
    
    m_ftpProcess = new QProcess(this);
    connect(m_ftpProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &SettingsDialog::onFtpProcessFinished);
    
    // Use curl to create directory via FTP
    QStringList arguments{
        "-Q", "MKD " + fullPath,
        "-u", ftpUser + ":" + ftpPassword,
        "ftp://" + ftpServer + ":" + ftpPort + "/"
    };
    
    qInfo() << "Creating FTP folder:" << fullPath;
    m_ftpProcess->start("/usr/bin/curl", arguments);
}

void SettingsDialog::onFtpProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    // Re-enable the button
    ui->createFolderButton->setEnabled(true);
    ui->createFolderButton->setText("Create New Folder");
    
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        QMessageBox::information(this, "Success", "Folder created successfully on FTP server!");
        qInfo() << "FTP folder created successfully";
    } else {
        QString errorMessage = "Failed to create folder on FTP server.\n";
        QString errorOutput;
        
        if (m_ftpProcess) {
            errorOutput = m_ftpProcess->readAllStandardError();
            QString standardOutput = m_ftpProcess->readAllStandardOutput();
            
            // Check for specific error conditions
            if (errorOutput.contains("550") || standardOutput.contains("550")) {
                errorMessage = "Folder already exists on FTP server.\nPlease choose a different name.";
            } else if (errorOutput.contains("QUOT command failed")) {
                errorMessage = "FTP server rejected the command.\nThis might be due to:\n"
                             "- Folder already exists\n"
                             "- Insufficient permissions\n"
                             "- Invalid folder name";
            } else if (!errorOutput.isEmpty()) {
                errorMessage += "Error: " + errorOutput;
            } else {
                errorMessage += "Unknown error occurred (exit code: " + QString::number(exitCode) + ")";
            }
        } else {
            errorMessage += "Unknown error occurred (exit code: " + QString::number(exitCode) + ")";
        }
        
        QMessageBox::warning(this, "Error", errorMessage);
        qWarning() << "Failed to create FTP folder, exit code:" << exitCode;
        if (!errorOutput.isEmpty()) {
            qWarning() << "Error output:" << errorOutput;
        }
    }
    
    // Clean up process
    if (m_ftpProcess) {
        delete m_ftpProcess;
        m_ftpProcess = nullptr;
    }
}

void SettingsDialog::onLoadDirectoriesButton()
{
    loadFtpDirectories();
}

void SettingsDialog::loadFtpDirectories()
{
    // Get FTP settings
    SettingManager& settings = SettingManager::getInstance();
    QString ftpPassword = settings.getValue(SettingManager::KEY_FTP_PASSWORD);
    QString ftpUser = settings.getValue(SettingManager::KEY_FTP_USER);
    QString ftpServer = settings.getValue(SettingManager::KEY_FTP_SERVER);
    QString ftpPort = settings.getValue(SettingManager::KEY_FTP_PORT);
    
    // Disable the button while loading directories
    ui->loadDirectoriesButton->setEnabled(false);
    ui->loadDirectoriesButton->setText("Loading...");
    
    // Create FTP process to list directories
    if (m_directoryListProcess) {
        m_directoryListProcess->kill();
        delete m_directoryListProcess;
    }
    
    m_directoryListProcess = new QProcess(this);
    connect(m_directoryListProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &SettingsDialog::onDirectoryListProcessFinished);
    
    // Use curl to list directories in the music folder
    QStringList arguments{
        "-l",
        "-u", ftpUser + ":" + ftpPassword,
        "ftp://" + ftpServer + ":" + ftpPort + "/music/"
    };
    
    qInfo() << "Loading FTP directories from music folder";
    m_directoryListProcess->start("/usr/bin/curl", arguments);
}

void SettingsDialog::onDirectoryListProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    // Re-enable the button
    ui->loadDirectoriesButton->setEnabled(true);
    ui->loadDirectoriesButton->setText("Load Directories");
    
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        QString output = m_directoryListProcess->readAllStandardOutput();
        QStringList directories = parseFtpDirectoryList(output);
        
        if (!directories.isEmpty()) {
            // Clear existing items and add new ones
            ui->remotePathComboBox->clear();
            
            // Add the loaded directories
            for (const QString& dir : directories) {
                ui->remotePathComboBox->addItem(dir);
            }
            
            // Add "Other" option at the end
            ui->remotePathComboBox->addItem("Other");
            
            QMessageBox::information(this, "Success", 
                QString("Loaded %1 directories from FTP server.").arg(directories.size()));
            qInfo() << "FTP directories loaded successfully:" << directories;
        } else {
            QMessageBox::warning(this, "Warning", "No directories found in the music folder.");
        }
    } else {
        QString errorMessage = "Failed to load directories from FTP server.\n";
        if (m_directoryListProcess) {
            QString errorOutput = m_directoryListProcess->readAllStandardError();
            if (!errorOutput.isEmpty()) {
                errorMessage += "Error: " + errorOutput;
            }
        }
        QMessageBox::warning(this, "Error", errorMessage);
        qWarning() << "Failed to load FTP directories, exit code:" << exitCode;
    }
    
    // Clean up process
    if (m_directoryListProcess) {
        delete m_directoryListProcess;
        m_directoryListProcess = nullptr;
    }
}

QStringList SettingsDialog::parseFtpDirectoryList(const QString& output)
{
    QStringList directories;
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    
    for (const QString& line : lines) {
        QString trimmedLine = line.trimmed();
        // Skip empty lines, progress lines, and files (we only want directories)
        if (!trimmedLine.isEmpty() && 
            !trimmedLine.startsWith("%") && 
            !trimmedLine.startsWith("Total") &&
            !trimmedLine.startsWith("Dload") &&
            !trimmedLine.startsWith("Speed") &&
            !trimmedLine.startsWith("--:--") &&
            !trimmedLine.contains('.') &&
            !trimmedLine.endsWith(".sh")) {
            // Add the directory with music/ prefix
            directories.append("music/" + trimmedLine);
        }
    }
    
    return directories;
}
