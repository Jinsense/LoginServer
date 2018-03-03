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
}

CLoginServer::~CLoginServer()
{

}

void CLoginServer::OnClientJoin(st_SessionInfo Info)
{
	CPlayer *pPlayer = _PlayerPool->Alloc();
	pPlayer->_ClientID = Info.iClientID;

	AcquireSRWLockExclusive(&_PlayerList_srwlock);
	_PlayerList.push_back(pPlayer);
	ReleaseSRWLockExclusive(&_PlayerList_srwlock);
	return;
}

void CLoginServer::OnClientLeave(unsigned __int64 iClientID)
{
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

	return true;
}


unsigned __stdcall CLoginServer::UpdateThread(void *pParam)
{

}

void CLoginServer::UpdateThread_Update()
{

}

void CLoginServer::Schedule_PlayerTimeout()
{

}

void CLoginServer::Schedule_ServerTimeout()
{

}

bool CLoginServer::InsertPlayer(st_Session iClientID)
{

	return true;
}

bool CLoginServer::RemovePlayer(st_Session iClientID)
{

	return true;
}

bool CLoginServer::DisconnectPlayer(st_Session iClientID)
{

	return true;
}

int CLoginServer::GetPlayerCount()
{
	return _PlayerList.size();
}

bool CLoginServer::PacketProc_ReqLogin(st_Session iClientID, CPacket *pPacket)
{

	return true;
}

bool CLoginServer::MakePacket_ResLogin(CPacket *pPacket, __int64 iAccountNo, WCHAR *szID, WCHAR *szNickname, BYTE byStatus)
{

}

void CLoginServer::MonitorThread_Update()
{

}