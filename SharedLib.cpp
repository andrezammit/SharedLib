
#include "stdafx.h"

#pragma comment(lib, "Winhttp.lib")

#include "SharedLib.h"

BOOL SocketSend(SOCKET* pSock, const void* buf, int len)
{
	int nRet = send(*pSock, (const char*) buf, len, 0);
	return nRet == len;
}

BOOL SocketReceive(SOCKET* pSock, void* buf, int len)
{
	int nRet = recv(*pSock, (char*) buf, len, 0);
	return nRet == len;
}

BOOL InitSockets()
{
	WSADATA wsaData;
	ZeroMemory(&wsaData, sizeof(wsaData));
	if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
	{
		AfxMessageBox(_T("Sockets not initialised."));
		return FALSE;
	}

	return TRUE;
}

BOOL InitServer(SOCKET& serverSock, int nPort)
{
	serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSock == INVALID_SOCKET) 
	{
        printf("socket failed with error: %ld\n", WSAGetLastError());
        return FALSE;
    }

	struct sockaddr_in sockAddress;
	ZeroMemory(&sockAddress, sizeof(sockAddress));

	sockAddress.sin_family = AF_INET;
	sockAddress.sin_addr.s_addr = inet_addr("0.0.0.0");
    sockAddress.sin_port = htons(nPort);

    if (bind(serverSock, (SOCKADDR*)&sockAddress, sizeof(sockAddress)) == SOCKET_ERROR)
	{
		int nError = WSAGetLastError();
		printf("connect failed with error: %d\n", nError);
        return FALSE;
	}
	
	if (listen(serverSock, SOMAXCONN) == SOCKET_ERROR)
	{
	    printf("Error listening on socket.\n");
		return FALSE;
	}

	return TRUE;
}

BOOL ConnectTo(LPCSTR pszServer, int nPort, SOCKET& clientSock)
{
	if (strlen(pszServer) == 0)
	{
		return FALSE;
	}
	
	if (clientSock != INVALID_SOCKET)
	{
		closesocket(clientSock);
		clientSock = INVALID_SOCKET;
	}

	clientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSock == INVALID_SOCKET) 
	{
        printf("socket failed with error: %ld\n", WSAGetLastError());
        return FALSE;
    }

	struct sockaddr_in sockAddress;
	ZeroMemory(&sockAddress, sizeof(sockAddress));

	sockAddress.sin_family = AF_INET;
	sockAddress.sin_addr.s_addr = inet_addr(pszServer);
    sockAddress.sin_port = htons(nPort);

    if (connect(clientSock, (SOCKADDR*)&sockAddress, sizeof(sockAddress)) == SOCKET_ERROR)
	{
		int nError = WSAGetLastError();
		printf("Connect failed with error: %d\n", nError);
        return FALSE;
	}
	
	return TRUE;
}

typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);

