// MolDynSrv.cpp : Implementation of WinMain


#include "stdafx.h"
#include "resource.h"
#include "MolDynSrv_i.h"

#include <stdio.h>
#include <conio.h>

#include "..\..\libconfig.h"
#include "..\..\model.h"
/*
#include <iostream>
using namespace std;
*/
bool ReadGPR(model &, istream &, bool, bool);			// see fileio.cpp!!!

#include "..\..\notice.h"

#include "..\..\eng1_mm_tripos52.h"
#include "..\..\eng1_mm_default.h"

#include "..\..\geomopt.h"
#include "..\..\moldyn.h"

class jobinfo_MolDyn_tst
{
public:

	model * mdl;
	moldyn_tst_param param;

	jobinfo_MolDyn_tst() 
		: param(NULL)
		, mdl(NULL)
	{
	}
};


class Logger
{
public:
	static void Write(TCHAR * str)
	{
		FILE * stream = fopen("D:\\MolDynSrv.log", "at");
		if (stream)
		{
			fprintf(stream, "%S\n", str);
			fclose(stream);
		}
	}
};


#include "ServiceModuleType.h"

class StringHelper
{
public:
	static LPTSTR ReverseFindOneOf(LPTSTR p1, LPCTSTR p2) throw()
	{
		LPTSTR _p1 = p1;
		while (_p1 != NULL && *_p1 != NULL)
		{
			_p1 = CharNext(_p1);
		}

		while (_p1 != NULL)
		{
			LPCTSTR p = p2;
			while (p != NULL && *p != NULL)
			{
				if (*_p1 == *p)
					return CharNext(_p1);
				p = CharNext(p);
			}
			_p1 = CharPrev(p1, _p1);
		}
		return NULL;
	}
};


