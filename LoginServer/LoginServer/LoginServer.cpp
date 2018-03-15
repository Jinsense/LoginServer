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

	InitializeSRWLock(&_DB_srwlock);
	InitializeSRWLock(&_PlayerList_srwlock);
	_PlayerPool = new CMemoryPool<PLAYER>();
	_pLanServer = new CLanServer;
	_pLanServer->Set(this);
}

CLoginServer::~CLoginServer()
{
	delete _PlayerPool;
	delete _pLanServer;
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

	PLAYER *pPlayer = FindPlayer_ClientID(iClientID);
	if (nullptr == pPlayer)
	{
		Disconnect(iClientID);
		return false;
	}

	WORD Type;
	*pPacket >> Type;

	if (en_PACKET_CS_LOGIN_REQ_LOGIN == Type)
	{
	//	int iNum;
		bool bRes;
		
		*pPacket >> pPlayer->_AccountNo;
		pPacket->PopData(&pPlayer->_SessionKey[0], sizeof(pPlayer->_SessionKey[0]));
		
		AcquireSRWLockExclusive(&_PlayerList_srwlock);
//		_AccountDB.Query(L"BEGIN");
		bRes = _AccountDB.Query(L"SELECT * FROM v_account WHERE accountno = %d", pPlayer->_AccountNo);
//		iNum = _AccountDB.FetchNum();
		MYSQL_ROW sql_row = _AccountDB.FetchRow();
	
		if (NULL == sql_row || false == bRes)
		{
			DisconnectPlayer(iClientID, dfLOGIN_STATUS_FAIL);
//			_AccountDB.Query(L"ROLLBACK");
			_AccountDB.FreeResult();
			ReleaseSRWLockExclusive(&_PlayerList_srwlock);
			return false;
		}

		//	SQL_ROW
		//	0 - accountno
		//	1 - userid
		//	2 - usernick
		//	3 - sessionkey
		//	4 - status

		UTF8toUTF16(sql_row[1], pPlayer->_ID, sizeof(pPlayer->_ID));
		UTF8toUTF16(sql_row[2], pPlayer->_NickName, sizeof(pPlayer->_NickName));
//		pPlayer->_SessionKey[0] = *sql_row[3];
		if (NULL != sql_row[3])
			strcpy_s(pPlayer->_SessionKey, sizeof(pPlayer->_SessionKey), sql_row[3]);
		else
			pPlayer->_SessionKey[0] = NULL;

		pPlayer->_Status = atoi(sql_row[4]);
//		pPlayer->_Status = (int)*sql_row[4];


		_AccountDB.FreeResult();		
	

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
//			_AccountDB.FreeResult();
//			ReleaseSRWLockExclusive(&_PlayerList_srwlock);
			return false;
		}
		ReleaseSRWLockExclusive(&_PlayerList_srwlock);
		/*
		pPlayer->_Status = dfPLAYER_LOGINSERVER_REQ;
		//	DB의 세션 상태 변경 
		AcquireSRWLockExclusive(&_PlayerList_srwlock);
		_AccountDB.Query_Save(L"UPDATE v_account SET status = %d where accountno = %d", itoa(dfPLAYER_LOGINSERVER_REQ), pPlayer->_AccountNo);
//		_AccountDB.Query(L"COMMIT");
		_AccountDB.FreeResult();
		ReleaseSRWLockExclusive(&_PlayerList_srwlock);
		*/
		Type = en_PACKET_SS_REQ_NEW_CLIENT_LOGIN;
		CPacket *pNewPacket = CPacket::Alloc();
		*pNewPacket << Type << pPlayer->_AccountNo;
		pNewPacket->PushData(&pPlayer->_SessionKey[0], sizeof(pPlayer->_SessionKey));
		*pNewPacket << pPlayer->_Parameter++;

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
	list<PLAYER*>::iterator iter;
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

void CLoginServer::UTF8toUTF16(const char *szText, WCHAR *szBuf, int iBufLen)
{
	int iRe = MultiByteToWideChar(CP_UTF8, 0, szText, strlen(szText), szBuf, iBufLen);
	if (iRe < iBufLen)
		szBuf[iRe] = L'\0';
	return;
}

void CLoginServer::UTF16toUTF8(WCHAR *szText, char *szBuf, int iBufLen)
{
	int iRe = WideCharToMultiByte(CP_UTF8, 0, szText, lstrlenW(szText), szBuf, iBufLen, NULL, NULL);
	return;
}

bool CLoginServer::InsertPlayer(unsigned __int64 iClientID)
{
	//	OnClientJoin에서 호출
	PLAYER *pPlayer = _PlayerPool->Alloc();
	pPlayer->_ClientID = iClientID;

	AcquireSRWLockExclusive(&_PlayerList_srwlock);
	_PlayerList.push_back(pPlayer);
	ReleaseSRWLockExclusive(&_PlayerList_srwlock);
	return true;
}

bool CLoginServer::RemovePlayer(unsigned __int64 iClientID)
{
	//	OnClientLeave에서 호출
	list<PLAYER*>::iterator iter;
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
	PLAYER *pPlayer = nullptr;
	list<PLAYER*>::iterator iter;
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

	if (nullptr == pPlayer)
	{
		return false;
	}

	WORD Type = en_PACKET_CS_LOGIN_RES_LOGIN;
	BYTE Status = byStatus;

	*pPacket << Type << pPlayer->_AccountNo << Status;
	pPacket->PushData((char*)pPlayer->_ID, sizeof(pPlayer->_ID));
	pPacket->PushData((char*)pPlayer->_NickName, sizeof(pPlayer->_NickName));
	//	추후 게임서버로 변경
	pPacket->PushData((char)inet_ntoa(_pLanServer->_ChatServerInfo.Addr.sin_addr));
	*pPacket << _pLanServer->_ChatServerInfo.Addr.sin_port;

	pPacket->PushData((char)inet_ntoa(_pLanServer->_ChatServerInfo.Addr.sin_addr));
	*pPacket << _pLanServer->_ChatServerInfo.Addr.sin_port;

	SendPacketAndDisConnect(iClientID, pPacket);
	pPacket->Free();
	return true;
}

int CLoginServer::GetPlayerCount()
{
	return _PlayerList.size();
}

PLAYER* CLoginServer::FindPlayer_ClientID(unsigned __int64 iClientID)
{
	PLAYER *pPlayer = nullptr;
	list<PLAYER*>::iterator iter;
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

PLAYER* CLoginServer::FindPlayer_AccountNo(INT64 AccountNo)
{
	PLAYER *pPlayer = nullptr;
	list<PLAYER*>::iterator iter;
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
	PLAYER *pPlayer = FindPlayer_AccountNo(AccountNo);
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

	WORD Type = en_PACKET_CS_LOGIN_RES_LOGIN;
	BYTE Status = dfLOGIN_STATUS_OK;

	CPacket *pPacket = CPacket::Alloc();
	*pPacket << Type << pPlayer->_AccountNo << Status;
	pPacket->PushData((char*)pPlayer->_ID, sizeof(pPlayer->_ID));
	pPacket->PushData((char*)pPlayer->_NickName, sizeof(pPlayer->_NickName));
	//	추후 게임서버로 변경
	pPacket->PushData((char)inet_ntoa(_pLanServer->_ChatServerInfo.Addr.sin_addr));
	*pPacket << _pLanServer->_ChatServerInfo.Addr.sin_port;

	pPacket->PushData((char)inet_ntoa(_pLanServer->_ChatServerInfo.Addr.sin_addr));
	*pPacket << _pLanServer->_ChatServerInfo.Addr.sin_port;
	
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
	_pLanServer->ChatReqLoginSendPacket(pPacket);
//	_pLanServer->GameReqLoginSendPacket(iClientID, pPacket);
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

	while (!m_bShutdown)
	{
		Sleep(1000);
		timer = time(NULL);
		localtime_s(t, &timer);

		if (true == m_bMonitorFlag)
		{
			wprintf(L"	[ServerStart : %d/%d/%d %d:%d:%d]\n\n", year, month, day, hour, min, sec);
			wprintf(L"	[%d/%d/%d %d:%d:%d]\n\n", t->tm_year + 1900, t->tm_mon + 1,
				t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

			wprintf(L"	ConnectSession			:	%I64d	\n", m_iConnectClient);
			wprintf(L"	MemoryPool_AllocCount		:	%d	\n\n", CPacket::GetAllocPool());

			wprintf(L"	LoginWait			:	%d	\n", _Monitor_LoginWait);
			wprintf(L"	SessionKey_UseCount		:	%d	\n\n", GetPlayerCount());

			wprintf(L"	LoginSuccessTPS			:	%d	\n", _Monitor_LoginSuccessTPS);
			wprintf(L"	LoginSuccessCounter		:	%d	\n\n", _Monitor_LoginSuccessCounter);

			wprintf(L"	LoginProcessTime_Max		:	%d	\n", _Monitor_LoginProcessTime_Max);
			wprintf(L"	LoginProcessTime_Min		:	%d	\n", _Monitor_LoginProcessTime_Min);
			if (0 != _Monitor_LoginProcessCall_Total)
				wprintf(L"	LoginProcessTime_Avr		:	%lf	\n", _Monitor_LoginProcessTime_Total % _Monitor_LoginProcessCall_Total);
			else
				wprintf(L"	LoginProcessTime_Avr		:	0	\n");
			wprintf(L"\n");
			wprintf(L"	Login_Accept_Total		:	%I64d	\n", m_iAcceptTotal);
			wprintf(L"	Login_Accept_TPS		:	%I64d	\n", m_iAcceptTPS);
			wprintf(L"	Login_SendPacket_TPS		:	%I64d	\n", m_iSendPacketTPS);
			wprintf(L"	Login_RecvPacket_TPS		:	%I64d	\n\n", m_iRecvPacketTPS);

			wprintf(L"	Lan_Connection			:	%I64d	\n", _pLanServer->m_iConnectClient);
			wprintf(L"	Lan_Accept_Total		:	%I64d	\n", _pLanServer->m_iAcceptTotal);
//			wprintf(L"	Lan_Accept_TPS			:	%I64d	\n", _pLanServer->m_iAcceptTPS);
			wprintf(L"	Lan_SendPacket_TPS		:	%I64d	\n", _pLanServer->m_iSendPacketTPS);
			wprintf(L"	Lan_RecvPacket_TPS		:	%I64d	\n\n", _pLanServer->m_iRecvPacketTPS);
		}
		m_iAcceptTPS = 0;
		m_iRecvPacketTPS = 0;
		m_iSendPacketTPS = 0;
		_pLanServer->m_iAcceptTPS = 0;
		_pLanServer->m_iSendPacketTPS = 0;
		_pLanServer->m_iRecvPacketTPS = 0;

	}
	delete t;
}