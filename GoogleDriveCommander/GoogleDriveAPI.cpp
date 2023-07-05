#include "GoogleDriveAPI.h"

#include <QDebug>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonObject>
#include <QJsonArray>
#include <QThread>
#include <QDir>

#define KEY             "key"
#define ACCESSTOKEN     "access_token"
#define FIELDS          "fields"
#define FILEID          "fileId"
#define ID              "id"

namespace GoogleClasses {

GoogleDriveAPI::GoogleDriveAPI(const QJsonDocument _oAuth2TokenRespond, QObject *parent)
    : QObject(parent)
    , requestType(RequestType::CreateDriveFileSystemDocument)
    , baseGoogleDriveUrl("https://www.googleapis.com/drive/v3/files")
{
    googleNetworkAccessManager  = new QNetworkAccessManager(this);
    deleteItemManager           = new QNetworkAccessManager(this);
    uploadableFileAccessManager = new QNetworkAccessManager(this);

    connect(googleNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), SLOT(onFinished(QNetworkReply*)));
    connect(this, SIGNAL(createDriveItemSignal(QString,QString,QString,GoogleClasses::GoogleDriveAPI::RequestType)),
            this, SLOT(createDriveItem(QString,QString,QString,GoogleClasses::GoogleDriveAPI::RequestType)));
    connect(this, SIGNAL(preparationForUploadFileSignal(QString,QFileInfo)), this, SLOT(preparationForUploadFile(QString,QFileInfo)));

    if(_oAuth2TokenRespond.isEmpty())
        errorTokenRespondFlag = true;

    createDriveFileSystemDocument(_oAuth2TokenRespond);
}

GoogleDriveAPI::~GoogleDriveAPI()
{

}

const QJsonDocument GoogleDriveAPI::driveFileSystemDocument() const
{
    return _driveFileSystemDocument;
}

const QMap<QString, QString> GoogleDriveAPI::driveItemType()
{
    return QMap<QString, QString>({{"dir", "folder"}, {".gdoc", "document"}, {".gspr", "spreadsheet"}});
}

QUrlQuery GoogleDriveAPI::keyAccessTokenQuerry(const QJsonDocument _oAuth2TokenRespond)
{
    QUrlQuery urlQuerry;
    urlQuerry.addQueryItem(KEY, "AIzaSyA3D8IpODGZYsb8vWnMosFiA988FPwUD0w");
    urlQuerry.addQueryItem(ACCESSTOKEN, _oAuth2TokenRespond.object().take(ACCESSTOKEN).toString());

    return urlQuerry;
}

const qint64 GoogleDriveAPI::uploadedDataCurrentSize() const
{
    return _uploadedDataCurrentSize;
}

//slots begin
void GoogleDriveAPI::onFinished(QNetworkReply *googleNetworkReply)
{
    qDebug() << "SLOT connect";

    if(googleNetworkReply->error())
    {
        errorTokenRespondFlag = true;
        emit errorNetworkReply(requestType);
    }
    else
    {
        errorTokenRespondFlag = false;

        QJsonDocument jDocumentFromReply = QJsonDocument::fromJson(googleNetworkReply->readAll());

        switch (requestType)
        {
        case RequestType::CreateDriveFileSystemDocument:
            _driveFileSystemDocument = jDocumentFromReply;
            emit driveFileSystemDocumentCreated(_driveFileSystemDocument);
            break;

        case RequestType::CreateDriveItem:
            emit newDriveItemCreated(jDocumentFromReply);
            break;

        case RequestType::CreateUploadRootFolder:
        {
            if(uploadableDataIterator->hasNext())
            {
                uploadedDataParents.insert(uploadableDataIterator->path(), jDocumentFromReply.object().take(ID).toString());
            }
        }

        case RequestType::UploadData:
        {
            uploadedDataId.insert(jDocumentFromReply.object().take(ID).toString());

            if(uploadedDataParents.find(uploadableDataIterator->filePath()) == uploadedDataParents.end() && !uploadableDataIterator->filePath().isEmpty())
            {
                uploadedDataParents.insert(uploadableDataIterator->filePath(), jDocumentFromReply.object().take(ID).toString());
                qDebug() << uploadableDataIterator->filePath();
            }

            if(uploadableDataIterator->hasNext())
            {
                QFileInfo fileInfo = uploadableDataIterator->nextFileInfo();

                if(fileInfo.isDir())
                {
                    qDebug() << uploadedDataParents[fileInfo.absolutePath()] << uploadableDataIterator->fileName();

                    emit createDriveItemSignal(fileInfo.fileName(), driveItemType()["dir"], uploadedDataParents[fileInfo.absolutePath()], RequestType::UploadData);
                }
                else if(fileInfo.isFile())
                {
                    emit preparationForUploadFileSignal(uploadedDataParents[fileInfo.absolutePath()], fileInfo);
                }
            }
            else
            {
                qDebug() << "delete and clear";

                delete uploadableDataIterator;
                uploadableDataIterator = nullptr;

                QString uploadedDataGoogleRequest = "(name%20%3D%20%27" + rootFileInfoForUploadData.fileName() + "%27%20and%20parents%20%3D%20%27" + rootIDForUploadData + "%27)";

                for (auto& fileParentId : qAsConst(uploadedDataParents))
                {
                    uploadedDataGoogleRequest += "%20or%20parents%20%3D%20%27" + fileParentId + "%27";
                }

                requestType = RequestType::EndUploadData;
                uploadedDataParents.clear();
                uploadedDataId.clear();

                QUrl uploadedDataGoogleRequestUrl = baseGoogleDriveUrl;

                QUrlQuery urlQuerry = keyAccessTokenQuerry(oAuth2TokenRespond);
                urlQuerry.addQueryItem(FIELDS, "files(mimeType%2C%20parents%2C%20size%2C%20id%2C%20name)");
                urlQuerry.addQueryItem("q", uploadedDataGoogleRequest);

                uploadedDataGoogleRequestUrl.setQuery(urlQuerry);

                googleNetworkAccessManager->get(QNetworkRequest(uploadedDataGoogleRequestUrl));
            }

        }
            break;

        case RequestType::EndUploadData:
        {
            emit driveFileSystemDocumentCreated(jDocumentFromReply);
        }
            break;

        default:
            break;
        }
    }
}

