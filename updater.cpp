#include "updater.h"
#include "cgminerapi.h"
#include "fetcher.h"

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
            // XXX notify port conversion error? log?
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

    Q_FOREACH(HostInformation miner, hosts)
    {
        try
        {
            Fetcher fetcher(miner, this);
            updates.append(fetcher.getUpdate(systemInfo));
        }
        catch (std::exception& e)
        {
            qDebug() << "Exception occurred building '" << miner.host << "' update:" << e.what();
        }
    }

    return QJsonDocument(updates);
}
