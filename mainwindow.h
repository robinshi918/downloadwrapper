#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QProcess downloadProcess;
    QProcess uploadProcess;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    bool isSingleMusic = true;
    QString downloadFileName;

private:
    void handleCancelButton();
    void handleStartButton();
    void handleTypeSelected();
    void handleUploadButton();

    void init();

    void downloadSingle(QString& url);
    void downloadPlayList(QString& url, unsigned int startPos, unsigned int endPos);
    void uploadToFtp(QString fileName);

    void showMessageBox(QString text);

    void printToOutput(QString str, bool replaceLastLine = false);

    void injectEnvironmentVar();

private slots:
    void readDownloadProcessOutput(void);
    void downloadCommandFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void readUploadProcessOutput(void);
    void uploadCommandFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void autoUploadStateChanged(int);
    void onFocusChanged(QWidget* old, QWidget* newWidget);

};
#endif // MAINWINDOW_H
