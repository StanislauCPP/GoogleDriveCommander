#include "UploadFileInGoogleDriveDialog.h"
#include "ui_UploadFileInGoogleDriveDialog.h"

UploadFileInGoogleDriveDialog::UploadFileInGoogleDriveDialog(QWidget *parent) :
    QDialog(parent)
  , ui(new Ui::UploadFileInGoogleDriveDialog)
  , doubleClickFlag(true)
{
    ui->setupUi(this);

    setModal(true);
    setWindowTitle("Upload file in Google Drive");

    fileSystemModel = new QFileSystemModel(this);
    fileSystemModel->setFilter(QDir::QDir::AllEntries);
    fileSystemModel->setRootPath(QDir::rootPath());

    ui->tableViewFileSystem->setModel(fileSystemModel);
}

UploadFileInGoogleDriveDialog::~UploadFileInGoogleDriveDialog()
{
    delete ui;
}

/*Slots begin*/
void UploadFileInGoogleDriveDialog::tableViewFileSystemDoubleClicked(QModelIndex index)
{
    if(index.data().toString() == "..")
    {
        QString path = fileSystemModel->fileInfo(index.parent()).absolutePath();
        ui->tableViewFileSystem->setRootIndex(fileSystemModel->index(path));
    }
    else if(index.data().toString() == ".")
    {
        ui->tableViewFileSystem->setRootIndex(fileSystemModel->index(""));
    }
    else if(fileSystemModel->fileInfo(index).isDir())
    {
        QString path = fileSystemModel->fileInfo(index).absoluteFilePath();
        ui->tableViewFileSystem->setRootIndex(fileSystemModel->index(path));
    }

    doubleClickFlag = true;
}

void UploadFileInGoogleDriveDialog::tableViewFileSystemClicked(QModelIndex)
{
    doubleClickFlag = false;
}

void UploadFileInGoogleDriveDialog::pushButtonUploadClicked()
{
    if(!doubleClickFlag)
    {
        QFileInfo passableFile;
        passableFile = fileSystemModel->fileInfo(ui->tableViewFileSystem->currentIndex());

        emit passFileInfoFromFileSystemModel(passableFile);

        ui->pushButtonUpload->setEnabled(false);
    }
}

void UploadFileInGoogleDriveDialog::endUploadData()
{
    ui->pushButtonUpload->setEnabled(true);
}

/*Slots end*/
