#include "GoogleDriveItemModel.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>
#include <QtDebug>
#include <QFileIconProvider>
#include <QIcon>

#define ID "id"
#define PARENTID "parent_id"

namespace GoogleClasses {

    GoogleDriveItemModel::GoogleDriveItemModel(QObject *parent) : QAbstractItemModel(parent)
    {
        googleDriveAPIObject = new GoogleClasses::GoogleDriveAPI(GoogleClasses::GoogleOAuth2API::oAuth2TokenRespond());

        rootItem = new QObject(this);
        rootItemForNew = rootItem;
        columns << "icon" << "objectName" << "type" << "size";

        uploadDataInGoogleDriveThread = new QThread(this);

        connect(this,                   SIGNAL(destroyed(QObject*)),        uploadDataInGoogleDriveThread,  SLOT(quit()));
        connect(this,                   SIGNAL(startRevertToMainThread(QThread*)), googleDriveAPIObject,    SLOT(revertToMainThread(QThread*)));
        connect(googleDriveAPIObject,   SIGNAL(driveFileSystemDocumentCreated(QJsonDocument)),              SLOT(createDriveFileSystem(QJsonDocument)));
        connect(this,                   SIGNAL(deleteDriveItem(QString)),   googleDriveAPIObject,           SLOT(deleteDriveItem(QString)));

        connect(googleDriveAPIObject,   SIGNAL(errorNetworkReply(GoogleClasses::GoogleDriveAPI::RequestType))
                , SLOT(handleDriveAPIErrorReply(GoogleClasses::GoogleDriveAPI::RequestType)));

        connect(googleDriveAPIObject,   SIGNAL(newDriveItemCreated(QJsonDocument)),                         SLOT(createNewDriveItemModelObject(QJsonDocument)));

        connect(this,                   SIGNAL(passRootUploadIDAndUploadDataFileInfo(QString,QFileInfo))
                , googleDriveAPIObject, SLOT(uploadDataInGoogleDrive(QString,QFileInfo)));
    }

    void GoogleDriveItemModel::setColumns(QStringList _columns)
    {
        columns = _columns;
    }

    const qint64 GoogleDriveItemModel::uploadedDataCurrentSize() const
    {
        return googleDriveAPIObject->uploadedDataCurrentSize();
    }


    QObject *GoogleDriveItemModel::objectFromIndex(const QModelIndex &index) const
    {
        if (!index.isValid())
            return rootItem;
        return static_cast<QObject*>(index.internalPointer());
    }

    QModelIndex GoogleDriveItemModel::index(int row, int column, const QModelIndex &parent) const
    {
        QObject* parentObject = objectFromIndex(parent);

        return createIndex(row, column, parentObject->children().at(row));
    }

    QModelIndex GoogleDriveItemModel::parent(const QModelIndex &child) const
    {
        QObject* parentObject = objectFromIndex(child)->parent();

        if(parentObject == rootItem)
            return QModelIndex();

        QObject* grandParentObject = parentObject->parent();
        int row = grandParentObject->children().indexOf(parentObject);

        return createIndex(row, 0, parentObject);
    }

    int GoogleDriveItemModel::rowCount(const QModelIndex &parent) const
    {
        return objectFromIndex(parent)->children().count();
    }

    int GoogleDriveItemModel::columnCount(const QModelIndex &) const
    {
        return columns.count();
    }

    QVariant GoogleDriveItemModel::data(const QModelIndex &index, int role) const
    {
        if(!index.isValid())
            return QVariant();

        if(role==Qt::DecorationRole)
        {
            return objectFromIndex(index)->property(columns.at(index.column()).toUtf8());
        }
        else if(role==Qt::DisplayRole)
        {
            return objectFromIndex(index)->property(columns.at(index.column()).toUtf8());
        }

        return QVariant();
    }

    QObject *GoogleDriveItemModel::rootObject() const
    {
        return rootItem;
    }

    void GoogleDriveItemModel::setRootObject(QObject *rootObject)
    {
        rootItem = rootObject;
        emit layoutChanged();
    }

/*slots begin*/
    void GoogleDriveItemModel::deleteFileDirItem(const QModelIndex &index)
    {
        beginRemoveRows(index.parent(), index.row(), index.row());

        QObject* delObject = objectFromIndex(index);

        emit deleteDriveItem(delObject->property(ID).toString());

        delete delObject;
        delObject = nullptr;

        endRemoveRows();
    }

