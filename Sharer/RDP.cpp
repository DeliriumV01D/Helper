// rdp.cpp
#include "rdp.h"

namespace RAS
	{
int RDPConnectObject(IUnknown* Container,REFIID riid,IUnknown* Advisor,IConnectionPointContainer** picpc,IConnectionPoint** picp)
	{
	HRESULT hr = 0;
	unsigned long tid = 0;
	IConnectionPointContainer* icpc = 0;
	IConnectionPoint* icp = 0;
	*picpc = 0;
	*picp = 0;

	Container->QueryInterface(IID_IConnectionPointContainer, (void **)&icpc);
	if (icpc)
		{
		*picpc = icpc;
		icpc->FindConnectionPoint(riid,&icp);
		if (icp)
		{
			*picp = icp;
			hr = icp->Advise(Advisor,&tid);
			//icp->Release();
		}
		//icpc->Release();
		}
	return tid;
	}

void RDPDisconnectObject(IConnectionPointContainer* icpc,IConnectionPoint* icp,unsigned int Cookie)
	{
	unsigned long hr  = 0;
	icp->Unadvise(Cookie);
	icp->Release();
	icpc->Release();
	}




void CLIENT ::Init(IUnknown* x)
	{
	Cookie = 0;
	picpc = 0;
	picp = 0;

	CLSID c1 = __uuidof(RDPViewer);
	CLSID c2 = __uuidof(IRDPSRAPIViewer);
	if (!x)
		CoCreateInstance(c1,0,CLSCTX_INPROC_SERVER,c2,(void**)&s);
	else
		{
		x->QueryInterface(__uuidof(IRDPSRAPIViewer),&s);
		}
	SetNotification(0,0,true);
	}


void CLIENT :: Shut()
	{
	if (s)
		{
		IRDPSRAPIViewer* ss = (IRDPSRAPIViewer*)s;
		ss->Release();
		}
	s = 0;
	}

CLIENT :: CLIENT(IUnknown* x)
	{
	IsServer = false;
	Init(x);
	}

CLIENT ::~CLIENT()
	{
	SetNotification(0,0,false);
	Disconnect();
	Shut();
	}

bool CLIENT :: Connect(const wchar_t*str,const wchar_t*name,const wchar_t* pwd)
	{
	IRDPSRAPIViewer* ss = (IRDPSRAPIViewer*)s;
	if (!ss)
		return false;

	HRESULT hr = ss->Connect(B(str),B(name),B(pwd));
	if (FAILED(hr))
		return false;
	return true;
	}


bool CLIENT :: Disconnect()
	{
	IRDPSRAPIViewer* ss = (IRDPSRAPIViewer*)s;
	if (!ss)
		return false;

	HRESULT hr = ss->Disconnect();
	if (FAILED(hr))
		return false;
	return true;
	}


wstring CLIENT :: SetReverseConnectionParameters(int prot,int port,const wchar_t* bstr,const wchar_t* n,const wchar_t* pwd)
	{
	IRDPSRAPIViewer* ss = (IRDPSRAPIViewer*)s;
	if (!ss)
		return false;

	IRDPSRAPISessionProperties* sp = 0;
	ss->get_Properties(&sp);
	if (sp)
		{
		if (prot && port)
			{
			VARIANT vt;
			vt.vt = VT_INT;
			vt.intVal = prot;
			sp->put_Property(L"PortProtocol",vt);
			vt.vt = VT_INT;
			vt.intVal = port;
			sp->put_Property(L"PortId",vt);
			}
		sp->Release();
		}

	BSTR b = 0;
	rev = L"";
	HRESULT hr = ss->StartReverseConnectListener(B(bstr),B(n),B(pwd),&b);
	if (!b)
		return rev;
	rev = b;
	SysFreeString(b);
	return rev;
	}

			
void CLIENT :: RequestControl(int cl)
	{
	IRDPSRAPIViewer* ss = (IRDPSRAPIViewer*)s;
	if (!ss)
		return;

	ss->RequestControl((CTRL_LEVEL)cl);
	}

void CLIENT :: SmartResize(bool x)
	{
	IRDPSRAPIViewer* ss = (IRDPSRAPIViewer*)s;
	if (!ss)
		return;

	ss->put_SmartSizing(x ? VARIANT_TRUE : VARIANT_FALSE);
	}

// ---- SERVER STUFF

S_INVITATION :: S_INVITATION(wchar_t*str,wchar_t*pwd,wchar_t*grp,int limit)
	{
	if (str)
		cstr = str;
	if (pwd)
		cpwd = pwd;
	if (grp)
		cgrp = grp;
	climit = limit;

	Revoked = false;
	}


bool S_INVITATION :: Revoke(bool X)
	{
	IRDPSRAPIInvitation* inv = (IRDPSRAPIInvitation*)ss;
	if (!inv)
		return false;
	return (SUCCEEDED(inv->put_Revoked(X ? VARIANT_TRUE : VARIANT_FALSE)));
	}




ATTENDEE :: ATTENDEE(IDispatch* x)
	{
	IRDPSRAPIAttendee* a = 0;
	IsTarget = true;
	x->QueryInterface(__uuidof(IRDPSRAPIAttendee),(void**)&a);
	ss = (void*)a;
	if (a)
		{
		a->get_Id(&lid);
		BSTR b = 0;
		a->get_RemoteName(&b);
		if (b)
			{
			n = b;
			SysFreeString(b);
			b = 0;
			}

		IUnknown* ci = 0;
		a->get_ConnectivityInfo(&ci);

		if (ci)
			{
			IRDPSRAPITcpConnectionInfo* u = 0;
			ci->QueryInterface(__uuidof(IRDPSRAPITcpConnectionInfo),(void**)&u);
			if (u)
				{
				b = 0;
				u->get_LocalIP(&b);
				if (b)
					{
					LocalIP = b;
					SysFreeString(b);
					b = 0;
					}
				b = 0;
				u->get_PeerIP(&b);
				if (b)
					{
					PeerIP = b;
					SysFreeString(b);
					b = 0;
					}

				u->get_LocalPort(&LocalPort);
				u->get_PeerPort(&PeerPort);

				u->Release();
				}
			ci->Release();
			}
		}
	}

HRESULT ATTENDEE :: Kill()
	{
	IRDPSRAPIAttendee* a = (IRDPSRAPIAttendee*)ss;
	if (!ss)
		return E_FAIL;
	return a->TerminateConnection();
	}

void ATTENDEE :: Release()
	{
	IRDPSRAPIAttendee* a = (IRDPSRAPIAttendee*)ss;
	if (!ss)
		return;
	a->Release();
	ss = 0;
	}

ATTENDEE :: ~ATTENDEE()
	{
	}

void ATTENDEE :: SetControl(int x)
	{
	IRDPSRAPIAttendee* a = (IRDPSRAPIAttendee*)ss;
	if (!ss)
		return;

	a->put_ControlLevel((CTRL_LEVEL)x);
	}



SERVER :: SERVER()
	{
	IsServer = true;
	Init();
	}



SERVER :: ~SERVER()
	{
	SetNotification(0,0,false);
	Close();
	Shut();
	}

bool BASERDP :: SetNotification(HWND en,UINT msg,bool A)
{
	ev.SetNotification(en,msg,this);
	IRDPSRAPISharingSession* ss = (IRDPSRAPISharingSession*)s;
	if (!ss)
		return false;

	if (A && picpc && picp)
		return true;
	if (!A && !picpc && !picp)
		return true;

	if (A)
		{
		nen = en;
		men = msg;
		Cookie = RDPConnectObject((IUnknown*)ss,__uuidof(_IRDPSessionEvents),(IUnknown*)&ev,&picpc,&picp);
		if (picp && picp)
			return true;
		return false;
		}
	else
		{
		RDPDisconnectObject(picpc,picp,Cookie);
		picp = 0;
		picpc = 0;
		return true;
		}
}

bool SERVER :: Init()
	{
	s = 0;
	Cookie = 0;
	picpc = 0;
	picp = 0;

	CLSID c1 = __uuidof(RDPSession);
	CLSID c2 = __uuidof(IRDPSRAPISharingSession);
	CoCreateInstance(c1,0,CLSCTX_ALL,c2,(void**)&s);
	if (!s)
		return true;
	SetNotification(0,0,true);
	return true;
	}

void SERVER :: Shut()
	{
	if (s)
		{
		IRDPSRAPISharingSession* ss = (IRDPSRAPISharingSession*)s;
		ss->Release();
		}
	s = 0;
	}

bool SERVER :: Open()
	{
	IRDPSRAPISharingSession* ss = (IRDPSRAPISharingSession*)s;
	if (!ss)
		return false;
	if (rcs.length() == 0)
		return (SUCCEEDED(ss->Open()));
	else
		return (SUCCEEDED(ss->ConnectToClient(B(rcs.c_str()))));
	}

bool SERVER :: ReverseOpen(const wchar_t* key)
	{
	IRDPSRAPISharingSession* ss = (IRDPSRAPISharingSession*)s;
	if (!ss)
		return false;
	if (!key || wcslen(key) == 0)
		return false;
	return (SUCCEEDED(ss->ConnectToClient(B(key))));
	}

bool SERVER :: Close()
	{
	IRDPSRAPISharingSession* ss = (IRDPSRAPISharingSession*)s;
	if (!ss)
		return false;
	return (SUCCEEDED(ss->Close()));
	}

bool SERVER :: TogglePause(bool P)
	{
	IRDPSRAPISharingSession* ss = (IRDPSRAPISharingSession*)s;
	if (!ss)
		return false;
	if (P)
		return (SUCCEEDED(ss->Pause()));
	else
		return (SUCCEEDED(ss->Resume()));
	}

bool SERVER :: SetDepth(int X)
	{
	IRDPSRAPISharingSession* ss = (IRDPSRAPISharingSession*)s;
	if (!ss)
		return false;
	return (SUCCEEDED(ss->put_ColorDepth(X)));
	}

int SERVER :: GetDepth()
	{
	IRDPSRAPISharingSession* ss = (IRDPSRAPISharingSession*)s;
	if (!ss)
		return false;
	long li = 0;
	ss->get_ColorDepth(&li);
	return li;
	}


bool SERVER :: SetConnectionParameters(int aftype,int port,bool DynamicMirror,int ColorDepth,char* ReverseConnectionString)
	{
	IRDPSRAPISharingSession* ss = (IRDPSRAPISharingSession*)s;
	if (!ss)
		return false;
	IRDPSRAPISessionProperties* p = 0;
	ss->get_Properties(&p);
	if (!p)
		return false;

	HRESULT hr1 = S_OK;
	HRESULT hr2 = S_OK;
	HRESULT hr3 = S_OK;
	HRESULT hr4 = S_OK;
	VARIANT vt1;
	vt1.vt = VT_INT;
	vt1.intVal = aftype;
	hr1 = p->put_Property(B(L"PortProtocol"),vt1);
	vt1.vt = VT_INT;
	vt1.intVal = port;
	hr2 = p->put_Property(B(L"PortId"),vt1);
	vt1.vt = VT_BOOL;
	vt1.boolVal = DynamicMirror ? VARIANT_TRUE : VARIANT_FALSE;
	hr3 = p->put_Property(B(L"DrvConAttach"),vt1);
	p->Release();
	hr4 = ss->put_ColorDepth(ColorDepth);

	if (ReverseConnectionString)
		rcs = ReverseConnectionString;
	else
		rcs = "";

	if (SUCCEEDED(hr1) && SUCCEEDED(hr2) && SUCCEEDED(hr3) && SUCCEEDED(hr4))
		return true;
	return false;
	}

bool SERVER :: GetConnectionParameters(int* aftype,int* port,bool* DynamicMirror,int* ColorDepth,char* ReverseConnectionString)
	{
	IRDPSRAPISharingSession* ss = (IRDPSRAPISharingSession*)s;
	if (!ss)
		return false;
	IRDPSRAPISessionProperties* p = 0;
	ss->get_Properties(&p);
	if (!p)
		return false;

	HRESULT hr1 = S_OK;
	HRESULT hr2 = S_OK;
	HRESULT hr3 = S_OK;
	HRESULT hr4 = S_OK;
	VARIANT vt1;
	if (aftype)
		{
		hr1 = p->get_Property(B(L"PortProtocol"),&vt1);
		*aftype = vt1.intVal;
		}
	if (port)
		{
		hr2 = p->get_Property(B(L"PortId"),&vt1);
		*port = vt1.intVal;
		}
	if (DynamicMirror)
		{
		hr3 = p->get_Property(B(L"DrvConAttach"),&vt1);
		*DynamicMirror = vt1.boolVal == VARIANT_TRUE ? true : false;
		}
	p->Release();
	if (ColorDepth)
		{
		long cd = 0;
		ss->get_ColorDepth(&cd);
		*ColorDepth = cd;
		}
	if (ReverseConnectionString)
		{
		lstrcpyA(ReverseConnectionString,"");
		if (rcs.length())
			lstrcpyA(ReverseConnectionString,rcs.c_str());
		}
	if (SUCCEEDED(hr1) && SUCCEEDED(hr2) && SUCCEEDED(hr3) && SUCCEEDED(hr4))
		return true;
	return false;
	}

bool SERVER :: SetDesktopRegion(const RECT& rc)
	{
	IRDPSRAPISharingSession* ss = (IRDPSRAPISharingSession*)s;
	if (!ss)
		return false;
	ss->SetDesktopSharedRect(rc.left,rc.top,rc.right,rc.bottom);
	return true;
	}

bool SERVER :: GetDesktopRegion(RECT& rc)
	{
	IRDPSRAPISharingSession* ss = (IRDPSRAPISharingSession*)s;
	if (!ss)
		return false;

	ss->GetDesktopSharedRect(&rc.left,&rc.top,&rc.right,&rc.bottom);
	return true;
	}


bool SERVER :: SetApplicationState(int pid,bool GlobalShare)
	{
	bool G = GlobalShare;
	return SetGetApplicationState(pid,1,G);
	}

void SERVER :: GetShareableApplications(vector<int>& pids,vector<wstring>& names,vector<int>& ST)
	{
	IRDPSRAPISharingSession* ss = (IRDPSRAPISharingSession*)s;
	if (!ss)
		return;

	// Get the application pids list
	vector<VARIANT> v;
	IRDPSRAPIApplicationFilter* af = 0;
	ss->get_ApplicationFilter(&af);
	bool F = false;
	if (af)
		{
		IRDPSRAPIApplicationList* afl = 0;
		af->get_Applications(&afl);
		if (afl)
			{
			IUnknown* ei = 0;
			afl->get__NewEnum(&ei);
			if (ei)
				{
				GetEnumeration<IEnumVARIANT,VARIANT>(ei,v);
				ei->Release();
				}
			afl->Release();
			}
		af->Release();
		}
	for(unsigned int i = 0 ; i < v.size() ; i++)
		{
		IRDPSRAPIApplication* app = 0;
		if (!v[i].pdispVal)
			continue;
		v[i].pdispVal->QueryInterface(__uuidof(IRDPSRAPIApplication),(void**)&app);
		if (!app)
			{
			v[i].pdispVal->Release();
			continue;
			}
		long lid = 0;
		BSTR bx = 0;
		VARIANT_BOOL bz;
		app->get_Id(&lid);
		app->get_Name(&bx);
		app->get_Shared(&bz);
		pids.push_back(lid);
		names.push_back(wstring(bx ? bx : L""));
		ST.push_back(bz == VARIANT_TRUE ? 1 : 0);
		if (bx)
			SysFreeString(bx);
		app->Release();
		v[i].pdispVal->Release();
		}
	}

void SERVER :: ShareOnlyTheseApplications(vector<int>& pids,bool X)
	{
	IRDPSRAPISharingSession* ss = (IRDPSRAPISharingSession*)s;
	if (!ss)
		return;

	// Get the application pids list
	vector<VARIANT> v;
	IRDPSRAPIApplicationFilter* af = 0;
	ss->get_ApplicationFilter(&af);
	bool F = false;
	if (af)
		{
		IRDPSRAPIApplicationList* afl = 0;
		af->get_Applications(&afl);
		if (afl)
			{
			IUnknown* ei = 0;
			afl->get__NewEnum(&ei);
			if (ei)
				{
				GetEnumeration<IEnumVARIANT,VARIANT>(ei,v);
				ei->Release();
				}
			afl->Release();
			}
		af->Release();
		}

	for(unsigned int i = 0 ; i < v.size() ; i++)
		{
		IRDPSRAPIApplication* app = 0;
		if (!v[i].pdispVal)
			continue;
		v[i].pdispVal->QueryInterface(__uuidof(IRDPSRAPIApplication),(void**)&app);
		if (!app)
			{
			v[i].pdispVal->Release();
			continue;
			}

		HRESULT hr = 0;
		if (!X)
			app->put_Shared(VARIANT_TRUE);
		else
			{
			long xpid = 0;
			app->get_Id(&xpid);

			// Is it there?
			bool Found = false;
			for(unsigned int jjj = 0 ; jjj < pids.size() ; jjj++)
				{
				if (pids[jjj] == xpid)
					{
					Found = true;
					break;
					}
				if (pids[jjj] == 0 && xpid == GetCurrentProcessId())
					{
					Found = true;
					break;
					}
				}

			if (Found)
				{
				hr = app->put_Shared(VARIANT_TRUE);
				}
			else
				{
				hr = app->put_Shared(VARIANT_FALSE);
				BSTR bn = 0;
				app->get_Name(&bn);
				SysFreeString(bn);

				// All windows non shared
				IRDPSRAPIWindowList* wl = 0;
				app->get_Windows(&wl);
				if (wl)
					{
					IUnknown* ei = 0;
					wl->get__NewEnum(&ei);
					if (ei)
						{
						vector<VARIANT> v2;
						GetEnumeration<IEnumVARIANT,VARIANT>(ei,v2);
						for(unsigned int jk = 0 ; jk < v2.size() ; jk++)
							{
							IRDPSRAPIWindow* wi = 0;
							if (!v2[jk].pdispVal)
								continue;
							v2[jk].pdispVal->QueryInterface(__uuidof(IRDPSRAPIWindow),(void**)&wi);
							if (wi)
								{
								wi->put_Shared(VARIANT_FALSE);
								wi->Release();
								}
							v2[jk].pdispVal->Release();
							}
						ei->Release();
						}
					wl->Release();
					}
				}
			}
		app->Release();
		v[i].pdispVal->Release();
		}
	}

bool SERVER :: GetApplicationState(int pid,bool* GlobalShare)
	{
	if (!GlobalShare)
		return false;

	bool G = false;
	bool X = SetGetApplicationState(pid,0,G);
	if (X)
		*GlobalShare = G;
	return X;
	}

bool SERVER :: SetGetApplicationState(int pid,int sg,bool& GlobalShare)
	{
	if (pid == 0)
		pid = GetCurrentProcessId();

	IRDPSRAPISharingSession* ss = (IRDPSRAPISharingSession*)s;
	if (!ss)
		return 0;

	// Add the application
	IRDPSRAPIApplicationFilter* af = 0;
	ss->get_ApplicationFilter(&af);
	bool F = false;
	if (af)
		{
		IRDPSRAPIApplicationList* afl = 0;
		af->get_Applications(&afl);
		if (afl)
			{
			IRDPSRAPIApplication* app = 0;
			afl->get_Item(pid,&app);
			if (app)
				{
				if (sg == 1) // set
					{
					HRESULT hr = app->put_Shared(GlobalShare ? VARIANT_TRUE : VARIANT_FALSE);
					if (SUCCEEDED(hr))
						F = true;
					}
				else // 0 - get
					{
					VARIANT_BOOL bx;
					HRESULT hr = app->get_Shared(&bx);
					if (SUCCEEDED(hr))
						{
						F = true;
						if (bx == VARIANT_TRUE)
							GlobalShare = true;
						else
							GlobalShare = false;
						}
					}
				app->Release();
				}
			afl->Release();
			}
		af->Release();
		}
	return F;
	}


bool SERVER :: SetGetApplicationSharedWindowList(int pid,int sg,vector<HWND>& h,vector<int>& shr)
	{
	if (sg == 1)
		{
		if (!SetApplicationState(pid,false))
			return false;
		}
	if (sg == 0)
		{
		bool g = false;
		GetApplicationState(pid,&g);
		if (g == true)
			return false; // entire app shared
		}

	if (pid == 0)
		pid = GetCurrentProcessId();
	IRDPSRAPISharingSession* ss = (IRDPSRAPISharingSession*)s;
	if (!ss)
		return 0;
	IRDPSRAPIApplicationFilter* af = 0;
	ss->get_ApplicationFilter(&af);
	bool F = false;
	if (af)
		{
		IRDPSRAPIApplicationList* afl = 0;
		af->get_Applications(&afl);
		if (afl)
			{
			IRDPSRAPIApplication* app = 0;
			afl->get_Item(pid,&app);
			if (app)
				{
				IRDPSRAPIWindowList* wl = 0;
				app->get_Windows(&wl);
				if (wl)
					{
//					IEnumYYYY<IRDPSRAPIWindow>* e = 0;
					IUnknown* ei = 0;
					wl->get__NewEnum(&ei);
					IEnumUnknown* e = 0;
					if (ei)
						{
						ei->QueryInterface(__uuidof(IEnumUnknown),(void**)e);
						ei->Release();
						}

					if (e)
						{
						e->Reset();
						for(unsigned int ji = 0 ; ; ji++)
							{
							IRDPSRAPIWindow* iw = 0;
							if (e->Next(1,(IUnknown**)&iw,0) != S_OK)
								break;

							if (iw)
								{
								if (sg == 1)
									{
									HRESULT hr = iw->put_Shared(shr[ji] == 1 ? VARIANT_TRUE : VARIANT_FALSE);
									if (SUCCEEDED(hr))
										F = true;
									}
								else
									{
									// Get
									VARIANT_BOOL vx = 0;
									HRESULT hr = iw->get_Shared(&vx);
									if (SUCCEEDED(hr))
										{
										F = true;
										if (shr.size() <= ji)
											shr.resize(ji + 1);
										shr[ji] = vx == VARIANT_TRUE ? 1 : 0;
										}
									}
								iw->Release();
								}
							}
				
						e->Release();
						}
					wl->Release();
					}
				app->Release();
				}
			}
		}
	return F;
	}

bool SERVER :: GetApplicationSharedWindowList(int pid,vector<HWND>& h)
	{
	vector<HWND> hws;
	vector<int> his;
	if (!SetGetApplicationSharedWindowList(pid,0,hws,his))
		return false;

	for(unsigned int i = 0 ; i < hws.size() ; i++)
		{
		if (his[i] == 1)
			h.push_back(hws[i]);
		}
	return true;
	}

bool SERVER :: SetApplicationSharedWindowList(int pid,vector<HWND>& h)
	{
	return false;
	}
void BASERDP :: RemoveAtt(IRDPSRAPIAttendee* at)
	{
	for(unsigned int i = 0 ; i < Atts.size() ; i++)
		{
		if (Atts[i].GetInterface() == (void*)at)
			{
			Atts[i].Release();
			Atts.erase(Atts.begin() + i);
			break;
			}
		}
	}

ATTENDEE* BASERDP :: FindAtt(IRDPSRAPIAttendee* at)
	{
	for(unsigned int i = 0 ; i < Atts.size() ; i++)
		{
		if (Atts[i].GetInterface() == (void*)at)
			{
			return &Atts[i];
			}
		}
	return 0;
	}


void BASERDP :: HandleNotification(DISPID id,DISPPARAMS* p)
	{
	IDispatch* j = 0;
	IUnknown* ju = 0;
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
			hr = DispGetParam(p,0,VT_DISPATCH,&x,&y);
			if (FAILED(hr))
				return;
			IDispatch* d = x.pdispVal;
			if (!d)
				return;

			ATTENDEE a(d);
			d->Release();
			Atts.push_back(a);
			break;
			}
		case DISPID_RDPSRAPI_EVENT_ON_ATTENDEE_DISCONNECTED:
			{
			x.vt = VT_DISPATCH;
			x.pdispVal = j;
			hr = DispGetParam(p,0,VT_DISPATCH,&x,&y);
			if (FAILED(hr))
				return;
			IDispatch* d = x.pdispVal;
			if (!d)
				return;

			IRDPSRAPIAttendeeDisconnectInfo* a = 0;
			d->QueryInterface(__uuidof(IRDPSRAPIAttendeeDisconnectInfo),(void**)&a);
			if (!a)
				return;

			IRDPSRAPIAttendee* at = 0;
			a->get_Attendee(&at);
			if (at)
				{
				RemoveAtt(at);
				at->Release();
				}

			a->Release();
			break;
			}
		case DISPID_RDPSRAPI_EVENT_ON_CTRLLEVEL_CHANGE_REQUEST:
			{
			x.vt = VT_INT;
			x.pdispVal = j;
			hr = DispGetParam(p,1,VT_INT,&x,&y);
			if (FAILED(hr))
				return;
			int Lev = x.intVal;
			x.vt = VT_INT;
			x.pdispVal = j;
			hr = DispGetParam(p,0,VT_DISPATCH,&x,&y);
			if (FAILED(hr))
				return;
			IDispatch* d = x.pdispVal;
			if (!d)
				return;
			IRDPSRAPIAttendee* a = 0;
			d->QueryInterface(__uuidof(IRDPSRAPIAttendee),(void**)&a);
			if (!a)
				return;

			ATTENDEE* sa = FindAtt(a);
			if (sa)
				sa->SetControl(Lev);
			a->Release();
			break;
			}
		case DISPID_RDPSRAPI_EVENT_ON_VIRTUAL_CHANNEL_JOIN:
			{
			break;
			}
		case DISPID_RDPSRAPI_EVENT_ON_VIRTUAL_CHANNEL_LEAVE:
			{
			break;
			}
		case DISPID_RDPSRAPI_EVENT_ON_VIRTUAL_CHANNEL_SENDCOMPLETED:
			{
			x.vt = VT_INT;
			hr = DispGetParam(p,2,VT_INT,&x,&y);
			if (FAILED(hr))
				return;
			int sz = x.intVal;
			x.vt = VT_INT;
			x.pdispVal = j;
			hr = DispGetParam(p,1,VT_INT,&x,&y);
			if (FAILED(hr))
				return;
			int lid = x.intVal;
			x.vt = VT_INT;
			x.punkVal = ju;
			hr = DispGetParam(p,0,VT_DISPATCH,&x,&y);
			if (FAILED(hr))
				return;
			IUnknown* u = x.punkVal;
			if (!u)
				return;

			IRDPSRAPIVirtualChannel* C = 0;
			u->QueryInterface(__uuidof(IRDPSRAPIVirtualChannel),(void**)&C);
			if (!C)
				return;

			RAS::CHANNEL* CC = 0;
			for(unsigned int i = 0 ; i < GetChannels().size() ; i++)
			{
			if (GetChannels()[i].GetInterface() == (void*)C)
				{
				CC = &GetChannels()[i];
				break;
				}
			}


		// Data notification
		OnReceiveData(CC,0,0,sz);

		// Check if this Channel has pending data
		CC->Flush();

		break;
		}
	case DISPID_RDPSRAPI_EVENT_ON_VIRTUAL_CHANNEL_DATARECEIVED:
		{
		x.vt = VT_BSTR;
		BSTR xa = 0;
		x.bstrVal = xa;

		hr = DispGetParam(p,2,VT_BSTR,&x,&y);
		if (FAILED(hr))
			return;
		BSTR data = x.bstrVal;
		x.vt = VT_INT;
		x.pdispVal = j;
		hr = DispGetParam(p,1,VT_INT,&x,&y);
		if (FAILED(hr))
			return;
		int lid = x.intVal;
		x.vt = VT_INT;
		x.punkVal = ju;
		hr = DispGetParam(p,0,VT_DISPATCH,&x,&y);
		if (FAILED(hr))
			return;
		IUnknown* u = x.punkVal;
		if (!u)
			return;

		IRDPSRAPIVirtualChannel* C = 0;
		u->QueryInterface(__uuidof(IRDPSRAPIVirtualChannel),(void**)&C);
		if (!C)
			return;

		RAS::CHANNEL* CC = 0;
		for(unsigned int i = 0 ; i < GetChannels().size() ; i++)
			{
			if (GetChannels()[i].GetInterface() == (void*)C)
				{
				CC = &GetChannels()[i];
				break;
				}
			}

		int rsz = SysStringByteLen(data);
		// Find Attendee
		ATTENDEE* A = 0;
		for(unsigned int jj = 0 ; jj < GetAttendees().size() ; jj++)
			{
			if (GetAttendees()[jj].GetID() == lid)
				{
				A = &GetAttendees()[jj];
				break;
				}
			}
		OnReceiveData(CC,A,(char*)data,rsz);

		C->Release();
		break;
		}
	}
}

