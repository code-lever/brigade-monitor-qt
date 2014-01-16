#ifndef UPDATER_H
#define UPDATER_H

#include <QtNetwork>

#define stringify(s) stringify_internal(s)
#define stringify_internal(s) #s

class Updater
{
public:
    static void update();

private:
    Updater();
};

#endif // UPDATER_H
