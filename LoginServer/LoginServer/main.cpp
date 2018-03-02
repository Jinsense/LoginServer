#include <conio.h>

#include "Config.h"
#include "LoginServer.h"


CConfig _Config;

int main()
{
	bool res = _Config.Set();
	if (false == res)
	{
		wprintf(L"[Server :: Main]	Config Error\n");
		return 0;
	}

	int _In;

	CLoginServer _Login;
	SYSTEM_INFO _SysInfo;

	GetSystemInfo(&_SysInfo);





	return 0;
}