#include "cgminerapi.h"
#include <stdexcept>

#include <QString>

CGMinerAPI::CGMinerAPI(const QString& host, quint16 port) :
    host(host), port(port)
{
    /* do nothing */
}

CGMinerAPI::~CGMinerAPI()
{
    /* do nothing */
}
QJsonDocument CGMinerAPI::devs()
{
    return command("devs");
}

QJsonDocument CGMinerAPI::pools()
{
    return command("pools");
}

QJsonDocument CGMinerAPI::summary()
{
    return command("summary");
}

QJsonDocument CGMinerAPI::version()
{
    return command("version");
}

QJsonDocument CGMinerAPI::command(const QString& name, const QString& param)
{
    QTcpSocket socket;
    socket.connectToHost(host, port);
    if (!socket.waitForConnected(5000))
    {
        QString message = "Failed to connect (%1)";
        throw std::runtime_error(message.arg(socket.errorString()).toStdString());
    }

    QString command("{ \"command\" : \"%1\", \"parameter\" : \"%2\" }");
    QByteArray cmdout;
    socket.write(cmdout.append(command.arg(name).arg(param)));
    if (!socket.waitForBytesWritten(5000))
    {
        QString message = "Failed to write command (%1)";
        throw std::runtime_error(message.arg(socket.errorString()).toStdString());
    }

    /* build up the response until no more data is received */
    QByteArray response;
    while (socket.waitForReadyRead(5000))
    {
        response.append(socket.readAll());

        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(response, &error);
        if (QJsonParseError::NoError == error.error)
        {
            return doc;
        }
    }

    QString message = "Received incomplete or empty response (%1)";
    throw std::runtime_error(message.arg(socket.errorString()).toStdString());
}
