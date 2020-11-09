#include <QCoreApplication>
#include "InterfaceManager.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    InterfaceManager    interface;
    if(interface.Initialise() == false)
        return 1;

    return a.exec();
}
