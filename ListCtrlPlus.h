#pragma once

class CListCtrlPlus : public CListCtrl
{
public:
	CListCtrlPlus();
	~CListCtrlPlus();

	DECLARE_MESSAGE_MAP()

public:
	BOOL m_bDisableCheckboxes;
	BOOL m_bDisableSelection;
	BOOL m_bClicked;

	int GetSelectedIndex();

	afx_msg void OnNmClicked(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemChanging(NMHDR *pNMHDR, LRESULT *pResult);
};

