#include <QtCore/QCoreApplication>
#include <THelperServer.h>

int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);

  quint16 port = (argc > 1) ?
    QString::fromLocal8Bit(argv[1]).toUShort() : 8086;
  THelperServer * server = new THelperServer(port, &a);

  if (!server->isListening()) {
    //logMessage(QString("Failed to bind to port %1").arg(daemon->serverPort()), QtServiceBase::Error);
    a.quit();
  }

  return a.exec();
  delete server;
}
