#include <THelperServer.h>

THelperServer :: THelperServer(quint16 port, QObject* parent/* = 0*/) : THTTPServer(port, parent)
{
  Clients.clear();
};

void THelperServer :: ProcessRequest(QTcpSocket* socket)
{
  if (!socket->canReadLine()) return;
  TClient temp_client;

  //Ïàðñèì çàïðîñ
  QStringList tokens = QString(socket->readLine()).split(QRegExp("[ \r\n&?][ \r\n&?]*"));
  if (tokens.size() == 0) return;

  if (tokens[0] == "GET") 
  {
    int request_type = 0,
        requested_client_id = 0;

    for (int i = 0; i < tokens.size(); i++)
    {
      if (tokens[i].startsWith("ID="))
        temp_client.ID = (tokens[i].mid(3,-1)).toInt();

      if (tokens[i].startsWith("REQUEST_TYPE="))
        request_type = (tokens[i].mid(13,-1)).toInt();
      
      if (tokens[i].startsWith("CONNECTION_STRING="))
        temp_client.ConnectionString = tokens[i].mid(18,-1);

      if (tokens[i].startsWith("REQUESTED_CLIENT_ID="))
        requested_client_id = (tokens[i].mid(20,-1)).toInt();
    };

    //
    Clients.push_back(temp_client);
    

    QTextStream text_stream(socket);
    text_stream.setAutoDetectUnicode(true);
    text_stream << "HTTP/1.0 200 Ok\r\n"
      "Content-Type: text/html; charset=\"utf-8\"\r\n"
      "\r\n"
      "REQUEST_PROCESSED "
      << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << "\n";
    
    if (request_type == REQUEST_TYPE_GET_CONNECTION_STRING)
    {
      QString temp_string;
      for (int i = Clients.size() - 1; i >= 0; i--)
      {
        if (Clients[i].ID == requested_client_id) 
        {
          temp_string = Clients[i].ConnectionString;
          break;
        };
      };
      text_stream << "CONNECTION_STRING=" << temp_string<<"\n";
    };
    
    socket->close();

    //QtServiceBase::instance()->logMessage("Wrote to client");
  };
};

//SLOTS

void THelperServer :: readClient() 
{
  if (Disabled) return;

  QTcpSocket* socket = (QTcpSocket*)sender();
  ProcessRequest(socket);

  if (socket->state() == QTcpSocket::UnconnectedState) 
      delete socket;
};
