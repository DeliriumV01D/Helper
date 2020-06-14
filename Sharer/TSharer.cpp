#include "TSharer.h"
#include "WriteToLog.h"

#include <QAction>
#include <QMenu>
#include <QTextCodec>
#include <QApplication>
#include <QString>
#include <fstream>

/*****************************************************************/
//TRDPSessionEvents
//Класс для получения событий (Подключение, Запрос управления, Отключение)
//от интерфейса _IRDPSessionEvents и передачи их классу TSharer
/*****************************************************************/

TRDPSessionEvents :: TRDPSessionEvents()
{
	refNum = 0;
	AddRef();
}

TRDPSessionEvents :: ~TRDPSessionEvents()
{
}

// IUnknown Methods
HRESULT _stdcall TRDPSessionEvents :: QueryInterface(REFIID iid,void**ppvObject)
{
	*ppvObject = 0;
	if (iid == IID_IUnknown || iid == IID_IDispatch || iid == __uuidof(_IRDPSessionEvents))
		*ppvObject = this;
	if (*ppvObject)
		{
		((IUnknown*)(*ppvObject))->AddRef();
		return S_OK;
		}
	return E_NOINTERFACE;
}

ULONG _stdcall TRDPSessionEvents :: AddRef()
{
	refNum++;
	return refNum;
}

ULONG _stdcall TRDPSessionEvents :: Release()
{
	refNum--;
	if (!refNum)
		{
		delete this;
		return 0;
		}
	return refNum;
}

// IDispatch Methods
HRESULT _stdcall TRDPSessionEvents :: GetTypeInfoCount(unsigned int * pctinfo) 
{
return E_NOTIMPL;
}

HRESULT _stdcall TRDPSessionEvents :: GetTypeInfo(unsigned int iTInfo,LCID lcid,ITypeInfo FAR* FAR* ppTInfo) 
{
	return E_NOTIMPL;
}

HRESULT _stdcall TRDPSessionEvents :: GetIDsOfNames(
REFIID riid,
OLECHAR FAR* FAR*,
unsigned int cNames,
LCID lcid,
DISPID FAR* ) 
{
	return E_NOTIMPL;
}

//Привязка событий к TSharer
void TRDPSessionEvents :: SetNotification(HWND n, UINT m, TRDPServer * p)
{
	nen = n;
	msg = m;
	Parent = p;
}

HRESULT _stdcall TRDPSessionEvents :: Invoke(
									DISPID dispIdMember,
									REFIID riid,
									LCID lcid,
									WORD wFlags,
									DISPPARAMS FAR* pDispParams,
									VARIANT FAR* pVarResult,
									EXCEPINFO FAR* pExcepInfo,
									unsigned int FAR* puArgErr)
{
	LRESULT rl = 0;
	if (nen && msg)
		rl = SendMessage(nen,msg,dispIdMember,(LPARAM)pDispParams);
	if (rl == 1)
		return S_OK;
	// Else, default implementation
	Parent->HandleNotification(dispIdMember,pDispParams);
	if (nen && msg)
		SendMessage(nen,msg,dispIdMember,0);
	return S_OK;
}

/*****************************************************************/
//TRDPServer
//Класс для обеспечения доступа к данному компьютеру через RDP
/*****************************************************************/
TRDPServer :: TRDPServer()
{
  pRDPSession = 0;
	pIConnectionPointContainer = 0;
	pIConnectionPoint = 0;
	//Инициализация COM 
	//CoInitialize(NULL);
	CoInitializeEx(0, COINIT_APARTMENTTHREADED);
};

TRDPServer :: ~TRDPServer()
{
  CloseSession();

	//Финализация COM 
	CoUninitialize();
};

void TRDPServer :: OpenSession()
{
	if (pRDPSession) return;
	HRESULT hr;

	CLSID c1 = __uuidof(RDPSession);
	CLSID c2 = __uuidof(IRDPSRAPISharingSession);
	hr = CoCreateInstance(c1, NULL, CLSCTX_ALL, c2, (LPVOID*)&pRDPSession);	
	if(FAILED(hr)) throw(exception("Rdpcomapi.RDPSession not registered"));

  hr = pRDPSession->Open();	
	if(FAILED(hr)) throw(exception("Unable to open RDP session"));

  SetNotification(/*this->winId()*/0, 0, true);
};

