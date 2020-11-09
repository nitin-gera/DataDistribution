#ifndef INTERFACEMANAGER_H
#define INTERFACEMANAGER_H

#include <QObject>
#include <QDir>
#include <QDomDocument>
#include <QHostAddress>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QDebug>

typedef struct
{
    QHostAddress    ip_address;
    quint16         port;
}STRUCT_ADDRESS;

class InterfaceManager : public QObject
{
    Q_OBJECT
public:
    explicit InterfaceManager(QObject *parent = nullptr);

    bool Initialise();

private:
    QString m_qstr_FileName;
    QString m_qstr_InputInterfaceName, m_qstr_OutputInterfaceName;

    STRUCT_ADDRESS        m_InputAddress;
    QList<STRUCT_ADDRESS> m_qlst_OutputAddress;

    QUdpSocket          *m_IncomingSocket, *m_OutgoingSocket;
signals:

private slots:
    bool ReadNetWorkXML();
    bool ConnectSocket();

public slots:

protected slots:
    void readPendingDatagrams(void);
};

#endif // INTERFACEMANAGER_H
