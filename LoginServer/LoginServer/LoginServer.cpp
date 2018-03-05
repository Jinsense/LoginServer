#include "LoginServer.h"

using namespace std;

CLoginServer::CLoginServer()
{
	_Monitor_LoginSuccessTPS = 0;
	_Monitor_LoginWait = 0;
	_Monitor_LoginProcessTime_Max = 0;
	_Monitor_LoginProcessTime_Min = 0;
	_Monitor_LoginProcessTime_Total = 0;
	_Monitor_LoginProcessCall_Total = 0;
	_Monitor_LoginSuccessCounter = 0;

	InitializeSRWLock(&_PlayerList_srwlock);
	_PlayerPool = new CMemoryPool<CPlayer>();
	_LanServer.Set(this);
}

CLoginServer::~CLoginServer()
{
	delete _PlayerPool;
}

void CLoginServer::OnClientJoin(st_SessionInfo Info)
{
	InsertPlayer(Info.iClientID);
	return;
}

void CLoginServer::OnClientLeave(unsigned __int64 iClientID)
{
	RemovePlayer(iClientID);
	return;
}

void CLoginServer::OnConnectionRequest(WCHAR * pClientIP, int iPort)
{

}

void CLoginServer::OnError(int iErrorCode, WCHAR *pError)
{

}

bool CLoginServer::OnRecv(unsigned __int64 iClientID, CPacket *pPacket)
{
	//	패킷의 타입 검사

	//	로그인 요청 패킷일 경우
	//	DB 쿼리 날려서 해당 테이블에서 정보 얻어와서 
	//	현재 로그인 중인지 플래그 체크와
	//	DB에서 얻은 세션키와 패킷의 세션키 비교 ( 현재는 더미테스트라 달라도 허용 )

	//	정상 접속일 경우 게임서버(현재없음), 채팅서버에 세션키 공유 패킷 전송 

	//	세션키 공유 패킷일 경우 플래그 변경 및 플래그 체크
	//	전부 완료일 경우 클라이언트에 로그인 허용 패킷 전송
	
	return true;
}


unsigned __stdcall CLoginServer::UpdateThread(void *pParam)
{
	CLoginServer *pUpdateThread = (CLoginServer*)pParam;
	if (NULL == pUpdateThread)
	{
		wprintf(L"[LoginServer :: UpdateThread] Init Error\n");
		return false;
	}
	pUpdateThread->UpdateThread_Update();
	return true;
}

void CLoginServer::UpdateThread_Update()
{
	while (1)
	{
		Sleep(3000);
//		Schedule_PlayerTimeout();
//		Schedule_ServerTimeout();
	}
}

void CLoginServer::Schedule_PlayerTimeout()
{
	//	로그인 서버에 접속한 유저 타임아웃 체크
	list<CPlayer*>::iterator iter;
	__int64 CurrentTick = GetTickCount64();
	AcquireSRWLockExclusive(&_PlayerList_srwlock);
	for (iter = _PlayerList.begin(); iter != _PlayerList.end(); iter++)
	{
		if (df_PLAYER_TIMEOUT <= (CurrentTick - (*iter)->_LoginTime))
		{
			DisconnectPlayer((*iter)->_ClientID, dfLOGIN_STATUS_NONE);
		}
	}
	ReleaseSRWLockExclusive(&_PlayerList_srwlock);
	return;
}

void CLoginServer::Schedule_ServerTimeout()
{
	//	랜서버 연결된 서버 타임아웃 체크
	return;
}

bool CLoginServer::InsertPlayer(unsigned __int64 iClientID)
{
	//	OnClientJoin에서 호출
	CPlayer *pPlayer = _PlayerPool->Alloc();
	pPlayer->_ClientID = iClientID;

	AcquireSRWLockExclusive(&_PlayerList_srwlock);
	_PlayerList.push_back(pPlayer);
	ReleaseSRWLockExclusive(&_PlayerList_srwlock);
	return true;
}

bool CLoginServer::RemovePlayer(unsigned __int64 iClientID)
{
	//	OnClientLeave에서 호출
	list<CPlayer*>::iterator iter;
	AcquireSRWLockExclusive(&_PlayerList_srwlock);
	for (iter = _PlayerList.begin(); iter != _PlayerList.end(); iter++)
	{
		if (iClientID == (*iter)->_ClientID)
		{
			_PlayerList.erase(iter);
			break;
		}
	}
	ReleaseSRWLockExclusive(&_PlayerList_srwlock);
	return true;
}