CString GetOSVersion()
{
	OSVERSIONINFOEX osvi;
	ZeroMemory(&osvi, sizeof(osvi));
	osvi.dwOSVersionInfoSize = sizeof(osvi);

	SYSTEM_INFO si;
	ZeroMemory(&si, sizeof(si));

	CString rOSVersion;
	DWORD dwType;

	GetSystemInfo(&si);
	if (GetVersionEx((OSVERSIONINFO*) &osvi))
	{
		if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId && 
			osvi.dwMajorVersion > 4)
		{
			rOSVersion += _T("Microsoft ");

			if (osvi.dwMajorVersion == 6)
			{
				if (osvi.dwMinorVersion == 1)
				{
					rOSVersion += _T("Windows 7 ");
				}

				if (osvi.dwMinorVersion == 0)
				{
					if(osvi.wProductType == VER_NT_WORKSTATION)
					{
						rOSVersion += _T("Windows Vista ");
					}
					else 
					{
						rOSVersion += _T("Windows Server 2008 ");
					}
				}

				PGPI pGPI;
				pGPI = (PGPI) GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "GetProductInfo");
				pGPI(6, 0, 0, 0, &dwType);

				switch(dwType)
				{
				case PRODUCT_ULTIMATE:
					rOSVersion += _T("Ultimate Edition");
					break;

				case PRODUCT_HOME_PREMIUM:
					rOSVersion += _T("Home Premium Edition");
					break;

				case PRODUCT_HOME_BASIC:
					rOSVersion += _T("Home Basic Edition");
					break;

				case PRODUCT_ENTERPRISE:
					rOSVersion += _T("Enterprise Edition");
					break;

				case PRODUCT_BUSINESS:
					rOSVersion += _T("Business Edition");
					break;

				case PRODUCT_STARTER:
					rOSVersion += _T("Starter Edition");
					break;

				case PRODUCT_CLUSTER_SERVER:
					rOSVersion += _T("Cluster Server Edition");
					break;

				case PRODUCT_DATACENTER_SERVER:
					rOSVersion += _T("Datacenter Edition");
					break;

				case PRODUCT_DATACENTER_SERVER_CORE:
					rOSVersion += _T("Datacenter Edition (Core)");
					break;

				case PRODUCT_ENTERPRISE_SERVER:
					rOSVersion += _T("Enterprise Edition");
					break;

				case PRODUCT_ENTERPRISE_SERVER_CORE:
					rOSVersion += _T("Enterprise Edition (Core)");
					break;

				case PRODUCT_ENTERPRISE_SERVER_IA64:
					rOSVersion += _T("Enterprise Edition for Itanium-based Systems");
					break;

				case PRODUCT_SMALLBUSINESS_SERVER:
					rOSVersion += _T("Small Business Server");
					break;

				case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
					rOSVersion += _T("Small Business Server Premium Edition");
					break;

				case PRODUCT_STANDARD_SERVER:
					rOSVersion += _T("Standard Edition");
					break;

				case PRODUCT_STANDARD_SERVER_CORE:
					rOSVersion += _T("Standard Edition (core installation)");
					break;

				case PRODUCT_WEB_SERVER:
					rOSVersion += _T("Web Server Edition");
					break;
				}

				if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
				{
					rOSVersion += _T(", 64-bit");
				}
				else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
				{
					rOSVersion += _T(", 32-bit");
				}
			}

			if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2)
			{
				if(GetSystemMetrics(SM_SERVERR2))
				{
					rOSVersion += _T("Windows Server 2003 R2, ");
				}
				else if (osvi.wSuiteMask == VER_SUITE_STORAGE_SERVER)
				{
					rOSVersion += _T("Windows Storage Server 2003");
				}
				else if(osvi.wProductType == VER_NT_WORKSTATION &&
					si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
				{
					rOSVersion += _T("Windows XP Professional x64 Edition");
				}
				else 
				{
					rOSVersion += _T("Windows Server 2003, ");
				}

				// Test for the server type.
				if (osvi.wProductType != VER_NT_WORKSTATION )
				{
					if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
					{
						if(osvi.wSuiteMask & VER_SUITE_DATACENTER)
						{
							rOSVersion += _T("Datacenter Edition for Itanium-based Systems");
						}
						else if(osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
						{
							rOSVersion += _T("Enterprise Edition for Itanium-based Systems");
						}
					}

					else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
					{
						if(osvi.wSuiteMask & VER_SUITE_DATACENTER)
						{
							rOSVersion += _T("Datacenter x64 Edition");
						}
						else if(osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
						{
							rOSVersion += _T("Enterprise x64 Edition");
						}
						else 
						{
							rOSVersion += _T("Standard x64 Edition");
						}
					}

					else
					{
						if (osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER)
						{
							rOSVersion += _T("Compute Cluster Edition");
						}
						else if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
						{
							rOSVersion += _T("Datacenter Edition");
						}
						else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
						{
							rOSVersion += _T("Enterprise Edition");
						}
						else if ( osvi.wSuiteMask & VER_SUITE_BLADE )
						{
							rOSVersion += _T("Web Edition");
						}
						else 
						{
							rOSVersion += _T("Standard Edition");
						}
					}
				}
			}

			if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1)
			{
				rOSVersion += _T("Windows XP ");
				if(osvi.wSuiteMask & VER_SUITE_PERSONAL)
				{
					rOSVersion += _T("Home Edition");
				}
				else 
				{
					rOSVersion += _T("Professional");
				}
			}

			if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
			{
				rOSVersion += _T("Windows 2000 ");

				if (osvi.wProductType == VER_NT_WORKSTATION)
				{
					rOSVersion += _T("Professional");
				}
				else 
				{
					if(osvi.wSuiteMask & VER_SUITE_DATACENTER)
					{
						rOSVersion += _T("Datacenter Server");
					}
					else if(osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
					{
						rOSVersion += _T("Advanced Server");
					}
					else
					{
						rOSVersion += _T("Server");
					}
				}
			}

			if( _tcslen(osvi.szCSDVersion) > 0)
			{
				rOSVersion += _T(" ");
				rOSVersion += osvi.szCSDVersion;
			}

			CString rBuildNo;
			rBuildNo.Format(_T(" (build %d)"), osvi.dwBuildNumber);
			rOSVersion += rBuildNo;
		}
	}

	return rOSVersion;
}

