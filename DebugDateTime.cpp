/*!	@file		DebugDateTime.cpp
 *	@brief		File target check class
 *	@details	Check whether the file is eligible based on the start and end dates
 *	@auther		t.kato
 *	@version	t.kato		2025.03.01		New create (1.0.0.0)
 */
#include "StdAfx.h"
#include "DebugDateTime.h"

CDebugDateTime::CDebugDateTime(void)
{
	SYSTEMTIME SysT;
	time_t t = time(nullptr);
	tm now;

	localtime_s(&now, &t);

	SysT.wYear			= (WORD)(now.tm_year + 1900);
	SysT.wMonth			= (WORD)(now.tm_mon + 1);
	SysT.wDay			= (WORD)now.tm_mday;
	SysT.wDayOfWeek		= (WORD)now.tm_wday;
	SysT.wHour			= (WORD)now.tm_hour;
	SysT.wMinute		= (WORD)now.tm_min;
	SysT.wSecond		= (WORD)now.tm_sec;
	SysT.wMilliseconds	= (WORD)0;
	//SystemTimeToFileTime(&SysT, &m_DateTime[0]);
	//SystemTimeToFileTime(&SysT, &m_DateTime[1]);

	//m_bLimitFlg[0] = true;		// 初期値はLimit有
	//m_bLimitFlg[1] = true;		// 初期値はLimit有
}


CDebugDateTime::~CDebugDateTime(void)
{
}

bool CDebugDateTime::SetCurrentDateTime(DateTimeParam *pParam)
{
	time_t t = time(nullptr);
	tm now;
	SYSTEMTIME SysT;

	localtime_s(&now, &t);

	if (pParam <= 0) {
		return false;
	}
	SysT.wDayOfWeek		= now.tm_wday;
	SysT.wYear			= (WORD)(now.tm_year + 1900);
	SysT.wMonth			= (WORD)(now.tm_mon + 1);
	SysT.wDay			= (WORD)now.tm_mday;
	SysT.wHour			= (WORD)now.tm_hour;
	SysT.wMinute		= (WORD)now.tm_min;
	SysT.wSecond		= (WORD)0;
	SysT.wMilliseconds	= (WORD)0;

	// 開始を0:00に設定
	pParam->pCmbStartHour->SetCurSel(0);
	pParam->pCmbStartMinute->SetCurSel(0);
	// No Limitを外す
	pParam->pChkStartNoLimit->SetCheck(0);
	pParam->pChkEndNoLimit->SetCheck(0);
	// 終了を23:59に設定
	pParam->pCmbEndHour->SetCurSel(pParam->pCmbEndHour->GetCount() - 1);
	pParam->pCmbEndMinute->SetCurSel(pParam->pCmbEndMinute->GetCount() - 1);
	// 日付を今日に設定
	pParam->pCalStartTime->SetToday(&SysT);
	pParam->pCalEndTime->SetToday(&SysT);

	return true;
}

bool CDebugDateTime::SetParam(DateTimeParam *pParam)
{
	SYSTEMTIME SysT;
	int iStatus;
	CString strData;

	if (pParam <= 0) {
		return false;
	}
	// Limit設定
	iStatus = pParam->pChkStartNoLimit->GetCheck();
	pParam->bLimitFlg[0] = (iStatus == BST_CHECKED) ? false : true;
	iStatus = pParam->pChkEndNoLimit->GetCheck();
	pParam->bLimitFlg[1] = (iStatus == BST_CHECKED) ? false : true;
	// 開始日時設定
	pParam->pCalStartTime->GetCurSel(&SysT);
	pParam->pCmbStartHour->GetWindowTextW(strData);
	SysT.wHour			= (WORD)_wtoi(strData);
	pParam->pCmbStartMinute->GetWindowTextW(strData);
	SysT.wMinute		= (WORD)_wtoi(strData);
	SysT.wSecond		= (WORD)0;
	SysT.wMilliseconds	= (WORD)0;
	SystemTimeToFileTime(&SysT, &pParam->dt.DateTime[0]);
	// 終了日時設定
	pParam->pCalEndTime->GetCurSel(&SysT);
	pParam->pCmbEndHour->GetWindowTextW(strData);
	SysT.wHour			= (WORD)_wtoi(strData);
	pParam->pCmbEndMinute->GetWindowTextW(strData);
	SysT.wMinute		= (WORD)_wtoi(strData);
	SysT.wSecond		= (WORD)0;
	SysT.wMilliseconds	= (WORD)0;
	SystemTimeToFileTime(&SysT, &pParam->dt.DateTime[1]);

	m_pParam = pParam;

	return true;
}