    void GoogleDriveItemModel::createDriveFileSystem(QJsonDocument driveFileSystemDocument)
    {
        QJsonArray jsonDataArray = driveFileSystemDocument.object()["files"].toArray();
        QObjectList objectListFromJSON;

        for(auto jsonValue = jsonDataArray.begin(); jsonValue != jsonDataArray.end(); ++jsonValue)
        {
            QObject* modelItem = new QObject(rootItemForNew);
            setStartModelItemPropperties(modelItem, jsonValue->toObject());
            modelItem->setProperty(PARENTID, jsonValue->toObject().value("parents")[0].toString());

            objectListFromJSON.push_back(modelItem);
        }

        for(int i = 0; i < objectListFromJSON.size(); ++i)
        {
            for(int j = 0; j < objectListFromJSON.size(); ++j)
            {
                if(objectListFromJSON[i]->property(ID) == objectListFromJSON[j]->property(PARENTID))
                {
                    objectListFromJSON[j]->setParent(objectListFromJSON[i]);
                }
            }
        }

        for(int i = 0; i < objectListFromJSON.size(); ++i)
        {
            if(objectListFromJSON[i]->parent() == rootItemForNew)
            {
                rootItemForNew->setProperty(ID, objectListFromJSON[i]->property(PARENTID));
                break;
            }
        }

        emit layoutChanged();

        if(oAuthConnection)
            oAuthConnection = disconnect(googleOAuth2APIObject, SIGNAL(tokenRespondFileCreated(QJsonDocument))
                                         , googleDriveAPIObject, SLOT(createDriveFileSystemDocument(QJsonDocument)));

        if(this->thread() != googleDriveAPIObject->thread())
        {
            qDebug() << "different threads";
            emit startRevertToMainThread(this->thread());
            emit uploadDataEnd();
        }
    }

    void GoogleDriveItemModel::createItem(QString itemName, QString itemType)
    {
        googleDriveAPIObject->createDriveItem(itemName, itemType, rootItem->property(ID).toString());
    }

    void GoogleDriveItemModel::handleDriveAPIErrorReply(GoogleDriveAPI::RequestType _requestType)
    {
        switch (_requestType) {
        case GoogleClasses::GoogleDriveAPI::RequestType::CreateDriveFileSystemDocument:
            googleOAuth2APIObject = new GoogleClasses::GoogleOAuth2API(this);
            oAuthConnection = connect(googleOAuth2APIObject, SIGNAL(tokenRespondFileCreated(QJsonDocument)), googleDriveAPIObject, SLOT(createDriveFileSystemDocument(QJsonDocument)));
            break;

        case GoogleClasses::GoogleDriveAPI::RequestType::CreateDriveItem:
            break;

        default:
            break;
        }
    }

    void GoogleDriveItemModel::createNewDriveItemModelObject(QJsonDocument newItemFromGoogleDriveApi)
    {
        QObject *modelItem = new QObject(rootItem);
        setStartModelItemPropperties(modelItem, newItemFromGoogleDriveApi.object());
        modelItem->setProperty(PARENTID, rootItem->property(ID));

        emit layoutChanged();
        emit newGoogleDriveItemCreated();
    }

    void GoogleDriveItemModel::fileInfoPassed(QFileInfo uploadableData)
    {
        googleDriveAPIObject->moveToThread(uploadDataInGoogleDriveThread);
        uploadDataInGoogleDriveThread->start();

        rootItemForNew = rootItem;

        emit passRootUploadIDAndUploadDataFileInfo(rootItem->property(ID).toString(), uploadableData);
    }

/*slots end*/
    void GoogleDriveItemModel::setStartModelItemPropperties(QObject *modelItem, QJsonObject objectFromJson)
    {
        modelItem->setObjectName(objectFromJson.value("name").toString());
        modelItem->setProperty(columns[2].toUtf8(), objectFromJson.value("mimeType").toString().remove("application/vnd.google-apps."));
        modelItem->setProperty(ID, objectFromJson.value(ID).toString());

        if(modelItem->property(columns[2].toUtf8()).toString() != "folder")
        {
            modelItem->setProperty(columns[0].toUtf8(), QFileIconProvider().icon(QFileIconProvider::File));
            modelItem->setProperty(columns[3].toUtf8(), objectFromJson.value("size").toString() + " bytes");
        }
        else
        {
            QObject* exitFolderItem = new QObject(modelItem);
            exitFolderItem->setObjectName("..");

            modelItem->setProperty(columns[0].toUtf8(), QFileIconProvider().icon(QFileIconProvider::Folder));
        }
    }

} // namespace ItemModels
