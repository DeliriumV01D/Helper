#ifndef TSHARER_H
#define TSHARER_H

/*

*/

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <ui_TSharer.h>
#include <THelperClient.h>

#include <Rdpencomapi.h>
#include <RDP.h>

class TRDPServer;

//Класс для получения событий (Подключение, Запрос управления, Отключение)
//от интерфейса _IRDPSessionEvents и передачи их классу TRDPServer
class TRDPSessionEvents : public _IRDPSessionEvents {
private:
	int refNum;
	HWND nen;
	UINT msg;
	TRDPServer * Parent;
public:
	TRDPSessionEvents();
	~TRDPSessionEvents();
  //Привязка событий к TSharer
	void SetNotification(HWND, UINT, TRDPServer * p);    

	// IUnknown
  virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
      /* [in] */ REFIID riid,
      /* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject);

  virtual ULONG STDMETHODCALLTYPE AddRef( void);

  virtual ULONG STDMETHODCALLTYPE Release( void);


	// IDispatch
	virtual HRESULT STDMETHODCALLTYPE GetTypeInfoCount( 
        /* [out] */ __RPC__out UINT *pctinfo);
    
	virtual HRESULT STDMETHODCALLTYPE GetTypeInfo( 
        /* [in] */ UINT iTInfo,
        /* [in] */ LCID lcid,
        /* [out] */ __RPC__deref_out_opt ITypeInfo **ppTInfo);
    
  virtual HRESULT STDMETHODCALLTYPE GetIDsOfNames( 
        /* [in] */ __RPC__in REFIID riid,
        /* [size_is][in] */ __RPC__in_ecount_full(cNames) LPOLESTR *rgszNames,
        /* [range][in] */ UINT cNames,
        /* [in] */ LCID lcid,
        /* [size_is][out] */ __RPC__out_ecount_full(cNames) DISPID *rgDispId);
    
  virtual /* [local] */ HRESULT STDMETHODCALLTYPE Invoke( 
        /* [in] */ DISPID dispIdMember,
        /* [in] */ REFIID riid,
        /* [in] */ LCID lcid,
        /* [in] */ WORD wFlags,
        /* [out][in] */ DISPPARAMS *pDispParams,
        /* [out] */ VARIANT *pVarResult,
        /* [out] */ EXCEPINFO *pExcepInfo,
        /* [out] */ UINT *puArgErr);
};

//Класс для обеспечения доступа к данному компьютеру через RDP
class TRDPServer{
protected:
  IRDPSRAPISharingSession * pRDPSession;	
	IRDPSRAPIInvitationManager * pInvitationManager;
	IRDPSRAPIInvitation * pInvitation;
	RAS::ATTENDEE * Attendee;
	TRDPSessionEvents SessionEvents;
	IConnectionPointContainer * pIConnectionPointContainer;
	IConnectionPoint * pIConnectionPoint;
	int Cookie;

  void OpenSession();
	void CloseSession();
	QString GetConnectionString();
public:
  TRDPServer();
  ~TRDPServer();

  bool SetNotification(HWND en, UINT msg, bool A);
	void HandleNotification(DISPID id, DISPPARAMS * p);
};


//Пользовательский интерфейс
class TSharer : public QMainWindow, public Ui::TSharer, public TRDPServer {
Q_OBJECT
private:
  THelperClient * HelperClient;
	QSystemTrayIcon TrayIcon;
	void CreateActionsAndMenus();
	void Init();

public:
	TSharer();
	~TSharer();
signals:
private slots:
	void OnActionOpenSession();
	void OnActionCloseSession();
  void OnActionSendInv();
	void OnActionExit();
public slots:
};

#endif