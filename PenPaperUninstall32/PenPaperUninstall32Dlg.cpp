/*=============================================================================
Copyright (C) 2018 ACE CAD Enterprise Co., Ltd., All Rights Reserved.

Module Name:
PenPaper HID Minidriver Uninstall Program

File Name:
PenPaperUninstall32Dlg.cpp : implementation file
=============================================================================*/

#include "stdafx.h"
#include "PenPaperUninstall32.h"
#include "PenPaperUninstall32Dlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 2018.4.9, Device Class GUID for PenPaper Writing Service
static GUID PenPaperPenWritingServiceGuidLong = { 0x0000ace0, 0x0000, 0x1000,{ 0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb } };

// CPenPaperUninstall32Dlg dialog



CPenPaperUninstall32Dlg::CPenPaperUninstall32Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_PENPAPERUNINSTALL32_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPenPaperUninstall32Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPenPaperUninstall32Dlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()


// CPenPaperUninstall32Dlg message handlers

BOOL CPenPaperUninstall32Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//=================================================================
	BOOL b_ItsWow64 = FALSE;

	LPTSTR*	FilePart64;
	WIN32_FIND_DATA	FindFileData;
	HANDLE hX64Program;

	DWORD dwError;
	CString csString;

	HDEVINFO hDevInfo;
	SP_DEVINFO_DATA DevInfoData;
	SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA pDeviceInterfaceDetailData = NULL;
	DWORD MemberIndex;
	DWORD InterfaceIndex;
	DWORD dwRequiredSize;
	DWORD bufferSize;
	PWSTR pService;
	PWSTR pProvider;
	DEVPROPTYPE ReturnedPropertyType;
	DWORD sizeRequired;
	WCHAR *pwInfPath;
	WCHAR *pwDriverProvider;
	HRESULT hRes;

	//static WCHAR	szCurrentDir[MAX_PATH];
	static WCHAR	sz64BitSetupFile[MAX_PATH];

	static WCHAR	szTargetFolder[MAX_PATH];
	static WCHAR	szTargetControlPanelFile[MAX_PATH];
	static WCHAR	szTargetUserGuideFile[MAX_PATH];

	static PWSTR	pszDesktopFolder = NULL;
	static WCHAR	szDesktopFile[MAX_PATH];
	static PWSTR	pszStartMenuFolder = NULL;
	static WCHAR	szStartMenuFolder[MAX_PATH];
	static WCHAR	szStartMenuFile[MAX_PATH];

	LPSTR*	FilePart;
	static CHAR	szCurrentDir[MAX_PATH];
	static CHAR szFinalCommand[MAX_PATH];

	//=======================================
	// Check if we are in 64-bit environment.
	//=======================================
	typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
	LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(L"kernel32"), "IsWow64Process");

	if (fnIsWow64Process != NULL) fnIsWow64Process(GetCurrentProcess(), &b_ItsWow64);

	/*--------------------------------*/
	/* Get path of current directory. */
	/*--------------------------------*/
	FilePart64 = NULL;
	//SearchPath(NULL, L"PenPaperUninstall32.exe", L".exe", MAX_PATH, szCurrentDir, FilePart64);
	//PathRemoveFileSpec(szCurrentDir);
	SearchPath(NULL, L"PenPaperUninstall32.exe", L".exe", MAX_PATH, sz64BitSetupFile, FilePart64);
	PathRemoveFileSpec(sz64BitSetupFile);

	if (b_ItsWow64)
	{
		/*-----------------------------------------------*/
		/* Check 64-bit setup program file exist or not. */
		/*-----------------------------------------------*/
		//wcscpy_s(sz64BitSetupFile, szCurrentDir);
		PathAppend(sz64BitSetupFile, TEXT("PenPaperUninstall64.exe"));
		hX64Program = FindFirstFile(sz64BitSetupFile, &FindFileData);
		if (hX64Program == INVALID_HANDLE_VALUE)
		{
			csString.Format(L"Missing PenPaperUninstall64.exe");
			MessageBox(csString, L"PenPaperUninstall32", MB_OK);
		}
		else
		{
			UINT ui_ret;
			ui_ret = WinExec("PenPaperUninstall64.exe", SW_SHOWDEFAULT);
			if (ui_ret <= 31)
			{
				if (ui_ret == ERROR_BAD_FORMAT) csString.Format(L"Run PenPaperUninstall64.exe error - ERROR_BAD_FORMAT");
				else if (ui_ret == ERROR_FILE_NOT_FOUND) csString.Format(L"Run PenPaperUninstall64.exe error - ERROR_FILE_NOT_FOUND");
				else csString.Format(L"Run PenPaperUninstall64.exe error - Unknown error");
			}
		}
		EndDialog(IDCANCEL);
		return 0;
	}


	//==============================================
	// Step 1: Uninstall the PenPaper HID minidriver
	//==============================================

	//-----------------------------------------------------
	// Get the DevInfo of all Bluetooth GATT Service device
	//-----------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------
	// 2018.4.9, For Windows 8.0, no GUID_BLUETOOTH_GATT_SERVICE_DEVICE_INTERFACE exist in the registry, so, we change to search
	// the PenPaper Writing Service GUID directly.
	//--------------------------------------------------------------------------------------------------------------------------
	//hDevInfo = SetupDiGetClassDevs(&GUID_BLUETOOTH_GATT_SERVICE_DEVICE_INTERFACE, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	hDevInfo = SetupDiGetClassDevs(&PenPaperPenWritingServiceGuidLong, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	if (hDevInfo == INVALID_HANDLE_VALUE)
	{
		dwError = GetLastError();
		csString.Format(L"SetupDiGetClassDevs() fail, error: 0x%x", dwError);
		MessageBox(csString, L"PenPaper.Uninhstall", MB_OK);
		EndDialog(IDCANCEL);
		return 0;
	}

	DevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	MemberIndex = 0;

	while (SetupDiEnumDeviceInfo(hDevInfo, MemberIndex++, &DevInfoData))
	{
		DeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		InterfaceIndex = 0;

		//--------------------------------------------------------------------------------------------------------------------------
		// 2018.4.9, For Windows 8.0, no GUID_BLUETOOTH_GATT_SERVICE_DEVICE_INTERFACE exist in the registry, so, we change to search
		// the PenPaper Writing Service GUID directly.
		//--------------------------------------------------------------------------------------------------------------------------
		//while (SetupDiEnumDeviceInterfaces(hDevInfo, &DevInfoData, &GUID_BLUETOOTH_GATT_SERVICE_DEVICE_INTERFACE, InterfaceIndex++, &DeviceInterfaceData))
		while (SetupDiEnumDeviceInterfaces(hDevInfo, &DevInfoData, &PenPaperPenWritingServiceGuidLong, InterfaceIndex++, &DeviceInterfaceData))
		{
			//--------------------------------------
			// Get and allocate required buffer size 
			//--------------------------------------
			SetupDiGetInterfaceDeviceDetail(hDevInfo, &DeviceInterfaceData, NULL, 0, &dwRequiredSize, NULL);
			dwError = GetLastError();

			//------------------------------------------------------------------------------
			// If its not ERROR_INSUFFICIENT_BUFFER error, continue to enumerate next anyway.
			//------------------------------------------------------------------------------
			if (dwError != ERROR_INSUFFICIENT_BUFFER) continue;

			//---------------------------------
			// Get Interface Device Detail Data
			//---------------------------------
			pDeviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(dwRequiredSize);
			bufferSize = dwRequiredSize;
			pDeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			if (SetupDiGetInterfaceDeviceDetail(hDevInfo, &DeviceInterfaceData, pDeviceInterfaceDetailData, bufferSize, &dwRequiredSize, NULL))
			{
				//------------------------------------------
				// We are looking for the 0x0000ace0 Service
				//------------------------------------------
				pService = wcsstr(pDeviceInterfaceDetailData->DevicePath, L"0000ace0");

				if (pService != NULL)
				{
					//-------------------------------------------------------------------------
					// Check the driver provider, we only uninstall the driver from us (ACECAD)
					//-------------------------------------------------------------------------
					pProvider = NULL;
					SetupDiGetDeviceProperty(hDevInfo, &DevInfoData, &DEVPKEY_Device_DriverProvider, &ReturnedPropertyType, NULL, 0, &sizeRequired, 0);
					dwError = GetLastError();
					if (dwError == ERROR_INSUFFICIENT_BUFFER)
					{
						pwDriverProvider = (WCHAR *)malloc(sizeRequired);
						bufferSize = sizeRequired;

						if (SetupDiGetDeviceProperty(hDevInfo, &DevInfoData, &DEVPKEY_Device_DriverProvider, &ReturnedPropertyType, (PBYTE)pwDriverProvider, bufferSize, &sizeRequired, 0))
						{
							pProvider = wcsstr(pwDriverProvider, L"ACECAD");
						}

						free(pwDriverProvider);
						pwDriverProvider = NULL;
					}

					if (pProvider == NULL)
					{
						free(pDeviceInterfaceDetailData);
						pDeviceInterfaceDetailData = NULL;
						continue;
					}

					//------------------------------------------------------------------------------------
					// Get Inf file path so that we can uninstall the driver package in driver store later
					//------------------------------------------------------------------------------------
					SetupDiGetDeviceProperty(hDevInfo, &DevInfoData, &DEVPKEY_Device_DriverInfPath, &ReturnedPropertyType, NULL, 0, &sizeRequired, 0);
					dwError = GetLastError();
					if (dwError != ERROR_INSUFFICIENT_BUFFER) pwInfPath = NULL;
					else
					{
						pwInfPath = (WCHAR *)malloc(sizeRequired);
						bufferSize = sizeRequired;

						if (!SetupDiGetDeviceProperty(hDevInfo, &DevInfoData, &DEVPKEY_Device_DriverInfPath, &ReturnedPropertyType, (PBYTE)pwInfPath, bufferSize, &sizeRequired, 0))
						{
							free(pwInfPath);
							pwInfPath = NULL;
						}
					}

					if (!DiUninstallDevice(NULL, hDevInfo, &DevInfoData, 0, NULL))
					{
						free(pDeviceInterfaceDetailData);
						pDeviceInterfaceDetailData = NULL;
						if (pwInfPath != NULL) free(pwInfPath);
						continue;
					}

					//csString.Format(L"Device Unistalled successful: %ws", pDeviceInterfaceDetailData->DevicePath);
					//MessageBox(csString, L"PenPaper.Uninhstall", MB_OK);

					//------------------------------------------------------
					// Uninstall INF file and driver package in driver store
					//------------------------------------------------------
					if (pwInfPath != NULL)
					{
						SetupUninstallOEMInf(pwInfPath, SUOI_FORCEDELETE, NULL);

						free(pwInfPath);
						pwInfPath = NULL;
					}
				}
			}

			//-------------------------------
			// Free previous allocated buffer
			//-------------------------------
			free(pDeviceInterfaceDetailData);
			pDeviceInterfaceDetailData = NULL;
		}
	}

	//======================================================
	// Step 2: Remove the Control Panel and User Guide files
	//======================================================
	SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, 0, szTargetFolder);
	PathAppend(szTargetFolder, TEXT("ACECAD"));
	wcscpy_s(szTargetControlPanelFile, szTargetFolder);
	wcscpy_s(szTargetUserGuideFile, szTargetFolder);

	PathAppend(szTargetControlPanelFile, TEXT("PenPaperControlPanel.exe"));
	_wremove(szTargetControlPanelFile);

	PathAppend(szTargetUserGuideFile, TEXT("PenPaper Control Panel User Guide.pdf"));
	_wremove(szTargetUserGuideFile);

	// 2018.4.11, The folder will be remove in step 4
	//_wrmdir(szTargetFolder);

	//============================================================
	// Step 3: Remove the Shortcut files on Desktop and Start Menu
	//============================================================
	hRes = SHGetKnownFolderPath(FOLDERID_PublicDesktop, KF_FLAG_DEFAULT_PATH, (HANDLE)-1, (PWSTR *)&pszDesktopFolder);

	wcscpy_s(szDesktopFile, pszDesktopFolder);
	PathAppend(szDesktopFile, TEXT("PenPaper.lnk"));
	_wremove(szDesktopFile);

	wcscpy_s(szDesktopFile, pszDesktopFolder);
	PathAppend(szDesktopFile, TEXT("PenPaper Control Panel User Guide.lnk"));
	_wremove(szDesktopFile);

	hRes = SHGetKnownFolderPath(FOLDERID_CommonPrograms, KF_FLAG_DEFAULT_PATH, (HANDLE)-1, (PWSTR *)&pszStartMenuFolder);
	wcscpy_s(szStartMenuFolder, pszStartMenuFolder);
	PathAppend(szStartMenuFolder, TEXT("PenPaper"));

	wcscpy_s(szDesktopFile, szStartMenuFolder);
	PathAppend(szDesktopFile, TEXT("PenPaper.lnk"));
	_wremove(szDesktopFile);

	wcscpy_s(szDesktopFile, szStartMenuFolder);
	PathAppend(szDesktopFile, TEXT("PenPaper Control Panel User Guide.lnk"));
	_wremove(szDesktopFile);

	wcscpy_s(szDesktopFile, szStartMenuFolder);
	PathAppend(szDesktopFile, TEXT("Uninstall PenPaper.lnk"));
	_wremove(szDesktopFile);

	MessageBox(L"PenPaper HID minidriver uninstalled.", L"PenPaper.Uninhstall", MB_OK);

	//-------------------------------------------------
	// Step 4: Delete ourself (PenPaperUninstall32.exe)
	//-------------------------------------------------
	//---------------------------------
	// Get full path of our executable.
	//---------------------------------
	FilePart = NULL;
	SearchPathA(NULL, "PenPaperUninstall32.exe", NULL, MAX_PATH, szCurrentDir, FilePart);
	PathRemoveFileSpecA(szCurrentDir);
	PathStripPathA(szCurrentDir);	// "ACECAD"
	//WinExec("cmd.exe /C choice /C Y /N /D Y /T 5 & Del PenPaperUninstall32.exe & cd.. & rd Test", SW_SHOWDEFAULT);
	strcpy_s(szFinalCommand, "cmd.exe /C choice /C Y /N /D Y /T 6 & cd.. & rd /S /Q ");
	strcat_s(szFinalCommand, szCurrentDir);
	WinExec(szFinalCommand, SW_HIDE);

	EndDialog(IDCANCEL);
	return 0;
	//return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPenPaperUninstall32Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPenPaperUninstall32Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