bool CLoginServer::DisconnectPlayer(unsigned __int64 iClientID, BYTE byStatus)
{
	CPacket *pPacket = CPacket::Alloc();
	CPlayer *pPlayer;
	list<CPlayer*>::iterator iter;
	AcquireSRWLockExclusive(&_PlayerList_srwlock);
	for (iter = _PlayerList.begin(); iter != _PlayerList.end(); iter++)
	{
		if (iClientID == (*iter)->_ClientID)
		{
			pPlayer = (*iter);
			break;
		}
	}
	ReleaseSRWLockExclusive(&_PlayerList_srwlock);

	if (dfLOGIN_STATUS_NONE == byStatus)
	{
		MakePacket_ResLogin(pPacket, pPlayer->_AccountNo, pPlayer->_ID, pPlayer->_NickName, dfLOGIN_STATUS_NONE);
	}
	else if(dfLOGIN_STATUS_FAIL == byStatus)
	{
		MakePacket_ResLogin(pPacket, pPlayer->_AccountNo, pPlayer->_ID, pPlayer->_NickName, dfLOGIN_STATUS_NONE);
	}
	else if (dfLOGIN_STATUS_OK == byStatus)
	{
		MakePacket_ResLogin(pPacket, pPlayer->_AccountNo, pPlayer->_ID, pPlayer->_NickName, dfLOGIN_STATUS_OK);
	}
	else if (dfLOGIN_STATUS_GAME == byStatus)
	{
		MakePacket_ResLogin(pPacket, pPlayer->_AccountNo, pPlayer->_ID, pPlayer->_NickName, dfLOGIN_STATUS_GAME);
	}
	else if (dfLOGIN_STATUS_ACCOUNT_MISS == byStatus)
	{
		MakePacket_ResLogin(pPacket, pPlayer->_AccountNo, pPlayer->_ID, pPlayer->_NickName, dfLOGIN_STATUS_ACCOUNT_MISS);
	}
	else if (dfLOGIN_STATUS_SESSION_MISS == byStatus)
	{
		MakePacket_ResLogin(pPacket, pPlayer->_AccountNo, pPlayer->_ID, pPlayer->_NickName, dfLOGIN_STATUS_SESSION_MISS);
	}
	else if (dfLOGIN_STATUS_STATUS_MISS == byStatus)
	{
		MakePacket_ResLogin(pPacket, pPlayer->_AccountNo, pPlayer->_ID, pPlayer->_NickName, dfLOGIN_STATUS_STATUS_MISS);
	}
	else if (dfLOGIN_STATUS_NOSERVER == byStatus)
	{
		MakePacket_ResLogin(pPacket, pPlayer->_AccountNo, pPlayer->_ID, pPlayer->_NickName, dfLOGIN_STATUS_NOSERVER);
	}
	SendPacketAndDisConnect(iClientID, pPacket);
	return true;
}

int CLoginServer::GetPlayerCount()
{
	return _PlayerList.size();
}

bool CLoginServer::PacketProc_ReqLogin(unsigned __int64 iClientID, CPacket *pPacket)
{

	return true;
}

bool CLoginServer::MakePacket_ResLogin(CPacket *pPacket, __int64 iAccountNo, WCHAR *szID, WCHAR *szNickname, BYTE byStatus)
{
	WORD Type = en_PACKET_CS_LOGIN_RES_LOGIN;
	*pPacket << Type << iAccountNo << byStatus;
	pPacket->PushData((char*)szID, sizeof(szID));
	pPacket->PushData((char*)szNickname, sizeof(szNickname));

	

}

void CLoginServer::MonitorThread_Update()
{
	wprintf(L"\n");
	
	struct tm *t = new struct tm;
	time_t timer;

	timer = time(NULL);
	localtime_s(t, &timer);

	int year = t->tm_year + 1900;
	int month = t->tm_mon + 1;
	int day = t->tm_mday;
	int hour = t->tm_hour;
	int min = t->tm_min;
	int sec = t->tm_sec;

	while (m_bShutdown)
	{
		Sleep(1000);
		timer = time(NULL);
		if (true == m_bMonitorFlag)
		{
			wprintf(L"	[ServerStart : %d/%d/%d %d:%d:%d]\n\n", year, month, day, hour, min, sec);
			wprintf(L"	[%d/%d/%d %d:%d:%d]\n\n", t->tm_year + 1900, t->tm_mon + 1,
				t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

			wprintf(L"	ConnectSession			:	%I64d	\n", m_iConnectClient);
			wprintf(L"	MemoryPool_AllocCount		:	%d	\n\n", CPacket::GetAllocPool());

			wprintf(L"	LoginWait			:	%d	\n", _Monitor_LoginWait);
			wprintf(L"	SessionKey_UseCount			:	%d	\n\n", GetPlayerCount());

			wprintf(L"	LoginSuccessTPS			:	%d	\n", _Monitor_LoginSuccessTPS);
			wprintf(L"	LoginSuccessCounter			:	%d	\n\n", _Monitor_LoginSuccessCounter);

			wprintf(L"	LoginProcessTime_Max		:	%d	\n", _Monitor_LoginProcessTime_Max);
			wprintf(L"	LoginProcessTime_Min			:	%d	\n", _Monitor_LoginProcessTime_Min);
			wprintf(L"	LoginProcessTime_Avr			:	%I64d	\n\n", _Monitor_LoginProcessTime_Total/ _Monitor_LoginProcessCall_Total);

			wprintf(L"	Login_Accept_Total			:	%I64d	\n", m_iAcceptTotal);
			wprintf(L"	Login_Accept_TPS			:	%I64d	\n", m_iAcceptTPS);
			wprintf(L"	Login_SendPacket_TPS			:	%I64d	\n", m_iSendPacketTPS);
			wprintf(L"	Login_RecvPacket_TPS			:	%I64d	\n", m_iRecvPacketTPS);

			wprintf(L"	Lan_Connection			:	%I64d	\n", _LanServer.m_iConnectClient);
			wprintf(L"	Lan_Accept_Total			:	%I64d	\n", _LanServer.m_iAcceptTotal);
			wprintf(L"	Lan_Accept_TPS			:	%I64d	\n", _LanServer.m_iAcceptTPS);
			wprintf(L"	Lan_SendPacket_TPS			:	%I64d	\n", _LanServer.m_iSendPacketTPS);
			wprintf(L"	Lan_RecvPacket_TPS			:	%I64d	\n\n", _LanServer.m_iRecvPacketTPS);
		}
		m_iAcceptTPS = 0;
		m_iRecvPacketTPS = 0;
		m_iSendPacketTPS = 0;
		_LanServer.m_iAcceptTPS = 0;
		_LanServer.m_iSendPacketTPS = 0;
		_LanServer.m_iRecvPacketTPS = 0;

	}
	delete t;
}