void BASERDP :: OnReceiveData(CHANNEL* C,ATTENDEE*A,char* d,int sz)
{
if (nData)
	nData(C,A,d,sz,nDatap);
}


S_INVITATION* SERVER :: Invite(wchar_t*str,wchar_t*pwd,wchar_t*grp,int limit)
{
IRDPSRAPISharingSession* ss = (IRDPSRAPISharingSession*)s;
if (!ss)
	return 0;

S_INVITATION tv(str ? str : 0,pwd ? pwd : 0,grp ? grp : 0,limit);

// Create the invitation
IRDPSRAPIInvitationManager* invm = 0;
ss->get_Invitations(&invm);
if (!invm)
	return 0;

IRDPSRAPIInvitation* inv = 0;
HRESULT hr = invm->CreateInvitation(B(str),B(grp),B(pwd),limit,&inv);
if (!inv)
	return 0;

invm->Release();
BSTR b = 0;
inv->get_ConnectionString(&b);
tv.GetTicket() = b;
SysFreeString(b);
inv->get_Password(&b);
SysFreeString(b);
tv.GetInterface() = (void*)inv;
Invs.push_back(tv);
return &Invs[Invs.size() - 1];
}

void BASERDP :: AssignChannelToAttendees(CHANNEL* C,vector<ATTENDEE*>& AssignAtts)
{
IRDPSRAPIVirtualChannel* c = (IRDPSRAPIVirtualChannel*)C->GetInterface();
if (!c)
	return;

for(unsigned int i = 0 ; i < Atts.size() ; i++)
	{
	bool FoundInList = false;
	for(unsigned int y = 0 ; y < AssignAtts.size() ; y++)
		{
		if (AssignAtts[y] == &Atts[i])
			{
			FoundInList = true;
			break;
			}
		}

	IRDPSRAPIAttendee* a = (IRDPSRAPIAttendee*)Atts[i].GetInterface();
	if (!a)
		continue;
	long lid = 0;
	a->get_Id(&lid);

	if (FoundInList)
		{
		c->SetAccess(lid,CHANNEL_ACCESS_ENUM_SENDRECEIVE);
		}
	else
		{
		c->SetAccess(lid,CHANNEL_ACCESS_ENUM_NONE);
		}
	}
}

