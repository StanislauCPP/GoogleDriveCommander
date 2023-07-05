#ifndef NEWGOOGLEDRIVEITEMDIALOG_H
#define NEWGOOGLEDRIVEITEMDIALOG_H

#include "GoogleDriveAPI.h"

#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>

namespace Ui {
class NewGoogleDriveItemDialog;
}

class NewGoogleDriveItemDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewGoogleDriveItemDialog(QWidget *parent = nullptr);
    ~NewGoogleDriveItemDialog();

private:
    Ui::NewGoogleDriveItemDialog *ui;

private slots:
    void newItemNameEntered();
    void itemTypeActivated(QString itemType);
    void itemTypeHighlighted(QString itemType);
    void closeNewGoogleDriveItemDialog();

signals:
    void passNewItemNameAndType(QString itemName, QString itemType = GoogleClasses::GoogleDriveAPI::driveItemType()["dir"]);
};

#endif // NEWGOOGLEDRIVEITEMDIALOG_H
