#ifndef GOOGLEDRIVECOMMANDER_H
#define GOOGLEDRIVECOMMANDER_H

#include "GoogleDriveItemModel.h"
#include "NewGoogleDriveItemDialog.h"
#include "UploadFileInGoogleDriveDialog.h"

#include <QMainWindow>
#include <QFileInfo>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class GoogleDriveCommander; }
QT_END_NAMESPACE

class GoogleDriveCommander : public QMainWindow
{
    Q_OBJECT

public:
    GoogleDriveCommander(QWidget *parent = nullptr);
    ~GoogleDriveCommander();

private:
    Ui::GoogleDriveCommander *ui;
    GoogleClasses::GoogleDriveItemModel *googleDriveItemModel;

    NewGoogleDriveItemDialog *newGoogleDriveItemDialog;
    UploadFileInGoogleDriveDialog *uploadFileInGoogleDriveDialog;
    QTimer *timerUploading;

private slots:
    void tableViewDoubleClicked(const QModelIndex& index);
    void deleteFileDirItem();
    void startCreatingNewGoogleDriveItem();
    void startUploadFileInGoogleDrive();
    void executeImportProgressBar(const QFileInfo uploadableData);
    void progressUploading();
    void uploadDataEnd();

signals:
    void endUploadData();
};
#endif // GOOGLEDRIVECOMMANDER_H
