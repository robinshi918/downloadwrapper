#ifndef RENAMEDIALOG_H
#define RENAMEDIALOG_H

#include <QDialog>

namespace Ui {
class RenameDialog;
}

class RenameDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RenameDialog(QWidget *parent = 0);
    ~RenameDialog();

    void setFileName(QString fileName);
    QString getFileName();
    void show();


private:
    Ui::RenameDialog *ui;
    QString fileName;
};

#endif // RENAMEDIALOG_H
