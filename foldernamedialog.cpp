#include "foldernamedialog.h"
#include <QMessageBox>

FolderNameDialog::FolderNameDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Create New Folder");
    setModal(true);
    setFixedSize(300, 120);
    
    // Create layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Create label
    QLabel *label = new QLabel("Enter folder name:", this);
    mainLayout->addWidget(label);
    
    // Create line edit
    folderNameEdit = new QLineEdit(this);
    folderNameEdit->setPlaceholderText("Folder name...");
    mainLayout->addWidget(folderNameEdit);
    
    // Create button layout
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    okButton = new QPushButton("OK", this);
    cancelButton = new QPushButton("Cancel", this);
    
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(okButton, &QPushButton::clicked, this, &FolderNameDialog::onOkClicked);
    connect(cancelButton, &QPushButton::clicked, this, &FolderNameDialog::onCancelClicked);
    
    // Set focus to line edit
    folderNameEdit->setFocus();
}

FolderNameDialog::~FolderNameDialog()
{
}

QString FolderNameDialog::getFolderName() const
{
    return folderNameEdit->text().trimmed();
}

void FolderNameDialog::onOkClicked()
{
    QString folderName = getFolderName();
    
    if (folderName.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Please enter a folder name.");
        return;
    }
    
    // Check for invalid characters
    if (folderName.contains('/') || folderName.contains('\\') || 
        folderName.contains(':') || folderName.contains('*') || 
        folderName.contains('?') || folderName.contains('"') || 
        folderName.contains('<') || folderName.contains('>') || 
        folderName.contains('|')) {
        QMessageBox::warning(this, "Invalid Input", 
                           "Folder name contains invalid characters.\n"
                           "Please avoid: / \\ : * ? \" < > |");
        return;
    }
    
    accept();
}

void FolderNameDialog::onCancelClicked()
{
    reject();
}
