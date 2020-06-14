/************************************************/
//THTTPServer.cpp
/************************************************/

#include <THTTPServer.h>

THTTPServer :: THTTPServer(quint16 port, QObject* parent/* = 0*/) : QTcpServer(parent), Disabled(false)
{
  listen(QHostAddress::Any, port);
};

void THTTPServer :: incomingConnection(int socket)
{
  if (Disabled) return;

  // When a new client connects, the server constructs a QTcpSocket and all
  // communication with the client is done over this QTcpSocket. QTcpSocket
  // works asynchronously, this means that all the communication is done
  // in the two slots readClient() and discardClient().
  QTcpSocket * temp_socket = new QTcpSocket(this);
  connect(temp_socket, SIGNAL(readyRead()), this, SLOT(readClient()));
  connect(temp_socket, SIGNAL(disconnected()), this, SLOT(discardClient()));
  temp_socket->setSocketDescriptor(socket);

  //QtServiceBase::instance()->logMessage("New Connection");
};

void THTTPServer :: Pause()
{
  Disabled = true;
};

void THTTPServer :: Resume()
{
  Disabled = false;
};

void THTTPServer :: readClient()
{
  if (Disabled) return;

  // This slot is called when the client sent data to the server. The
  // server looks if it was a get request and sends a very simple HTML
  // document back.
  QTcpSocket* socket = (QTcpSocket*)sender();
  if (!socket->canReadLine()) return;

  QStringList tokens = QString(socket->readLine()).split(QRegExp("[ \r\n][ \r\n]*"));
  for (int i = 0; i < tokens.size(); i++)
    std::cout <<(tokens[i]).toStdString()<<" ";
  std::cout<<std::endl;


  if (tokens[0] == "GET") 
  {
    QTextStream text_stream(socket);
    text_stream.setAutoDetectUnicode(true);
    text_stream << "HTTP/1.0 200 Ok\r\n"
      "Content-Type: text/html; charset=\"utf-8\"\r\n"
      "\r\n"
      "<h1>Nothing to see here</h1>\n"
      << QDateTime::currentDateTime().toString() << "\n";
    socket->close();

    //QtServiceBase::instance()->logMessage("Wrote to client");

    if (socket->state() == QTcpSocket::UnconnectedState) 
    {
      delete socket;
      //QtServiceBase::instance()->logMessage("Connection closed");
    }
  }
};

void THTTPServer :: discardClient()
{
  QTcpSocket* socket = (QTcpSocket*)sender();
  socket->deleteLater();

  //QtServiceBase::instance()->logMessage("Connection closed");
};

