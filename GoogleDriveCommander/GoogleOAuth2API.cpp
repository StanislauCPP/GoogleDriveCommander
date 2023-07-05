/*
 * Redirection to Google's authorization server

https://accounts.google.com/o/oauth2/v2/auth?
scope=https://www.googleapis.com/auth/drive&
access_type=offline&
include_granted_scopes=true&
response_type=code&
state=state_parameter_passthrough_value&
redirect_uri=http://localhost&
client_id=74401920343-qgicosqqpvj2pl6ppcijgermllpb9v6p.apps.googleusercontent.com

Perhaps, I'll make implementation of user login and consent via WebEngine

After completing the OAuth 2.0 flow, you should be redirected to

http://localhost/?state=state_parameter_passthrough_value&code=4/0AWtgzh4jkiQjZCI5lowSavhIvtgCe5moH4QREmwxS2S1eFA80NmrAoSxkz512I2Exyt2Bg&scope=https://www.googleapis.com/auth/drive
*/

#include <QDebug>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QNetworkReply>

#define SCOPE                   "scope"
#define ACCESSTYPE              "access_type"
#define INCLUDEGRANTEDSCOPES    "include_granted_scopes"
#define RESPONSETYPE            "response_type"
#define STATE                   "state"
#define REDIRECTURI             "redirect_uri"
#define CLIENTID                "client_id"

#define CODE            "code"
#define CLIENTSECRET    "client_secret"
#define GRANTTYPE       "grant_type"

#include "GoogleOAuth2API.h"

namespace GoogleClasses {

GoogleOAuth2API::GoogleOAuth2API(QObject *parent) : QObject(parent)
{
    QUrl urlApplication("https://accounts.google.com/o/oauth2/v2/auth");

    QUrlQuery queryApplication;
    queryApplication.addQueryItem(SCOPE,                    queryApplicationScope);
    queryApplication.addQueryItem(ACCESSTYPE,               queryApplicationAccessType);
    queryApplication.addQueryItem(INCLUDEGRANTEDSCOPES,     queryApplicationIncludeGrantedScopes);
    queryApplication.addQueryItem(RESPONSETYPE,             queryApplicationResponseType);
    queryApplication.addQueryItem(STATE,                    queryApplicationState);
    queryApplication.addQueryItem(REDIRECTURI,              queryApplicationRedirectUri);
    queryApplication.addQueryItem(CLIENTID,                 queryApplicationClientID);
    urlApplication.setQuery(queryApplication);

    userAuthenticationWebPage = new QWebEngineView();
    userAuthenticationWebPage->setUrl(urlApplication);
    userAuthenticationWebPage->load(urlApplication);
    userAuthenticationWebPage->show();

    oAuth2NetworkAccessManager = new QNetworkAccessManager(this);

    connect(userAuthenticationWebPage,  SIGNAL(urlChanged(QUrl)),           SLOT(codeFromUrl(QUrl)));
    connect(oAuth2NetworkAccessManager, SIGNAL(finished(QNetworkReply*)),   SLOT(createTokenRespondFile(QNetworkReply*)));
}

GoogleOAuth2API::~GoogleOAuth2API()
{
    delete userAuthenticationWebPage;
}

const QJsonDocument GoogleOAuth2API::oAuth2TokenRespond()
{
    QFile fileOAuthTokenRespond("../oAuth2TokenRespond.json");
    fileOAuthTokenRespond.open(QIODevice::ReadOnly);

    return QJsonDocument::fromJson(fileOAuthTokenRespond.readAll());
}

//slots begin
void GoogleOAuth2API::codeFromUrl(QUrl userAuthenticationUrl)
{
    QUrlQuery urlQuery(userAuthenticationUrl);

    if(urlQuery.hasQueryItem(CODE))
    {
        QUrl urlRequestForToken("https://oauth2.googleapis.com/token");

        QString queryClientSecret   = "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",
                queryGrantType      = "authorization_code";

        QUrlQuery querryRequestForToken;
        querryRequestForToken.addQueryItem(CODE,            urlQuery.queryItemValue(CODE));
        querryRequestForToken.addQueryItem(CLIENTID,        queryApplicationClientID);
        querryRequestForToken.addQueryItem(CLIENTSECRET,    queryClientSecret);
        querryRequestForToken.addQueryItem(REDIRECTURI,     queryApplicationRedirectUri);
        querryRequestForToken.addQueryItem(GRANTTYPE,       queryGrantType);
        urlRequestForToken.setQuery(querryRequestForToken);

        QNetworkRequest networkRequestForToken(urlRequestForToken);
        networkRequestForToken.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

        oAuth2NetworkAccessManager->post(networkRequestForToken, urlRequestForToken.toEncoded());
    }
}

void GoogleOAuth2API::createTokenRespondFile(QNetworkReply *tokenReply)
{
    QFile fileOAuthTokenRespond("../oAuth2TokenRespond.json");
    fileOAuthTokenRespond.open(QFile::WriteOnly);
    fileOAuthTokenRespond.write(tokenReply->readAll());

    userAuthenticationWebPage->close();
    fileOAuthTokenRespond.close();

    emit tokenRespondFileCreated(oAuth2TokenRespond());
}

//slots end
} // namespace GoogleClasses
