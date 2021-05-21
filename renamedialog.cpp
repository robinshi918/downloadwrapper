#include "renamedialog.h"
#include "ui_renamedialog.h"

RenameDialog::RenameDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RenameDialog)
{
    ui->setupUi(this);
}

RenameDialog::~RenameDialog()
{
    delete ui;
}


void RenameDialog::setFileName(QString fileName) {
    this->fileName = fileName;
    this->ui->fileNameEdit->setText(fileName);
}

QString RenameDialog::getFileName() {
    return this->ui->fileNameEdit->text();
}


void RenameDialog::show() {
    this->ui->fileNameEdit->setFocus();
    QDialog::show();
}
