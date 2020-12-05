#pragma once
#include "framework.h"
#include "resource.h"

class CClientApp : public CWinApp
{
public:
	CClientApp();

public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

extern CClientApp theApp;
