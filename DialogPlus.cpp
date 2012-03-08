// DialogPlus.cpp : implementation file
//

#include "stdafx.h"
#include "SharedLib.h"
#include "DialogPlus.h"
#include "afxdialogex.h"


// CDialogPlus dialog

IMPLEMENT_DYNAMIC(CDialogPlus, CResizeDlg)

CDialogPlus::CDialogPlus(UINT nID, CWnd* pParent /*=NULL*/)
	: CResizeDlg(nID, pParent)
{

}

CDialogPlus::~CDialogPlus()
{
}

void CDialogPlus::DoDataExchange(CDataExchange* pDX)
{
	CResizeDlg::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDialogPlus, CResizeDlg)
END_MESSAGE_MAP()

// CDialogPlus message handlers

void CDialogPlus::ShowDlgItem(UINT uID, BOOL bShow /*= TRUE*/)
{
	CWnd* pWnd = (CWnd*) GetDlgItem(uID);
	if (pWnd == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	pWnd->ShowWindow(bShow);
}