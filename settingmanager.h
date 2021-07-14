#ifndef SETTINGMANAGER_H
#define SETTINGMANAGER_H

#include <QSettings>

class SettingManager
{

public:
    static SettingManager& getInstance()
    {
        static SettingManager    instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }

    QString getValue(QString key);
    void setValue(QString key, QString value);

private:
    SettingManager();



public:
    static const QString KEY_FTP_SERVER;
    static const QString KEY_FTP_PORT;
    static const QString KEY_FTP_REMOTE_PATH;
    static const QString KEY_FTP_USER;
    static const QString KEY_FTP_PASSWORD;
    static const QString KEY_REMOTE_PATH_OPTION;

    static const QString KEY_SUBSONIC_SERVER;
    static const QString KEY_SUBSONIC_PORT;
    static const QString KEY_SUBSONIC_USER;
    static const QString KEY_SUBSONIC_PASSWORD;
    static const QString KEY_SUBSONIC_SALT;

    static const QString KEY_DOWNLOAD_FOLDER_PATH;

private:
    QSettings* settings;
};

#endif // SETTINGMANAGER_H
