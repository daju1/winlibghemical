#pragma once
#include "atlbase.h"



template <class T, UINT nServiceNameID>
class ATL_NO_VTABLE CServiceModuleType : public CAtlExeModuleT<T>
{
public :

    void get_service_name()
    {
        LoadString(_AtlBaseModule.GetModuleInstance(), nServiceNameID, m_szServiceName, sizeof(m_szServiceName) / sizeof(TCHAR));
    }

	CServiceModuleType() throw()
	{
		m_bService = TRUE;
        T * pThis = static_cast<T*>(this);
        pThis->get_service_name();
        //LoadString(_AtlBaseModule.GetModuleInstance(), nServiceNameID, m_szServiceName, sizeof(m_szServiceName) / sizeof(TCHAR));

        m_dwDependOnServiceNamesLength = 0;
		ZeroMemory(m_szDependOnServiceNames,sizeof(m_szDependOnServiceNames));
		Checked::tcscpy_s(m_szDependOnServiceNames, _countof(m_szDependOnServiceNames), _T("RPCSS"));
		m_dwDependOnServiceNamesLength = 1 + Checked::_tcsnlen(m_szDependOnServiceNames, _countof(m_szDependOnServiceNames));

		// set up the initial service status 
		m_hServiceStatus = NULL;
		m_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
		m_status.dwCurrentState = SERVICE_STOPPED;
		m_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
		m_status.dwWin32ExitCode = 0;
		m_status.dwServiceSpecificExitCode = 0;
		m_status.dwCheckPoint = 0;
		m_status.dwWaitHint = 0;

		m_failure_action.Delay = 200;
		m_failure_action.Type = SC_ACTION_RESTART;
		m_failure_actions.dwResetPeriod = 0;
		m_failure_actions.lpRebootMsg = NULL;
		m_failure_actions.lpCommand = NULL;
		m_failure_actions.cActions = 1;
		m_failure_actions.lpsaActions = &m_failure_action;
	}

	int WinMain(int nShowCmd) throw()
	{
		if (CAtlBaseModule::m_bInitFailed)
		{
			ATLASSERT(0);
			return -1;
		}

		T* pT = static_cast<T*>(this);
		HRESULT hr = S_OK;

		LPTSTR lpCmdLine = GetCommandLine();
		if (pT->ParseCommandLine(lpCmdLine, &hr) == true)
			hr = pT->Start(nShowCmd);

#ifdef _DEBUG
		// Prevent false memory leak reporting. ~CAtlWinModule may be too late.
		_AtlWinModule.Term();		
#endif	// _DEBUG
		return hr;
	}

	HRESULT Start(int nShowCmd) throw()
	{
		T* pT = static_cast<T*>(this);
		// Are we Service or Local Server
		CRegKey keyAppID;
		LONG lRes = keyAppID.Open(HKEY_CLASSES_ROOT, _T("AppID"), KEY_READ);
		if (lRes != ERROR_SUCCESS)
		{
			m_status.dwWin32ExitCode = lRes;
			return m_status.dwWin32ExitCode;
		}

		CRegKey key;
		lRes = key.Open(keyAppID, pT->GetAppIdT(), KEY_READ);
		if (lRes != ERROR_SUCCESS)
		{
			m_status.dwWin32ExitCode = lRes;
			return m_status.dwWin32ExitCode;
		}

		TCHAR szValue[MAX_PATH];
		DWORD dwLen = MAX_PATH;
		lRes = key.QueryStringValue(_T("LocalService"), szValue, &dwLen);

		m_bService = FALSE;
		if (lRes == ERROR_SUCCESS)
			m_bService = TRUE;

		if (m_bService)
		{
			SERVICE_TABLE_ENTRY st[] =
			{
				{ m_szServiceName, _ServiceMain },
				{ NULL, NULL }
			};
			if (::StartServiceCtrlDispatcher(st) == 0)
				m_status.dwWin32ExitCode = GetLastError();
			return m_status.dwWin32ExitCode;
		}
		// local server - call Run() directly, rather than
		// from ServiceMain()
		m_status.dwWin32ExitCode = pT->Run(nShowCmd);
		return m_status.dwWin32ExitCode;
	}