CHANNEL* BASERDP :: CreateVirtualChannel(const wchar_t* n,bool Compressed,int Priority)
{
IRDPSRAPIVirtualChannelManager* cm = 0;
if (IsServer)
	{
	IRDPSRAPISharingSession* ss = (IRDPSRAPISharingSession*)s;
	if (!ss)
		return 0;
	ss->get_VirtualChannelManager(&cm);
	}
else
	{
	IRDPSRAPIViewer* ss = (IRDPSRAPIViewer*)s;
	if (ss)
		ss->get_VirtualChannelManager(&cm);
	}

if (!cm)
	return 0;

IRDPSRAPIVirtualChannel* c = 0;
cm->CreateVirtualChannel(B(n),(CHANNEL_PRIORITY)Priority,Compressed ? 0 : CHANNEL_FLAGS_UNCOMPRESSED,&c);
if (!c)
	return 0;

CHANNEL cx(n,this,c);
ch.push_back(cx);
CHANNEL* C =  &ch[ch.size() - 1];
return C;
}

HRESULT BASERDP :: SendData(CHANNEL* C,char* d,int sz,bool Unc,vector<ATTENDEE*>* List)
{
if (!C)
	return E_FAIL;
if (!sz)
	return S_FALSE;

IRDPSRAPIVirtualChannel* c = (IRDPSRAPIVirtualChannel*)C->GetInterface();
if (!c)
	return E_FAIL;

int BytesToSend = sz;
int BytesSent = 0;
HRESULT hr = S_OK;
for(;;)
	{
	int BytesNow = (sz - BytesSent);
	if (BytesNow > CONST_MAX_CHANNEL_MESSAGE_SIZE)
		{
		BytesNow = CONST_MAX_CHANNEL_MESSAGE_SIZE;
		}
	if (BytesNow == 0)
		break;

	BSTR b = SysAllocStringByteLen(d + BytesSent,BytesNow);
	if (!List)
		{
		hr = c->SendData(b,CONST_ATTENDEE_ID_EVERYONE,Unc ? CHANNEL_FLAGS_UNCOMPRESSED : 0);
		if (hr == E_PENDING)
			{
			C->AddPendingData(d + BytesSent,BytesToSend - BytesSent,Unc);
			SysFreeString(b);
			break;
			}
		}
	else
		{
		for(unsigned int kk = 0 ; kk < List->size() ; kk++)
			{
			IRDPSRAPIAttendee* a = (IRDPSRAPIAttendee*)(*List)[kk]->GetInterface();
			if (!a)
				continue;
			long lid = 0;
			a->get_Id(&lid);
			hr = c->SendData(b,lid,Unc ? CHANNEL_FLAGS_UNCOMPRESSED : 0);
			if (hr == E_PENDING)
				{
				// For all the rest attendees, pending
				for(unsigned int kl = kk ; kl < List->size() ; kl++)
					C->AddPendingData(d + BytesSent,BytesToSend - BytesSent,Unc,(*List)[kl]);
				break;
				}
			if (FAILED(hr))
				break; // Whops
			}
		}

	SysFreeString(b);
	if (FAILED(hr))
		{
		break;
		}

	BytesSent += BytesNow;
	}

return hr;
}

