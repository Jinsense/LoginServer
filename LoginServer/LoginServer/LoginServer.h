#ifndef _LOGINSERVER_NETSERVER_LOGINSERVER_H_
#define _LOGINSERVER_NETSERVER_LOGINSERVER_H_

#include <list>
#include <map>

#include "Player.h"
#include "NetServer.h"
#include "DB_Connector.h"

class CLoginServer : public CNetServer
{
public:

	CLoginServer();
	virtual ~CLoginServer();

protected:
	//	�����Լ���...
	virtual void		OnClientJoin(st_SessionInfo Info);
	virtual void		OnClientLeave(unsigned __int64 iClientID);
	virtual void		OnConnectionRequest(WCHAR * pClientIP, int iPort);
	virtual void		OnError(int iErrorCode, WCHAR *pError);
	virtual bool		OnRecv(unsigned __int64 iClientID, CPacket *pPacket);

		//-----------------------------------------------------------
		// �α��μ��� �����췯 ����
		//-----------------------------------------------------------
	static unsigned __stdcall	UpdateThread(void *pParam);
	void		UpdateThread_Update();

	void		Schedule_PlayerTimeout(void);
	void		Schedule_ServerTimeout(void);

public:
	//	DB ��ü
	CDBConnector		_AccountDB;

	/////////////////////////////////////////////////////////////
	// ����� �����Լ���.  
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	// OnClientJoin, OnClientLeave ���� ȣ���.
	/////////////////////////////////////////////////////////////
	bool		InsertPlayer(st_Session iClientID);
	bool		RemovePlayer(st_Session iClientID);

	/////////////////////////////////////////////////////////////
	// OnRecv ���� �α������� ó�� �� ���,  UpdateThread ���� �����ð� ���� ������ ���.
	/////////////////////////////////////////////////////////////
	bool		DisconnectPlayer(st_Session iClientID);

	/////////////////////////////////////////////////////////////
	// ���� ����� �� 
	/////////////////////////////////////////////////////////////
	int			GetPlayerCount(void);


	/////////////////////////////////////////////////////////////
	// White IP ����
	/////////////////////////////////////////////////////////////

//	..WhiteIP ����� �����ϱ� �˻�, Ȯ�� �� �� �ִ� ���


protected:

	// �α��� ��û ��Ŷó��
	bool				PacketProc_ReqLogin(st_Session iClientID, CPacket *pPacket);

	// ��Ŷ ������
	bool				MakePacket_ResLogin(CPacket *pPacket, __int64 iAccountNo, WCHAR *szID, WCHAR *szNickname, BYTE byStatus);



protected:

	//-------------------------------------------------------------
	// ������ ����.
	// 
	// �����ڴ� �� ����Ʈ�� ������.  ������ ����ȭ�� ���� SRWLock �� ����Ѵ�.
	//-------------------------------------------------------------
	std::list<CPlayer *>	_PlayerList;
	SRWLOCK					_PlayerList_srwlock;

	CMemoryPool<CPlayer> *_PlayerPool;

	//-------------------------------------------------------------
	// ��Ÿ ����͸��� ����,������ �Լ�.
	//-------------------------------------------------------------
	long				_Monitor_LoginSuccessTPS;

	long				_Monitor_LoginWait;					// �α��� ��Ŷ ���� �� ������� ��

	long				_Monitor_LoginProcessTime_Max;		// �α��� ó�� �ð� �ִ�
	long				_Monitor_LoginProcessTime_Min;		// �α��� ó�� �ð� �ּ�
	long long			_Monitor_LoginProcessTime_Total;	// �� ��
	long long			_Monitor_LoginProcessCall_Total;	// �α��� ó�� ��û �� ��

private:

	long				_Monitor_LoginSuccessCounter;

	virtual void		MonitorThread_Update();
};

#endif _LOGINSERVER_NETSERVER_LOGINSERVER_H_






