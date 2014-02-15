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
    QJsonObject update = getHostInfo(sum, ver);

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
            gpus.append(getGpuInfo(devObj));
        }
        else if (devObj.contains("ASC"))
        {
            asics.append(getAsicInfo(devObj));
        }
        else if (devObj.contains("PGA"))
        {
            fpgas.append(getFpgaInfo(devObj));
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
        jpools.append(getPoolInfo(value.toObject()));
    }
    update["pools"] = jpools;

    return update;
}

QJsonObject Fetcher::getHostInfo(const QJsonObject& summary, const QJsonObject& version)
{
    QJsonObject obj;
    obj["host"] = miner.name;
    obj["uptime"] = summary["Elapsed"];
    obj["mhash_average"] = summary["MHS av"];
    obj["mhash_current"] = findMhashCurrent(summary);
    obj["found_blocks"] = summary["Found Blocks"];
    obj["getworks"] = summary["Getworks"];
    obj["accepted"] = summary["Accepted"];
    obj["rejected"] = summary["Rejected"];
    obj["hardware_errors"] = summary["Hardware Errors"];
    obj["utility"] = summary["Utility"];
    obj["discarded"] = summary["Discarded"];
    obj["stale"] = summary["Stale"];
    obj["get_failures"] = summary["Get Failures"];
    obj["local_work"] = summary["Local Work"];
    obj["remote_failures"] = summary["Remote Failures"];
    obj["network_blocks"] = summary["Network Blocks"];
    obj["total_mhash"] = summary["Total MH"];
    obj["work_utility"] = summary["Work Utility"];
    obj["difficulty_accepted"] = summary["Difficulty Accepted"];
    obj["difficulty_rejected"] = summary["Difficulty Rejected"];
    obj["difficulty_stale"] = summary["Difficulty Stale"];
    obj["best_share"] = summary["Best Share"];
    obj["device_hardware_percent"] = summary["Device Hardware%"];
    obj["device_rejected_percent"] = summary["Device Rejected%"];
    obj["pool_rejected_percent"] = summary["Pool Rejected%"];
    obj["pool_stale_percent"] = summary["Pool Stale%"];
    obj["api_version"] = version["API"];
    obj["cgminer_version"] = version["CGMiner"];
    obj["sgminer_version"] = version["SGMiner"];
    return obj;
}

double Fetcher::findMhashCurrent(const QJsonObject& mhashy)
{
    QStringList keys = mhashy.keys().filter(QRegularExpression("MHS \\ds"));
    return keys.size() == 1 ? mhashy[keys.first()].toDouble() : 0.0f;
}

QJsonObject Fetcher::getDeviceInfo(const QJsonObject& device)
{
    QJsonObject obj;
    obj["temperature"] = device["Temperature"];
    obj["enabled"] = device["Enabled"].toString() == "Y";
    obj["status"] = device["Status"];
    obj["uptime"] = device["Device Elapsed"];
    obj["mhash_average"] = device["MHS av"];
    obj["mhash_current"] = findMhashCurrent(device);
    obj["accepted"] = device["Accepted"];
    obj["rejected"] = device["Rejected"];
    obj["hardware_errors"] = device["Hardware Errors"];
    obj["utility"] = device["Utility"];
    obj["rejected_percent"] = device["Device Rejected%"];
    obj["last_share_pool"] = device["Last Share Pool"];
    obj["last_share_time"] = device["Last Share Time"];
    obj["total_mhash"] = device["Total MH"];
    obj["diff1_work"] = device["Diff1 Work"];
    obj["difficulty_accepted"] = device["Difficulty Accepted"];
    obj["difficulty_rejected"] = device["Difficulty Rejected"];
    obj["last_share_difficulty"] = device["Last Share Difficulty"];
    obj["last_valid_work"] = device["Last Valid Work"];
    return obj;
}

QJsonObject Fetcher::getAsicInfo(const QJsonObject& device)
{
    QJsonObject obj = getDeviceInfo(device);
    obj["index"] = device["ASC"];
    return obj;
}

QJsonObject Fetcher::getFpgaInfo(const QJsonObject& device)
{
    QJsonObject obj = getDeviceInfo(device);
    obj["index"] = device["PGA"];
    obj["frequency"] = device["Frequency"];
    return obj;
}

QJsonObject Fetcher::getGpuInfo(const QJsonObject& device)
{
    QJsonObject obj = getDeviceInfo(device);
    obj["index"] = device["GPU"];
    obj["fan_speed"] = device["Fan Speed"];
    obj["fan_percent"] = device["Fan Percent"];
    obj["gpu_clock"] = device["GPU Clock"];
    obj["memory_clock"] = device["Memory Clock"];
    obj["gpu_voltage"] = device["GPU Voltage"];
    obj["gpu_activity"] = device["GPU Activity"];
    obj["powertune"] = device["Powertune"];
    obj["intensity"] = device["Intensity"];
    return obj;
}

QJsonObject Fetcher::getPoolInfo(const QJsonObject& pool)
{
    QJsonObject obj;
    obj["index"] = pool["POOL"];
    obj["url"] = pool["URL"];
    obj["status"] = pool["Status"];
    obj["priority"] = pool["Priority"];
    obj["quota"] = pool["Quota"];
    obj["longpoll"] = pool["Long Poll"].toString() == "Y";
    obj["getworks"] = pool["Getworks"];
    obj["accepted"] = pool["Accepted"];
    obj["rejected"] = pool["Rejected"];
    obj["works"] = pool["Works"];
    obj["discarded"] = pool["Discarded"];
    obj["stale"] = pool["Stale"];
    obj["get_failures"] = pool["Get Failures"];
    obj["remote_failures"] = pool["Remote Failures"];
    obj["user"] = pool["User"];
    obj["last_share_time"] = pool["Last Share Time"];
    obj["diff1_shares"] = pool["Diff1 Shares"];
    obj["proxy_type"] = pool["Proxy Type"];
    obj["proxy"] = pool["Proxy"];
    obj["difficulty_accepted"] = pool["Difficulty Accepted"];
    obj["difficulty_rejected"] = pool["Difficulty Rejected"];
    obj["difficulty_stale"] = pool["Difficulty Stale"];
    obj["last_share_difficulty"] = pool["Last Share Difficulty"];
    obj["has_stratum"] = pool["Has Stratum"];
    obj["stratum_url"] = pool["Stratum URL"];
    obj["has_gbt"] = pool["Has GBT"];
    obj["best_share"] = pool["Best Share"];
    obj["active"] = pool["Stratum Active"];
    obj["pool_rejected_percent"] = pool["Pool Rejected%"];
    obj["pool_stale_percent"] = pool["Pool Stale%"];
    return obj;
}
