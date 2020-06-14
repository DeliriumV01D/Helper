// rdp.h
#ifndef _RDP_H
#define _RDP_H

#define INITGUID
//#define WINVER 0x602
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <Rdpencomapi.h>
#include <string>
#include <vector>
using namespace std;

namespace RAS
{
int RDPConnectObject(IUnknown* Container,REFIID riid,IUnknown* Advisor,IConnectionPointContainer** picpc,IConnectionPoint** picp);
void RDPDisconnectObject(IConnectionPointContainer* icpc,IConnectionPoint* icp,unsigned int Cookie);

// Quick BSTR stuff
class B 
{
private:
	BSTR bb;
public:
	B(const wchar_t* s)
		{
		bb = 0;
		if (s)
			bb = SysAllocString(s);
		}
	B(const char* s)
		{
		bb = 0;
		if (!s)
			return;

		int sl = lstrlenA(s);
		int nsl = sl* 2 + 100;
		wchar_t* ws = new wchar_t[nsl];
		memset(ws,0,sizeof(wchar_t)*nsl);
		MultiByteToWideChar(CP_UTF8,0,s,-1,ws,nsl);
		bb = SysAllocString(ws);
		delete[] ws;
		}
	~B()
		{
		if (bb)
			SysFreeString(bb);
		bb = 0;
		}
	operator BSTR()
		{
		return bb;
		}

};

template <typename I,typename Y> void GetEnumeration(IUnknown* x,vector<Y>& y)
{
I* e = 0;
if (x)
	{
	x->QueryInterface(__uuidof(I),(void**)&e);
	x->Release();
	}
if (e)
	{
	e->Reset();
	Y v;
	for(;;)
		{
		if (e->Next(1,&v,0) != S_OK)
			break;
		
		y.push_back(v);
		}
	}
}


class MyRDPSessionEvents : public _IRDPSessionEvents
{
private:

	int refNum;
	HWND nen;
	UINT msg;
	class BASERDP* Parent;
	

public:


	MyRDPSessionEvents();
	~MyRDPSessionEvents();
	void SetNotification(HWND,UINT,BASERDP* p);

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

// CHANNEL class
struct ATDATA
{
string PendingData;
class ATTENDEE* att;
bool PendingUnc;
};

class CHANNEL
{
private:
	void* ss;
	wstring n;
	BASERDP* Parent;

	// Per At data
	vector<ATDATA> AttPendingData;
	// All data
	string _PendingData;
	bool _PendingUnc;

public:
	CHANNEL(const wchar_t* n,BASERDP*,IRDPSRAPIVirtualChannel*);
	void Release();
	void* GetInterface() {return ss;}
	wstring& GetName() {return n;}
	void AddPendingData(char* d,int sz,bool Unc,class ATTENDEE* att = 0);
	void Flush();

};


class ATTENDEE 
{
private:

	void* ss;

	wstring n;
	long lid;
	int Protocol;
	wstring LocalIP;
	long LocalPort;
	wstring PeerIP;
	long PeerPort;
	bool IsTarget;



public:

	void SetControl(int x);
	void Release();
	void*& GetInterface() {return ss;}
	ATTENDEE(IDispatch*);
	~ATTENDEE();
	wstring& GetName() {return n;}
	bool& GetTarget() {return IsTarget;}
	int GetID() {return lid;}
	int GetProtocol() {return Protocol;}
	wstring GetLocalIP()  {return LocalIP;}
	wstring GetPeerIP()  {return PeerIP;}
	long GetLocalPort() {return LocalPort;}
	long GetPeerPort() {return PeerPort;}
	HRESULT Kill();

};

class BASERDP
{
friend class MyRDPSessionEvents;

protected:

	bool IsServer;
	void* s;
	string rcs;

	MyRDPSessionEvents ev;
	IConnectionPointContainer* picpc;
	IConnectionPoint* picp;
	int Cookie;
	HWND nen;
	UINT men;
	void* nDatap;
	void (*nData)(CHANNEL*,ATTENDEE*,char*,int,void*);


