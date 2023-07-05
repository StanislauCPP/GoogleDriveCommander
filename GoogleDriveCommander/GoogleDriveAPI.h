#ifndef GOOGLECLASSES_GOOGLEDRIVEAPI_H
#define GOOGLECLASSES_GOOGLEDRIVEAPI_H

#include <QObject>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMap>
#include <QFileInfo>
#include <QDirIterator>

namespace GoogleClasses {

class GoogleDriveAPI : public QObject
{
    Q_OBJECT

public:
    GoogleDriveAPI(const QJsonDocument oAuth2TokenRespond, QObject *parent = nullptr);
    ~GoogleDriveAPI();

    const QJsonDocument driveFileSystemDocument() const;
    static const QMap<QString, QString> driveItemType();
    const qint64 uploadedDataCurrentSize() const;

    enum class RequestType
    {
        CreateDriveFileSystemDocument,
        DeleteItem,
        CreateDriveItem,
        CreateUploadRootFolder,
        UploadData,
        EndUploadData
    };

private:
    RequestType requestType;

    QNetworkAccessManager *googleNetworkAccessManager;
    QNetworkAccessManager *deleteItemManager;
    QNetworkAccessManager* uploadableFileAccessManager;

    bool errorTokenRespondFlag = false;
    QUrl baseGoogleDriveUrl;

    QJsonDocument _driveFileSystemDocument;
    QJsonDocument oAuth2TokenRespond;

    QUrlQuery keyAccessTokenQuerry(const QJsonDocument _oAuth2TokenRespond);

    QString rootIDForUploadData;
    QFileInfo rootFileInfoForUploadData;
    QMap<QString, QString> uploadedDataParents;
    QSet<QString> uploadedDataId;
    QDirIterator* uploadableDataIterator;
    qint64 _uploadedDataCurrentSize;

private slots:
    void onFinished(QNetworkReply* googleNetworkReply);
    void deleteDriveItem(QString id);
    void uploadDataInGoogleDrive(QString, QFileInfo);
    void preparationForUploadFile(QString googleDriveFolderID, QFileInfo uploadableData);
    void fileAboutToBeStartUpload(QNetworkReply* networkReply);
    void uploadFile(QNetworkReply* networkReply);
    void checkEndUploadFile(QNetworkReply* networkReply);
    void revertToMainThread(QThread* mainThread);

public slots:
    void createDriveFileSystemDocument(const QJsonDocument _oAuth2TokenRespond);
    void createDriveItem(QString itemName, QString itemType, QString parentId, GoogleClasses::GoogleDriveAPI::RequestType _requestType = RequestType::CreateDriveItem);

signals:
    void driveFileSystemDocumentCreated(QJsonDocument);
    void errorNetworkReply(GoogleClasses::GoogleDriveAPI::RequestType _requestType);
    void newDriveItemCreated(QJsonDocument);

    /*createDriveItemSignal is used so we didn't have some trouble with different threads*/
    void createDriveItemSignal(QString itemName, QString itemType, QString parentId, GoogleClasses::GoogleDriveAPI::RequestType _requestType = RequestType::CreateDriveItem);
    void preparationForUploadFileSignal(QString googleDriveFolderID, QFileInfo uploadableFile);
    void uploadFileSignal(QNetworkReply* networkReply);
};

} // namespace GoogleClasses

#endif // GOOGLECLASSES_GOOGLEDRIVEAPI_H
