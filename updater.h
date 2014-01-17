#ifndef UPDATER_H
#define UPDATER_H

#include "qtsysteminfo.h"

#include <QObject>
#include <QtNetwork>

#define stringify(s) stringify_internal(s)
#define stringify_internal(s) #s

class Updater : public QObject
{
    //Q_OBJECT

public:
    struct HostInformation
    {
        QString name;
        QString host;
        quint16 port;
    };

    Updater(QObject *parent = 0);

    virtual ~Updater();

    void update();

private:
    QList<HostInformation> hosts;
    QString token;
    QtSystemInfo systemInfo;

    QJsonObject getUpdate(const HostInformation& miner);
};

#endif // UPDATER_H