QString TRDPServer :: GetConnectionString()
{
	HRESULT hr;
	
	//IRDPSRAPIInvitation* inv = 0;
	//HRESULT hr = invm->CreateInvitation(B(str),B(grp),B(pwd),limit,&inv);

	wchar_t * auth_string = L"Sharer",
					* group = L"HeplerGroup",
					* pass = L"";
	BSTR		connection_string;

  hr = pRDPSession->get_Invitations(&pInvitationManager);
	if(FAILED(hr)) throw(exception("Unable to get invitation manager"));

	long L = 1;
	hr = pInvitationManager->CreateInvitation(
		NULL,//auth_string,
		RAS::B(group),
		RAS::B(pass),
		L,
		&pInvitation
	);
  if(FAILED(hr)) throw(exception("Unable to create invitation"));

	hr = pInvitation->get_ConnectionString(&connection_string);
	if(FAILED(hr)) throw(exception("Unable to get connection string"));
	
	//В Qt 5.1.1 почему то не работает fromWCharArray
  QString result;
  std::wstring wstr(connection_string);
  std::string str(wstr.begin(), wstr.end());
  result = QString::fromStdString(str);
  //result = QString::fromStdWString(std::wstring(connection_string));
	//result = QString::fromWCharArray(connection_string);

	return result;
};

void TRDPServer :: CloseSession()
{
	//Поскольку после закрытия данную сессию нельзя повторно использовать, 
	//Сразу освобождаем объект
	if (pRDPSession)
	{
		IRDPSRAPISharingSession* ss = (IRDPSRAPISharingSession*)pRDPSession;
		SetNotification(0, 0, false);
		ss->Close();
		ss->Release();
		pRDPSession = 0;
	};
};

bool TRDPServer :: SetNotification(HWND en, UINT msg, bool A)
{
	SessionEvents.SetNotification(en, msg, this);
	IRDPSRAPISharingSession* ss = (IRDPSRAPISharingSession*)pRDPSession;
	if (!ss)
		return false;

	if (A && pIConnectionPointContainer && pIConnectionPoint)
		return true;
	if (!A && !pIConnectionPointContainer && !pIConnectionPoint)
		return true;

	if (A)
	{
		//nen = en;
		//men = msg;
		Cookie = RAS::RDPConnectObject(
			(IUnknown*)ss,
			__uuidof(_IRDPSessionEvents),
			(IUnknown*)&SessionEvents,
			&pIConnectionPointContainer,
			&pIConnectionPoint
		);
		
		/*Cookie = RAS :: RDPConnectObject(
			(IUnknown*)pRDPSession,
			__uuidof(_IRDPSessionEvents),
			(IUnknown*)&SessionEvents,
			&pIConnectionPointContainer,
			&pIConnectionPoint
		);*/

		if (pIConnectionPoint && pIConnectionPointContainer)
			return true;
		return false;
	}	else {
		RAS :: RDPDisconnectObject(
			pIConnectionPointContainer, 
			pIConnectionPoint, 
			Cookie
		);
		pIConnectionPoint = 0;
		pIConnectionPointContainer = 0;
		return true;
	}
};

void TRDPServer :: HandleNotification(DISPID id, DISPPARAMS * p)
{
	IDispatch* j = 0;
	//IUnknown* ju = 0;
	VARIANT x;
	unsigned int y = 0;
	HRESULT hr = 0;

	switch(id)
	{
		case DISPID_RDPSRAPI_EVENT_ON_ATTENDEE_CONNECTED:
		{
		  x.vt = VT_DISPATCH;
			x.pdispVal = j;
			x.vt = VT_DISPATCH;
			hr = DispGetParam(p, 0, VT_DISPATCH, &x, &y);
			if (FAILED(hr))
				return;
			IDispatch* d = x.pdispVal;
			if (!d)
				return;

			//Пока разрешено только одно подключение
			Attendee = new RAS::ATTENDEE(d);
			//RAS::ATTENDEE a(d);
			d->Release();
			//Atts.push_back(a);
			break;
		};

		case DISPID_RDPSRAPI_EVENT_ON_ATTENDEE_DISCONNECTED:
		{
			x.vt = VT_DISPATCH;
			x.pdispVal = j;
			hr = DispGetParam(p, 0, VT_DISPATCH, &x, &y);
			if (FAILED(hr))
				return;
			IDispatch* d = x.pdispVal;
			if (!d)
				return;

			IRDPSRAPIAttendeeDisconnectInfo* a = 0;
			d->QueryInterface(__uuidof(IRDPSRAPIAttendeeDisconnectInfo), (void**)&a);
			if (!a)
				return;

			IRDPSRAPIAttendee* at = 0;
			a->get_Attendee(&at);
			if (at)
			{
				Attendee->Release();
				//RemoveAtt(at);
				at->Release();
			};

			a->Release();
			break;
		};

		case DISPID_RDPSRAPI_EVENT_ON_CTRLLEVEL_CHANGE_REQUEST:
		{
			x.vt = VT_INT;
			x.pdispVal = j;
			hr = DispGetParam(p, 1, VT_INT, &x, &y);
			if (FAILED(hr))
				return;
			int Lev = x.intVal;
			x.vt = VT_INT;
			x.pdispVal = j;
			hr = DispGetParam(p, 0, VT_DISPATCH, &x, &y);
			if (FAILED(hr))
				return;
			IDispatch* d = x.pdispVal;
			if (!d)
				return;
			IRDPSRAPIAttendee* a = 0;
			d->QueryInterface(__uuidof(IRDPSRAPIAttendee), (void**)&a);
			if (!a)
				return;

			//ATTENDEE* sa = FindAtt(a);
			//if (sa)
			//	sa->SetControl(Lev);
			Attendee->SetControl(Lev);
			a->Release();
			break;
		};
	};	//switch
};