void CHANNEL :: AddPendingData(char* d,int sz,bool Unc,ATTENDEE* at)
{
	if (!at)
		{
		if (_PendingData.empty())
			_PendingUnc = Unc;
		_PendingData.append(d,sz);
		}
	else
		{
		ATDATA* a = 0;
		for(unsigned int i = 0 ; i < AttPendingData.size() ; i++)
			{
			if (AttPendingData[i].att == at)
				{
				a = &AttPendingData[i];
				break;
				}
			}
		if (!a)
			{
			// Create an emtry
			ATDATA atx;
			atx.att = at;
			atx.PendingData.clear();
			AttPendingData.push_back(atx);
			a = &AttPendingData[AttPendingData.size() - 1];
			}
		if (a->PendingData.empty())
			a->PendingUnc = Unc;
		a->PendingData.append(d,sz);
		}
}

void CHANNEL :: Flush()
{
	// Attendee Specific
	for(signed int i = AttPendingData.size() - 1 ;  i >= 0 ; i--)
		{
		ATDATA at = AttPendingData[i]; // Not ref, we will delete it!

		vector<ATTENDEE*> List;
		List.push_back(at.att);

		int nl = at.PendingData.length();
		char* nd = new char[nl + 1];
		memcpy(nd,at.PendingData.data(),at.PendingData.length());
		at.PendingData.clear();
		AttPendingData.erase(AttPendingData.begin() + i);

		if (Parent)
			Parent->SendData(this,nd,nl,at.PendingUnc,&List);
		delete[] nd;
		}

	// Entire stuff
	if (_PendingData.length())
		{
		int nl = _PendingData.length();
		char* nd = new char[nl + 1];
		memcpy(nd,_PendingData.data(),_PendingData.length());
		_PendingData.clear();

		if (Parent)
			Parent->SendData(this,nd,nl,_PendingUnc,0);
		delete[] nd;
		}
}


