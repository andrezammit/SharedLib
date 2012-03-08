#pragma once

#include "ResizeDlg.h"

// CDialogPlus dialog

class CDialogPlus : public CResizeDlg
{
	DECLARE_DYNAMIC(CDialogPlus)

public:
	CDialogPlus(UINT nID, CWnd* pParent = NULL);
	virtual ~CDialogPlus();

	void ShowDlgItem(UINT uID, BOOL bShow = TRUE);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
};
