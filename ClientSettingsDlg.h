#pragma once

// CClientSettingsDlg dialog
#include "resource.h"

class CClientSettingsDlg : public CDialog
{
	DECLARE_DYNAMIC(CClientSettingsDlg)

public:
	CClientSettingsDlg(CWnd* pParent = NULL); 
	virtual ~CClientSettingsDlg();

	enum { IDD = IDD_CLIENTSETTINGS_DLG };

	CString GetServer() { return m_rServer; }

protected:
	virtual void DoDataExchange(CDataExchange* pDX); 

	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedOk();

private:

	CString m_rServer;
};
