#ifndef _LOGINSERVER_NETSERVER_PLAYER_H_
#define _LOGINSERVER_NETSERVER_PLAYER_H_


class CPlayer
{
public:
	CPlayer();
	~CPlayer();

	bool	_bGameServerComplete;		//	���Ӽ��� ����Ű ���� Ȯ��
	bool	_bChatServerComplete;		//	ä�ü��� ����Ű ���� Ȯ��
	unsigned __int64	_LoginTime;		//	���� �α��� �ð� ( Ÿ�Ӿƿ� üũ )

	unsigned __int64	_ClientID;		//	NetServer���� ������ Ŭ���̾�Ʈ ������ȣ
	__int64		_AccountNo;				//	DB�� ����� ���� ������ȣ
	WCHAR		_ID[20];				//	ID - null����
	WCHAR		_NickName[20];			//	Nick - null����
	char		_SessionKey[64];		//	���������� ���� ����Ű
	
	in_addr		_ClientIP;				//	Ŭ���̾�Ʈ ���� IP
	unsigned short _ClientPort;			//	Ŭ���̾�Ʈ ���� Port


};

#endif _LOGINSERVER_NETSERVER_PLAYER_H_