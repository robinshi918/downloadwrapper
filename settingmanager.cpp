#include "settingmanager.h"

#include <QApplication>
#include <QDir>

#define DEFAULT_FTP_USER_NAME QString("pi")
#define DEFAULT_FTP_PASSWORD QString("hallo")
#define DEFAULT_FTP_SERVER QString("192.168.1.21")
#define DEFAULT_FTP_REMOTE_PATH QString("upload")
#define DEFAULT_FTP_PORT QString("21")
#define DEFAULT_SUBSONIC_SERVER QString("192.168.1.21")
#define DEFAULT_SUBSONIC_PORT QString("4040")
#define DEFAULT_SUBSONIC_USER QString("admin")
#define DEFAULT_SUBSONIC_SALT QString("c19b2d")

// ftp
const QString SettingManager::KEY_FTP_SERVER = "ftp_server";
const QString SettingManager::KEY_FTP_PORT ="ftp_port";
const QString SettingManager::KEY_FTP_REMOTE_PATH="ftp_remote_path";
const QString SettingManager::KEY_FTP_USER="ftp_user";
const QString SettingManager::KEY_FTP_PASSWORD="ftp_password";
// subsonic
const QString SettingManager::KEY_SUBSONIC_SERVER="subsonic_server";
const QString SettingManager::KEY_SUBSONIC_PORT="subsonic_port";
const QString SettingManager::KEY_SUBSONIC_USER="subsonic_user";
const QString SettingManager::KEY_SUBSONIC_PASSWORD="subsonic_password";
const QString SettingManager::KEY_SUBSONIC_SALT="subsonic_salt";
// local download path
const QString SettingManager::KEY_DOWNLOAD_FOLDER_PATH="download_path";



SettingManager::SettingManager()
{
    if (settings == NULL) {
        settings = new QSettings("MySettings", QSettings::NativeFormat);
    }
}

QString SettingManager::getValue(QString key)
{
    if (key == KEY_FTP_SERVER) {
        return settings->value(key, DEFAULT_FTP_SERVER).toString();
    } else if (key == KEY_FTP_PORT) {
        return settings->value(key, DEFAULT_FTP_PORT).toString();
    } else if (key == KEY_FTP_REMOTE_PATH) {
        return settings->value(key, DEFAULT_FTP_REMOTE_PATH).toString();
    } else if (key == KEY_FTP_USER) {
        return settings->value(key, DEFAULT_FTP_USER_NAME).toString();
    } else if (key == KEY_FTP_PASSWORD) {
        return settings->value(key, DEFAULT_FTP_PASSWORD).toString();
    } else if (key == KEY_SUBSONIC_SERVER) {
        return settings->value(key, DEFAULT_SUBSONIC_SERVER).toString();
    } else if (key == KEY_SUBSONIC_PORT) {
        return settings->value(key, DEFAULT_SUBSONIC_PORT).toString();
    } else if (key == KEY_SUBSONIC_USER) {
        return settings->value(key, DEFAULT_SUBSONIC_USER).toString();
    } else if (key == KEY_SUBSONIC_PASSWORD) {
        return settings->value(key, "").toString();
    } else if (key == KEY_SUBSONIC_SALT) {
        return settings->value(key, DEFAULT_SUBSONIC_SALT).toString();
    } else if (key == KEY_DOWNLOAD_FOLDER_PATH) {
        return settings->value(key, QDir::home().path() +  QDir::separator() + "Desktop/mp3/download/").toString();
    }else {
        return "value_not_exist for " + key;
    }
}

void SettingManager::setValue(QString key, QString value)
{
    settings->setValue(key, value);
}
