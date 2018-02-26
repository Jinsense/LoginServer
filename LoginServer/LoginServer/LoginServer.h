#ifndef _LOGINSERVER_IOCP_LOGINSERVER_H_
#define _LOGINSERVER_IOCP_LOGINSERVER_H_

#include "NetServer.h"

class CLoginServer : public CNetServer
{
public:

	CLoginServer(int iMaxClient);
	virtual ~CLoginServer();

protected:
//	�����Լ���...

		//-----------------------------------------------------------
		// �α��μ��� �����췯 ����
		//-----------------------------------------------------------
	static unsigned __stdcall	UpdateThread(void *pParam);

	void				Schedule_PlayerTimeout(void);
	void				Schedule_ServerTimeout(void);

public:


	/////////////////////////////////////////////////////////////
	// ����� �����Լ���.  
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	// OnClientJoin, OnClientLeave ���� ȣ���.
	/////////////////////////////////////////////////////////////
	bool				InsertPlayer(CLIENT_ID ClientID);
	bool				RemovePlayer(CLIENT_ID ClientID);

	/////////////////////////////////////////////////////////////
	// OnRecv ���� �α������� ó�� �� ���,  UpdateThread ���� �����ð� ���� ������ ���.
	/////////////////////////////////////////////////////////////
	bool				DisconnectPlayer(CLIENT_ID ClientID);

	/////////////////////////////////////////////////////////////
	// ���� ����� �� 
	/////////////////////////////////////////////////////////////
	int				GetPlayerCount(void);


	/////////////////////////////////////////////////////////////
	// White IP ����
	/////////////////////////////////////////////////////////////

//	..WhiteIP ����� �����ϱ� �˻�, Ȯ�� �� �� �ִ� ���



protected:

	// �α��� ��û ��Ŷó��
	bool				PacketProc_ReqLogin(CLIENT_ID ClientID, CPacket *pPacket);

	// ��Ŷ ������
	bool				MakePacket_ResLogin(CPacket *pPacket, __int64 iAccountNo, WCHAR *szID, WCHAR *szNickname, BYTE byStatus);



protected:

	//-------------------------------------------------------------
	// ������ ����.
	// 
	// �����ڴ� �� ����Ʈ�� ������.  ������ ����ȭ�� ���� SRWLock �� ����Ѵ�.
	//-------------------------------------------------------------
	list<CPlayer *>			_PlayerList;
	SRWLOCK				_PlayerList_srwlock;


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


	HANDLE				_MonitorTPS_Thread;
	static unsigned __stdcall	MonitorTPS_Thread(void *pParam);
	bool				MonitorTPS_Thread_update(void);

};

#endif _LOGINSERVER_IOCP_LOGINSERVER_H_





