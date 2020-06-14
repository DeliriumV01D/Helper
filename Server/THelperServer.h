/************************************************/
//THelperServer.h
/************************************************/
#ifndef THELPER_SERVER_H
#define THELPER_SERVER_H

#include <THTTPServer.h>
#include <THelperProtocol.h>
#include <vector>

struct TClient
{
  int ID;
  QString ConnectionString;
};

class THelperServer : public THTTPServer{
private:
  std::vector <TClient> Clients; //Лучше использовать какой-нибудь map, для быстрого поиска по ID
  void ProcessRequest(QTcpSocket* socket);
public:
  THelperServer(quint16 port, QObject* parent = 0);
private slots:
  void readClient();
};

#endif