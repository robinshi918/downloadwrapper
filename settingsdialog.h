#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

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

private:
    void initUi();
    void connectSetup();
    QString md5(QString password);

private slots:
    void onSelectDownloadPath();
    void onOkButton();
    void onCancelButton();
    void onRemotePathComboBoxCurrentIndexChanged(int index);
};

#endif // SETTINGSDIALOG_H