void BASERDP :: SetWindowNotification(HWND hh,UINT msg)
{
	SetNotification(hh,msg,true);
}

void BASERDP :: SetDataNotification(void (*p)(CHANNEL*,ATTENDEE*,char*,int,void*),void* lp)
{
	nData = p;
	nDatap = lp;
}



MyRDPSessionEvents :: MyRDPSessionEvents()
{
	refNum = 0;
	AddRef();
}

MyRDPSessionEvents :: ~MyRDPSessionEvents()
{
}

// IUnknown Methods
HRESULT _stdcall MyRDPSessionEvents :: QueryInterface(REFIID iid,void**ppvObject)
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

ULONG _stdcall MyRDPSessionEvents :: AddRef()
{
	refNum++;
	return refNum;
}

ULONG _stdcall MyRDPSessionEvents :: Release()
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
HRESULT _stdcall MyRDPSessionEvents :: GetTypeInfoCount(unsigned int * pctinfo) 
{
return E_NOTIMPL;
}

HRESULT _stdcall MyRDPSessionEvents :: GetTypeInfo(unsigned int iTInfo,LCID lcid,ITypeInfo FAR* FAR* ppTInfo) 
{
	return E_NOTIMPL;
}

HRESULT _stdcall MyRDPSessionEvents :: GetIDsOfNames(
REFIID riid,
OLECHAR FAR* FAR*,
unsigned int cNames,
LCID lcid,
DISPID FAR* ) 
{
	return E_NOTIMPL;
}

void MyRDPSessionEvents :: SetNotification(HWND n,UINT m,BASERDP* p)
{
	nen = n;
	msg = m;
	Parent = p;
}


HRESULT _stdcall MyRDPSessionEvents :: Invoke(
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

CHANNEL :: CHANNEL(const wchar_t*nn,BASERDP* P,IRDPSRAPIVirtualChannel* x)
{
	this->n = nn;
	Parent = P;
	ss = (void*)x;
	_PendingData.clear();
	AttPendingData.clear();
}

void CHANNEL :: Release()
{
	IRDPSRAPIVirtualChannel* c = (IRDPSRAPIVirtualChannel*)ss;
	if (c)
		c->Release();
	ss = 0;
}


};