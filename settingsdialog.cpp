#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <settingmanager.h>
#include <QFileDialog>
#include <QDir>
#include <QCryptographicHash>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    connectSetup();
}

SettingsDialog::~SettingsDialog()
{
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
}

void SettingsDialog::initUi()
{
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

void SettingsDialog::onOkButton()
{
    SettingManager& settings = SettingManager::getInstance();

    settings.setValue(SettingManager::KEY_FTP_SERVER, ui->ftpServerIP->text());
    settings.setValue(SettingManager::KEY_FTP_PORT, ui->ftpServerPort->text());
    settings.setValue(SettingManager::KEY_FTP_REMOTE_PATH, ui->ftpRemotePath->text());
    settings.setValue(SettingManager::KEY_FTP_USER, ui->ftpUser->text());
    settings.setValue(SettingManager::KEY_FTP_PASSWORD, ui->ftpPasswd->text());

    settings.setValue(SettingManager::KEY_SUBSONIC_SERVER, ui->subsonicServerIP->text());
    settings.setValue(SettingManager::KEY_SUBSONIC_PORT, ui->subsonicServerPort->text());
    settings.setValue(SettingManager::KEY_SUBSONIC_USER, ui->subsonicUser->text());
    settings.setValue(SettingManager::KEY_SUBSONIC_PASSWORD, md5(ui->subsonicPassword->text() + ui->subsonicSalt->text()));
    settings.setValue(SettingManager::KEY_SUBSONIC_SALT, ui->subsonicSalt->text());

    settings.setValue(SettingManager::KEY_DOWNLOAD_FOLDER_PATH, ui->downloadPath->text());
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

}
