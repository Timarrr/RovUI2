#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QtCore/QObject>
#include <QtWebSockets/QWebSocket>
#include <QMainWindow>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QAbstractSocket>
#include <QTimer>

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(const QUrl &url, QObject *parent = nullptr);

signals:
    void recieveReady(QJsonObject obj);

public slots:
    void sendText(QString message);

private slots:
    void onConnected();
    void closedConnect();
    void tryConnect();
    void stateCheck(QAbstractSocket::SocketState state);
    void onTextMessageReceived(QString message);

private:
    QWebSocket m_webSocket;
    QUrl m_url;
    QTimer *m_timerConnect;
};

#endif // CLIENT_H