void GoogleDriveAPI::createDriveFileSystemDocument(const QJsonDocument _oAuth2TokenRespond)
{
    oAuth2TokenRespond = _oAuth2TokenRespond;

    QUrl createDriveFileSystemDocumentUrl = baseGoogleDriveUrl;

    QUrlQuery urlQuerry = keyAccessTokenQuerry(_oAuth2TokenRespond);
    urlQuerry.addQueryItem(FIELDS, "files(mimeType%2C%20parents%2C%20size%2C%20id%2C%20name)");

    createDriveFileSystemDocumentUrl.setQuery(urlQuerry);

    googleNetworkAccessManager->get(QNetworkRequest(createDriveFileSystemDocumentUrl));
}

void GoogleDriveAPI::deleteDriveItem(QString id)
{
    QUrl deleteDriveItemUrl(baseGoogleDriveUrl.toString() + "/" + id);

    QUrlQuery urlQuerry = keyAccessTokenQuerry(oAuth2TokenRespond);
    deleteDriveItemUrl.setQuery(urlQuerry);

    deleteItemManager->deleteResource(QNetworkRequest(deleteDriveItemUrl));
}

void GoogleDriveAPI::createDriveItem(QString itemName, QString itemType, QString parentId, GoogleClasses::GoogleDriveAPI::RequestType _requestType)
{
    requestType = _requestType;

    QJsonObject newDriveItemData;

    QJsonArray parentValue;
    parentValue.push_back(parentId);
    newDriveItemData.insert("name", itemName);
    newDriveItemData.insert("mimeType", "application/vnd.google-apps." + itemType);
    newDriveItemData.insert("parents", parentValue);

    QUrl createDriveItemUrl = baseGoogleDriveUrl;
    QUrlQuery urlQuerry = keyAccessTokenQuerry(oAuth2TokenRespond);

    createDriveItemUrl.setQuery(urlQuerry);

    QNetworkRequest request(createDriveItemUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");    

    googleNetworkAccessManager->post(request, QJsonDocument(newDriveItemData).toJson());
}

void GoogleDriveAPI::uploadDataInGoogleDrive(QString googleDriveFolderID, QFileInfo uploadableData)
{
    rootIDForUploadData = googleDriveFolderID;
    rootFileInfoForUploadData = uploadableData;
    _uploadedDataCurrentSize = 0;

    uploadableDataIterator = new QDirIterator(uploadableData.absoluteFilePath(), QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);

    if(uploadableData.isDir())
    {
        qDebug() << uploadableDataIterator->path();

        emit createDriveItemSignal(uploadableData.fileName(), driveItemType()["dir"], googleDriveFolderID, RequestType::CreateUploadRootFolder);
    }
    else if(uploadableData.isFile())
    {
        emit preparationForUploadFileSignal(googleDriveFolderID, uploadableData);
    }

}

void GoogleDriveAPI::preparationForUploadFile(QString googleDriveFolderID, QFileInfo uploadableFile)
{
    QUrl url("https://www.googleapis.com/upload/drive/v3/files");
    QUrlQuery urlQuerry = keyAccessTokenQuerry(oAuth2TokenRespond);
    urlQuerry.addQueryItem("uploadType", "resumable");
    url.setQuery(urlQuerry);

    QNetworkRequest networkRequest(url);
    networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=UTF-8");

    QJsonObject uploadableFileItemParam;
    uploadableFileItemParam.insert("name", uploadableFile.fileName());
    uploadableFileItemParam.insert("parents", QJsonArray({googleDriveFolderID}));

    QFile* _uploadableFile = new QFile(this);
    _uploadableFile->setFileName(uploadableFile.absoluteFilePath());
    _uploadableFile->open(QIODevice::ReadOnly);
    QVariant ptrUpFile;
    ptrUpFile.setValue(_uploadableFile);

    uploadableFileAccessManager->setProperty("uploadedDataSize", quint64(0));
    uploadableFileAccessManager->setProperty("uploadableFile", ptrUpFile);

    connect(uploadableFileAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(fileAboutToBeStartUpload(QNetworkReply*)));

    uploadableFileAccessManager->post(networkRequest, QJsonDocument(uploadableFileItemParam).toJson());
}

void GoogleDriveAPI::fileAboutToBeStartUpload(QNetworkReply* networkReply)
{
    if(!networkReply->error())
    {
        uploadableFileAccessManager->setProperty("location", QVariant(networkReply->rawHeader("location")));

        disconnect(uploadableFileAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(fileAboutToBeStartUpload(QNetworkReply*)));
        connect(uploadableFileAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(checkEndUploadFile(QNetworkReply*)));
        connect(this, SIGNAL(uploadFileSignal(QNetworkReply*)), this, SLOT(uploadFile(QNetworkReply*)));

        emit uploadFileSignal(networkReply);
    }
}

void GoogleDriveAPI::uploadFile(QNetworkReply* networkReply)
{
    if(!networkReply->error())
    {
        QFile* uploadableFile = uploadableFileAccessManager->property("uploadableFile").value<QFile*>();

        quint64 uploadedDataSize = uploadableFileAccessManager->property("uploadedDataSize").toLongLong(), dataPortionSize = 262144;
        QByteArray data = uploadableFile->read(dataPortionSize);
        _uploadedDataCurrentSize += data.size();

        QString locationHeaderValue = uploadableFileAccessManager->property("location").toString();
        QNetworkRequest networkRequest(locationHeaderValue);
        networkRequest.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);
        networkRequest.setRawHeader(QByteArray("Content-Length"), QString::number(dataPortionSize).toUtf8());
        networkRequest.setRawHeader(QByteArray("Content-Range"), ("bytes " + QString::number(uploadedDataSize) + "-" +
                                                                  QString::number(uploadedDataSize + data.size()-1) + "/" + QString::number(uploadableFile->size())).toUtf8());

        uploadableFileAccessManager->setProperty("uploadedDataSize", uploadedDataSize + data.size());
        uploadableFileAccessManager->put(networkRequest, data);
    }
    else
    {
        qDebug() << networkReply->error() << networkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    }
}

void GoogleDriveAPI::checkEndUploadFile(QNetworkReply* networkReply)
{
    if(!networkReply->error() && (networkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute) == QVariant(308)))
    {
        emit uploadFileSignal(networkReply);
    }
    else
    {
        QNetworkAccessManager* uploadableFileAccessManager = networkReply->manager();
        QFile* uploadableFile = uploadableFileAccessManager->property("uploadableFile").value<QFile*>();
        disconnect(uploadableFileAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(checkEndUploadFile(QNetworkReply*)));
        disconnect(this, SIGNAL(uploadFileSignal(QNetworkReply*)), this, SLOT(uploadFile(QNetworkReply*)));

        uploadableFile->close();
        delete uploadableFile;
        uploadableFile = nullptr;

        qDebug() << uploadableDataIterator->path();

        requestType = RequestType::UploadData;
        emit googleNetworkAccessManager->finished(networkReply);
    }
}

void GoogleDriveAPI::revertToMainThread(QThread* mainThread)
{
    QThread* oldThisThread;
    oldThisThread = this->thread();
    this->moveToThread(mainThread);
    oldThisThread->quit();
}
//slots end

} // namespace GoogleClasses
