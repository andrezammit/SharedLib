
#ifndef SHAREDLIB_H
#define SHAREDLIB_H

#ifndef _DONOTLINKLIB
#ifdef _DEBUG
#pragma comment(lib, "..\\..\\Bin\\Debug\\Libs\\SharedLib.lib")
#else
#pragma comment(lib, "..\\..\\Bin\\Release\\Libs\\SharedLib.lib")
#endif
#endif

#ifdef _USE_XMLPARSER
#include "XMLParser.h"
#endif

#ifdef _USE_DATABASE
#include "../DatabaseInterface/DatabaseInterface.h"
#endif

#include "Winhttp.h"

typedef struct _NETWORK_HEADER
{
	DWORD		dwSignature;
	DWORD		dwCmd;
	DWORD		dwVersion;
	DWORD		dwLen;

} NETWORK_HEADER, *PNETWORK_HEADER;

typedef struct _XML_PACKET
{
	NETWORK_HEADER		header;
	DWORD				dwXMLLen;

} XML_PACKET, *PXML_PACKET;

#define SERVER_LISTEN_PORT						5000
#define CLIENT_LISTEN_PORT						5001

void EnableDlgItem(HWND hWnd, UINT uID, BOOL bEnable = TRUE);
void DDV_MinMaxChars(CDataExchange* pDX, CString& rValue, int nMin, int nMax = 255);

BOOL InitSockets();
BOOL InitServer(SOCKET& serverSock, int nPort);
BOOL ConnectTo(LPCSTR pszServer, int nPort, SOCKET& clientSock);
BOOL SocketSend(SOCKET* pSock, const void* buf, int len);
BOOL SocketReceive(SOCKET* pSock, void* buf, int len);
BOOL DownloadFile(CString rServer, CString rFilePath, CString rAgentName, CStringA& rBuffer, int nPort = INTERNET_DEFAULT_HTTP_PORT);

CString GetOSVersion();
CString GetModulePath();
CString GetModuleFolder();

#endif