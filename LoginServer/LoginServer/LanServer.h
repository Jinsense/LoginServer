#ifndef _LOGINSERVER_LANSERVER_LANSERVER_H_
#define _LOGINSERVER_LANSERVER_LANSERVER_H_

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "winmm.lib")

#include "Packet.h"
#include "RingBuffer.h"
#include "MemoryPool.h"
#include "LockFreeStack.h"
#include "LockFreeQueue.h"
#include "LoginServer.h"


#define		LAN_WSABUF_NUMBER		100
#define		LAN_QUEUE_SIZE			10000
#define		LAN_HEADER_SIZE			2

#define		SET_INDEX(Index, SessionKey)		Index = Index << 48; SessionKey = Index | SessionKey;
#define		GET_INDEX(Index, SessionKey)		Index = SessionKey >> 48;

struct st_ServerInfo
{
	SOCKADDR_IN Addr;
	WCHAR ServerName[32];
};

struct st_SessionInfo
{
	unsigned __int64 iSessionKey;
	SOCKADDR_IN Addr;
	BYTE byServerType;

	st_SessionInfo() :
		iSessionKey(NULL) {}
};

struct st_IO_RELEASE_COMPARE
{
	__int64	iIOCount;
	__int64	iReleaseFlag;

	st_IO_RELEASE_COMPARE() :
		iIOCount(0),
		iReleaseFlag(false) {}
};

struct st_Session
{
	bool				bLoginFlag;
	bool				bRelease;
	long				lIOCount;
	long				lSendFlag;
	long				lSendCount;
	unsigned __int64	iSessionKey;
	SOCKET				sock;
	OVERLAPPED			SendOver;
	OVERLAPPED			RecvOver;
	CRingBuffer			RecvQ;
	CRingBuffer			PacketQ;
	CLockFreeQueue<CPacket*> SendQ;
	st_SessionInfo		Info;

	st_Session() :
		RecvQ(LAN_QUEUE_SIZE),
		PacketQ(LAN_QUEUE_SIZE),
		lIOCount(0),
		lSendFlag(true) {}
};

class CLoginServer;

class CLanServer
{
public:
	CLanServer();
	~CLanServer();

	void				Disconnect(unsigned __int64 iSessionKey);
	/*virtual void		OnClientJoin(st_SessionInfo *pInfo) = 0;
	virtual void		OnClientLeave(unsigned __int64 iSessionKey) = 0;
	virtual void		OnConnectionRequest(WCHAR * pClientIP, int iPort) = 0;
	virtual void		OnError(int iErrorCode, WCHAR *pError) = 0;*/
	unsigned __int64	GetClientCount();

	bool				Set(CLoginServer *pLoginServer);
	bool				ServerStart(char *pOpenIP, int iPort, int iMaxWorkerThread,
		bool bNodelay, int iMaxSession);
	bool				ServerStop();
	bool				SendPacket(unsigned __int64 iSessionKey, CPacket *pPacket);
	bool				GetShutDownMode() { return m_bShutdown; }
	bool				GetWhiteIPMode() { return m_bWhiteIPMode; }
	bool				SetShutDownMode(bool bFlag);
	bool				SetWhiteIPMode(bool bFlag);

	st_Session*			SessionAcquireLock(unsigned __int64 SessionKey);
	void				SessionAcquireFree(st_Session *pSession);

private:
	bool				ServerInit();
	bool				ClientShutdown(st_Session *pSession);
	bool				ClientRelease(st_Session *pSession);

	static unsigned int WINAPI WorkerThread(LPVOID arg)
	{
		CLanServer *_pWorkerThread = (CLanServer *)arg;
		if (_pWorkerThread == NULL)
		{
			wprintf(L"[Server :: WorkerThread]	Init Error\n");
			return false;
		}
		_pWorkerThread->WorkerThread_Update();
		return true;
	}

	static unsigned int WINAPI AcceptThread(LPVOID arg)
	{
		CLanServer *_pAcceptThread = (CLanServer*)arg;
		if (_pAcceptThread == NULL)
		{
			wprintf(L"[Server :: AcceptThread]	Init Error\n");
			return false;
		}
		_pAcceptThread->AcceptThread_Update();
		return true;
	}

	void				PutIndex(unsigned __int64 iIndex);
	void				WorkerThread_Update();
	void				AcceptThread_Update();
	void				StartRecvPost(st_Session *pSession);
	void				RecvPost(st_Session *pSession);
	void				SendPost(st_Session *pSession);
	void				CompleteRecv(st_Session *pSession, DWORD dwTransfered);
	void				CompleteSend(st_Session *pSession, DWORD dwTransfered);
	bool				OnRecv(st_Session *pSession, CPacket *pPacket);
	unsigned __int64*	GetIndex();

public:
	unsigned __int64		m_iAcceptTPS;
	unsigned __int64		m_iAcceptTotal;
	unsigned __int64		m_iRecvPacketTPS;
	unsigned __int64		m_iSendPacketTPS;
	unsigned __int64		m_iConnectClient;

	st_ServerInfo			_ChatServerInfo;
	st_ServerInfo			_GameServerInfo;

private:
	CLockFreeStack<UINT64*>	SessionStack;
	st_IO_RELEASE_COMPARE	*pIOCompare;
	st_Session				*pSessionArray;
	SOCKET					m_listensock;
	CRITICAL_SECTION		m_SessionCS;

	HANDLE					m_hIOCP;
	HANDLE					m_hWorkerThread[100];
	HANDLE					m_hAcceptThread;
	HANDLE					m_hMonitorThread;
	HANDLE					m_hAllthread[200];

	bool					m_bWhiteIPMode;
	bool					m_bShutdown;

	unsigned __int64		m_iAllThreadCnt;
	unsigned __int64		*pIndex;
	unsigned __int64		m_iSessionKeyCnt;

	CLoginServer			*pLogin;
};

#endif _LOGINSERVER_LANSERVER_LANSERVER_H_