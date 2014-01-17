#ifndef CGMINERAPI_H
#define CGMINERAPI_H

#include <QtNetwork>

class CGMinerAPI
{
public:
    CGMinerAPI(const QString& host, quint16 port);
    virtual ~CGMinerAPI();

    QJsonDocument config();
    QJsonDocument devdetails();
    QJsonDocument devs();
    QJsonDocument pools();
    QJsonDocument summary();
    QJsonDocument version();

private:
    QString host;
    quint16 port;

    QJsonDocument command(const QString& name, const QString& param = "");
};

#endif // CGMINERAPI_H
