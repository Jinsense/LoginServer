#ifndef _LOGINSERVER_NETSERVER_LOGINSERVER_H_
#define _LOGINSERVER_NETSERVER_LOGINSERVER_H_

#include <list>
#include <map>

#include "Player.h"
#include "LanServer.h"
#include "NetServer.h"
#include "DB_Connector.h"

#define		df_PLAYER_TIMEOUT		10000

enum en_PLAYER_STATUS
{
	dfPLAYER_NOT_LOGIN = 0,			//	�α��� ���� �ƴ�, �α׾ƿ� ����
	dfPLAYER_LOGINSERVER_REQ = 1,	//	�α��� ��û ����
	dfPLAYER_LOGINSERVER_LOGIN = 2,	//	�α��� ���� �α��� ����
	dfPLAYER_GAMESERVER_REQ = 3,	//	���Ӽ����� �α����ϴ� ����
	dfPLAYER_GAMESERVER_LOGIN = 4,	//	���Ӽ��� ���� ���� ����
	dfPLAYER_GAMESERVER_LOGOUT_REQ = 5,		//	���Ӽ��� �α׾ƿ� ��û����
};

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
	CLanServer			_LanServer;
	CDBConnector		_AccountDB;

	/////////////////////////////////////////////////////////////
	// ����� �����Լ���.  
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	// OnClientJoin, OnClientLeave ���� ȣ���.
	/////////////////////////////////////////////////////////////
	bool		InsertPlayer(unsigned __int64 iClientID);
	bool		RemovePlayer(unsigned __int64 iClientID);

	/////////////////////////////////////////////////////////////
	// OnRecv ���� �α������� ó�� �� ���,  UpdateThread ���� �����ð� ���� ������ ���.
	/////////////////////////////////////////////////////////////
	bool		DisconnectPlayer(unsigned __int64 iClientID, BYTE byStatus);

	/////////////////////////////////////////////////////////////
	// ���� ����� �� 
	/////////////////////////////////////////////////////////////
	int			GetPlayerCount(void);

	/////////////////////////////////////////////////////////////
	// ������ �÷��̾� ã�� 
	/////////////////////////////////////////////////////////////
	CPlayer*	FindPlayer_ClientID(unsigned __int64 iClientID);
	CPlayer*	FindPlayer_AccountNo(INT64 AccountNo);

	/////////////////////////////////////////////////////////////
	// ����Ű ���� �Ϸ� 
	/////////////////////////////////////////////////////////////
	void		ChatResSessionKey(INT64 AccountNo, INT64 Parameter);
	void		GameResSessionKey(INT64 AccountNo, INT64 Parameter);

	/////////////////////////////////////////////////////////////
	// White IP ����
	/////////////////////////////////////////////////////////////

//	..WhiteIP ����� �����ϱ� �˻�, Ȯ�� �� �� �ִ� ���


protected:

	// �α��� ��û ��Ŷó��
	bool				PacketProc_ReqLogin(unsigned __int64 iClientID, CPacket *pPacket);

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

	unsigned __int64	_Parameter;
private:

	long				_Monitor_LoginSuccessCounter;

	virtual void		MonitorThread_Update();
};

#endif _LOGINSERVER_NETSERVER_LOGINSERVER_H_






