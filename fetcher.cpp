#include "fetcher.h"
#include "cgminerapi.h"

#include <QSettings>

#include <stdexcept>

Fetcher::Fetcher(const HostInformation& miner, QObject *parent)
    : QObject(parent), miner(miner)
{
    /* do nothing */
}

Fetcher::~Fetcher()
{
    /* do nothing */
}

QJsonObject Fetcher::getUpdate(QtSystemInfo& systemInfo)
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
            gpu["fan_speed"] = devObj["Fan Speed"];
            gpu["fan_percent"] = devObj["Fan Percent"];
            gpu["gpu_clock"] = devObj["GPU Clock"];
            gpu["memory_clock"] = devObj["Memory Clock"];
            gpu["gpu_voltage"] = devObj["GPU Voltage"];
            gpu["gpu_activity"] = devObj["GPU Activity"];
            gpu["powertune"] = devObj["Powertune"];
            gpu["enabled"] = devObj["Enabled"].toString() == "Y";
            gpu["status"] = devObj["Status"];
            gpu["uptime"] = devObj["Device Elapsed"];
            gpu["mhash_average"] = devObj["MHS av"];
            gpu["mhash_current"] = devObj["MHS 5s"];
            gpu["accepted"] = devObj["Accepted"];
            gpu["rejected"] = devObj["Rejected"];
            gpu["hardware_errors"] = devObj["Hardware Errors"];
            gpu["utility"] = devObj["Utility"];
            gpu["intensity"] = devObj["Intensity"];
            gpu["rejected_percent"] = devObj["Device Rejected%"];
            gpu["last_share_pool"] = devObj["Last Share Pool"];
            gpu["last_share_time"] = devObj["Last Share Time"];
            gpu["total_mhash"] = devObj["Total MH"];
            gpu["diff1_work"] = devObj["Diff1 Work"];
            gpu["difficulty_accepted"] = devObj["Difficulty Accepted"];
            gpu["difficulty_rejected"] = devObj["Difficulty Rejected"];
            gpu["last_share_difficulty"] = devObj["Last Share Difficulty"];
            gpu["last_valid_work"] = devObj["Last Valid Work"];
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
            asc["mhash_average"] = devObj["MHS av"];
            asc["mhash_current"] = devObj["MHS 5s"];
            asc["accepted"] = devObj["Accepted"];
            asc["rejected"] = devObj["Rejected"];
            asc["hardware_errors"] = devObj["Hardware Errors"];
            asc["utility"] = devObj["Utility"];
            asc["rejected_percent"] = devObj["Device Rejected%"];
            asc["last_share_pool"] = devObj["Last Share Pool"];
            asc["last_share_time"] = devObj["Last Share Time"];
            asc["total_mhash"] = devObj["Total MH"];
            asc["diff1_work"] = devObj["Diff1 Work"];
            asc["difficulty_accepted"] = devObj["Difficulty Accepted"];
            asc["difficulty_rejected"] = devObj["Difficulty Rejected"];
            asc["last_share_difficulty"] = devObj["Last Share Difficulty"];
            asc["last_valid_work"] = devObj["Last Valid Work"];
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
            pga["mhash_average"] = devObj["MHS av"];
            pga["mhash_current"] = devObj["MHS 5s"];
            pga["accepted"] = devObj["Accepted"];
            pga["rejected"] = devObj["Rejected"];
            pga["hardware_errors"] = devObj["Hardware Errors"];
            pga["utility"] = devObj["Utility"];
            pga["rejected_percent"] = devObj["Device Rejected%"];
            pga["last_share_pool"] = devObj["Last Share Pool"];
            pga["last_share_time"] = devObj["Last Share Time"];
            pga["total_mhash"] = devObj["Total MH"];
            pga["frequency"] = devObj["Frequency"];
            pga["diff1_work"] = devObj["Diff1 Work"];
            pga["difficulty_accepted"] = devObj["Difficulty Accepted"];
            pga["difficulty_rejected"] = devObj["Difficulty Rejected"];
            pga["last_share_difficulty"] = devObj["Last Share Difficulty"];
            pga["last_valid_work"] = devObj["Last Valid Work"];
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
