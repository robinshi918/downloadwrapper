#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <settingmanager.h>
#include <QFileDialog>
#include <QDir>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    initUi();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}


void SettingsDialog::initUi()
{

    connect(ui->pathSelectButton, &QPushButton::released, this, &SettingsDialog::onSelectDownloadPath);

    SettingManager& settings = SettingManager::getInstance();

    ui->ftpServerIP->setText(settings.getValue(SettingManager::KEY_FTP_SERVER));
    ui->ftpServerPort->setText(settings.getValue(SettingManager::KEY_FTP_PORT));
    ui->ftpRemotePath->setText(settings.getValue(SettingManager::KEY_FTP_REMOTE_PATH));
    ui->ftpUser->setText(settings.getValue(SettingManager::KEY_FTP_USER));
    ui->ftpPasswd->setText(settings.getValue(SettingManager::KEY_FTP_PASSWORD));

    ui->subsonicServerIP->setText(settings.getValue(SettingManager::KEY_SUBSONIC_SERVER));
    ui->subsonicServerPort->setText(settings.getValue(SettingManager::KEY_SUBSONIC_PORT));
    ui->subsonicUser->setText(settings.getValue(SettingManager::KEY_SUBSONIC_USER));
    ui->subsonicPassword->setText(settings.getValue(SettingManager::KEY_SUBSONIC_PASSWORD));
    ui->subsonicSalt->setText(settings.getValue(SettingManager::KEY_SUBSONIC_SALT));

    ui->downloadPath->setText(settings.getValue(SettingManager::KEY_DOWNLOAD_FOLDER_PATH));
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
