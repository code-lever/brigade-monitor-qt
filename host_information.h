#ifndef HOST_INFORMATION_H
#define HOST_INFORMATION_H

#include <QString>
#include <QtGlobal>

struct HostInformation
{
    QString name;
    QString host;
    quint16 port;
};

#endif // HOST_INFORMATION_H
