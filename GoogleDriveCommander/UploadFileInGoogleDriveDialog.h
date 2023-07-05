#ifndef UPLOADFILEINGOOGLEDRIVEDIALOG_H
#define UPLOADFILEINGOOGLEDRIVEDIALOG_H

#include <QDialog>
#include <QFileSystemModel>
#include <QModelIndex>
#include <QFileInfo>

namespace Ui {
class UploadFileInGoogleDriveDialog;
}

class UploadFileInGoogleDriveDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UploadFileInGoogleDriveDialog(QWidget *parent = nullptr);
    ~UploadFileInGoogleDriveDialog();

private:
    Ui::UploadFileInGoogleDriveDialog *ui;

    QFileSystemModel *fileSystemModel;

    bool doubleClickFlag;

private slots:
    void tableViewFileSystemDoubleClicked(QModelIndex index);
    void tableViewFileSystemClicked(QModelIndex index);
    void pushButtonUploadClicked();
    void endUploadData();

signals:
    void passFileInfoFromFileSystemModel(QFileInfo);
};

#endif // UPLOADFILEINGOOGLEDRIVEDIALOG_H
