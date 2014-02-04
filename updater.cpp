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

    QJsonObject sum = summary.object()["SUMMARY"].toArray()[0].toObject();
    QJsonObject ver = version.object()["VERSION"].toArray()[0].toObject();
    QJsonObject update;
    update["host"] = miner.name;
    update["uptime"] = sum["Elapsed"];
    update["mhash_average"] = sum["MHS av"];
    update["mhash_current"] = sum["MHS 5s"];
    update["found_blocks"] = sum["Found Blocks"];
    update["getworks"] = sum["Getworks"];
    update["accepted"] = sum["Accepted"];
    update["rejected"] = sum["Rejected"];
    update["hardware_errors"] = sum["Hardware Errors"];
    update["utility"] = sum["Utility"];
    update["discarded"] = sum["Discarded"];
    update["stale"] = sum["Stale"];
    update["get_failures"] = sum["Get Failures"];
    update["local_work"] = sum["Local Work"];
    update["remote_failures"] = sum["Remote Failures"];
    update["network_blocks"] = sum["Network Blocks"];
    update["total_mhash"] = sum["Total MH"];
    update["work_utility"] = sum["Work Utility"];
    update["difficulty_accepted"] = sum["Difficulty Accepted"];
    update["difficulty_rejected"] = sum["Difficulty Rejected"];
    update["difficulty_stale"] = sum["Difficulty Stale"];
    update["best_share"] = sum["Best Share"];
    update["device_hardware_percent"] = sum["Device Hardware%"];
    update["device_rejected_percent"] = sum["Device Rejected%"];
    update["pool_rejected_percent"] = sum["Pool Rejected%"];
    update["pool_stale_percent"] = sum["Pool Stale%"];
    update["api_version"] = ver["API"];
    update["cgminer_version"] = ver["CGMiner"];
    update["sgminer_version"] = ver["SGMiner"];

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
            gpu["rejected_percent"] = devObj["Device Rejected%"];
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
            asc["rejected_percent"] = devObj["Device Rejected%"];
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
            pga["rejected_percent"] = devObj["Device Rejected%"];
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

    QJsonArray jpools;
    Q_FOREACH (QJsonValue value, pools.object()["POOLS"].toArray())
    {
        QJsonObject devObj = value.toObject();
        QJsonObject pool;
        pool["index"] = devObj["POOL"];
        pool["url"] = devObj["URL"];
        pool["status"] = devObj["Status"];
        pool["priority"] = devObj["Priority"];
        pool["quota"] = devObj["Quota"];
        pool["longpoll"] = devObj["Long Poll"].toString() == "Y";
        pool["getworks"] = devObj["Getworks"];
        pool["accepted"] = devObj["Accepted"];
        pool["rejected"] = devObj["Rejected"];
        pool["works"] = devObj["Works"];
        pool["discarded"] = devObj["Discarded"];
        pool["stale"] = devObj["Stale"];
        pool["get_failures"] = devObj["Get Failures"];
        pool["remote_failures"] = devObj["Remote Failures"];
        pool["user"] = devObj["User"];
        pool["last_share_time"] = devObj["Last Share Time"];
        pool["diff1_shares"] = devObj["Diff1 Shares"];
        pool["proxy_type"] = devObj["Proxy Type"];
        pool["proxy"] = devObj["Proxy"];
        pool["difficulty_accepted"] = devObj["Difficulty Accepted"];
        pool["difficulty_rejected"] = devObj["Difficulty Rejected"];
        pool["difficulty_stale"] = devObj["Difficulty Stale"];
        pool["last_share_difficulty"] = devObj["Last Share Difficulty"];
        pool["has_stratum"] = devObj["Has Stratum"];
        pool["stratum_url"] = devObj["Stratum URL"];
        pool["has_gbt"] = devObj["Has GBT"];
        pool["best_share"] = devObj["Best Share"];
        pool["active"] = devObj["Stratum Active"];
        pool["pool_rejected_percent"] = devObj["Pool Rejected%"];
        pool["pool_stale_percent"] = devObj["Pool Stale%"];
        jpools.append(pool);
    }
    update["pools"] = jpools;

    return update;
}