	inline HRESULT RegisterAppId(bool bService = false) throw()
	{
		if (!Uninstall())
			return E_FAIL;

		HRESULT hr = T::UpdateRegistryAppId(TRUE);
		if (FAILED(hr))
			return hr;

		CRegKey keyAppID;
		LONG lRes = keyAppID.Open(HKEY_CLASSES_ROOT, _T("AppID"), KEY_WRITE);
		if (lRes != ERROR_SUCCESS)
			return AtlHresultFromWin32(lRes);

		CRegKey key;

		lRes = key.Create(keyAppID, T::GetAppIdT());
		if (lRes != ERROR_SUCCESS)
			return AtlHresultFromWin32(lRes);

		key.DeleteValue(_T("LocalService"));

		if (!bService)
			return S_OK;

		key.SetStringValue(_T("LocalService"), m_szServiceName);

		// Create service
		if (!Install())
			return E_FAIL;
		return S_OK;
	}

	HRESULT UnregisterAppId() throw()
	{
		if (!Uninstall())
			return E_FAIL;
		// First remove entries not in the RGS file.
		CRegKey keyAppID;
		LONG lRes = keyAppID.Open(HKEY_CLASSES_ROOT, _T("AppID"), KEY_WRITE);
		if (lRes != ERROR_SUCCESS)
			return AtlHresultFromWin32(lRes);

		CRegKey key;
		lRes = key.Open(keyAppID, T::GetAppIdT(), KEY_WRITE);
		if (lRes != ERROR_SUCCESS)
			return AtlHresultFromWin32(lRes);
		key.DeleteValue(_T("LocalService"));

		return T::UpdateRegistryAppId(FALSE);
	}

