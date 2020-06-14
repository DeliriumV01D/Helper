/************************************************/
//THelperClient.h
/************************************************/

#ifndef THELPER_CLIENT_H
#define THELPER_CLIENT_H

#include <QObject.h>
#include <QNetworkAccessManager.h>
#include <THelperProtocol.h>



class THelperClient : public QObject { //: public QTcpSocket{
Q_OBJECT
protected:
  bool Finished;
  int ID, ServerPort;
  QString ServerIP;
  QNetworkAccessManager * NetworkAccessManager;
  void ConnectSignalsWithSlots();
public:
  THelperClient(int id, const QString server_ip, int server_port);
  ~THelperClient();

  bool IsFinished(){return Finished;};
  void PostConnectionString(const QString &connection_string);
  void GetConnectionString(const int requested_client_id);
public slots:
  void OnFinished(QNetworkReply * network_replay);
signals:
  void ConnectionStringReceived(QString &connection_string);
};

#endif