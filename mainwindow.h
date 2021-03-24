#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    bool isSingleMusic = true;

private:
    void handleCancelButton();
    void handleStartButton();
    void handleTypeSelected();

    void init();
    void downloadSingle(QString& url);
    void downloadPlayList(QString& url, unsigned int startPos, unsigned int endPos);
};
#endif // MAINWINDOW_H
