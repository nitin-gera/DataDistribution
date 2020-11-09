#include "InterfaceManager.h"

InterfaceManager::InterfaceManager(QObject *parent) : QObject(parent)
{
    m_qstr_FileName = "NetworkDetails.xml";
    qDebug() << "File to read:" << m_qstr_FileName;
}

bool InterfaceManager::Initialise()
{
    if(ReadNetWorkXML() == false)
        return false;

    if(ConnectSocket() == false)
        return false;

    return true;
}

bool InterfaceManager::ReadNetWorkXML()
{
    QFile file(m_qstr_FileName);

    if(file.open(QIODevice::ReadOnly) == false)
    {
        qWarning() << "Unable to open file = " << file.errorString();
        return false;
    }

    QDomDocument    doc("myDocument");
    if(!doc.setContent(&file))
    {
        qWarning() << "Unable to set content. XML file not properly made.";
        file.close();
        return false;
    }

    file.close();

    QDomElement root_node = doc.documentElement();

    if(root_node.tagName() != "network_details")
    {
        qWarning() << "The given file is not network details file";
        return false;
    }

    QDomNode child = root_node.firstChild();

    while(child.isNull() == false)
    {
        QDomElement root_element = child.toElement();
        if(root_element.tagName() == "input")
        {
            QDomNode sub_child_node = root_element.firstChild();
            while (sub_child_node.isNull() == false)
            {
                if(sub_child_node.hasAttributes())
                {
                    for(quint16 i(0); i < sub_child_node.attributes().size(); i++)
                    {
                        if(sub_child_node.attributes().item(i).nodeName() == "interface")
                        {
                            m_qstr_InputInterfaceName = sub_child_node.attributes().item(i).nodeValue();
                        }
                        else if(sub_child_node.attributes().item(i).nodeName() == "ip_address")
                        {
                            m_InputAddress.ip_address.setAddress(sub_child_node.attributes().item(i).nodeValue());
                        }
                        else if(sub_child_node.attributes().item(i).nodeName() == "port")
                        {
                            m_InputAddress.port = sub_child_node.attributes().item(i).nodeValue().toUShort();
                        }
                    }
                }
                sub_child_node = sub_child_node.nextSibling();
            }
        }
        else if(root_element.tagName() == "output")
        {
            QDomNode sub_child_node = root_element.firstChild();
            while (sub_child_node.isNull() == false)
            {
                if(sub_child_node.hasAttributes())
                {
                    STRUCT_ADDRESS  addr_obj;
                    for(quint16 i(0); i < sub_child_node.attributes().size(); i++)
                    {
                        if(sub_child_node.attributes().item(i).nodeName() == "interface")
                        {
                            m_qstr_OutputInterfaceName = sub_child_node.attributes().item(i).nodeValue();
                        }
                        else if(sub_child_node.attributes().item(i).nodeName() == "ip_address")
                        {
                            addr_obj.ip_address.setAddress(sub_child_node.attributes().item(i).nodeValue());
                        }
                        else if(sub_child_node.attributes().item(i).nodeName() == "port")
                        {
                            addr_obj.port = sub_child_node.attributes().item(i).nodeValue().toUShort();
                        }
                    }

                    m_qlst_OutputAddress.append(addr_obj);
                }
                sub_child_node = sub_child_node.nextSibling();
            }
        }
        child = child.nextSibling();
    }

    qInfo() << "Input Details:" << "Interface = " << m_qstr_InputInterfaceName
            << " IP:" << m_InputAddress.ip_address.toString()
            << " PORT:" << m_InputAddress.port;

    foreach (STRUCT_ADDRESS address, m_qlst_OutputAddress)
    {
        qInfo() << "Output Details:" << "Interface = " << m_qstr_OutputInterfaceName
                << " IP:" << address.ip_address.toString()
                << " PORT:" << address.port;
    }

    return true;
}

bool InterfaceManager::ConnectSocket()
{
    m_IncomingSocket = new QUdpSocket(this);

    if(m_IncomingSocket->bind(QHostAddress::AnyIPv4, m_InputAddress.port, QAbstractSocket::ShareAddress) == false)
    {
        qWarning() << "Unable to Bind:" << m_IncomingSocket->errorString();
        return false;
    }

    if(m_InputAddress.ip_address.isMulticast())
    {
        if(m_IncomingSocket->joinMulticastGroup(m_InputAddress.ip_address) == false)
        {
            qWarning() << "Unable to join multicast group:" << m_IncomingSocket->errorString();
            return false;
        }

        QString routeAdd = "route add -net 224.0.0.0 netmask 240.0.0.0 " + m_qstr_InputInterfaceName;

        system(routeAdd.toLatin1());
    }

    connect(m_IncomingSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));

    m_OutgoingSocket = new QUdpSocket(this);

    return true;
}

void InterfaceManager::readPendingDatagrams()
{
    while(m_IncomingSocket->hasPendingDatagrams())
    {
        QNetworkDatagram inDatagram = m_IncomingSocket->receiveDatagram();

        foreach(STRUCT_ADDRESS address, m_qlst_OutputAddress)
        {
            QNetworkDatagram outDatagram(inDatagram.data(), address.ip_address, address.port);
            m_OutgoingSocket->writeDatagram(outDatagram);
        }
    }
}