	void UpdateDependency(LPCTSTR szServiceName)
	{
		if(WordCmpI(szServiceName, _T("RPCSS"))==0)
			return;
		// делаем перезапуск при ошибке для службы, от которой зависим
		SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if(hSCM)
		{
			SC_HANDLE hService = ::OpenService(hSCM,szServiceName,GENERIC_WRITE | GENERIC_READ);
			if(hService)
			{
				if(::ChangeServiceConfig2(hService,SERVICE_CONFIG_FAILURE_ACTIONS,&m_failure_actions))
				{
#ifdef _DEBUG
					OutputDebugString(_T("Update service '"));
					OutputDebugString(szServiceName);
					OutputDebugString(_T("' (ChangeServiceConfig2)\n"));
#endif
				}
				else
				{
					bool bKeyValid = false;
					LPBYTE pData = NULL;
					DWORD DataType = 0;
					DWORD DataSize = 0;
					TCHAR szKeyName[1024];
					Checked::tcscpy_s(szKeyName, _countof(szKeyName),_T("SYSTEM\\CurrentControlSet\\Services\\"));
					Checked::tcscat_s(szKeyName, _countof(szKeyName),m_szServiceName);
					HKEY hServiceKey = NULL;
					// получаем ключ из нашей службы
					if(ERROR_SUCCESS == ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,szKeyName,0,KEY_READ,&hServiceKey))
					{
						::RegQueryValueEx(hServiceKey,_T("FailureActions"),NULL,&DataType,NULL,&DataSize);
						if(DataSize)
						{
							pData = new BYTE[DataSize];
							bKeyValid = ERROR_SUCCESS == ::RegQueryValueEx(hServiceKey,_T("FailureActions"),NULL,&DataType,pData,&DataSize);
						}
					}
					if(hServiceKey)
						::RegCloseKey(hServiceKey);

					if(bKeyValid)
					{
						Checked::tcscpy_s(szKeyName, _countof(szKeyName),_T("SYSTEM\\CurrentControlSet\\Services\\"));
						Checked::tcscat_s(szKeyName, _countof(szKeyName),szServiceName);
						// копируем ключ в настройки службы, от которой зависим
						if(ERROR_SUCCESS == ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,szKeyName,0,KEY_WRITE,&hServiceKey))
						{
							if(ERROR_SUCCESS == ::RegSetValueEx(hServiceKey,_T("FailureActions"),0,DataType,pData,DataSize))
							{
#ifdef _DEBUG
								OutputDebugString(_T("Update service '"));
								OutputDebugString(szServiceName);
								OutputDebugString(_T("' (Registry)\n"));
							}
							else
							{
								TCHAR szBuf[1024];
								Checked::tcscpy_s(szBuf, _countof(szBuf), _T("Could not change service configuration ("));
								OutputDebugString(szBuf);
								_ultot_s(GetLastError(), szBuf, _countof(szBuf), 10);
								OutputDebugString(szBuf);
								OutputDebugString(_T(")\n"));
#endif
							}
						}
						if(hServiceKey)
							::RegCloseKey(hServiceKey);
					}


					if(pData)
						delete [] pData;
				}
				::CloseServiceHandle(hService);
			}
			::CloseServiceHandle(hSCM);
		}
	}

	void AppendDependencies(LPCTSTR szBegin, LPCTSTR szEnd)
	{
		size_t Index = m_dwDependOnServiceNamesLength;
		size_t StartIndex = m_dwDependOnServiceNamesLength;
		for (;szBegin != szEnd && Index < DependOnServiceNamesSize;
				++Index, ++szBegin)
			m_szDependOnServiceNames[Index] = *szBegin;
		if(Index < DependOnServiceNamesSize)
		{
			m_szDependOnServiceNames[Index++] = _T('\0');
			m_dwDependOnServiceNamesLength = Index;
		}
		else
			m_dwDependOnServiceNamesLength = StartIndex;
	}

	// Parses the command line and registers/unregisters the rgs file if necessary
	bool ParseCommandLine(LPCTSTR lpCmdLine, HRESULT* pnRetCode) throw()
	{
		if (!CAtlExeModuleT<T>::ParseCommandLine(lpCmdLine, pnRetCode))
			return false;

		TCHAR szTokens[] = _T("-/");
		*pnRetCode = S_OK;

		T* pT = static_cast<T*>(this);
		LPCTSTR lpszToken = FindOneOf(lpCmdLine, szTokens);
		bool bRegisterMe = false;
		while (lpszToken != NULL)
		{
			if (WordCmpI(lpszToken, _T("Service"))==0)
				bRegisterMe = true;
			else if (WordCmpI(lpszToken, _T("Depend"))==0)
			{
				const TCHAR szTokensDelimiters[] = _T("\"");
				const TCHAR szTokensSpaces[] = _T(" ");
				LPCTSTR lpszNameStart = FindOneOf(lpszToken, szTokensDelimiters);
				bool bEnclosed = lpszNameStart != NULL;
				if(!lpszNameStart)
					lpszNameStart = FindOneOf(lpszToken, szTokensSpaces);
				LPCTSTR lpszNameEnd   = NULL;
				if(lpszNameStart)
				{
					lpszNameEnd = FindOneOf(lpszNameStart, bEnclosed
						? szTokensDelimiters
						: szTokensSpaces);
					if(bEnclosed)
					{
						if(lpszNameEnd)
							lpszNameEnd = CharPrev(lpszNameStart,lpszNameEnd);
					}
					else
					{
						if(!lpszNameEnd)
							lpszNameEnd = lpszNameStart + _tcslen(lpszNameStart);
					}
				}
				if(lpszNameStart && lpszNameEnd)
				{
					AppendDependencies(lpszNameStart,lpszNameEnd);
					lpszToken = lpszNameEnd;
				}
			}
			lpszToken = FindOneOf(lpszToken, szTokens);
		}
		if(bRegisterMe)
		{
			*pnRetCode = pT->RegisterAppId(true);
			if (SUCCEEDED(*pnRetCode))
				*pnRetCode = pT->RegisterServer(TRUE);
			return false;
		}
		return true;
	}

	void ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv) throw()
	{
		lpszArgv;
		dwArgc;
		// Register the control request handler
		m_status.dwCurrentState = SERVICE_START_PENDING;
		m_hServiceStatus = RegisterServiceCtrlHandler(m_szServiceName, _Handler);
		if (m_hServiceStatus == NULL)
		{
			LogEvent(_T("Handler not installed"));
			return;
		}
		SetServiceStatus(SERVICE_START_PENDING);

		m_status.dwWin32ExitCode = S_OK;
		m_status.dwCheckPoint = 0;
		m_status.dwWaitHint = 0;

		T* pT = static_cast<T*>(this);
#ifndef _ATL_NO_COM_SUPPORT

		HRESULT hr = E_FAIL;
		hr = T::InitializeCom();
		if (FAILED(hr))
		{
			// Ignore RPC_E_CHANGED_MODE if CLR is loaded. Error is due to CLR initializing
			// COM and InitializeCOM trying to initialize COM with different flags.
			if (hr != RPC_E_CHANGED_MODE || GetModuleHandle(_T("Mscoree.dll")) == NULL)
			{
				return;
			}
		}
		else
		{
			m_bComInitialized = true;
		}

		m_bDelayShutdown = false;
#endif //_ATL_NO_COM_SUPPORT
		// When the Run function returns, the service has stopped.
		m_status.dwWin32ExitCode = pT->Run(SW_HIDE);

#ifndef _ATL_NO_COM_SUPPORT
		if (m_bService && m_bComInitialized)
			T::UninitializeCom();
#endif

		SetServiceStatus(SERVICE_STOPPED);
		LogEvent(_T("Service stopped"));
	}

	HRESULT Run(int nShowCmd = SW_HIDE) throw()
	{
		HRESULT hr = S_OK;
		T* pT = static_cast<T*>(this);

		hr = pT->PreMessageLoop(nShowCmd);

		if (hr == S_OK)
		{
			if (m_bService)
			{
				LogEvent(_T("Service started"));
				SetServiceStatus(SERVICE_RUNNING);
			}

			pT->RunMessageLoop();
		}

		if (SUCCEEDED(hr))
		{
			hr = pT->PostMessageLoop();
		}

		return hr;
	}

	HRESULT PreMessageLoop(int nShowCmd) throw()
	{
		HRESULT hr = S_OK;
		if (m_bService)
		{
			m_dwThreadID = GetCurrentThreadId();

			T* pT = static_cast<T*>(this);
			hr = pT->InitializeSecurity();

			if (FAILED(hr))
				return hr;
		}

		hr = CAtlExeModuleT<T>::PreMessageLoop(nShowCmd);
		if (FAILED(hr))
			return hr;

		return hr;
	}

	void OnStop() throw()
	{
		SetServiceStatus(SERVICE_STOP_PENDING);
		PostThreadMessage(m_dwThreadID, WM_QUIT, 0, 0);
	}

	void OnPause() throw()
	{
	}

	void OnContinue() throw()
	{
	}

	void OnInterrogate() throw()
	{
	}

	void OnShutdown() throw()
	{
	}

	void OnUnknownRequest(DWORD /*dwOpcode*/) throw()
	{
		LogEvent(_T("Bad service request"));	
	}

	void Handler(DWORD dwOpcode) throw()
	{
		T* pT = static_cast<T*>(this);

		switch (dwOpcode)
		{
		case SERVICE_CONTROL_STOP:
			pT->OnStop();
			break;
		case SERVICE_CONTROL_PAUSE:
			pT->OnPause();
			break;
		case SERVICE_CONTROL_CONTINUE:
			pT->OnContinue();
			break;
		case SERVICE_CONTROL_INTERROGATE:
			pT->OnInterrogate();
			break;
		case SERVICE_CONTROL_SHUTDOWN:
			pT->OnShutdown();
			break;
		default:
			pT->OnUnknownRequest(dwOpcode);
		}
	}

	BOOL IsInstalled() throw()
	{
		BOOL bResult = FALSE;

		SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

		if (hSCM != NULL)
		{
			SC_HANDLE hService = ::OpenService(hSCM, m_szServiceName, SERVICE_QUERY_CONFIG);
			if (hService != NULL)
			{
				bResult = TRUE;
				::CloseServiceHandle(hService);
			}
			::CloseServiceHandle(hSCM);
		}
		return bResult;
	}
	BOOL Install() throw()
	{
		if (IsInstalled())
			return TRUE;

		// Get the executable file path
		const size_t FilePathSize = MAX_PATH + _ATL_QUOTES_SPACE;
		TCHAR szFilePath[FilePathSize];
		DWORD dwFLen = 0;
		{
			TCHAR szFilePath_x[MAX_PATH];
			dwFLen = ::GetModuleFileName(NULL, szFilePath_x, MAX_PATH);
			if( dwFLen == 0 || dwFLen == MAX_PATH )
				return FALSE;
			//конвертим имя модуля (может быть в короткой форме)
			//в длинную форму
			dwFLen = ::GetLongPathName(szFilePath_x,szFilePath + 1,MAX_PATH);
			if( dwFLen == 0 || dwFLen == MAX_PATH )
				return FALSE;
		}

		// Quote the FilePath before calling CreateService
		szFilePath[0] = _T('\"');
		szFilePath[dwFLen + 1] = _T('\"');
		szFilePath[dwFLen + 2] = 0;

		SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (hSCM == NULL)
		{
			TCHAR szBuf[1024];
			if (AtlLoadString(ATL_SERVICE_MANAGER_OPEN_ERROR, szBuf, 1024) == 0)
				Checked::tcscpy_s(szBuf, _countof(szBuf), _T("Could not open Service Manager"));
			MessageBox(NULL, szBuf, m_szServiceName, MB_OK);
			return FALSE;
		}

		if(m_dwDependOnServiceNamesLength < DependOnServiceNamesSize)
			m_szDependOnServiceNames[m_dwDependOnServiceNamesLength] = _T('\0');

		SC_HANDLE hService = ::CreateService(
			hSCM, m_szServiceName, m_szServiceName,
			SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
			SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
			szFilePath, NULL, NULL,
			(m_dwDependOnServiceNamesLength < DependOnServiceNamesSize)
				?m_szDependOnServiceNames
				:_T("RPCSS\0"),
			NULL, NULL);

		if (hService == NULL)
		{
			::CloseServiceHandle(hSCM);
			TCHAR szBuf[1024];
			if (AtlLoadString(ATL_SERVICE_START_ERROR, szBuf, 1024) == 0)
				Checked::tcscpy_s(szBuf, _countof(szBuf), _T("Could not start service"));
			MessageBox(NULL, szBuf, m_szServiceName, MB_OK);
			return FALSE;
		}

		if (!::ChangeServiceConfig2(hService,SERVICE_CONFIG_FAILURE_ACTIONS,&m_failure_actions))
		{
			TCHAR szBuf[1024];
			Checked::tcscpy_s(szBuf, _countof(szBuf), _T("Could not change service configuration"));
			MessageBox(NULL, szBuf, m_szServiceName, MB_OK | MB_ICONWARNING);
		}
		else
		{
			for(LPCTSTR szServiceName = m_szDependOnServiceNames, szServiceNameEnd = &m_szDependOnServiceNames[m_dwDependOnServiceNamesLength];
				*szServiceName && szServiceName < szServiceNameEnd;
				szServiceName += _tcslen(szServiceName) + 1)
				UpdateDependency(szServiceName);
		}

		::CloseServiceHandle(hService);
		::CloseServiceHandle(hSCM);
		return TRUE;
	}

	BOOL Uninstall() throw()
	{
		if (!IsInstalled())
			return TRUE;

		SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

		if (hSCM == NULL)
		{
			TCHAR szBuf[1024];
			if (AtlLoadString(ATL_SERVICE_MANAGER_OPEN_ERROR, szBuf, 1024) == 0)
				Checked::tcscpy_s(szBuf, _countof(szBuf), _T("Could not open Service Manager"));
			MessageBox(NULL, szBuf, m_szServiceName, MB_OK);
			return FALSE;
		}

		SC_HANDLE hService = ::OpenService(hSCM, m_szServiceName, SERVICE_STOP | DELETE);

		if (hService == NULL)
		{
			::CloseServiceHandle(hSCM);
			TCHAR szBuf[1024];
			if (AtlLoadString(ATL_SERVICE_OPEN_ERROR, szBuf, 1024) == 0)
				Checked::tcscpy_s(szBuf, _countof(szBuf), _T("Could not open service"));
			MessageBox(NULL, szBuf, m_szServiceName, MB_OK);
			return FALSE;
		}
		SERVICE_STATUS status;
		BOOL bRet = ::ControlService(hService, SERVICE_CONTROL_STOP, &status);
		if (!bRet)
		{
			DWORD dwError = GetLastError();
			if (!((dwError == ERROR_SERVICE_NOT_ACTIVE) ||  (dwError == ERROR_SERVICE_CANNOT_ACCEPT_CTRL && status.dwCurrentState == SERVICE_STOP_PENDING)))
			{
				TCHAR szBuf[1024];
				if (AtlLoadString(ATL_SERVICE_STOP_ERROR, szBuf, 1024) == 0)
					Checked::tcscpy_s(szBuf, _countof(szBuf), _T("Could not stop service"));
				MessageBox(NULL, szBuf, m_szServiceName, MB_OK);
			}
		}


		BOOL bDelete = ::DeleteService(hService);
		::CloseServiceHandle(hService);
		::CloseServiceHandle(hSCM);

		if (bDelete)
			return TRUE;

		TCHAR szBuf[1024];
		if (AtlLoadString(ATL_SERVICE_DELETE_ERROR, szBuf, 1024) == 0)
			Checked::tcscpy_s(szBuf, _countof(szBuf), _T("Could not delete service"));
		MessageBox(NULL, szBuf, m_szServiceName, MB_OK);
		return FALSE;
	}

	LONG Unlock() throw()
	{
		LONG lRet;
		if (m_bService)
		{
			// We are running as a service, therefore transition to zero does not
			// unload the process
			lRet = CAtlModuleT<T>::Unlock();
		}
		else
		{
			// We are running as EXE, use MonitorShutdown logic provided by CExeModule
			lRet = CAtlExeModuleT<T>::Unlock();
		}
		return lRet;
	}

	void LogEventEx(int id, LPCTSTR pszMessage=NULL, WORD type = EVENTLOG_INFORMATION_TYPE) throw()
	{
		HANDLE hEventSource;
		if (m_szServiceName)
		{
			/* Get a handle to use with ReportEvent(). */
			hEventSource = RegisterEventSource(NULL, m_szServiceName);
			if (hEventSource != NULL)
			{
				/* Write to event log. */
				ReportEvent(hEventSource, 
					type,
					(WORD)0,
					id,
					NULL,
					(WORD)(pszMessage != NULL ? 1 : 0),
					0,
					pszMessage != NULL ? &pszMessage : NULL,
					NULL);
				DeregisterEventSource(hEventSource);
			}
		}
	}

