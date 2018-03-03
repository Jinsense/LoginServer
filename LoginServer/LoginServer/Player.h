#ifndef _LOGINSERVER_NETSERVER_PLAYER_H_
#define _LOGINSERVER_NETSERVER_PLAYER_H_


class CPlayer
{
public:
	CPlayer();
	~CPlayer();

	bool	_bGameServerComplete;		//	게임서버 세션키 공유 확인
	bool	_bChatServerComplete;		//	채팅서버 세션키 공유 확인
	unsigned __int64	_LoginTime;		//	최초 로그인 시간 ( 타임아웃 체크 )

	unsigned __int64	_ClientID;		//	NetServer에서 생성한 클라이언트 고유번호
	__int64		_AccountNo;				//	DB에 저장된 계정 고유번호
	WCHAR		_ID[20];				//	ID - null포함
	WCHAR		_NickName[20];			//	Nick - null포함
	char		_SessionKey[64];		//	웹서버에서 받은 세션키
	
	in_addr		_ClientIP;				//	클라이언트 접속 IP
	unsigned short _ClientPort;			//	클라이언트 접속 Port


};

#endif _LOGINSERVER_NETSERVER_PLAYER_H_