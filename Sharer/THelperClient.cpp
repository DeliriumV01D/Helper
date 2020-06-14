#include <THelperClient.h>
#include <QUrl.h>
#include <QNetworkRequest.h>
#include <QNetworkReply.h>

THelperClient :: THelperClient(
  const int id, 
  const QString server_ip,
  const int server_port
){
  ID = id;
  ServerIP = server_ip;
  ServerPort = server_port;
  Finished = true;

  NetworkAccessManager = new QNetworkAccessManager();

  ConnectSignalsWithSlots();
};

THelperClient :: ~THelperClient()
{
  delete NetworkAccessManager;
};

void THelperClient :: ConnectSignalsWithSlots()
{
  connect(NetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(OnFinished(QNetworkReply*)));
};

void THelperClient :: PostConnectionString(const QString &connection_string)
{
  QUrl url(
    "http://" + 
    ServerIP + ":" + 
    QString::number(ServerPort) + 
    "?" + 
    "ID=" + QString::number(ID) + "&" +
    "REQUEST_TYPE=" + QString::number(REQUEST_TYPE_POST_CONNECTION_STRING) + "&" +
    "CONNECTION_STRING=" + connection_string.toUtf8().toHex()
  );
  QNetworkReply * reply = NetworkAccessManager->get(QNetworkRequest(url));
  //QNetworkReply * reply = NetworkAccessManager->post(
  //  QNetworkRequest(url), 
  //  QByteArray("action=do_smth&param=value")
  //);
  Finished = false;
};

void THelperClient :: GetConnectionString(const int requested_client_id)
{
  QUrl url(
    "http://" + 
    ServerIP + ":" + 
    QString::number(ServerPort) + 
    "?" + 
    "ID=" + QString::number(ID) + "&" +
    "REQUEST_TYPE=" + QString::number(REQUEST_TYPE_GET_CONNECTION_STRING) + "&" +
    "REQUESTED_CLIENT_ID=" + QString::number(requested_client_id)
  );
  QNetworkReply * reply = NetworkAccessManager->get(QNetworkRequest(url));
  Finished = false;
};

//SLOTS

void THelperClient :: OnFinished(QNetworkReply * network_replay)
{
  // Не произошло-ли ошибки?
  if (network_replay->error() == QNetworkReply::NoError)
  {
    // Читаем ответ от сервера
    QStringList tokens = QString(network_replay->readAll()).split(QRegExp("[ \r\n][ \r\n]*"));
    
    for (int i = 0; i < tokens.size(); i++)
    {
      if (tokens[i].startsWith("CONNECTION_STRING="))
      {
        QString temp_string = QByteArray().fromHex(tokens[i].mid(18,-1).toUtf8());
        emit ConnectionStringReceived(temp_string);
      }
    };

  }
  // Произошла какая-то ошибка
  else
  {
    // обрабатываем ошибку
    qDebug() << network_replay->errorString();
  };
  Finished = true;
  delete network_replay;
};