bool CDebugDateTime::IsTargetFile(WIN32_FIND_DATAW *pData)
{
	FILETIME FTime[2];
	bool bTarget = false;
	LONG lRet;

	if (pData <= 0) {
		return false;
	}

	// 作成日時と更新日時を範囲とする
	// 範囲はどちらかが対象範囲に入っていれば、対象とする必要がある(ファイルがまたがっている場合があるため)
	FileTimeToLocalFileTime(&pData->ftCreationTime, &FTime[0]);
	FileTimeToLocalFileTime(&pData->ftLastWriteTime, &FTime[1]);
	if (m_pParam->bLimitFlg[0] == true) {		// 開始Limit有
		if (m_pParam->bLimitFlg[1] == true) {	// 終了Limit有
			// 開始・終了両リミット
			// Time Series			Target1		Target2		Target3		Target4		Target5		Target6		NonTarget1	NonTarget2
			// ↓-------------------FileCreate--FileCreate--------------------------------------FileCreate--------------FileCreate
			// ↓-------------------・・・・・--・・・・・--------------------------------------・・・・・--------------FileUpdate
			// ↓File Area Start----FileUpdate--・・・・・--------------------------------------・・・・・
			// ↓・・・・・---------------------・・・・・--FileCreate--------------------------・・・・・
			// ↓・・・・・---------------------FileUpdate--・・・・・--------------------------・・・・・
			// ↓・・・・・---------------------------------・・・・・--FileCreate--------------・・・・・
			// ↓・・・・・---------------------------------FileUpdate--・・・・・--------------・・・・・
			// ↓File Area End------------------------------------------・・・・・--FileCreate--・・・・・
			// ↓-------------------------------------------------------・・・・・--・・・・・--・・・・・--FileCreate
			// ↓-------------------------------------------------------FileUpdate--FileUpdate--FileUpdate--FileUpdate
			lRet = CompareFileTime(&m_pParam->dt.DateTime[0], &FTime[0]);
			if (lRet <= 0) {		// 作成日時が開始以上(Target3～5,NonTarget1)
				lRet = CompareFileTime(&m_pParam->dt.DateTime[1], &FTime[0]);
				if (lRet >= 0) {	// 作成日時が終了以下(Target3～5)
					bTarget = true;
				}
				else {				// 作成日時が終了より後(NonTarget1)
					bTarget = false;
				}
			}
			else {					// 作成日時が開始より前(Target1～2,6,NonTarget2)
				lRet = CompareFileTime(&m_pParam->dt.DateTime[0], &FTime[1]);
				if (lRet <= 0) {	// 更新日時が開始以上(Target1～2,6)
					bTarget = true;
				}
				else {				// 更新日時が開始より前(NonTarget2)
					bTarget = false;
				}
			}
		}
		else {							// 終了Limit無
			// Time Series			Target1		Target2		Target3		Target4		Target5		Target6		Target7		NonTarget
			// ↓-------------------FileCreate--FileCreate--------------------------------------FileCreate--------------FileCreate
			// ↓-------------------・・・・・--・・・・・--------------------------------------・・・・・--------------FileUpdate
			// ↓File Area Start----FileUpdate--・・・・・--------------------------------------・・・・・
			// ↓・・・・・---------------------・・・・・--FileCreate--------------------------・・・・・
			// ↓・・・・・---------------------FileUpdate--・・・・・--------------------------・・・・・
			// ↓・・・・・---------------------------------・・・・・--FileCreate--------------・・・・・
			// ↓・・・・・---------------------------------FileUpdate--・・・・・--------------・・・・・
			// ↓File Area End------------------------------------------・・・・・--FileCreate--・・・・・
			// ↓-------------------------------------------------------・・・・・--・・・・・--・・・・・--FileCreate
			// ↓-------------------------------------------------------FileUpdate--FileUpdate--FileUpdate--FileUpdate
			lRet = CompareFileTime(&m_pParam->dt.DateTime[0], &FTime[1]);
			if (lRet <= 0) {			// 更新日時が開始以上(Target1～7)
				bTarget = true;
			}
			else {						// 更新日時が開始より前(NonTarget)
				bTarget = false;
			}
		}
	}
	else {								// 開始Limit無
		if (m_pParam->bLimitFlg[1] == true) {	// 終了Limit有
			// Time Series			Target1		Target2		Target3		Target4		Target5		Target6		NonTarget	Target7
			// ↓-------------------FileCreate--FileCreate--------------------------------------FileCreate--------------FileCreate
			// ↓-------------------・・・・・--・・・・・--------------------------------------・・・・・--------------FileUpdate
			// ↓File Area Start----FileUpdate--・・・・・--------------------------------------・・・・・
			// ↓・・・・・---------------------・・・・・--FileCreate--------------------------・・・・・
			// ↓・・・・・---------------------FileUpdate--・・・・・--------------------------・・・・・
			// ↓・・・・・---------------------------------・・・・・--FileCreate--------------・・・・・
			// ↓・・・・・---------------------------------FileUpdate--・・・・・--------------・・・・・
			// ↓File Area End------------------------------------------・・・・・--FileCreate--・・・・・
			// ↓-------------------------------------------------------・・・・・--・・・・・--・・・・・--FileCreate
			// ↓-------------------------------------------------------FileUpdate--FileUpdate--FileUpdate--FileUpdate
			lRet = CompareFileTime(&m_pParam->dt.DateTime[1], &FTime[0]);
			if (lRet >= 0) {			// 作成日時が終了以下(Target1～6,7)
				bTarget = true;
			}
			else {						// 作成日時が終了以上(NonTarget)
				bTarget = false;
			}
		}
		else {							// 終了Limit無(全Target)
			bTarget = true;
		}
	}
	return bTarget;
}
