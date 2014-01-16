#include "updater.h"
#include "cgminerapi.h"

#include <QSettings>

struct HostInformation
{
    QString name;
    QString host;
    quint16 port;
};

void Updater::update()
{
    qDebug() << "Updater::update()";

    QList<HostInformation> hosts;

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

            qDebug() << "host   : " << info.host;
            qDebug() << "version: " << version;
            qDebug() << "summary: " << summary;
            qDebug() << "devs   : " << devs;
            qDebug() << "pools  : " << pools;
        }
        catch (std::exception& e)
        {
            // XXX bark
        }
    }
}