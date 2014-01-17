#include "updater.h"
#include "cgminerapi.h"

#include <QSettings>

#include <stdexcept>

Updater::Updater(QObject *parent) : QObject(parent), hosts(), token(), systemInfo(this)
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

    token = settings.value("api-key").toString();
}

Updater::~Updater()
{
    /* do nothing */
}

void Updater::update()
{
    qDebug() << "Updater::update()";

    QList<HostInformation> hosts;

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
            QJsonArray updates;
            updates.append(getUpdate(info));

            QUrl url("http://localhost:3000/api/v1/hosts");
            QNetworkRequest request(url);
            request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

            QUrlQuery query;
            query.addQueryItem("token", token);
            query.addQueryItem("updates", QString(QJsonDocument(updates).toJson()));

            //qDebug() << query.query(QUrl::FullyEncoded);
            url.setQuery(query);

            QByteArray paramsb;

            qDebug() << "About to post: " << url;

            QNetworkAccessManager nwam(this);
            QNetworkReply *reply = nwam.post(request, paramsb);

            if (!reply->waitForBytesWritten(5000))
            {
                QString message = "Failed to write command (%1)";
                throw std::runtime_error(message.arg(reply->errorString()).toStdString());
            }

            /* build up the response until no more data is received */
            QByteArray response;
            while (reply->waitForReadyRead(5000))
            {
                response.append(reply->readAll());

                if (reply->isFinished())
                {
                    qDebug() << "Finished!!";
                    break;
                }
            }

            qDebug() << response;
        }
        catch (std::exception& e)
        {
            qDebug() << "An exception occurred:" << e.what();
        }
    }
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
    }
    update["gpus"] = gpus;
    update["asics"] = asics;
    update["fpgas"] = fpgas;

    // XXX pools

    // XXX use VERSION information

    return update;
}
