// ClientSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ClientSettingsDlg.h"


// CClientSettingsDlg dialog

IMPLEMENT_DYNAMIC(CClientSettingsDlg, CDialog)

CClientSettingsDlg::CClientSettingsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CClientSettingsDlg::IDD, pParent)
{
}

CClientSettingsDlg::~CClientSettingsDlg()
{
}

void CClientSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_SERVER, m_rServer);
	if (m_rServer.IsEmpty() && pDX->m_bSaveAndValidate)
		pDX->Fail();
}


BEGIN_MESSAGE_MAP(CClientSettingsDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CClientSettingsDlg message handlers

void CClientSettingsDlg::OnBnClickedOk()
{
	OnOK();
}
