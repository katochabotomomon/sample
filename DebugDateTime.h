/*!	@file		CsvFile.h
 *	@brief		CSV Read/Write class header
 *	@version	1.0.0.0
 *	@auther		t.kato
 *	@date		t.kato		2018.11.14		New create
 *	@date		t.kato		2025.03.10		Modify for current application
 */
#pragma once

#include <string>
#include <iostream>
#include <ctime>
using namespace std;

struct FileDateTime {
	FILETIME DateTime[2];		// 0:Start/1:End
};

struct DateTimeParam {
	CButton *pChkStartNoLimit;
	CButton *pChkEndNoLimit;
	CMonthCalCtrl *pCalStartTime;
	CMonthCalCtrl *pCalEndTime;
	CComboBox *pCmbStartHour;
	CComboBox *pCmbStartMinute;
	CComboBox *pCmbEndHour;
	CComboBox *pCmbEndMinute;
	FileDateTime dt;
	bool bLimitFlg[2];			// true:Limit—L/false:Limit–³
};

class CDebugDateTime
{
private:
	DateTimeParam *m_pParam;

//public:
//	FILETIME m_DateTime[2];		// 0:Start/1:End
//	bool m_bLimitFlg[2];		// true:Limit—L/false:Limit–³

public:
	CDebugDateTime(void);
	~CDebugDateTime(void);

public:
	bool SetCurrentDateTime(DateTimeParam *pParam);
	bool SetParam(DateTimeParam *pParam);
	bool IsTargetFile(WIN32_FIND_DATAW *pData);
};

