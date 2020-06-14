/************************************************/
//THTTPServer.h
/************************************************/

#ifndef THTTPSERVER_H
#define THTTPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
//#include <QtService>
#include <QRegExp>
#include <QDateTime>
#include <QStringList>

#include <iostream>


// HttpDaemon is the the class that implements the simple HTTP server.
class THTTPServer : public QTcpServer {
Q_OBJECT
protected:
  bool Disabled;
public:
  THTTPServer(quint16 port, QObject* parent = 0);

  void incomingConnection(int socket);

  void Pause();
  void Resume();

private slots:

  virtual void readClient();
  void discardClient();
};

#endif