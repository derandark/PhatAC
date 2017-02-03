
#include "StdAfx.h"
#include "Server.h"
#include "World.h"

CPhatServer *g_pPhatServer = 0;
HINSTANCE g_hInstance = 0;
HWND g_hWndLauncher = 0;
HWND g_hWndStatus = 0;
HWND g_hWndMain = 0;

DWORD g_dwMagicNumber = 0; // This is used for the admin login.

UINT64 framerecord = 0;

void UpdateFramesHUD(UINT64 frames)
{
	if (g_hWndStatus)
	{
		SetWindowText(GetDlgItem(g_hWndStatus, IDC_FRAMERATE), csprintf("%I64u", frames));
		SetWindowText(GetDlgItem(g_hWndStatus, IDC_RP), csprintf("%I64u", g_pGlobals->GetPacketRecvCount()));
		SetWindowText(GetDlgItem(g_hWndStatus, IDC_SP), csprintf("%I64u", g_pGlobals->GetPacketSendCount()));
		SetWindowText(GetDlgItem(g_hWndStatus, IDC_BR), csprintf("%I64u", g_pGlobals->GetPacketRecvSize()));
		SetWindowText(GetDlgItem(g_hWndStatus, IDC_BS), csprintf("%I64u", g_pGlobals->GetPacketSendSize()));
	}

	if (frames > framerecord)
		framerecord = frames;

	if (g_hWndMain)
	{
		HWND hWndCPUText = GetDlgItem(g_hWndMain, IDC_CPULOADTEXT);
		HWND hWndCPUBar = GetDlgItem(g_hWndMain, IDC_CPULOADBAR);

		double fraction = framerecord ? ((double)frames / framerecord) : 1.0f;
		fraction = (1.0 - fraction) * 100.0;

		SendMessage(hWndCPUBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
		SendMessage(hWndCPUBar, PBM_SETPOS, (WORD)fraction, 0);

		SetWindowText(hWndCPUText, csprintf("CPU Lag (%.2f%%):", fraction));
	}
}

//Costly and ineffecient, but saves coding time.
void UpdateClientsHUD(CClient **clients, WORD slotrange)
{
	if (slotrange >= 400)
	{
		slotrange = 400 - 1;
	}

	if (!g_hWndMain)
	{
		return;
	}

	HWND hWndClients = GetDlgItem(g_hWndMain, IDC_CLIENTS);

	if (!hWndClients)
	{
		return;
	}

	SendMessage(hWndClients, LB_RESETCONTENT, 0, 0);

	WORD wCount = 0;
	for (CClient **it = &clients[1]; it <= &clients[slotrange]; it++)
	{
		CClient *client = *it;
		if (!client)
			continue;

		const char *lbText = client->GetDescription();
		DWORD lbIndex = (DWORD)SendMessage(hWndClients, LB_ADDSTRING, 0, (LPARAM)lbText);
		wCount++;
	}

	g_pGlobals->SetClientCount(wCount);

	HWND hWndLoadText = GetDlgItem(g_hWndMain, IDC_NETWORKLOADTEXT);
	HWND hWndLoad = GetDlgItem(g_hWndMain, IDC_NETWORKLOADBAR);

	SetWindowText(hWndLoadText, csprintf("Server Load (%u)", g_pGlobals->GetClientCount()));
	SendMessage(hWndLoad, PBM_SETRANGE, 0, MAKELPARAM(0, 400));
	SendMessage(hWndLoad, PBM_SETPOS, g_pGlobals->GetClientCount(), 0);
}

LRESULT CALLBACK AboutProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		case IDC_WEBSITE:
			ShellExecute(0, "open", "http://hunczak.com/", NULL, NULL, SW_SHOW);
			break;
		default:
			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hDlg);
		break;
	}
	return FALSE;
}

