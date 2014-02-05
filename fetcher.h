#ifndef FETCHER_H
#define FETCHER_H

#include "host_information.h"
#include "qtsysteminfo.h"

#include <QObject>
#include <QtNetwork>

#define stringify(s) stringify_internal(s)
#define stringify_internal(s) #s

class Fetcher : public QObject
{
    Q_OBJECT

public:
    Fetcher(const HostInformation& miner, QObject *parent = 0);

    virtual ~Fetcher();

    QJsonObject getUpdate(QtSystemInfo& systemInfo);

private:
    HostInformation miner;

    QJsonObject getHostInfo(const QJsonObject& summary, const QJsonObject& version);
    QJsonObject getAsicInfo(const QJsonObject& device);
    QJsonObject getFpgaInfo(const QJsonObject& device);
    QJsonObject getGpuInfo(const QJsonObject& device);
    QJsonObject getPoolInfo(const QJsonObject& pool);
};

#endif // FETCHER_H
