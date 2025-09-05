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

private:
    void initUi();
    void connectSetup();
    QString md5(QString password);
    void createFtpFolder(const QString &folderName);

private slots:
    void onSelectDownloadPath();
    void onOkButton();
    void onCancelButton();
    void onRemotePathComboBoxCurrentIndexChanged(int index);
    void onCreateFolderButton();
    void onFtpProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
};

#endif // SETTINGSDIALOG_H
