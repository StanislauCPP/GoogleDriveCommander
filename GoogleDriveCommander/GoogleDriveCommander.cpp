#include "GoogleDriveCommander.h"
#include "ui_GoogleDriveCommander.h"

GoogleDriveCommander::GoogleDriveCommander(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::GoogleDriveCommander)
{
    ui->setupUi(this);

    googleDriveItemModel = new GoogleClasses::GoogleDriveItemModel(this);

    ui->tableView->setModel(googleDriveItemModel);
    ui->importProgressBar->hide();
    ui->importProgressBar->setFormat("%v bytes");
    ui->importProgressBar->setValue(0);

    newGoogleDriveItemDialog        = new NewGoogleDriveItemDialog(this);
    uploadFileInGoogleDriveDialog   = new UploadFileInGoogleDriveDialog(this);
    timerUploading                  = new QTimer(this);

    connect(newGoogleDriveItemDialog,       SIGNAL(passNewItemNameAndType(QString,QString)),    googleDriveItemModel,           SLOT(createItem(QString,QString)));
    connect(googleDriveItemModel,           SIGNAL(newGoogleDriveItemCreated()),                newGoogleDriveItemDialog,       SLOT(closeNewGoogleDriveItemDialog()));
    connect(uploadFileInGoogleDriveDialog,  SIGNAL(passFileInfoFromFileSystemModel(QFileInfo)), googleDriveItemModel,           SLOT(fileInfoPassed(QFileInfo)));
    connect(uploadFileInGoogleDriveDialog,  SIGNAL(passFileInfoFromFileSystemModel(QFileInfo)), this,                           SLOT(executeImportProgressBar(QFileInfo)));
    connect(this,                           SIGNAL(endUploadData()),                            uploadFileInGoogleDriveDialog,  SLOT(endUploadData()));
}

GoogleDriveCommander::~GoogleDriveCommander()
{
    delete ui;
}

/*Slots begin*/
void GoogleDriveCommander::tableViewDoubleClicked(const QModelIndex &index)
{
    QObject* object = static_cast<QObject*>(index.internalPointer());

    if(object->property("type").toString() == "folder")
        googleDriveItemModel->setRootObject(object);
    else if(object->property("objectName").toString() == "..")
        googleDriveItemModel->setRootObject(googleDriveItemModel->rootObject()->parent());
}

void GoogleDriveCommander::deleteFileDirItem()
{
    googleDriveItemModel->deleteFileDirItem(ui->tableView->currentIndex());
}

void GoogleDriveCommander::startCreatingNewGoogleDriveItem()
{
    newGoogleDriveItemDialog->show();
}

void GoogleDriveCommander::startUploadFileInGoogleDrive()
{
    uploadFileInGoogleDriveDialog->show();
}

void GoogleDriveCommander::executeImportProgressBar(const QFileInfo uploadableData)
{
    QDirIterator uploadableDataSizeIterator(uploadableData.absoluteFilePath(), QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);

    qint64 uploadableDataSize = uploadableData.size();
    while(uploadableDataSizeIterator.hasNext())
    {
        QFileInfo _uploadableData = uploadableDataSizeIterator.nextFileInfo();
        uploadableDataSize += _uploadableData.size();
    }

    ui->importProgressBar->setMaximum(uploadableDataSize);
    ui->importProgressBar->show();

    timerUploading->start(1000);
    connect(timerUploading,         SIGNAL(timeout()),          this, SLOT(progressUploading()));
    connect(googleDriveItemModel,   SIGNAL(uploadDataEnd()),    this, SLOT(uploadDataEnd()));
}

void GoogleDriveCommander::progressUploading()
{
    ui->importProgressBar->setValue(googleDriveItemModel->uploadedDataCurrentSize());
}

void GoogleDriveCommander::uploadDataEnd()
{
    ui->importProgressBar->hide();
    ui->importProgressBar->setValue(0);
    timerUploading->stop();
    disconnect(timerUploading,         SIGNAL(timeout()),          this, SLOT(progressUploading()));
    disconnect(googleDriveItemModel,   SIGNAL(uploadDataEnd()),    this, SLOT(uploadDataEnd()));

    emit endUploadData();
}

/*Slots end*/
