#ifndef _LOGINSERVER_NETSERVER_LOGINSERVER_H_
#define _LOGINSERVER_NETSERVER_LOGINSERVER_H_

#include <list>
#include <map>

#include "LanServer.h"
#include "NetServer.h"
#include "DB_Connector.h"

#define		df_PLAYER_TIMEOUT		10000

typedef struct st_Player
{
	st_Player()
	{
		_bGameServerComplete = false;
		_bChatServerComplete = false;
		_LoginTime = NULL;
		_ClientID = NULL;
		_AccountNo = NULL;

		ZeroMemory(_ID, sizeof(_ID));
		ZeroMemory(_NickName, sizeof(_NickName));
		ZeroMemory(_SessionKey, sizeof(_SessionKey));

		_ClientPort = NULL;
		_Status = NULL;
		_Parameter = 1;
	}
	long	_bGameServerComplete;		//	���Ӽ��� ����Ű ���� Ȯ��
	long	_bChatServerComplete;		//	ä�ü��� ����Ű ���� Ȯ��
	unsigned __int64	_LoginTime;		//	���� �α��� �ð� ( Ÿ�Ӿƿ� üũ )

	unsigned __int64	_ClientID;		//	NetServer���� ������ Ŭ���̾�Ʈ ������ȣ
	INT64		_AccountNo;				//	DB�� ����� ���� ������ȣ
	WCHAR		_ID[20];				//	ID - null����
	WCHAR		_NickName[20];			//	Nick - null����
	char		_SessionKey[64];		//	���������� ���� ����Ű

	in_addr		_ClientIP;				//	Ŭ���̾�Ʈ ���� IP
	unsigned short _ClientPort;			//	Ŭ���̾�Ʈ ���� Port

	int		_Status;				//	Ŭ���̾�Ʈ ���� 
	INT64		_Parameter;				//	����Ű �Ķ���� Ȯ��
}PLAYER;

enum en_PLAYER_STATUS
{
	dfPLAYER_NOT_LOGIN = 0,			//	�α��� ���� �ƴ�, �α׾ƿ� ����
	dfPLAYER_LOGINSERVER_REQ = 1,	//	�α��� ��û ����
	dfPLAYER_LOGINSERVER_LOGIN = 2,	//	�α��� ���� �α��� ����
	dfPLAYER_GAMESERVER_REQ = 3,	//	���Ӽ����� �α����ϴ� ����
	dfPLAYER_GAMESERVER_LOGIN = 4,	//	���Ӽ��� ���� ���� ����
	dfPLAYER_GAMESERVER_LOGOUT_REQ = 5,		//	���Ӽ��� �α׾ƿ� ��û����
};

class CLanServer;

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
	CLanServer			*_pLanServer;
	CDBConnector		_AccountDB;

	/////////////////////////////////////////////////////////////
	// ����� �����Լ���.  
	/////////////////////////////////////////////////////////////

	void		UTF8toUTF16(const char *szText, WCHAR *szBuf, int iBufLen);
	void		UTF16toUTF8(WCHAR *szText, char *szBuf, int iBufLen);

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
	PLAYER*	FindPlayer_ClientID(unsigned __int64 iClientID);
	PLAYER*	FindPlayer_AccountNo(INT64 AccountNo);

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


protected:
	SRWLOCK		_DB_srwlock;
	//-------------------------------------------------------------
	// ������ ����.
	// 
	// �����ڴ� �� ����Ʈ�� ������.  ������ ����ȭ�� ���� SRWLock �� ����Ѵ�.
	//-------------------------------------------------------------
	std::list<PLAYER*>	_PlayerList;
	SRWLOCK				_PlayerList_srwlock;

	CMemoryPool<PLAYER> *_PlayerPool;

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






