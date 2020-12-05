#pragma once
#include "framework.h"

class CFilePathEdit : public CEdit
{
	DECLARE_DYNAMIC(CFilePathEdit)

public:
	CFilePathEdit(BOOL bLocalPath);
	virtual ~CFilePathEdit();

protected:
	DECLARE_MESSAGE_MAP()

public:
	BOOL m_bLocalPath;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};


