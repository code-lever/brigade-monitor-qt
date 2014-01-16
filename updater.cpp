#include "updater.h"
#include "cgminerapi.h"

#include <QSettings>
#include <QtNetwork>

struct HostInformation
{
    QString name;
    QString host;
    short port;
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
        HostInformation info = { name, host, port.toShort(&ok) };

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
}
