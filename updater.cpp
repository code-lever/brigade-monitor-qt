#include "updater.h"
#include "cgminerapi.h"

#include <QSettings>

#include <stdexcept>

Updater::Updater(QObject *parent)
    : QObject(parent), hosts(), systemInfo(this)
{
    QSettings settings;
    int size = settings.beginReadArray("monitored-hosts");
    for (int i = 0; i < size; ++i)
    {
        settings.setArrayIndex(i);
        QString name = settings.value("name").toString();
        QString host = settings.value("host").toString();
        QString port = settings.value("port").toString();

        /* skip incomplete entries */
        if (name.isEmpty() || host.isEmpty() || port.isEmpty())
        {
            continue;
        }

        bool ok = true;
        HostInformation info = { name, host, port.toUShort(&ok) };

        if (!ok)
        {
            // XXX notify port error?
            continue;
        }

        hosts.append(info);
    }
    settings.endArray();
}

Updater::~Updater()
{
    /* do nothing */
}

QJsonDocument Updater::update()
{
    QJsonArray updates;

    Q_FOREACH(HostInformation info, hosts)
    {
        try
        {
            updates.append(getUpdate(info));
        }
        catch (std::exception& e)
        {
            qDebug() << "Exception occurred building '" << info.host << "' update:" << e.what();
        }
    }

    return QJsonDocument(updates);
}

QJsonObject Updater::getUpdate(const HostInformation& miner)
{
    CGMinerAPI api(miner.host, miner.port);
    QJsonDocument version = api.version();
    QJsonDocument summary = api.summary();
    QJsonDocument devs = api.devs();
    QJsonDocument pools = api.pools();

    QJsonObject update;
    update["host"] = miner.name;
    update["uptime"] = summary.object()["SUMMARY"].toArray()[0].toObject()["Elapsed"];
    update["mhash"] = summary.object()["SUMMARY"].toArray()[0].toObject()["MHS av"];
    update["rejectpct"] = summary.object()["SUMMARY"].toArray()[0].toObject()["Pool Rejected%"];
    update["api-version"] = version.object()["VERSION"].toArray()[0].toObject()["API"];
    update["cgminer-version"] = version.object()["VERSION"].toArray()[0].toObject()["CGMiner"];
    update["sgminer-version"] = version.object()["VERSION"].toArray()[0].toObject()["SGMiner"];

    QJsonObject agent;
    agent["name"] = QString("brigade-monitor-qt");
    agent["platform"] = systemInfo.getSystemInformation();
    agent["version"] = QString(stringify(APP_VERSION));
    update["agent"] = agent;

    QJsonArray gpus, asics, fpgas;
    Q_FOREACH (QJsonValue value, devs.object()["DEVS"].toArray())
    {
        QJsonObject devObj = value.toObject();

        if (devObj.contains("GPU"))
        {
            QJsonObject gpu;
            gpu["index"] = devObj["GPU"];
            gpu["temperature"] = devObj["Temperature"];
            gpu["enabled"] = devObj["Enabled"].toString() == "Y";
            gpu["status"] = devObj["Status"];
            gpu["uptime"] = devObj["Device Elapsed"];
            gpu["mhash"] = devObj["MHS av"];
            gpu["hwerrors"] = devObj["Hardware Errors"];
            gpu["rejectpct"] = devObj["Device Rejected%"];
            gpus.append(gpu);
        }
        else if (devObj.contains("ASC"))
        {
            QJsonObject asc;
            asc["index"] = devObj["ASC"];
            asc["temperature"] = devObj["Temperature"];
            asc["enabled"] = devObj["Enabled"].toString() == "Y";
            asc["status"] = devObj["Status"];
            asc["uptime"] = devObj["Device Elapsed"];
            asc["mhash"] = devObj["MHS av"];
            asc["hwerrors"] = devObj["Hardware Errors"];
            asc["rejectpct"] = devObj["Device Rejected%"];
            asics.append(asc);
        }
        else if (devObj.contains("PGA"))
        {
            QJsonObject pga;
            pga["index"] = devObj["PGA"];
            pga["temperature"] = devObj["Temperature"];
            pga["enabled"] = devObj["Enabled"].toString() == "Y";
            pga["status"] = devObj["Status"];
            pga["uptime"] = devObj["Device Elapsed"];
            pga["mhash"] = devObj["MHS av"];
            pga["hwerrors"] = devObj["Hardware Errors"];
            pga["rejectpct"] = devObj["Device Rejected%"];
            fpgas.append(pga);
        }
        else
        {
            qDebug() << "Skipped unknown device:" << devObj;
        }
    }
    update["gpus"] = gpus;
    update["asics"] = asics;
    update["fpgas"] = fpgas;

    // XXX pools

    // XXX use VERSION information

    return update;
}
