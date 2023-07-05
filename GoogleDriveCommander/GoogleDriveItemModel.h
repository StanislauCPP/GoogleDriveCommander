#ifndef ITEMMODELS_ADDITIOANLITEMMODEL_H
#define ITEMMODELS_ADDITIOANLITEMMODEL_H

#include "GoogleDriveAPI.h"
#include "GoogleOAuth2API.h"

#include <QAbstractItemModel>
#include <QFileInfo>
#include <QThread>

namespace GoogleClasses {

class GoogleDriveItemModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    GoogleDriveItemModel(QObject* parent = nullptr);

    void setColumns(QStringList _columns);
    const qint64 uploadedDataCurrentSize() const;

private:
    QStringList columns;

    QObject* rootItem;
    QObject* rootItemForNew;
    QObject* objectFromIndex(const QModelIndex &index) const;

    GoogleClasses::GoogleDriveAPI *googleDriveAPIObject;
    GoogleClasses::GoogleOAuth2API *googleOAuth2APIObject = nullptr;

    bool oAuthConnection = false;

    QThread *uploadDataInGoogleDriveThread;

    void setStartModelItemPropperties(QObject *modelItem, QJsonObject objectFromJson);

    // QAbstractItemModel interface
public:
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

    QObject* rootObject() const;
    void setRootObject(QObject* rootObject);

public slots:
    void deleteFileDirItem(const QModelIndex &index);

private slots:
    void createDriveFileSystem(QJsonDocument);
    void createItem(QString itemName, QString itemType);
    void handleDriveAPIErrorReply(GoogleClasses::GoogleDriveAPI::RequestType _requestType);
    void createNewDriveItemModelObject(QJsonDocument);
    void fileInfoPassed(QFileInfo);

signals:
    void deleteDriveItem(QString id);
    void newGoogleDriveItemCreated();
    void passRootUploadIDAndUploadDataFileInfo(QString parentId, QFileInfo uploadableFile);
    void uploadDataEnd();
    void startRevertToMainThread(QThread *mainThread);
};

} // namespace ItemModels

#endif // ITEMMODELS_ADDITIOANLITEMMODEL_H