#pragma warning(push)
#pragma warning(disable : 4793)
	void __cdecl LogEvent(LPCTSTR pszFormat, ...) throw()
	{
		const int LOG_EVENT_MSG_SIZE = 256;

		TCHAR chMsg[LOG_EVENT_MSG_SIZE];
		HANDLE hEventSource;
		LPTSTR lpszStrings[1];
		va_list pArg;

		va_start(pArg, pszFormat);
		_vsntprintf_s(chMsg, LOG_EVENT_MSG_SIZE, LOG_EVENT_MSG_SIZE-1, pszFormat, pArg);
		va_end(pArg);

		chMsg[LOG_EVENT_MSG_SIZE - 1] = 0;

		lpszStrings[0] = chMsg;

		if (!m_bService)
		{
			// Not running as a service, so print out the error message 
			// to the console if possible
			_putts(chMsg);
		}

		/* Get a handle to use with ReportEvent(). */
		hEventSource = RegisterEventSource(NULL, m_szServiceName);
		if (hEventSource != NULL)
		{
			/* Write to event log. */
			ReportEvent(hEventSource, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL);
			DeregisterEventSource(hEventSource);
		}
	}
#pragma warning(pop)

	void SetServiceStatus(DWORD dwState) throw()
	{
		m_status.dwCurrentState = dwState;
		::SetServiceStatus(m_hServiceStatus, &m_status);
	}

	//Implementation
protected:
	static void WINAPI _ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv) throw()
	{
		((T*)_pAtlModule)->ServiceMain(dwArgc, lpszArgv);
	}
	static void WINAPI _Handler(DWORD dwOpcode) throw()
	{
		((T*)_pAtlModule)->Handler(dwOpcode); 
	}

	// data members
public:
	TCHAR m_szServiceName[256];
	enum{
		DependOnServiceNamesSize = 1024
	};
	TCHAR m_szDependOnServiceNames[DependOnServiceNamesSize];
	size_t m_dwDependOnServiceNamesLength;
	SERVICE_STATUS_HANDLE m_hServiceStatus;
	SERVICE_STATUS m_status;
	SERVICE_FAILURE_ACTIONS m_failure_actions;
	SC_ACTION m_failure_action;
	BOOL m_bService;
	DWORD m_dwThreadID;
};