CString GetModulePath()
{
	TCHAR szModuleFileName[MAX_PATH] = { '\0' };
	if (GetModuleFileName(NULL, szModuleFileName, MAX_PATH) == 0)
	{
		ASSERT(FALSE);
		return _T("");
	}

	CString rModuleFileName(szModuleFileName);
	return rModuleFileName;
}

CString GetModuleFolder()
{
	CString rModulePath = GetModulePath();
	int nPos = rModulePath.ReverseFind(_T('\\'));

	rModulePath = rModulePath.Mid(0, nPos + 1);
	return rModulePath;
}

BOOL DownloadFile(CString rServer, CString rFilePath, CString rAgentName, CStringA& rBuffer, int nPort /*= INTERNET_DEFAULT_HTTP_PORT*/)
{
	int nRetries = 0;

retry:

	BOOL bRet = FALSE;
	BOOL bResults = FALSE;

	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;

	HINTERNET hSession = NULL;
	HINTERNET hConnect = NULL;
	HINTERNET hRequest = NULL;

	LPSTR pszOutBuffer;
	CString rError;

	hSession = WinHttpOpen(rAgentName, 
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME, 
		WINHTTP_NO_PROXY_BYPASS, 0);

	if (!hSession)
		goto error;

	hConnect = WinHttpConnect(hSession, rServer, nPort, 0);

	if (!hConnect)
		goto error;

	hRequest = WinHttpOpenRequest(hConnect, _T("GET"), 
		rFilePath, 
		NULL,
		WINHTTP_NO_REFERER, 
		WINHTTP_DEFAULT_ACCEPT_TYPES, 
		0);

	if (!hRequest)
		goto error;

	bResults = WinHttpSendRequest(hRequest,
		WINHTTP_NO_ADDITIONAL_HEADERS,
		0, WINHTTP_NO_REQUEST_DATA, 0, 
		0, 0);

	if (!bResults)
		goto error;

	bResults = WinHttpReceiveResponse(hRequest, NULL);

	if (!bResults)
		goto error;

	do 
	{
		dwSize = 0;
		if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
			goto error;

		pszOutBuffer = new char[dwSize + 1];
		if (!pszOutBuffer)
		{
			dwSize = 0;
			goto error;
		}

		ZeroMemory(pszOutBuffer, dwSize + 1);

		if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded))
			goto error;

		rBuffer += pszOutBuffer;
		delete [] pszOutBuffer;

	} while (dwSize > 0);

	bRet = TRUE;
	goto exit;

error:
	DWORD dwError = GetLastError();

#ifdef _DEBUG
	rError.Format(_T("Error %d has occurred.\n"), dwError);
	AfxMessageBox(rError, MB_ICONERROR | MB_OK);
#endif

	if (dwError == ERROR_WINHTTP_INVALID_SERVER_RESPONSE && nRetries <= 3)
	{
		nRetries++;
		goto retry;
	}

	rError.Format(_T("Error %d has occurred.\n"), dwError);
	AfxMessageBox(rError, MB_ICONERROR | MB_OK);
	
	bRet = FALSE;

exit:
	if (hRequest) 
		WinHttpCloseHandle(hRequest);

	if (hConnect) 
		WinHttpCloseHandle(hConnect);

	if (hSession) 
		WinHttpCloseHandle(hSession);

	return bRet;
}

void EnableDlgItem(HWND hWnd, UINT uID, BOOL bEnable /*= TRUE*/)
{
	CWnd* pWnd = CWnd::FromHandle(GetDlgItem(hWnd, uID));
	if (pWnd == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	pWnd->EnableWindow(bEnable);
}

void DDV_MinMaxChars(CDataExchange* pDX, CString& rValue, int nMin, int nMax)
{
	if (!pDX->m_bSaveAndValidate)
		return;

	DDV_MaxChars(pDX, rValue, nMax);
	
	if (rValue.GetLength() < nMin)
	{
		CString rTxt;
		rTxt.Format(_T("Enter a value between %d and %d characters."), nMin, nMax);

		AfxMessageBox(rTxt);
		pDX->Fail();
	}
}