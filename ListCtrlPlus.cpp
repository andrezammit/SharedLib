#include "StdAfx.h"
#include "ListCtrlPlus.h"


CListCtrlPlus::CListCtrlPlus()
{
	m_bDisableCheckboxes = FALSE;
	m_bDisableSelection = FALSE;
}

CListCtrlPlus::~CListCtrlPlus()
{
}

BEGIN_MESSAGE_MAP(CListCtrlPlus, CListCtrl)

	ON_NOTIFY_REFLECT(NM_CLICK, OnNmClicked)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNmClicked)
	ON_NOTIFY_REFLECT(NM_RCLICK, OnNmClicked)
	ON_NOTIFY_REFLECT(NM_RDBLCLK, OnNmClicked)

	ON_NOTIFY_REFLECT(LVN_ITEMCHANGING, OnLvnItemChanging)
	
END_MESSAGE_MAP()

void CListCtrlPlus::OnNmClicked(NMHDR *pNMHDR, LRESULT *pResult)
{
	m_bClicked = TRUE;
}

void CListCtrlPlus::OnLvnItemChanging(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (!m_bClicked)
		return;

	m_bClicked = FALSE;

	BOOL bCheckboxClicked = FALSE;
	BOOL bLabelClicked = FALSE;

	POINT pt = { 0 };
	GetCursorPos(&pt);

	ScreenToClient(&pt);

	UINT uFlags = 0;
	HitTest(pt, &uFlags);

	if (uFlags == LVHT_ONITEMSTATEICON)
		bCheckboxClicked = TRUE;

	if (uFlags == LVHT_ONITEM)
		bLabelClicked = TRUE;

	if (GetItemCount() == 0)
	{
		*pResult = FALSE;
		return;
	}

	if (m_bDisableCheckboxes && bCheckboxClicked)
	{
		*pResult = TRUE;
	}
	/*else if (m_bDisableSelection)
	{
		*pResult = TRUE;
	}*/
	else
	{
		*pResult = FALSE;
	}

	return;
}

int CListCtrlPlus::GetSelectedIndex()
{
	if (GetSelectedCount() != 1)
		return -1;

	POSITION pos = GetFirstSelectedItemPosition();
	return GetNextSelectedItem(pos);
}
