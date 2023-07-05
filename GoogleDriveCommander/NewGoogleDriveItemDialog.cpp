#include "NewGoogleDriveItemDialog.h"
#include "ui_NewGoogleDriveItemDialog.h"

#include <QMap>
#include <QRegularExpression>
#include <QMouseEvent>
#include <QToolTip>

NewGoogleDriveItemDialog::NewGoogleDriveItemDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewGoogleDriveItemDialog)
{
    ui->setupUi(this);

    QMap<QString, QString> googleDriveItemType = GoogleClasses::GoogleDriveAPI::driveItemType();

    for (auto it = googleDriveItemType.keyBegin(); it != googleDriveItemType.keyEnd(); ++it)
    {
        ui->comboBoxItemType->addItem(*it);
    }

    setModal(true);
    setWindowTitle("Create new GoogleDrive Item");
}

NewGoogleDriveItemDialog::~NewGoogleDriveItemDialog()
{
    delete ui;
}


/*Slots begin*/
void NewGoogleDriveItemDialog::newItemNameEntered()
{
    QString itemName = ui->lineEditItemName->text();
    static QRegularExpression regularExp("\\.[\\w]{1,}$");
    QRegularExpressionMatch regularExpMatch = regularExp.match(itemName);

    if(regularExpMatch.hasMatch())
    {
        QString typeKey = regularExpMatch.captured();

        if(GoogleClasses::GoogleDriveAPI::driveItemType().contains(typeKey))
        {
            itemName.remove(regularExpMatch.captured());
            emit passNewItemNameAndType(itemName, GoogleClasses::GoogleDriveAPI::driveItemType()[typeKey]);
        }
    }
    else
        emit passNewItemNameAndType(itemName);
}

void NewGoogleDriveItemDialog::itemTypeActivated(QString itemType)
{
    QString itemName = ui->lineEditItemName->text();
    static QRegularExpression regularExp("\\.[\\w]{1,}$");
    QRegularExpressionMatch regularExpMatch = regularExp.match(itemName);

    if(regularExpMatch.hasMatch())
    {
        itemName.remove(regularExpMatch.captured());
    }

    if(itemType != "dir")
        itemName += itemType;

    ui->lineEditItemName->setText(itemName);
}

void NewGoogleDriveItemDialog::itemTypeHighlighted(QString itemType)
{
    QToolTip::showText(ui->comboBoxItemType->cursor().pos(), GoogleClasses::GoogleDriveAPI::driveItemType()[itemType]);
}

void NewGoogleDriveItemDialog::closeNewGoogleDriveItemDialog()
{
    close();
}

/*Slots end*/
