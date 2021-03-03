#include "ericssontelnet.h"

EricssonTelnet::EricssonTelnet(QString host, QString login, QString password, int port)
    : __login(login), __password(password), __host(host), __port(port)
{
    telnet = nullptr;
    __connecting = false;
    TOKENS.append("END");
    TOKENS.append("NOT ACCEPTED");
    TOKENS.append("UNREASONABLE VALUE");
    TOKENS.append("FORMAT ERROR");
    TOKENS.append("INHIBITED");
    TOKENS.append("FUNCTION BUSY");
    TOKENS.append("FAULT CODE");
    TOKENS.append("ORDERED"); //????
    connect(&timer, &QTimer::timeout, [=] {__connecting = false;});
    __connect();
}

EricssonTelnet::~EricssonTelnet()
{
    delete telnet;
}

void EricssonTelnet::reconnect()
{
    __connect();
}

void EricssonTelnet::tryToLogin(QString &answer)
{
    if(answer.contains("login")){
        __sendCommand(__login);
    } else if(answer.contains("assword")){
        __sendCommand(__password);
    } else if(answer.contains("Terminal")){
        QString xterm("xterm");
        __sendCommand(xterm);
    } else if(answer.contains("Domain")){
        QString emptyLine("");
        __sendCommand(emptyLine);
    } else if(answer.contains(">")){
        QString mml("mml");
        __sendCommand(mml);
    } else if(answer.contains("WO")){
        logedIn = true;
        QString emptyLine("");
        __sendCommand(emptyLine);
    }
}

bool EricssonTelnet::__checkAnswer(QString &answer)
{
    if(answer.size()){
        for(QString token : TOKENS){
            if(answer.contains(token)){
                return true;
            }
        }
    }
    return false;
}

void EricssonTelnet::__connect()
{
    __connecting = true;
    logedIn = false;
    if(telnet){
        delete telnet;
    }

    telnet = new QTelnet();
    telnet->connectToHost(__host, __port);

    connect( telnet, &QTelnet::newData, [=] (const char* text,int size) {
        QString answer;
        for(int i = 0; i < size; i++){
            answer.append(text[i]);
        }
        answer.replace("\u0003<","");
        qDebug() << answer;
        switchAction(answer);
    });
}

void EricssonTelnet::__reconnect()
{
    if(__connecting == true){
        return;
    }
    if(telnet->isConnected()){

    } else {
        emit disconnected();
        __connect();
    }
}

void EricssonTelnet::__delay()
{
    timer.start(3000);
}

void EricssonTelnet::switchAction(QString &answer)
{
    if(__connecting)
        __connecting = false;
    if(logedIn){
        if(answer.contains("TIME OUT")){
            QString emptyLine("");
            __sendCommand(emptyLine);
        } else {
            if(answer.startsWith("WO")){
                QStringList answerLines = answer.split('\n');
                answerLines.pop_front();
                answer = answerLines.join('\n');
            }
            __buffer.append(answer);
            if(__checkAnswer(answer)){
                QString data = __buffer.trimmed();
                emit answerReceived(data);
                __buffer.clear();
            }
        }
        while(__commands.size()){
            __sendCommand(__commands[0]);
            __commands.pop_front();
        }
    } else {
        tryToLogin(answer);
    }
    if(__lastCommands.size() > 300){
        __lastCommands = __lastCommands.mid(150);
    }
}

void EricssonTelnet::sendCommand(QString &command)
{
    if(telnet->isConnected() == false){
        __reconnect();
    }
    if(logedIn){
        __sendCommand(command);
    } else {
        __commands.append(command);
    }
}

void EricssonTelnet::sendCommand(char *command) // del this
{
    QString commandText(command);
    sendCommand(commandText);
}

void EricssonTelnet::__sendCommand(QString &command)
{
    if(command.endsWith("\n")){

    } else {
        command.append(("\n"));
    }
    __lastCommands.append(command);
    if(telnet->isConnected()){
        telnet->sendData(command.toLatin1());
    } else {
        __reconnect();
    }
}
