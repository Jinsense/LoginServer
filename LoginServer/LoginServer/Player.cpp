//#include <wchar.h>
//#include <Windows.h>

#include "Player.h"



CPlayer::CPlayer()
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


CPlayer::~CPlayer()
{

}
