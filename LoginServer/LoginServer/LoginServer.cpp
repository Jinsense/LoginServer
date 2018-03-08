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
	_Parameter = 0;

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
	m_iRecvPacketTPS++;

	CPlayer *pPlayer = FindPlayer_ClientID(iClientID);
	if (nullptr == pPlayer)
	{
		Disconnect(iClientID);
		return false;
	}

	WORD Type;
	*pPacket >> Type;

	if (en_PACKET_CS_LOGIN_REQ_LOGIN == Type)
	{
		bool bRes;
		
		*pPacket >> pPlayer->_AccountNo;
		pPacket->PopData(&pPlayer->_SessionKey[0], sizeof(pPlayer->_SessionKey[0]));
		
//		_AccountDB.Query(L"BEGIN");
		bRes = _AccountDB.Query(L"SELECT * FROM v_account WHERE accountno = %d", pPlayer->_AccountNo);
		MYSQL_ROW sql_row = _AccountDB.FetchRow();

		if (NULL == sql_row || false == bRes)
		{
			DisconnectPlayer(iClientID, dfLOGIN_STATUS_FAIL);
//			_AccountDB.Query(L"ROLLBACK");
			_AccountDB.FreeResult();
			return false;
		}

		//	SQL_ROW
		//	0 - accountno
		//	1 - userid
		//	2 - usernick
		//	3 - sessionkey
		//	4 - status
		pPlayer->_ID[0] = (WCHAR)sql_row[1];
		pPlayer->_NickName[0] = (WCHAR)sql_row[2];
		pPlayer->_SessionKey[0] = (char)sql_row[3];
		pPlayer->_Status = (int)sql_row[4];

		if (dfPLAYER_NOT_LOGIN != pPlayer->_Status)
		{
			if (dfPLAYER_GAMESERVER_REQ == pPlayer->_Status || dfPLAYER_GAMESERVER_LOGIN == pPlayer->_Status || dfPLAYER_GAMESERVER_LOGOUT_REQ == pPlayer->_Status)
			{
				//	로그인 서버 로그인 요청 상태가 아닌
				//	게임서버 플레이 상태일 경우
				//	추후 게임서버에 종료 요청을 보내야함
			}
			DisconnectPlayer(iClientID, dfLOGIN_STATUS_GAME);
//			_AccountDB.Query(L"ROLLBACK");
			_AccountDB.FreeResult();
			return false;
		}
		_AccountDB.FreeResult();
		/*
		pPlayer->_Status = dfPLAYER_LOGINSERVER_REQ;
		//	DB의 세션 상태 변경 
		_AccountDB.Query_Save(L"UPDATE v_account SET status = %d where accountno = %d", dfPLAYER_LOGINSERVER_REQ, pPlayer->_AccountNo);
//		_AccountDB.Query(L"COMMIT");
		_AccountDB.FreeResult();
		*/
		CPacket *pNewPacket = CPacket::Alloc();
		*pPacket << pPlayer->_AccountNo;
		pPacket->PushData(&pPlayer->_SessionKey[0], sizeof(pPlayer->_SessionKey));
		*pPacket << pPlayer->_Parameter++;

		PacketProc_ReqLogin(iClientID, pNewPacket);
		pNewPacket->Free();
		return true;
	}	
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

CPlayer* CLoginServer::FindPlayer_ClientID(unsigned __int64 iClientID)
{
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
	return pPlayer;
}

CPlayer* CLoginServer::FindPlayer_AccountNo(INT64 AccountNo)
{
	CPlayer *pPlayer;
	list<CPlayer*>::iterator iter;
	AcquireSRWLockExclusive(&_PlayerList_srwlock);
	for (iter = _PlayerList.begin(); iter != _PlayerList.end(); iter++)
	{
		if (AccountNo == (*iter)->_AccountNo)
		{
			pPlayer = (*iter);
			break;
		}
	}
	ReleaseSRWLockExclusive(&_PlayerList_srwlock);
	return pPlayer;
}

void CLoginServer::ChatResSessionKey(INT64 AccountNo, INT64 Parameter)
{
	CPlayer *pPlayer = FindPlayer_AccountNo(AccountNo);
	if (nullptr == pPlayer)
	{
		//	세션키 공유되기 전 클라이언트 종료로 체크
		return;
	}
	//	채팅서버 플래그 변경
	//	게임서버 완료됬는지 체크 후 둘다 완료 시 클라이언트에 완료 패킷 전송
	//	현재는 게임서버 체크 X
	InterlockedExchange(&pPlayer->_bChatServerComplete, true);

//	게임서버 추가 후 주석 해제
//	pPlayer->_Status = dfPLAYER_GAMESERVER_REQ;

	CPacket *pPacket = CPacket::Alloc();
	MakePacket_ResLogin(pPacket, pPlayer->_AccountNo, pPlayer->_ID, pPlayer->_NickName, dfLOGIN_STATUS_OK);
	SendPacketAndDisConnect(pPlayer->_ClientID, pPacket);
	pPacket->Free();

//	게임서버 추가 후 주석 해제
//	_AccountDB.Query_Save(L"UPDATE v_account SET status = %d where accountno = %d", dfPLAYER_GAMESERVER_REQ, pPlayer->_AccountNo);
	return;
}

void CLoginServer::GameResSessionKey(INT64 AccountNo, INT64 Parameter)
{
	//	게임서버 플래그 변경
	//	채팅서버 완료됬는지 체크 후 둘다 완료 시 클라이언트에 완료 패킷 전송
	//	현재는 게임서버 체크 X
	return;
}

bool CLoginServer::PacketProc_ReqLogin(unsigned __int64 iClientID, CPacket *pPacket)
{
	_LanServer.ChatReqLoginSendPacket(pPacket);
//	_LanServer.GameReqLoginSendPacket(iClientID, pPacket);
	return true;
}

bool CLoginServer::MakePacket_ResLogin(CPacket *pPacket, __int64 iAccountNo, WCHAR *szID, WCHAR *szNickname, BYTE byStatus)
{
	WORD Type = en_PACKET_CS_LOGIN_RES_LOGIN;
	*pPacket << Type << iAccountNo << byStatus;
	pPacket->PushData((char*)szID, sizeof(szID));
	pPacket->PushData((char*)szNickname, sizeof(szNickname));
	//	추후 게임서버로 변경
	pPacket->PushData((char)inet_ntoa(_LanServer._ChatServerInfo.Addr.sin_addr));
	*pPacket << _LanServer._ChatServerInfo.Addr.sin_port;

	pPacket->PushData((char)inet_ntoa(_LanServer._ChatServerInfo.Addr.sin_addr));
	*pPacket << _LanServer._ChatServerInfo.Addr.sin_port;
	
	return true;
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
//			wprintf(L"	Lan_Accept_TPS			:	%I64d	\n", _LanServer.m_iAcceptTPS);
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