#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QProcess>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();
    void show();

Q_SIGNALS:
    void okButtonClicked();
    void cancelButtonClicked();

private:
    Ui::SettingsDialog *ui;
    QProcess *m_ftpProcess;
    QProcess *m_directoryListProcess;

private:
    void initUi();
    void connectSetup();
    QString md5(QString password);
    void createFtpFolder(const QString &folderName);
    void loadFtpDirectories();
    QStringList parseFtpDirectoryList(const QString& output);

private slots:
    void onSelectDownloadPath();
    void onOkButton();
    void onCancelButton();
    void onRemotePathComboBoxCurrentIndexChanged(int index);
    void onCreateFolderButton();
    void onFtpProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onLoadDirectoriesButton();
    void onDirectoryListProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
};

#endif // SETTINGSDIALOG_H
