#include <conio.h>

#include "Config.h"
#include "LoginServer.h"

int main()
{
	int _In;
	CLoginServer _LoginServer;
	SYSTEM_INFO _SysInfo;

	GetSystemInfo(&_SysInfo);

	if (false == _LoginServer._Config.Set())
	{
		wprintf(L"[Server :: Main]	Config Error\n");
		return 0;
	}

	if (false == _LoginServer._AccountDB.Set(_LoginServer._Config.ACCOUNT_IP, 
		_LoginServer._Config.ACCOUNT_USER, _LoginServer._Config.ACCOUNT_PASSWORD,
		_LoginServer._Config.ACCOUNT_DBNAME, _LoginServer._Config.ACCOUNT_PORT))
	{
		wprintf(L"[Server :: Main]	AccountDB Set Error\n");
		return 0;
	}

	if (false == _LoginServer.ServerStart(_LoginServer._Config.BIND_IP, _LoginServer._Config.BIND_PORT,
		_LoginServer._Config.WORKER_THREAD, true, _LoginServer._Config.CLIENT_MAX))
	{
		wprintf(L"[Server :: Server_Start] Error\n");
	}

	while (!_LoginServer.GetShutDownMode())
	{
		_In = _getch();
		switch (_In)
		{
		case 'q': case 'Q':
		{
			_LoginServer.SetShutDownMode(true);
			wprintf(L"[Main] 서버를 종료합니다.\n");
			_getch();
			break;
		}
		case 'm': case 'M':
		{
			if (false == _LoginServer.GetMonitorMode())
			{
				_LoginServer.SetMonitorMode(true);
				wprintf(L"[Main] MonitorMode Start\n");
			}
			else
			{
				_LoginServer.SetMonitorMode(false);
				wprintf(L"[Main] MonitorMode Stop\n");
			}
		}
		}
	}

	return 0;
}