/*****************************************************************/
//TRDPSessionEvents
//Пользовательский интерфейс
/*****************************************************************/

TSharer :: TSharer()
{
	//Устанавливаем локаль для перевода
	QLocale::setDefault(QLocale(QLocale::Russian, QLocale::RussianFederation));
  QTextCodec::setCodecForLocale(QTextCodec::codecForName("cp1251"));

	//QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());  // Для внутренних преобразований Qt
	//QTextCodec::setCodecForTr(QTextCodec::codecForName("Windows-1251"));// Для функций перевода tr()

	this->setupUi(this);
	Init();
	TrayIcon.show();

  HelperClient = new THelperClient(123, "127.0.0.1", 8086);
};

TSharer :: ~TSharer()
{
  delete HelperClient;
	TrayIcon.hide();
	//Действия и меню уничтожатся вместе с объектом Sharer
};

void TSharer :: CreateActionsAndMenus()
{
	QAction * ac_open_session = new QAction(QTextCodec::codecForLocale()->toUnicode("Открыть сессию") /*, icon*/, this);
	connect(ac_open_session, SIGNAL(triggered()), this, SLOT(OnActionOpenSession()));

	QAction * ac_close_session = new QAction(QTextCodec::codecForLocale()->toUnicode("Закрыть сессию") /*, icon*/, this);
	connect(ac_close_session, SIGNAL(triggered()), this, SLOT(OnActionCloseSession()));

  QAction * ac_send_inv = new QAction(QTextCodec::codecForLocale()->toUnicode("Отослать приглашение") /*, icon*/, this);
	connect(ac_send_inv, SIGNAL(triggered()), this, SLOT(OnActionSendInv()));

	QAction * ac_exit = new QAction(QTextCodec::codecForLocale()->toUnicode("Выход") /*, icon*/, this);
	connect(ac_exit, SIGNAL(triggered()), this, SLOT(OnActionExit()));

	QMenu * tray_icon_menu = new QMenu(this);
  tray_icon_menu->addAction(ac_open_session);
	tray_icon_menu->addAction(ac_close_session);
  tray_icon_menu->addAction(ac_send_inv);
	tray_icon_menu->addAction(ac_exit);

  TrayIcon.setContextMenu(tray_icon_menu);
};

void TSharer :: Init()
{
	CreateActionsAndMenus();
};

//
//SLOTS
//


void TSharer :: OnActionOpenSession()
{
  QString connection_string;
  try {
    OpenSession();
    connection_string = GetConnectionString();
  } catch (exception &e) {
    TrayIcon.showMessage(
			"Ошибка", 
			QString(e.what()), 
			QSystemTrayIcon::Critical, 
			10000
		);
  };

  HelperClient->PostConnectionString(connection_string);

	std::filebuf of_buffer;
	std::ostream out_stream(&of_buffer);

	of_buffer.open("inv.xml", std::ios::out|std::ios::trunc);					//Запись
	out_stream<<connection_string.toStdString();
	
	out_stream.clear();
	of_buffer.close();
};

void TSharer :: OnActionCloseSession()
{
	CloseSession();
};

void TSharer :: OnActionSendInv()
{
  HelperClient->PostConnectionString("");
};

void TSharer :: OnActionExit()
{
	QApplication::exit();
};
