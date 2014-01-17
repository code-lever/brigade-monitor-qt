#include "updater.h"
#include "cgminerapi.h"
#include "qtsysteminfo.h"

#include <QSettings>

struct HostInformation
{
    QString name;
    QString host;
    quint16 port;
};

void Updater::update()
{
    static QtSystemInfo systemInfo;

    qDebug() << "Updater::update()";

    QList<HostInformation> hosts;
    QString token;

    {
        QSettings settings;
        int size = settings.beginReadArray("monitored-hosts");
        for (int i = 0; i < size; ++i) {
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

            if (ok)
            {
                hosts.append(info);
                qDebug() << "Appended: " << info.name << ", " << info.host << ", " << info.port;
            }
            else
            {
                // XXX notify port error?
            }
        }
        settings.endArray();

        token = settings.value("api-key").toString();
    }

    if (token.isEmpty())
    {
        qDebug() << "TOKEN EMPTY, skipping update";
        return;
    }

    qDebug() << "Built host list, " << hosts.length() << " entries";

    Q_FOREACH(HostInformation info, hosts)
    {
        try
        {
            CGMinerAPI api(info.host, info.port);
            QJsonDocument version = api.version();
            QJsonDocument summary = api.summary();
            QJsonDocument devs = api.devs();
            QJsonDocument pools = api.pools();

            QJsonObject update;
            update["host"] = info.name;
            update["uptime"] = summary.object()["SUMMARY"].toArray()[0].toObject()["Elapsed"];
            update["mhash"] = summary.object()["SUMMARY"].toArray()[0].toObject()["MHS av"];
            update["rejectpct"] = summary.object()["SUMMARY"].toArray()[0].toObject()["Pool Rejected%"];

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
            }
            update["gpus"] = gpus;
            update["asics"] = asics;
            update["fpgas"] = fpgas;

            // XXX pools

            // XXX use VERSION information

            QJsonObject agent;
            agent["name"] = QString("brigade-monitor-qt");
            agent["platform"] = systemInfo.getSystemInformation();
            agent["version"] = QString(stringify(APP_VERSION));
            update["agent"] = agent;

            qDebug() << "update: " << update;
        }
        catch (std::exception& e)
        {
            qDebug() << "An exception occurred:" << e.what();
        }
    }
}
