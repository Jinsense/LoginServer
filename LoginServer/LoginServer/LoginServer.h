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
	long	_bGameServerComplete;		//	게임서버 세션키 공유 확인
	long	_bChatServerComplete;		//	채팅서버 세션키 공유 확인
	unsigned __int64	_LoginTime;		//	최초 로그인 시간 ( 타임아웃 체크 )

	unsigned __int64	_ClientID;		//	NetServer에서 생성한 클라이언트 고유번호
	INT64		_AccountNo;				//	DB에 저장된 계정 고유번호
	WCHAR		_ID[20];				//	ID - null포함
	WCHAR		_NickName[20];			//	Nick - null포함
	char		_SessionKey[64];		//	웹서버에서 받은 세션키

	in_addr		_ClientIP;				//	클라이언트 접속 IP
	unsigned short _ClientPort;			//	클라이언트 접속 Port

	int		_Status;				//	클라이언트 상태 
	INT64		_Parameter;				//	세션키 파라미터 확인
}PLAYER;

enum en_PLAYER_STATUS
{
	dfPLAYER_NOT_LOGIN = 0,			//	로그인 중이 아님, 로그아웃 상태
	dfPLAYER_LOGINSERVER_REQ = 1,	//	로그인 요청 상태
	dfPLAYER_LOGINSERVER_LOGIN = 2,	//	로그인 서버 로그인 상태
	dfPLAYER_GAMESERVER_REQ = 3,	//	게임서버로 로그인하는 상태
	dfPLAYER_GAMESERVER_LOGIN = 4,	//	게임서버 게임 중인 상태
	dfPLAYER_GAMESERVER_LOGOUT_REQ = 5,		//	게임서버 로그아웃 요청상태
};

class CLanServer;

class CLoginServer : public CNetServer
{
public:

	CLoginServer();
	virtual ~CLoginServer();

protected:
	//	가상함수들...
	virtual void		OnClientJoin(st_SessionInfo Info);
	virtual void		OnClientLeave(unsigned __int64 iClientID);
	virtual void		OnConnectionRequest(WCHAR * pClientIP, int iPort);
	virtual void		OnError(int iErrorCode, WCHAR *pError);
	virtual bool		OnRecv(unsigned __int64 iClientID, CPacket *pPacket);

		//-----------------------------------------------------------
		// 로그인서버 스케쥴러 역할
		//-----------------------------------------------------------
	static unsigned __stdcall	UpdateThread(void *pParam);
	void		UpdateThread_Update();

	void		Schedule_PlayerTimeout(void);
	void		Schedule_ServerTimeout(void);

public:
	CLanServer			*_pLanServer;
	CDBConnector		_AccountDB;

	/////////////////////////////////////////////////////////////
	// 사용자 관리함수들.  
	/////////////////////////////////////////////////////////////

	void		UTF8toUTF16(const char *szText, WCHAR *szBuf, int iBufLen);
	void		UTF16toUTF8(WCHAR *szText, char *szBuf, int iBufLen);

	/////////////////////////////////////////////////////////////
	// OnClientJoin, OnClientLeave 에서 호출됨.
	/////////////////////////////////////////////////////////////
	bool		InsertPlayer(unsigned __int64 iClientID);
	bool		RemovePlayer(unsigned __int64 iClientID);

	/////////////////////////////////////////////////////////////
	// OnRecv 에서 로그인인증 처리 후 사용,  UpdateThread 에서 일정시간 지난 유저에 사용.
	/////////////////////////////////////////////////////////////
	bool		DisconnectPlayer(unsigned __int64 iClientID, BYTE byStatus);

	/////////////////////////////////////////////////////////////
	// 접속 사용자 수 
	/////////////////////////////////////////////////////////////
	int			GetPlayerCount(void);

	/////////////////////////////////////////////////////////////
	// 접속한 플레이어 찾기 
	/////////////////////////////////////////////////////////////
	PLAYER*	FindPlayer_ClientID(unsigned __int64 iClientID);
	PLAYER*	FindPlayer_AccountNo(INT64 AccountNo);

	/////////////////////////////////////////////////////////////
	// 세션키 공유 완료 
	/////////////////////////////////////////////////////////////
	void		ChatResSessionKey(INT64 AccountNo, INT64 Parameter);
	void		GameResSessionKey(INT64 AccountNo, INT64 Parameter);

	/////////////////////////////////////////////////////////////
	// White IP 관련
	/////////////////////////////////////////////////////////////

//	..WhiteIP 목록을 저장하구 검색, 확인 할 수 있는 기능


protected:

	// 로그인 요청 패킷처리
	bool				PacketProc_ReqLogin(unsigned __int64 iClientID, CPacket *pPacket);


protected:
	SRWLOCK		_DB_srwlock;
	//-------------------------------------------------------------
	// 접속자 관리.
	// 
	// 접속자는 본 리스트로 관리함.  스레드 동기화를 위해 SRWLock 을 사용한다.
	//-------------------------------------------------------------
	std::list<PLAYER*>	_PlayerList;
	SRWLOCK				_PlayerList_srwlock;

	CMemoryPool<PLAYER> *_PlayerPool;

	//-------------------------------------------------------------
	// 기타 모니터링용 변수,스레드 함수.
	//-------------------------------------------------------------
	long				_Monitor_LoginSuccessTPS;

	long				_Monitor_LoginWait;					// 로그인 패킷 수신 후 대기중인 수

	long				_Monitor_LoginProcessTime_Max;		// 로그인 처리 시간 최대
	long				_Monitor_LoginProcessTime_Min;		// 로그인 처리 시간 최소
	long long			_Monitor_LoginProcessTime_Total;	// 총 합
	long long			_Monitor_LoginProcessCall_Total;	// 로그인 처리 요청 총 합

	unsigned __int64	_Parameter;
private:

	long				_Monitor_LoginSuccessCounter;

	virtual void		MonitorThread_Update();
};

#endif _LOGINSERVER_NETSERVER_LOGINSERVER_H_






