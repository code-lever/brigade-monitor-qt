#include "cgminerapi.h"

CGMinerAPI::CGMinerAPI(const QHostAddress& host, short port) :
    host(host), port(port)
{
}

CGMinerAPI::~CGMinerAPI()
{
    /* do nothing */
}