LRESULT CALLBACK StatusProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
	{
		HWND hWndPriority = GetDlgItem(hDlg, IDC_PRIORITY);
		SendMessage(hWndPriority, CB_ADDSTRING, 0, (LPARAM)"Lowest");
		SendMessage(hWndPriority, CB_ADDSTRING, 0, (LPARAM)"Below Normal");
		SendMessage(hWndPriority, CB_ADDSTRING, 0, (LPARAM)"Normal");
		SendMessage(hWndPriority, CB_ADDSTRING, 0, (LPARAM)"Above Normal");
		SendMessage(hWndPriority, CB_ADDSTRING, 0, (LPARAM)"Highest");

		int tp = GetThreadPriority(GetCurrentThread());
		int index;
		switch (tp)
		{
		case THREAD_PRIORITY_LOWEST:		index = 0; break;
		case THREAD_PRIORITY_BELOW_NORMAL:	index = 1; break;
		case THREAD_PRIORITY_NORMAL:		index = 2; break;
		case THREAD_PRIORITY_ABOVE_NORMAL:	index = 3; break;
		case THREAD_PRIORITY_HIGHEST:		index = 4; break;
		default:							index = 2; break;
		};
		SendMessage(hWndPriority, CB_SETCURSEL, index, 0);

		return TRUE;
	}
	case WM_COMMAND:
	{
		WORD wEvent = HIWORD(wParam);
		WORD wID = LOWORD(wParam);

		switch (wID)
		{
		case IDC_PRIORITY:
			if (wEvent == CBN_SELCHANGE)
			{
				int index = SendMessage(GetDlgItem(hDlg, IDC_PRIORITY), CB_GETCURSEL, 0, 0);
				int tp;
				switch (index)
				{
				case 0: tp = THREAD_PRIORITY_LOWEST; break;
				case 1: tp = THREAD_PRIORITY_BELOW_NORMAL; break;
				case 3: tp = THREAD_PRIORITY_ABOVE_NORMAL; break;
				case 4:	tp = THREAD_PRIORITY_HIGHEST; break;
				case 2:
				default: tp = THREAD_PRIORITY_NORMAL; break;
				}

				SetThreadPriority(GetCurrentThread(), tp);
			}
		case IDOK:
			if (wEvent == BN_CLICKED)
			{
				g_hWndStatus = 0;
				DestroyWindow(hDlg);
			}
			break;
		}
		break;
	}
	case WM_CLOSE:
		g_hWndStatus = 0;
		DestroyWindow(hDlg);
		break;
	}

	return FALSE;
}

