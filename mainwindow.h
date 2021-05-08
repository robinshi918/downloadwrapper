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
    QProcess p;

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

    void addToOutput(QString str, bool replaceLastLine = false);


    void injectEnvironmentVar();

private slots:
    void readSubProcess(void);
    void commandFinished(int exitCode, QProcess::ExitStatus exitStatus);

};
#endif // MAINWINDOW_H
