#ifndef CGMINERAPI_H
#define CGMINERAPI_H

#include <QtNetwork>

class CGMinerAPI
{
public:
    CGMinerAPI(const QHostAddress& host, short port);
    virtual ~CGMinerAPI();

private:
    QHostAddress host;
    short port;
};

#endif // CGMINERAPI_H