LRESULT CALLBACK LauncherProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
	{
		char account[256];
		char password[256];
		char remoteport[256];
		DWORD remoteip;
		if (ReadConfigKey("account", account, 256))
			SetWindowText(GetDlgItem(hDlg, IDC_ACCOUNT), account);
		if (ReadConfigKey("password", password, 256))
			SetWindowText(GetDlgItem(hDlg, IDC_PASSWORD), password);
		if (ReadConfigKey("remoteport", remoteport, 256))
			SetWindowText(GetDlgItem(hDlg, IDC_REMOTEPORT), remoteport);
		if (ReadConfigKey("remoteip", &remoteip))
			SendMessage(GetDlgItem(hDlg, IDC_REMOTEIP), IPM_SETADDRESS, 0, remoteip);

		return TRUE;
	}
	case WM_COMMAND:
	{
		WORD wEvent = HIWORD(wParam);
		WORD wID = LOWORD(wParam);

		switch (wID)
		{
		case IDC_LAUNCHCLIENT:
			if (wEvent == BN_CLICKED)
			{
				char szRemotePort[10];
				char szAccount[64]; *szAccount = 0;
				char szPassword[64]; *szPassword = 0;
				DWORD dwRemoteIP;
				DWORD dwFields = (DWORD)SendMessage(GetDlgItem(hDlg, IDC_REMOTEIP), IPM_GETADDRESS, 0, (LPARAM)&dwRemoteIP);
				SendMessage(GetDlgItem(hDlg, IDC_REMOTEPORT), WM_GETTEXT, 10, (LPARAM)&szRemotePort);
				SendMessage(GetDlgItem(hDlg, IDC_ACCOUNT), WM_GETTEXT, 64, (LPARAM)&szAccount);
				SendMessage(GetDlgItem(hDlg, IDC_PASSWORD), WM_GETTEXT, 64, (LPARAM)&szPassword);

				if (!strlen(szAccount) || !strlen(szPassword))
				{
					MsgBox("Please specify the account name/password to connect with.");
					break;
				}

				if (dwFields == 4)
				{
					dwRemoteIP = htonl(dwRemoteIP);
					char szLaunch[256];
					char szLaunchDir[MAX_PATH + 1];
					sprintf(szLaunch, "-h %s -p %u -a %s:%s", inet_ntoa(*((in_addr *)&dwRemoteIP)), atol(szRemotePort), szAccount, szPassword);

					DWORD	dwLen = MAX_PATH + 1;
					memset(szLaunchDir, 0, dwLen);
					dwLen -= 1;

					/*
					HKEY hKey;
					if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Microsoft Games\\Asheron's Call\\1.00", NULL, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
					{
						RegQueryValueEx(hKey, "Portal Dat", NULL, NULL, (BYTE*)szLaunchDir, &dwLen);
						RegCloseKey(hKey);
					}
					else
					{
						MsgBox("Couldn't find installation directory. Is AC installed?\r\n");
						break;
					}
					*/

					char dir[MAX_PATH];
					GetCurrentDirectory(MAX_PATH, dir);
					dir[MAX_PATH - 1] = '\0';

					sprintf(szLaunchDir, "%s\\Client", dir);

					if (szLaunchDir[strlen(szLaunchDir) - 1] != '\\')
					{
						char *end = &szLaunchDir[strlen(szLaunchDir)];
						end[0] = '\\';
						end[1] = '\0';
					}

					if (!FileExists(((std::string)szLaunchDir + "\\acclient.exe").c_str()))
					{
						MsgBox("Please copy your Asheron's Call client to the Client folder of PhatAC.\r\n");
					}
					else
					{
						//LOG(Temp, Normal, "Launching %s %s\n", szLaunch, szLaunchDir);
						ShellExecute(0, "open", "acclient.exe", szLaunch, szLaunchDir, SW_SHOW);
					}
				}
				else
					MsgBox("Please specify the remote IP to connect to.");
			}
			break;
		}
		break;
	}
	case WM_CLOSE:
		char account[256]; *account = 0;
		char password[256]; *password = 0;
		char remoteport[256]; *remoteport = 0;
		DWORD remoteip;
		if (GetWindowText(GetDlgItem(hDlg, IDC_ACCOUNT), account, 256))
			SaveConfigKey("account", account);
		if (GetWindowText(GetDlgItem(hDlg, IDC_PASSWORD), password, 256))
			SaveConfigKey("password", password);
		if (GetWindowText(GetDlgItem(hDlg, IDC_REMOTEPORT), remoteport, 256))
			SaveConfigKey("remoteport", remoteport);
		if (4 == SendMessage(GetDlgItem(hDlg, IDC_REMOTEIP), IPM_GETADDRESS, 0, (LPARAM)&remoteip))
			SaveConfigKey("remoteip", remoteip);

		g_hWndLauncher = 0;
		DestroyWindow(hDlg);
		break;
	}

	return FALSE;
}

