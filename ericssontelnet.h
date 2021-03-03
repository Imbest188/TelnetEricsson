#ifndef ERICSSONTELNET_H
#define ERICSSONTELNET_H

#include <QObject>
#include "QTelnet.h"
#include <QTimer>

class EricssonTelnet : public QObject
{
    Q_OBJECT
public:
    EricssonTelnet(QString host, QString login, QString password, int port=23);
    ~EricssonTelnet();
    bool connected() { return telnet->isConnected(); };
    void reconnect();
private:
    QTelnet *telnet;
    void tryToLogin(QString &answer);
    QString __login;
    QString __password;
    QString __host;
    int __port;
    bool logedIn;
    QStringList __commands;
    QString __lastCommands;
    QString __buffer;
    QStringList TOKENS;
    bool __checkAnswer(QString &answer);
    void __connect();
    void __reconnect();
    bool __connecting;
    void __delay();
    QTimer timer;
signals:
    QString answerReceived(QString &data);
    void disconnected();
private slots:
    void switchAction(QString &text);
    void __sendCommand(QString &command);
public slots:
    void sendCommand(QString &command);
    void sendCommand(char *command);
};

#endif // ERICSSONTELNET_H
