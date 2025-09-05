#ifndef FOLDERNAMEDIALOG_H
#define FOLDERNAMEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>

class FolderNameDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FolderNameDialog(QWidget *parent = nullptr);
    ~FolderNameDialog();
    
    QString getFolderName() const;

private:
    QLineEdit *folderNameEdit;
    QPushButton *okButton;
    QPushButton *cancelButton;
    
private slots:
    void onOkClicked();
    void onCancelClicked();
};

#endif // FOLDERNAMEDIALOG_H