void OutputConsole(int category, int level, const char *text)
{
	if (level < LOGLEVEL_Normal)
	{
		return;
	}

	HWND hWndConsole = g_pGlobals->GetConsoleWindowHandle();

	if (!hWndConsole)
		return;

	int len = (int)SendMessage(hWndConsole, WM_GETTEXTLENGTH, 0, 0);
	DWORD start, end;
	SendMessage(hWndConsole, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
	SendMessage(hWndConsole, EM_SETSEL, len, len);
	SendMessage(hWndConsole, EM_REPLACESEL, FALSE, (LPARAM)text);
	SendMessage(hWndConsole, EM_SETSEL, start, end);
}

int CALLBACK MainProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
	{
		g_pGlobals->SetWindowHandle(hDlg);
		g_pGlobals->SetConsoleWindowHandle(GetDlgItem(hDlg, IDC_CONSOLE));

		g_Logger.AddLogCallback(OutputConsole);

		HWND hVersion = GetDlgItem(hDlg, IDC_VERSION);
		SetWindowText(hVersion, "PhatAC compiled " __TIMESTAMP__);

		HICON hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_PHATAC));
		SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
		DeleteObject(hIcon);

		char hostname[256];
		gethostname(hostname, 256);
		DWORD hostaddr = *((DWORD *)gethostbyname(hostname)->h_addr);
		SendMessage(GetDlgItem(hDlg, IDC_SERVERIP), IPM_SETADDRESS, 0, ntohl(hostaddr));
		SendMessage(GetDlgItem(hDlg, IDC_SERVERPORT), WM_SETTEXT, 0, (LPARAM)"9050");
		return TRUE;
	}
	case WM_COMMAND:
	{
		WORD wEvent = HIWORD(wParam);
		WORD wID = LOWORD(wParam);

		switch (wID)
		{
		case IDM_EXIT:
			PostQuitMessage(0);
			DestroyWindow(hDlg);
			break;
		case IDM_ABOUT:
			DialogBox(g_hInstance, (LPCTSTR)IDD_ABOUT, hDlg, (DLGPROC)AboutProc);
			break;
		case IDM_LAUNCHER:
			if (!g_hWndLauncher)
			{
				g_hWndLauncher = CreateDialog(g_hInstance, (LPCTSTR)IDD_LAUNCHER, hDlg, (DLGPROC)LauncherProc);
				ShowWindow(g_hWndLauncher, SW_SHOW);
			}
			break;
		case IDM_STATUS:
			if (!g_hWndStatus)
			{
				g_hWndStatus = CreateDialog(g_hInstance, (LPCTSTR)IDD_STATUS, hDlg, (DLGPROC)StatusProc);
				ShowWindow(g_hWndStatus, SW_SHOW);
			}
			break;
		case IDC_CLEARLOG:
		{
			if (wEvent == BN_CLICKED)
			{
				HWND hWndConsole = GetDlgItem(hDlg, IDC_CONSOLE);

				SetWindowText(hWndConsole, "");
				LOG(Temp, Normal, "Console cleared.\n");
			}
		}
		break;
		case IDC_SAVELOG:
		{
			if (wEvent == BN_CLICKED)
			{
				HWND hWndConsole = GetDlgItem(hDlg, IDC_CONSOLE);

				int console_len = GetWindowTextLength(hWndConsole) + 1;
				char* console_text = new char[console_len];
				GetWindowText(GetDlgItem(hDlg, IDC_CONSOLE), console_text, console_len);

				FILE *fp = fopen(csprintf("%s\\console.txt", g_pGlobals->GetGameDirectory()), "wt");
				if (fp)
				{
					fprintf(fp, "Console Log:\n\n");
					fprintf(fp, "%s\n", console_text);
					fprintf(fp, "End of Console Log.\n");
					fclose(fp);

					LOG(Temp, Normal, "Console log saved.\n");
				}
				else
					LOG(Temp, Normal, "Failed to open console file.\n");
			}
		}
		break;
		case IDC_BROADCAST:
			if (wEvent == BN_CLICKED)
			{
				if (!g_pPhatServer)
				{
					LOG(Temp, Normal, "You must be running a server to broadcast a system message.\n");
					break;
				}

				char text[400];
				memset(text, 0, 400);
				SendMessage(GetDlgItem(hDlg, IDC_BROADCASTTEXT), WM_GETTEXT, 399, (LPARAM)text);

				g_pPhatServer->SystemBroadcast(text);
			}
			break;
		case IDC_KICK:
			if (wEvent == BN_CLICKED)
			{
				if (!g_pPhatServer)
					break;

				g_pWorld->Test();
			}
			break;
		case IDC_BAN:
			if (wEvent == BN_CLICKED)
			{
				if (!g_pPhatServer)
					break;

				HWND hWndClients = GetDlgItem(hDlg, IDC_CLIENTS);
				LRESULT lResult = SendMessage(hWndClients, LB_GETCURSEL, 0, 0);

				if (lResult == CB_ERR)
					break;

				//This assumes the description begins with 4 numbers indicating the celit index.

				LRESULT lLen = SendMessage(hWndClients, LB_GETTEXTLEN, lResult, 0);
				if (lLen >= 4)
				{
					char *szText = new char[lLen + 1];
					SendMessage(hWndClients, LB_GETTEXT, lResult, (LPARAM)szText);
					szText[4] = 0;
					int clientIndex = atoi(szText);
					g_pPhatServer->BanClient(clientIndex);
					delete[]szText;
				}
			}
			break;
		case IDC_LAUNCH:
			if (wEvent == BN_CLICKED)
			{
				if (!g_pPhatServer)
				{
					MsgBox("You might want to start the server first. ;)");
					break;
				}
				char szServerPort[10];
				DWORD dwServerIP;
				DWORD dwFields = (DWORD)SendMessage(GetDlgItem(hDlg, IDC_SERVERIP), IPM_GETADDRESS, 0, (LPARAM)&dwServerIP);
				SendMessage(GetDlgItem(hDlg, IDC_SERVERPORT), WM_GETTEXT, 10, (LPARAM)&szServerPort);

				if (dwFields != 0)
				{
					dwServerIP = htonl(dwServerIP);
					char szLaunch[256];
					char szLaunchDir[MAX_PATH + 10];
					sprintf(szLaunch, "-h %s -p %u -a admin:%06lu", inet_ntoa(*((in_addr *)&dwServerIP)), atol(szServerPort), g_dwMagicNumber);

					DWORD dwLen = MAX_PATH + 10;
					memset(szLaunchDir, 0, dwLen);
					dwLen -= 1;

					/*
					HKEY hKey;
					if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Microsoft Games\\Asheron's Call\\1.00", NULL, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
					{
						RegQueryValueEx(hKey, "Portal Dat", NULL, NULL, (BYTE*)szLaunchDir, &dwLen);
						RegCloseKey(hKey);
					}
					else
					{
						MsgBox("Couldn't find installation directory. Is AC installed?\r\n");
						break;
					}
					*/

					char dir[MAX_PATH];
					GetCurrentDirectory(MAX_PATH, dir);
					dir[MAX_PATH - 1] = '\0';

					sprintf(szLaunchDir, "%s\\Client", dir);

					if (szLaunchDir[strlen(szLaunchDir) - 1] != '\\')
					{
						char *end = &szLaunchDir[strlen(szLaunchDir)];
						end[0] = '\\';
						end[1] = '\0';
					}

					if (!FileExists(((std::string)szLaunchDir + "\\acclient.exe").c_str()))
					{
						MsgBox("Please copy your Asheron's Call client to the Client folder of PhatAC.\r\n");
					}
					else
					{
						//LOG(Temp, Normal, "Launching %s %s\n", szLaunch, szLaunchDir);
						ShellExecute(0, "open", "acclient.exe", szLaunch, szLaunchDir, SW_SHOW);
					}
				}
				else
				{
					MsgBox("Please specify the server's external IP.");
				}
			}
			break;
		case IDC_TOGGLE:
			if (wEvent == BN_CLICKED)
			{
				if (!g_pPhatServer)
				{
					char szServerPort[10];
					DWORD dwServerIP;
					DWORD dwFields = (DWORD)SendMessage(GetDlgItem(hDlg, IDC_SERVERIP), IPM_GETADDRESS, 0, (LPARAM)&dwServerIP);
					SendMessage(GetDlgItem(hDlg, IDC_SERVERPORT), WM_GETTEXT, 10, (LPARAM)&szServerPort);

					if (dwFields != 0)
					{
						dwServerIP = htonl(dwServerIP);
						g_pPhatServer = new CPhatServer(*((in_addr *)&dwServerIP), atoi(szServerPort));
						SetWindowText(GetDlgItem(hDlg, IDC_TOGGLE), "Stop");
					}
					else
						MsgBox("Please specify the server's external IP.");
				}
				else
				{
					delete g_pPhatServer; g_pPhatServer = NULL;
					SendMessage(GetDlgItem(hDlg, IDC_CLIENTS), LB_RESETCONTENT, 0, 0);

					SetWindowText(GetDlgItem(hDlg, IDC_TOGGLE), "Start");
					SetWindowText(GetDlgItem(hDlg, IDC_NETWORKLOADTEXT), "Server Load (0)");
					SetWindowText(GetDlgItem(hDlg, IDC_CPULOADTEXT), "CPU Load (0.00%)");
					SendMessage(GetDlgItem(hDlg, IDC_NETWORKLOADBAR), PBM_SETRANGE, 0, MAKELPARAM(0, 400));
					SendMessage(GetDlgItem(hDlg, IDC_NETWORKLOADBAR), PBM_SETPOS, 0, 0);
					SendMessage(GetDlgItem(hDlg, IDC_CPULOADBAR), PBM_SETRANGE, 0, MAKELPARAM(0, 100));
					SendMessage(GetDlgItem(hDlg, IDC_CPULOADBAR), PBM_SETPOS, 0, 0);

					LOG(Temp, Normal, "Server shutdown.\n");
				}
			}
			break;
		}
	}
	break;
	case WM_CLOSE:
		if (g_hWndLauncher)	SendMessage(g_hWndLauncher, WM_CLOSE, 0, 0);
		if (g_hWndStatus)		SendMessage(g_hWndStatus, WM_CLOSE, 0, 0);

		PostQuitMessage(0);
		DestroyWindow(hDlg);
		break;
	}
	return FALSE;
}

