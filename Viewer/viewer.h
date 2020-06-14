#ifndef VIEWER_H
#define VIEWER_H

#include <QMainWindow>
#include <ui_viewer.h>
#include <THelperClient.h>

class Viewer : public QMainWindow,  public Ui::ViewerClass
{
	Q_OBJECT
private:
  QString ConnectionString;
  THelperClient * HelperClient;
public:
	Viewer(QWidget *parent = 0, Qt::WindowFlags flags = 0);
	~Viewer();
private slots:
  void OnBtnGetConnectionString();
	void OnBtnConnect();
	void OnBtnControl();
  void OnConnectionStringReceived(QString &connection_string);
};

#endif // VIEWER_H