class CMolDynSrvModule 
	: public CServiceModuleType< CMolDynSrvModule, IDS_SERVICENAME >
	, public StringHelper
{
public :
	DECLARE_LIBID(LIBID_MolDynSrvLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_MOLDYNSRV, "{5AE54E2A-36F9-464F-A4D8-3290B3001BA8}")

	HRESULT InitializeSecurity() throw()
	{
		return CoInitializeSecurity(0, -1, 0, 0, RPC_C_AUTHN_LEVEL_NONE, RPC_C_IMP_LEVEL_ANONYMOUS, 0, EOAC_NONE, 0);
	}

	void RunMessageLoop() throw()
	{
		if (m_bService)
		{
			MSG msg;
			while (GetMessage(&msg, 0, 0, 0) > 0)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			_getch();
		}
	}

	static DWORD WINAPI pcs_job_MolDyn_tst(void* p)
	{
		jobinfo_MolDyn_tst * ji = (jobinfo_MolDyn_tst *) p;

		const bool updt = true;

		ji->mdl->DoMolDyn_tst(ji->param, updt);

		return NULL;
	}


	HANDLE start_job_MolDyn_tst(jobinfo_MolDyn_tst * ji)
	{
		DWORD dwChildID;
		HANDLE hThread = ::CreateThread(NULL, 0, 
			pcs_job_MolDyn_tst, 
			reinterpret_cast<LPVOID>(ji), 
			0,
			&dwChildID );

		return hThread;
	}

	bool StartingJobs()
	{

		LogEvent(_T("StartingJobs()"));

		// Get the executable file path
		const size_t FilePathSize = MAX_PATH + _ATL_QUOTES_SPACE;
		TCHAR szFilePath[FilePathSize];
		DWORD dwFLen = 0;
		{
			//TCHAR szFilePath_x[MAX_PATH];
			dwFLen = ::GetModuleFileName(NULL, szFilePath, MAX_PATH);
			if( dwFLen == 0 || dwFLen == MAX_PATH )
				return FALSE;
		}

		TCHAR szTokens[] = _T("/\\");
		LPTSTR lpszToken = ReverseFindOneOf(szFilePath, szTokens);
		if ( NULL != lpszToken)
		{
			(*lpszToken) = 0;
		}

		LPCTSTR iniFileName = _T("MolDynSrv.ini");
		TCHAR szIniFileName[FilePathSize];
#ifdef UNICODE
		wcscpy(szIniFileName, szFilePath);
		wcscat(szIniFileName, iniFileName);
#else
		strcpy(szIniFileName, szFilePath);
		strcat(szIniFileName, iniFileName);
#endif


		int nDefaultThreadsCount = 0;

		int nThreadsCount = GetPrivateProfileInt(_T("threads"),	_T("count"),
			nDefaultThreadsCount, szIniFileName);

		for (int i = 1; i <= nThreadsCount;++i)
		{
			const size_t nFilePathSize = MAX_PATH + _ATL_QUOTES_SPACE;
			TCHAR szDir[nFilePathSize];
			TCHAR szGpr[nFilePathSize];
			TCHAR szBox[nFilePathSize];
			TCHAR szTemperature[nFilePathSize];
			TCHAR szTimeStep[nFilePathSize];

			TCHAR szAppName[nFilePathSize];
			wsprintf(szAppName,_T("%i"), i);

			GetPrivateProfileString(szAppName,  _T("dir"), _T(""),
				szDir,	nFilePathSize, szIniFileName);
			GetPrivateProfileString(szAppName,  _T("gpr"), _T(""),
				szGpr,	nFilePathSize, szIniFileName);
			GetPrivateProfileString(szAppName,  _T("box"), _T(""),
				szBox,	nFilePathSize, szIniFileName);

			GetPrivateProfileString(szAppName,  _T("T"), _T(""),
				szTemperature,	nFilePathSize, szIniFileName);
			GetPrivateProfileString(szAppName,  _T("tstep"), _T(""),
				szTimeStep,	nFilePathSize, szIniFileName);

			int nDefaultFrame = 1;
			int nFrame = GetPrivateProfileInt(szAppName, _T("frame"),
				nDefaultFrame, szIniFileName);
			int nNewFrame = nFrame + 1;


			TCHAR szInputFile[nFilePathSize];
			wsprintf(szInputFile,_T("%s\\%s.gpr"), szDir, szGpr);

			char szBoxFile[nFilePathSize];
#ifdef  UNICODE
			sprintf(szBoxFile, "%S\\%S.box", szDir, szBox);
#else
			sprintf(szBoxFile, "%s\\%s.box", szDir, szBox);
#endif

			ifstream ifile;
			ifile.open(szInputFile, ios::in);

			if (ifile.fail())
			{
				ifile.close();

				//ASSERT(FALSE);
				continue;
			}

			//TODO: solve memory leak
			jobinfo_MolDyn_tst * ji = new jobinfo_MolDyn_tst();

	
			//TODO: solve memory leak
			ji->mdl = new model();
			ReadGPR(* ji->mdl, ifile, false, false);
			ifile.close();


			ji->mdl->LoadBox(szBoxFile);

			const bool updt = true;
			setup * su = ji->mdl->GetCurrentSetup();

			ji->param = moldyn_tst_param(su);

			ji->param.load_last_frame = true;

			ji->param.nsteps_h = 0;		// heat
			ji->param.nsteps_e = 0;		// equilibration
			ji->param.nsteps_s = 10000000;		// simulation

			ji->param.temperature = 400.;
			ji->param.timestep = 0.5;

#ifdef  UNICODE
			ji->param.temperature = _wtof(szTemperature);
			ji->param.timestep = _wtof(szTimeStep);
#else
			ji->param.temperature = atof(szTemperature);
			ji->param.timestep = atof(szTimeStep);
#endif


#ifdef  UNICODE
			sprintf(ji->param.filename_input_frame,  "%S\\%d.frame", szDir, nFrame);
			sprintf(ji->param.filename_output_frame, "%S\\%d.frame", szDir, nNewFrame);
			sprintf(ji->param.filename_traj,         "%S\\%d.traj",  szDir, nNewFrame);
#else
			sprintf(ji->param.filename_input_frame,  "%s\\%d.frame", szDir, nFrame);
			sprintf(ji->param.filename_output_frame, "%s\\%d.frame", szDir, nNewFrame);
			sprintf(ji->param.filename_traj,         "%s\\%d.traj",  szDir, nNewFrame);
#endif

			// TODO: copy frame file
			CopyFileA(ji->param.filename_input_frame, ji->param.filename_output_frame, TRUE);

			TCHAR szNewFrame[nFilePathSize];
			wsprintf(szNewFrame, _T("%d"), nNewFrame);
			WritePrivateProfileString(szAppName, _T("frame"),
				szNewFrame, szIniFileName);

			HANDLE hThread = start_job_MolDyn_tst(ji);
		}

		return true;
	}

	HRESULT Run(int nShowCmd = SW_HIDE) throw()
	{
		HRESULT hr = PreMessageLoop(nShowCmd);

		if (m_bService)
		{
			LogEvent(_T("Service started"));
			SetServiceStatus(SERVICE_RUNNING);
		}
#if 1

		bool init_failed = !StartingJobs();

		run_tests();

		if ( !init_failed)
		{
//			ServerProxyPtr server_proxy;
//			server_proxy.reset( new ServerProxy());
			RunMessageLoop();
//			server_proxy.reset();
		}
		else
#endif
			RunMessageLoop();
#if 0
		// Останов логгера
		Logger::Journal::instance().stop();
#endif
		if (SUCCEEDED(hr))
			hr = PostMessageLoop();

		return hr;
	}

private:

	void run_tests()
	{
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
/*
		if (!m_bService)
		{
			// Not running as a service, so print out the error message 
			// to the console if possible
			_putts(chMsg);
		}*/
		Logger::Write(chMsg);

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

};

CMolDynSrvModule service;

//HINSTANCE hInst;


//
#if 1
extern "C" int WINAPI _tWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, 
                                LPTSTR /*lpCmdLine*/, int nShowCmd)
{
    return service.WinMain(nShowCmd);
}
#else
//
int _tmain(int argc, _TCHAR* argv[])
{
	return service.WinMain(SW_NORMAL);
}
#endif