int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
	srand((unsigned int)time(NULL));

	g_pGlobals = new CGlobals();
	g_Logger.Open();

	INITCOMMONCONTROLSEX iccex;
	iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	iccex.dwICC = ICC_INTERNET_CLASSES;
	InitCommonControlsEx(&iccex);

	WSADATA	wsaData;
	USHORT wVersionRequested = 0x0202;
	WSAStartup(wVersionRequested, &wsaData);

	g_hInstance = hInstance;
	g_hWndMain = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, MainProc);

	if (!g_hWndMain)
	{
		MsgBoxError(GetLastError(), "creating main dialog box");
		return 0;
	}

	g_dwMagicNumber = RandomLong(0, 1234567890);

	LOG(UserInterface, Normal, "Welcome to PhatAC..\n");

	ShowWindow(g_hWndMain, nCmdShow);

	MSG msg;
	msg.message = WM_NULL;

	while (msg.message != WM_QUIT)
	{
		Sleep(1);

		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			if (IsDialogMessage(g_hWndMain, &msg))
				continue;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			if (g_pPhatServer)
				g_pPhatServer->Tick();
		}
	}

	SafeDelete(g_pPhatServer);
	SafeDelete(g_pGlobals);
	WSACleanup();

#ifdef _DEBUG
	if (_CrtDumpMemoryLeaks())
		OutputDebugString("Memory leak found!\n");
	else
		OutputDebugString("No memory leaks found!\n");
#endif

	return 0;
}