	// Channel
	vector<CHANNEL> ch;
	vector<ATTENDEE> Atts;

	virtual bool SetNotification(HWND,UINT,bool);
	virtual void HandleNotification(DISPID id,DISPPARAMS* p);

public:

	virtual void* GetInterface() {return s;}
	void SetWindowNotification(HWND,UINT);
	void SetDataNotification(void (*)(CHANNEL*,ATTENDEE*,char*,int,void*),void*);

	void AssignChannelToAttendees(CHANNEL* c,vector<ATTENDEE*>& AssignAtts);
	virtual CHANNEL* CreateVirtualChannel(const wchar_t* n,bool Compressed,int Priority);
	virtual HRESULT SendData(CHANNEL* C,char* d,int sz,bool Unc = false,vector<ATTENDEE*>* List = 0);
	virtual void OnReceiveData(CHANNEL* C,ATTENDEE* A,char* d,int sz);
	virtual void RemoveAtt(IRDPSRAPIAttendee*);
	virtual ATTENDEE* FindAtt(IRDPSRAPIAttendee*);
	vector<CHANNEL>& GetChannels() {return ch;}
	vector<ATTENDEE>& GetAttendees() {return Atts;}


};

class S_INVITATION
{
private:

	void* ss;
	bool Revoked;
	wstring cstr;
	wstring cpwd;
	wstring cgrp;
	wstring ticket;
	int climit;

public:

	S_INVITATION(wchar_t*str,wchar_t*pwd,wchar_t*grp,int limit);
	bool IsRevoked() {return Revoked;}
	wstring& GetString() {return cstr;}
	wstring& GetPassword() {return cpwd;}
	wstring& GetGroup() {return cstr;}
	wstring& GetTicket() {return ticket;}
	int& GetLimit() {return climit;}
	void*& GetInterface() {return ss;}
	bool Revoke(bool = true);


};


class SERVER : public BASERDP
{
private:
	vector<S_INVITATION> Invs;
	bool Init();
	void Shut();

public:

	SERVER();
	~SERVER();

	bool Open();
	bool ReverseOpen(const wchar_t* revstr);
	bool Close();
	bool TogglePause(bool P);
	bool SetConnectionParameters(int aftype = AF_INET,int port = 3389,bool DynamicMirror = true,int ColorDepth = 24,char* ReverseConnectionString = 0);
	bool GetConnectionParameters(int* aftype,int* port,bool* DynamicMirror,int* ColorDepth,char* ReverseConnectionString = 0);
	bool SetDepth(int);
	int  GetDepth();

	bool SetDesktopRegion(const RECT& rc);
	bool GetDesktopRegion(RECT& rc);
	const vector<S_INVITATION>& GetInvites() {return Invs;}
	void ShareOnlyTheseApplications(vector<int>& pid,bool);
	void GetShareableApplications(vector<int>& pids,vector<wstring>& names,vector<int>& ST);
	bool SetApplicationState(int pid,bool GlobalShare);
	bool GetApplicationState(int pid,bool* GlobalShare);

	bool SetGetApplicationState(int pid,int sg,bool& GlobalShare);
	bool SetGetApplicationSharedWindowList(int pid,int sg,vector<HWND>& h,vector<int>& shr);

	bool SetApplicationSharedWindowList(int pid,vector<HWND>& h);
	bool GetApplicationSharedWindowList(int pid,vector<HWND>& h);

	S_INVITATION* Invite(wchar_t*str,wchar_t*pwd,wchar_t*grp,int limit);

};

class CLIENT : public BASERDP
{
private:

	void Init(IUnknown* = 0);
	void Shut();
	wstring rev;

public:


	CLIENT(IUnknown* = 0);
	~CLIENT();
	bool Connect(const wchar_t*str,const wchar_t*name,const wchar_t* pwd);
	void SmartResize(bool);
	bool Disconnect();
	wstring SetReverseConnectionParameters(int prot,int port,const wchar_t* bstr,const wchar_t* n,const wchar_t* pwd);
	void RequestControl(int cl);
};
};


#endif