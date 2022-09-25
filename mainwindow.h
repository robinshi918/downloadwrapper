#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <downloadstate.h>
#include <renamedialog.h>
#include <settingsdialog.h>
#include <QSettings>
#include <settingmanager.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QProcess m_downloadProcess;
    QProcess m_uploadProcess;
    QProcess m_publishProcess;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    bool m_isSingleMusic = true;
    QString m_downloadFileFullPath;

    int m_state = DownloadState::STATE_IDLE;
    RenameDialog* m_renameDialog;
    SettingsDialog* m_settingDialog;
    SettingManager* m_setting;

private:
    void handleCancelButton();
    void handleStartButton();
    void handleTypeSelected();
    void handleUploadButton();
    void handleSettingsButton();

    void handleYtDlpCommandOutput(QString& stdout);
    void handleYoutubeDlCommandOutput(QString& stdout);

    void init();
    void connectSignals();
    void initUI();

    void downloadSingle(QString& url);
    void downloadPlayList(QString& url, unsigned int startPos, unsigned int endPos);
    void uploadToFtp(QString fileName);

    void showMessageBox(QString text);

    void printToOutput(QString str, bool printTimestamp = true);
    void printToStatusBar(QString str);

    void injectEnvironmentVar();
    QString getDownloadFolder();

    void publishSubsonic();

private slots:
    void onDownloadProgress(void);
    void onDownloadFinish(int exitCode, QProcess::ExitStatus exitStatus);

    void onUploadProgress(void);
    void onUploadFinish(int exitCode, QProcess::ExitStatus exitStatus);

    void autoUploadStateChanged(int);
    void onFocusChanged(QWidget* old, QWidget* newWidget);

    void onFileRenameAccepted();
    void onFileRenameRejected();

    void onPublishProgress();
    void onPublishFinish(int exitCode, QProcess::ExitStatus exitStatus);

    void onRemotePathComboBoxCurrentIndexChanged(int index);
    void onSettingDialogFinished();

};
#endif // MAINWINDOW_H
