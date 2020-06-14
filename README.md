# Helper
Simple RDP client-server application written in c++ using Qt and WinAPI

Программа "Помощник" состоит из трех частей, являющихся отдельными программами

Server - отладочный http-сервер для связи Sharer и Viewer

Sharer - Программа клиента. Предназначена для создания RDP-сесии, 
генерации приглашения и отправки приглашения на сервер для последующей передачи
его программе специалиста.

Viewer - Программа специалиста. Умеет запрашивать с сервера приглашение для
подключения к клиенту. Подключается к клиенту с целью управления посредством RDP.

Все программы написаны на языке с++ в Microsoft Visual Studio 2010 с использованием
библиотеки Qt

1. Sharer
Верхний класс в иерархии TSharer - пользовательский интерфейс (модули TSharer.h, TSharer.cpp)
наследуется от QMainWindow, TSharer, TRDPServer
Содержит в качестве делегата THelperClient

TRDPServer (модули TSharer.h, TSharer.cpp) - Класс для обеспечения доступа к данному компьютеру через RDP
Открытие/зыкрытие сессии, генерация приглашения, получение событий от RDP-клиента.

THelperClient (модули THelperClient.h, THelperClient.cpp) - общая часть в программах клиента и 
специалиста для общения с HTTP-сервером по протоколу.

Программа такая разнородная, потому что начал писать ее с нуля, не найдя никаких готовых
библиотек. Но потом все таки нашел RDP.h, RDP.cpp, но для экономии времени работающий кусок было 
решено не переписывать заново. 

RDP.h и RDP.cpp - набор классов, являющихся оберткой вокруг WindowsAPI функций для работы с RDP.

TSharer.ui - GUI форм, пока не используется. (вся работа через трей)

2. Viewer
класс Viewer (модули Viewer.h и Viewer.cpp) наследуется от QMainWindow и Ui::ViewerClass
в качестве делегата имеет THelperClient

Вся его работа сейчас крутится вокруг расположенного на визуальной форме ActiveX компонента
axRDPClientWidget обеспечивающего функционал RDP - клиента.

Визуальный форм - Viewer.ui


3. Server
Консольная программа.

Основной функционал заключен в классе THelperServer(модули THelperServer.h, THelperServer.cpp)
В нем переопределены методы readClient() для получения данных с клиента
и ProcessRequest() для обработки клиентских запросов и генерации ответов сервера.
Класс THelperServer унаследован от класса THTTPServer 

THTTPServer (модули THTTPServer.h, THTTPServer.cpp) 
унаследован от кутэшного QTcpServer

В модуль THelperProtocol.h планируется вынести парсеры клиентских и серверных запросов,
чтобы можно было использовать в разных программах.


ПРОТОКОЛ:


1. PostConnectionString (c->s)

  QUrl url(
    "http://" + 
    ServerIP + ":" + 
    QString::number(ServerPort) + 
    "?" + 
    "ID=" + QString::number(ID) + "&" +
    "REQUEST_TYPE=" + QString::number(REQUEST_TYPE_POST_CONNECTION_STRING) + "&" +
    "CONNECTION_STRING=" + connection_string.toUtf8().toHex()
  );

2. Reply (s->c)

    text_stream << "HTTP/1.0 200 Ok\r\n"
      "Content-Type: text/html; charset=\"utf-8\"\r\n"
      "\r\n"
      "REQUEST_PROCESSED "
      << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << "\n";

3. GetConnectionString  (c2->s)
  
  QUrl url(
    "http://" + 
    ServerIP + ":" + 
    QString::number(ServerPort) + 
    "?" + 
    "ID=" + QString::number(ID) + "&" +
    "REQUEST_TYPE=" + QString::number(REQUEST_TYPE_GET_CONNECTION_STRING) + "&" +
    "REQUESTED_CLIENT_ID=" + QString::number(requested_client_id)
  );

4. Reply (s->c2)
    text_stream << "HTTP/1.0 200 Ok\r\n"
      "Content-Type: text/html; charset=\"utf-8\"\r\n"
      "\r\n"
      "REQUEST_PROCESSED "
      << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << "\n";

  text_stream << "CONNECTION_STRING=" << temp_string<<"\n";
