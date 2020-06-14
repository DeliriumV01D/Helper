#include <viewer.h>
#include <iostream>
#include <fstream>
#include <QMessageBox>
#include <string>

using namespace std;

//#include <qplatformnativeinterface.h>

//#define MESSAGE_NOTIFY WM_USER + 2

Viewer :: Viewer(QWidget *parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags)
{
  setupUi(this);
  
  connect(btnGetConnectionString, SIGNAL(clicked()), this, SLOT(OnBtnGetConnectionString()));
  connect(btnConnect, SIGNAL(clicked()), this, SLOT(OnBtnConnect()));
  connect(btnControl, SIGNAL(clicked()), this, SLOT(OnBtnControl()));
  
  axRDPClientWidget->setControl("{32BE5ED2-5C86-480F-A914-0FF8885A1B3F}");
  
  HelperClient = new THelperClient(124, "127.0.0.1", 8086);
  connect(HelperClient, SIGNAL(ConnectionStringReceived(QString &)), this, SLOT(OnConnectionStringReceived(QString &)));
}

Viewer :: ~Viewer()
{
  delete HelperClient;
};

//SLOTS

void Viewer :: OnBtnGetConnectionString()
{
  HelperClient->GetConnectionString(123);
};

void Viewer :: OnBtnConnect()
{
  //Получение приглашения
  string ticket;
	//std::ifstream if_stream;  
	//if_stream.open("D:/Delirium/Helper/Sharer/inv.xml");
 // if_stream.seekg(0, ios_base::end);
 // int l = if_stream.tellg();
 // if_stream.seekg(0, ios_base::beg);
	//ticket.resize(l);
 // if_stream.read((char*)&(*ticket.begin()), l);
	//if_stream.close();
  ticket = ConnectionString.toStdString();
  std::wstring wticket(ticket.begin(), ticket.end());

  wchar_t * auth_string = L"Sharer",
					* group = L"HeplerGroup",
					* pass = L"";
  //try {
  
  axRDPClientWidget->dynamicCall(
    "Connect(QString,QString,QString)", 
    QString().fromStdString(ticket),
    QString("HeplerGroup"),
    QString("")
  );
 
  //} catch (exception &e) {
  //   QMessageBox(
  //     QMessageBox::Critical,
  //     tr(""),
  //     tr(e.what()),       
  //     QMessageBox::Ok       
  //   );
  //};
};

void Viewer :: OnBtnControl()
{
  axRDPClientWidget->dynamicCall(
    "RequestControl(3)"
  );
};

void Viewer :: OnConnectionStringReceived(QString &connection_string)
{
  ConnectionString = connection_string;
  textEdit->setPlainText(ConnectionString);
};