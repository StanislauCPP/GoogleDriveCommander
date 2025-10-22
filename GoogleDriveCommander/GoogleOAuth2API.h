#ifndef GOOGLECLASSES_GOOGLEOAUTH2API_H
#define GOOGLECLASSES_GOOGLEOAUTH2API_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QWebEngineView>
#include <QUrl>
#include <QJsonDocument>
#include <QFileInfo>

namespace GoogleClasses {

class GoogleOAuth2API : public QObject
{
    Q_OBJECT
public:
    explicit GoogleOAuth2API(QObject *parent = nullptr);
    ~GoogleOAuth2API();

    static const QJsonDocument oAuth2TokenRespond();

private:
    QNetworkAccessManager *oAuth2NetworkAccessManager;
    QWebEngineView *userAuthenticationWebPage;

    QString queryApplicationScope                   = "https://www.googleapis.com/auth/drive",
            queryApplicationAccessType              = "offline",
            queryApplicationIncludeGrantedScopes    = "true",
            queryApplicationResponseType            = "code",
            queryApplicationState                   = "state_parameter_passthrough_value",
            queryApplicationRedirectUri             = "http://localhost",
            queryApplicationClientID                = "";

private slots:
    void codeFromUrl(QUrl);
    void createTokenRespondFile(QNetworkReply*);

signals:
    void tokenRespondFileCreated(QJsonDocument);
};

} // namespace GoogleClasses

#endif // GOOGLECLASSES_GOOGLEOAUTH2API_H
