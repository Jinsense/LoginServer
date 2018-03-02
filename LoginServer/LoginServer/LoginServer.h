#ifndef _LOGINSERVER_NETSERVER_LOGINSERVER_H_
#define _LOGINSERVER_NETSERVER_LOGINSERVER_H_

#include <list>
#include <map>

#include "Player.h"
#include "NetServer.h"


class CLoginServer : public CNetServer
{
public:

	CLoginServer(int iMaxClient);
	virtual ~CLoginServer();

protected:
//	가상함수들...

		//-----------------------------------------------------------
		// 로그인서버 스케쥴러 역할
		//-----------------------------------------------------------
	static unsigned __stdcall	UpdateThread(void *pParam);

	void		Schedule_PlayerTimeout(void);
	void		Schedule_ServerTimeout(void);

public:


	/////////////////////////////////////////////////////////////
	// 사용자 관리함수들.  
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	// OnClientJoin, OnClientLeave 에서 호출됨.
	/////////////////////////////////////////////////////////////
	bool		InsertPlayer(st_Session ClientID);
	bool		RemovePlayer(st_Session ClientID);

	/////////////////////////////////////////////////////////////
	// OnRecv 에서 로그인인증 처리 후 사용,  UpdateThread 에서 일정시간 지난 유저에 사용.
	/////////////////////////////////////////////////////////////
	bool		DisconnectPlayer(st_Session ClientID);

	/////////////////////////////////////////////////////////////
	// 접속 사용자 수 
	/////////////////////////////////////////////////////////////
	int			GetPlayerCount(void);


	/////////////////////////////////////////////////////////////
	// White IP 관련
	/////////////////////////////////////////////////////////////

//	..WhiteIP 목록을 저장하구 검색, 확인 할 수 있는 기능



protected:

	// 로그인 요청 패킷처리
	bool				PacketProc_ReqLogin(st_Session ClientID, CPacket *pPacket);

	// 패킷 생성부
	bool				MakePacket_ResLogin(CPacket *pPacket, __int64 iAccountNo, WCHAR *szID, WCHAR *szNickname, BYTE byStatus);



protected:

	//-------------------------------------------------------------
	// 접속자 관리.
	// 
	// 접속자는 본 리스트로 관리함.  스레드 동기화를 위해 SRWLock 을 사용한다.
	//-------------------------------------------------------------
	list<CPlayer *>		_PlayerList;
	SRWLOCK				_PlayerList_srwlock;


	//-------------------------------------------------------------
	// 기타 모니터링용 변수,스레드 함수.
	//-------------------------------------------------------------
	long				_Monitor_LoginSuccessTPS;

	long				_Monitor_LoginWait;					// 로그인 패킷 수신 후 대기중인 수

	long				_Monitor_LoginProcessTime_Max;		// 로그인 처리 시간 최대
	long				_Monitor_LoginProcessTime_Min;		// 로그인 처리 시간 최소
	long long			_Monitor_LoginProcessTime_Total;	// 총 합
	long long			_Monitor_LoginProcessCall_Total;	// 로그인 처리 요청 총 합

private:

	long				_Monitor_LoginSuccessCounter;


	HANDLE				_MonitorTPS_Thread;
	static unsigned __stdcall	MonitorTPS_Thread(void *pParam);
	bool				MonitorTPS_Thread_update(void);

};

#endif _LOGINSERVER_NETSERVER_LOGINSERVER_H_






