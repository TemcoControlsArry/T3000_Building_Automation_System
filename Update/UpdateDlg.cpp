
// UpdateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Update.h"
#include "UpdateDlg.h"
#include "afxdialogex.h"
#include "tlhelp32.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "..\\Update\\MFC16API.h"
#pragma comment(lib,"..\\Update\\MFC16API.lib")

bool run_t3000 = true;

HANDLE getftpthread = NULL;
HANDLE unzipthread = NULL;
HANDLE installthread = NULL;
DWORD WINAPI GetFtpFileThread(LPVOID lPvoid);
DWORD WINAPI UnzipFileThread(LPVOID lPvoid);
DWORD WINAPI InstallFileThread(LPVOID lPvoid);
CString CS_Info;
int m_static_step = UPDATE_STEP_IDLE;
CString APP_RUN_FOLDER;
CString DownloadIniFilePath;
CString DownloadFileFolder;
CString DesDownloadFilePath;
CString UnzipFileFolder;
CString T3000_ini_file_path;
int PC_T3000_Version = 0;
int T3000_FTP_Version = 0;
	int is_local_temco_net = false;
	int local_persent = 0;
//#pragma comment(lib,"..\\Debug\\MSVC10APIW.lib")

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CUpdateDlg dialog




CUpdateDlg::CUpdateDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CUpdateDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON_SW_UPDATE);
}

void CUpdateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_DOWNLOAD, m_static_download);
	DDX_Control(pDX, IDC_STATIC_UNCOMPRESS, m_static_uncompress);
	DDX_Control(pDX, IDC_STATIC_INSTALL, m_static_install);
	DDX_Control(pDX, IDC_STATIC_FINISHED, m_static_finish);
	DDX_Control(pDX, IDC_STATIC_PIC_STEP, m_static_pic_step);
	DDX_Control(pDX, IDC_STATIC_PACKAGE, m_static_persent);
	DDX_Control(pDX, IDC_PROGRESS_UPDATE, m_progress);
	DDX_Control(pDX, IDC_STATIC_CHECK_UPDATE, m_static_checkupdate);
	DDX_Control(pDX, IDC_STATIC_UPDATE_INFO, m_static_cs_info);
}

BEGIN_MESSAGE_MAP(CUpdateDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, &CUpdateDlg::OnBnClickedButtonCancel)
	ON_BN_CLICKED(IDC_CHECK_OPEN_T3000, &CUpdateDlg::OnBnClickedCheckOpenT3000)
END_MESSAGE_MAP()


bool CUpdateDlg::DeleteDirectory( CString DirName)
{
	CString PUBPATH;
	PUBPATH=DirName;
	CFileFind tempFind;
	DirName+="\\*.*";
	BOOL IsFinded=(BOOL)tempFind.FindFile(DirName);
	while(IsFinded)
	{
		IsFinded=(BOOL)tempFind.FindNextFile();
		if(!tempFind.IsDots())
		{
			CString strDirName;
			strDirName+=PUBPATH;
			strDirName+="\\";
			strDirName+=tempFind.GetFileName();
			//AfxMessageBox("strDirName :"+strDirName);
			if(tempFind.IsDirectory())
			{
				//strDirName += PUBPATH;
				DeleteDirectory(strDirName);
			}
			else
			{
				SetFileAttributes(strDirName,FILE_ATTRIBUTE_NORMAL); //ȥ���ļ���ϵͳ����������
				DeleteFile(strDirName);
			}
		}
	}
	tempFind.Close();
	if(!RemoveDirectory(PUBPATH))
	{
		return false ;
	}
	return true;
}
// CUpdateDlg message handlers

BOOL CUpdateDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	CString m_strVersion;
	m_strVersion.LoadString(IDS_UPDATE_VERSION);
	SetWindowTextW(m_strVersion);

	//APP_RUN_FOLDER
	CString tempFilePath;
	CString tempApplicationFolder;
	GetModuleFileName(NULL, tempApplicationFolder.GetBuffer(MAX_PATH), MAX_PATH);
	PathRemoveFileSpec(tempApplicationFolder.GetBuffer(MAX_PATH));
	tempApplicationFolder.ReleaseBuffer();
	APP_RUN_FOLDER = tempApplicationFolder;

	T3000_ini_file_path = APP_RUN_FOLDER + _T("\\Database") + _T("\\") + _T("temp\\MonitorIndex.ini") ;
	PC_T3000_Version = GetPrivateProfileInt(_T("Version"),_T("T3000"),0,T3000_ini_file_path);


	is_local_temco_net  = GetPrivateProfileInt(_T("Setting"),_T("LocalTemcoNet"),0,T3000_ini_file_path);


	DownloadFileFolder = APP_RUN_FOLDER + _T("\\Database");
	CreateDirectory(DownloadFileFolder,NULL);
	DownloadFileFolder = DownloadFileFolder + _T("\\Update");
		CreateDirectory(DownloadFileFolder,NULL);
	UnzipFileFolder = DownloadFileFolder + _T("\\UNZIP");
	DeleteDirectory(UnzipFileFolder);
	CreateDirectory(UnzipFileFolder,NULL);
	//int a = 123123; int b = 11;
	//int c = 0;
	//c = Test_Add(a,b);
	SetTimer(1,300,NULL);
	CS_Info.Format(_T(""));

	InitialTemcoLogo();

	if(getftpthread == NULL)
	{
	 m_static_step = UPDATE_STEP_CHECK_NEWVERSION;
		getftpthread = CreateThread(NULL,NULL,GetFtpFileThread,this,NULL,NULL);
		CloseHandle(getftpthread);
	}


	InitialStatic();


	((CButton *)GetDlgItem(IDC_CHECK_OPEN_T3000))->SetCheck(true);


	Sleep(1);
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CUpdateDlg::InitialTemcoLogo()
{
	hBitmap =(HBITMAP)LoadImage(AfxGetInstanceHandle(),  
		MAKEINTRESOURCE(IDB_BITMAP_T3000_UPDATE),  
		IMAGE_BITMAP,0,0,  
		LR_LOADMAP3DCOLORS);  

	CStatic *pStatic=(CStatic *)GetDlgItem(IDC_STATIC_T3000_UPDATE); //���ָ��̬�ؼ���ָ�� 
	pStatic->ModifyStyle(0xF,SS_BITMAP/*|SS_CENTERIMAGE*/);  //���þ�̬�ؼ�����ʽ��ʹ��λͼ����  ;
	pStatic->SetBitmap(hBitmap);     //���þ�̬�ؼ���ʾλͼ 
}

void CUpdateDlg::InitialStatic()
{
	
	m_static_cs_info.SetWindowTextW(_T(""));
	m_static_cs_info.textColor(RGB(0,0,0));
	//m_static_cs_info.bkColor(RGB(255,255,255));
	m_static_cs_info.setFont(20,15,NULL,_T("Times New Roman"));

	m_static_checkupdate.SetWindowTextW(_T("Check Update"));
	m_static_checkupdate.textColor(RGB(0,0,0));
	//m_static_download.bkColor(RGB(255,255,255));
	m_static_checkupdate.setFont(20,15,NULL,_T("Times New Roman"));

	m_static_download.SetWindowTextW(_T("Download"));
	m_static_download.textColor(RGB(0,0,0));
	//m_static_download.bkColor(RGB(255,255,255));
	m_static_download.setFont(20,15,NULL,_T("Times New Roman"));



	m_static_uncompress.SetWindowTextW(_T("Extracting"));
	m_static_uncompress.textColor(RGB(0,0,0));
	//m_static_uncompress.bkColor(RGB(255,255,255));
	m_static_uncompress.setFont(20,15,NULL,_T("Times New Roman"));

	m_static_install.SetWindowTextW(_T("Install"));
	m_static_install.textColor(RGB(0,0,0));
	//m_static_install.bkColor(RGB(255,255,255));
	m_static_install.setFont(20,15,NULL,_T("Times New Roman"));

	m_static_finish.SetWindowTextW(_T("Done"));
	m_static_finish.textColor(RGB(0,0,0));
	//m_static_finish.bkColor(RGB(255,255,255));
	m_static_finish.setFont(20,15,NULL,_T("Times New Roman"));


	
	m_static_persent.SetWindowTextW(_T(""));
	m_static_persent.textColor(RGB(0,0,0));
	//m_static_finish.bkColor(RGB(255,255,255));
	m_static_persent.setFont(20,15,NULL,_T("Times New Roman"));
	//m_static_pic_step.SetWindowPos( NULL,252,165 + 45*(input_step-1),0,0,SWP_NOZORDER | SWP_NOSIZE );    
}


bool CopyDirW(CString strSrcPath, CString strDstPath,bool bFailIfExists)
{ 
	CreateDirectory(strDstPath,NULL);// ����Ŀ���ļ��У���������򲻴���������������򴴽���
	if(PathIsDirectory(strDstPath)&&PathFileExists(strSrcPath))//Ŀ��Ŀ¼�Ƿ���� ����strSrcPath Ҫô���ļ���Ҫô���ļ�
	{
		if(::PathIsDirectory(strSrcPath))//������ļ��п���
		{
			CFileFind finder;
			// ��ָ�����ļ��н�������
			BOOL bWorking = finder.FindFile(strSrcPath + _T("\\") + _T("*.*")); 
			while(bWorking)
			{
				// �ӵ�ǰĿ¼�����ļ�
				bWorking = finder.FindNextFile();
				CString strFileName = finder.GetFileName();
				CString strSrc = strSrcPath + _T("\\") + strFileName;
				CString strDst = strDstPath + _T("\\") + strFileName;
				// �ж����������ǲ���"."��".."Ŀ¼
				if(!finder.IsDots())
				{
					// �ж���������Ŀ¼�Ƿ����ļ���
					if(finder.IsDirectory())
					{ // ������ļ��еĻ������еݹ�
						if(!CopyDirW(strSrc, strDst,bFailIfExists)) 
							return false;
					}
					else
					{// ������ļ������и���
						if(!CopyFileW(strSrc, strDst, bFailIfExists))
							return false;
					}
				}
			}       
		}
		else//������ļ�����
		{
			strDstPath+=_T("\\");
			strDstPath+=PathFindFileName(strSrcPath);
			if(!CopyFile(strSrcPath, strDstPath, bFailIfExists))
				return false;
		}
	}
	else//Ŀ��·��������
		return false;
	return true;


}


DWORD WINAPI InstallFileThread(LPVOID lPvoid)
{
	CUpdateDlg * mparent = (CUpdateDlg *)lPvoid;

	bool copy_ret = false;
	SetCurrentDirectoryW(APP_RUN_FOLDER);
	copy_ret = CopyDirW(UnzipFileFolder,APP_RUN_FOLDER,FALSE);
	if(copy_ret)
	{
		CS_Info.Format(_T("Install success!"));
		
	}
	else
	{
		CS_Info.Format(_T("Install failed!"));
	}
	installthread = NULL;
	m_static_step = UPDATE_STEP_DONE;
	return 0;
}

DWORD WINAPI UnzipFileThread(LPVOID lPvoid)
{
	CUpdateDlg * mparent = (CUpdateDlg *)lPvoid;
	CS_Info.Format(_T("Extracting files"));
	UnzipItem(DesDownloadFilePath,UnzipFileFolder);
	CS_Info.Format(_T("Extracting files OK!"));

	

	if(installthread == NULL)
	{
		m_static_step = UPDATE_STEP_INSTALL;
		Sleep(1000);
		installthread = CreateThread(NULL,NULL,InstallFileThread,mparent,NULL,NULL);
		CloseHandle(installthread);
	}
		unzipthread = NULL;
	return 0;
}


BOOL KillProcessFromName(CString strProcessName)
{
	//�������̿���(TH32CS_SNAPPROCESS��ʾ�������н��̵Ŀ���)

	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);



	//PROCESSENTRY32���̿��յĽṹ��

	PROCESSENTRY32 pe;



	//ʵ������ʹ��Process32First��ȡ��һ�����յĽ���ǰ�����ĳ�ʼ������

	pe.dwSize = sizeof(PROCESSENTRY32);





	//�����IFЧ��ͬ:

	//if(hProcessSnap == INVALID_HANDLE_VALUE)   ��Ч�ľ��

	if(!Process32First(hSnapShot,&pe))

	{

		return FALSE;

	}



	//���ַ���ת��ΪСд

	strProcessName.MakeLower();



	//��������Ч  ��һֱ��ȡ��һ�����ѭ����ȥ

	while (Process32Next(hSnapShot,&pe))

	{



		//pe.szExeFile��ȡ��ǰ���̵Ŀ�ִ���ļ�����

		CString scTmp = pe.szExeFile;





		//����ִ���ļ���������Ӣ����ĸ�޸�ΪСд

		scTmp.MakeLower();



		//�Ƚϵ�ǰ���̵Ŀ�ִ���ļ����ƺʹ��ݽ������ļ������Ƿ���ͬ

		//��ͬ�Ļ�Compare����0

		if(!scTmp.Compare(strProcessName))

		{



			//�ӿ��ս����л�ȡ�ý��̵�PID(������������е�PID)

			DWORD dwProcessID = pe.th32ProcessID;

			HANDLE hProcess = ::OpenProcess(PROCESS_TERMINATE,FALSE,dwProcessID);

			::TerminateProcess(hProcess,0);

			CloseHandle(hProcess);

			return TRUE;

		}

		scTmp.ReleaseBuffer();

	}

	strProcessName.ReleaseBuffer();

	return FALSE;

}

DWORD WINAPI GetFtpFileThread(LPVOID lPvoid)
{

	CUpdateDlg * mparent = (CUpdateDlg *)lPvoid;


	if(is_local_temco_net == FALSE)
	{
		CS_Info.Format(_T("Connecting to server...."));
		bool conncect_ret = ConnectToFtp(_T("server139.web-hosting.com"),_T("temcoftp@temcocontrols.com"),_T("BwpNXWPgE7hg"));
		if(conncect_ret == false)
		{
			CS_Info.Format(_T("Connect to server failed.Please check your network."));
			Sleep(3000);
			m_static_step = UPDATE_STEP_READY_TO_CLOASE;
			getftpthread = NULL;
			return 0;
		}
		CS_Info.Format(_T("Checking for updates ...."));
		bool download_ret = false;
		download_ret = DownloadFileFromFtp(_T("//software//T3000Version.ini") ,DownloadFileFolder);
		if(download_ret == false)
		{
			CS_Info.Format(_T("Check version  from FTP Server failed."));
			Sleep(2000);
			DisconnectFtp();
			getftpthread = NULL;
			m_static_step = UPDATE_STEP_READY_TO_CLOASE;
			return 0;
		}
		DownloadIniFilePath = DownloadFileFolder + _T("//T3000Version.ini");
		T3000_FTP_Version = GetPrivateProfileIntW(_T("Version"),_T("T3000Version"),0,DownloadIniFilePath);
		if(T3000_FTP_Version == 0)
		{
			CS_Info.Format(_T("Connect to server failded!"));
			m_static_step = UPDATE_STEP_READY_TO_CLOASE;
			getftpthread = NULL;
			return 0;
		}


		if((PC_T3000_Version < T3000_FTP_Version) || (PC_T3000_Version == 0 ))
		{
			CS_Info.Format(_T("Find new version , ready to update."));
		}
		else
		{
			CS_Info.Format(_T("Your T3000.exe is up-to-date"));
			m_static_step = UPDATE_STEP_READY_TO_CLOASE;
			getftpthread = NULL;
			return 0;
		}
	}

	KillProcessFromName(_T("T3000.exe")) ;
	KillProcessFromName(_T("ISP.exe")) ;
	m_static_step = UPDATE_STEP_DOWNLOAD;

	if(is_local_temco_net == FALSE)
	{
		bool download_ret = false;
		download_ret = DownloadFileFromFtp(_T("//software//T3000Update.zip") ,DownloadFileFolder);

		if(download_ret)
			CS_Info.Format(_T("Download finished."));
		else
		{
			//bool http_download_ret;
			//CS_Info.Format(_T("Trying download from http server."));
			//http_download_ret = mparent->DownloadFileHttp(_T("http://www.temcocontrols.com/ftp/software/T3000Update.zip"),DownloadFileFolder + _T("\\T3000Update.zip"));
			//

			CS_Info.Format(_T("Download from FTP Server failed."));
			Sleep(2000);
			DisconnectFtp();
			getftpthread = NULL;
			 m_static_step = UPDATE_STEP_READY_TO_CLOASE;
			return 0;
		}
		DisconnectFtp();
		CS_Info.Format(_T("Disconnect with server."));
		DesDownloadFilePath = DownloadFileFolder + _T("\\T3000Update.zip");
	}
	else
	{
		bool copy_ret = false;
		CString temp123;
		temp123 = DownloadFileFolder + _T("\\T3000Update.zip");
		DesDownloadFilePath = temp123;
		copy_ret =  CopyFileW(_T("Z:\\TemcoSoftware\\Release\\T3000ForInstallation\\T3000Update.zip"),temp123,FALSE);
		//copy_ret = CopyDirW(_T("Z:\\TemcoSoftware\\Release\\T3000ForInstallation\\T3000Update.zip"),DownloadFileFolder,FALSE);
		if(copy_ret)
		{
			for (int j=1 ;j<=10;j++)
			{
				local_persent = j*10;
				CS_Info.Format(_T("Downloading (%d%%)"),local_persent);
				Sleep(800);
			}
			CS_Info.Format(_T("Install success!"));

		}
		else
		{
			CS_Info.Format(_T("Install failed!"));
			return 0;
		}
	}


	//����Ҫ����Ƿ��������;


download_pass:

	if(unzipthread == NULL)
	{
	 m_static_step = UPDATE_STEP_UNCOMPRESS;
		unzipthread = CreateThread(NULL,NULL,UnzipFileThread,mparent,NULL,NULL);
		CloseHandle(unzipthread);
	}


	getftpthread = NULL;
	return 0 ;
}

void CUpdateDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CUpdateDlg::OnPaint()
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
HCURSOR CUpdateDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CUpdateDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	CString temp_cs;
static	int pic_static_step = 0;

	switch(m_static_step)
	{
	case UPDATE_STEP_IDLE :
		{
			CS_Info.Empty();
			m_static_persent.ShowWindow(FALSE);
			m_progress.ShowWindow(FALSE);
		}
		break;
	case UPDATE_STEP_CHECK_NEWVERSION:
		{
			if(pic_static_step == 1)
			{
				pic_static_step = 0;
			}
			else
			{
				pic_static_step = 1;
			}
			m_static_pic_step.SetWindowPos( NULL,10 + 18*(1- pic_static_step),12 ,0,0,SWP_NOZORDER | SWP_NOSIZE );   
		}
		break;
	case UPDATE_STEP_DOWNLOAD :
		{
			m_static_checkupdate.textColor(RGB(0,0,255));
			if(pic_static_step == 1)
			{
				pic_static_step = 0;
			}
			else
			{
				pic_static_step = 1;
			}
			m_static_pic_step.SetWindowPos( NULL,10 + 18*(1- pic_static_step),42 ,0,0,SWP_NOZORDER | SWP_NOSIZE );    
			
			m_static_persent.ShowWindow(TRUE);
			m_progress.ShowWindow(TRUE);
			unsigned int total_byte = 0;
			unsigned int finished_bye = 0;

			int finished_persent = 0;
			if(is_local_temco_net == false)
			{
				GetFtpTransferPersent(&total_byte,&finished_bye,&finished_persent);
				temp_cs.Format(_T("%d/%d (Kb)   %d%%"),finished_bye/1024,total_byte/1024,finished_persent);
				m_static_persent.SetWindowTextW(temp_cs);
				m_progress.SetPos(finished_persent);
			}
			else
			{
				
				temp_cs.Format(_T("%d%%"),local_persent);
				m_static_persent.SetWindowTextW(temp_cs);
				m_progress.SetPos(local_persent);
			}
			

		}
		break;
	case UPDATE_STEP_UNCOMPRESS:
		{
			m_static_download.textColor(RGB(0,0,255));
			if(pic_static_step == 1)
			{
				pic_static_step = 0;
			}
			else
			{
				pic_static_step = 1;
			}
			m_static_pic_step.SetWindowPos( NULL,10 + 18*(1- pic_static_step),90 ,0,0,SWP_NOZORDER | SWP_NOSIZE );    


			temp_cs.Format(_T(""));
			m_static_persent.SetWindowTextW(temp_cs);
		}
		break;
	case UPDATE_STEP_INSTALL:
		{
			m_static_download.textColor(RGB(0,0,255));
			m_static_uncompress.textColor(RGB(0,0,255));
			if(pic_static_step == 1)
			{
				pic_static_step = 0;
			}
			else
			{
				pic_static_step = 1;
			}
			m_static_pic_step.SetWindowPos( NULL,10 + 18*(1- pic_static_step),138 ,0,0,SWP_NOZORDER | SWP_NOSIZE );   
			CS_Info.Format(_T("Recover new files ,please wait."));

		}
		break;
	case UPDATE_STEP_DONE:
		{
			m_static_install.textColor(RGB(0,0,255));
			m_static_finish.textColor(RGB(0,0,255));
			pic_static_step = 0;

			m_static_pic_step.SetWindowPos( NULL,10 + 14*(1- pic_static_step),185 ,0,0,SWP_NOZORDER | SWP_NOSIZE );   
			CS_Info.Format(_T("Update done."));
			m_static_step = UPDATE_STEP_READY_TO_CLOASE;
		}
		break;
	case UPDATE_STEP_READY_TO_CLOASE:
		{
			m_static_persent.ShowWindow(FALSE);
			m_progress.ShowWindow(FALSE);
			Invalidate(TRUE);
			static int ncount = 0;
			if(++ncount > 10)
			{

				CString tempApplicationFolder;
				GetModuleFileName(NULL, tempApplicationFolder.GetBuffer(MAX_PATH), MAX_PATH);
				PathRemoveFileSpec(tempApplicationFolder.GetBuffer(MAX_PATH));
				tempApplicationFolder.ReleaseBuffer();

				if(run_t3000)
					ShellExecute(NULL,_T("open"),_T("T3000.exe"),NULL,tempApplicationFolder,SW_SHOWNORMAL);

				PostMessage(WM_CLOSE,NULL,NULL);
			}
		}
		break;
	default:
		break;

	}
	GetDlgItem(IDC_STATIC_UPDATE_INFO)->SetWindowTextW(CS_Info);
	CDialogEx::OnTimer(nIDEvent);
}


void CUpdateDlg::OnBnClickedButtonCancel()
{
	// TODO: Add your control notification handler code here
	if(getftpthread)
	{
		TerminateThread(getftpthread,0);
		getftpthread = NULL;
	}
	if(unzipthread)
	{
		TerminateThread(unzipthread,0);
		unzipthread = NULL;
	}
	if(installthread)
	{
		TerminateThread(installthread,0);
		installthread = NULL;
	}
	PostMessage(WM_CLOSE,NULL,NULL);
}


BOOL CUpdateDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message==WM_KEYDOWN && pMsg->wParam==VK_RETURN) 
	{
		return 1;
	}
	else if(pMsg->message==WM_KEYDOWN && pMsg->wParam==VK_ESCAPE) 
	{
		return 1;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

BOOL CUpdateDlg::DownloadFileHttp(const CString& strFileURLInServer, //�������ļ���URL
	const CString & strFileLocalFullPath)//��ŵ����ص�·��
{
	ASSERT(strFileURLInServer != "");
	ASSERT(strFileLocalFullPath != "");
	CInternetSession session;
	CHttpConnection* pHttpConnection = NULL;
	CHttpFile* pHttpFile = NULL;
	CString strServer, strObject;
	INTERNET_PORT wPort;

	DWORD dwType;
	const int nTimeOut = 2000;
	session.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, nTimeOut); //����֮��ĵȴ���ʱ
	session.SetOption(INTERNET_OPTION_CONNECT_RETRIES, 1);   //���Դ���
	char* pszBuffer = NULL;  

	try
	{
		AfxParseURL(strFileURLInServer, dwType, strServer, strObject, wPort);
		pHttpConnection = session.GetHttpConnection(strServer, wPort);
		pHttpFile = pHttpConnection->OpenRequest(CHttpConnection::HTTP_VERB_GET, strObject);
		if(pHttpFile->SendRequest() == FALSE)
			return false;
		DWORD dwStateCode;

		pHttpFile->QueryInfoStatusCode(dwStateCode);
		if(dwStateCode == HTTP_STATUS_OK)
		{
			HANDLE hFile = CreateFile(strFileLocalFullPath, GENERIC_WRITE,
				FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
				NULL);  //���������ļ�
			if(hFile == INVALID_HANDLE_VALUE)
			{
				pHttpFile->Close();
				pHttpConnection->Close();
				session.Close();
				return false;
			}

			char szInfoBuffer[1000];  //������Ϣ
			DWORD dwFileSize = 0;   //�ļ�����
			DWORD dwInfoBufferSize = sizeof(szInfoBuffer);
			BOOL bResult = FALSE;
			bResult = pHttpFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH,
				(void*)szInfoBuffer,&dwInfoBufferSize,NULL);

			dwFileSize = atoi(szInfoBuffer);
			const int BUFFER_LENGTH = 1024 * 10;
			pszBuffer = new char[BUFFER_LENGTH];  //��ȡ�ļ��Ļ���
			DWORD dwWrite, dwTotalWrite;
			dwWrite = dwTotalWrite = 0;
			UINT nRead = pHttpFile->Read(pszBuffer, BUFFER_LENGTH); //��ȡ������������

			while(nRead > 0)
			{
				WriteFile(hFile, pszBuffer, nRead, &dwWrite, NULL);  //д�������ļ�
				dwTotalWrite += dwWrite;
				nRead = pHttpFile->Read(pszBuffer, BUFFER_LENGTH);
			}

			delete[]pszBuffer;
			pszBuffer = NULL;
			CloseHandle(hFile);
		}
		else
		{
			delete[]pszBuffer;
			pszBuffer = NULL;
			if(pHttpFile != NULL)
			{
				pHttpFile->Close();
				delete pHttpFile;
				pHttpFile = NULL;
			}
			if(pHttpConnection != NULL)
			{
				pHttpConnection->Close();
				delete pHttpConnection;
				pHttpConnection = NULL;
			}
			session.Close();
			return false;
		}
	}
	catch(...)
	{
		delete[]pszBuffer;
		pszBuffer = NULL;
		if(pHttpFile != NULL)
		{
			pHttpFile->Close();
			delete pHttpFile;
			pHttpFile = NULL;
		}
		if(pHttpConnection != NULL)
		{
			pHttpConnection->Close();
			delete pHttpConnection;
			pHttpConnection = NULL;
		}
		session.Close();
		return false;
	}

	if(pHttpFile != NULL)
		pHttpFile->Close();
	if(pHttpConnection != NULL)
		pHttpConnection->Close();
	session.Close();
	return true;
}





void CUpdateDlg::OnBnClickedCheckOpenT3000()
{
	// TODO: Add your control notification handler code here
	run_t3000 =  ((CButton *)GetDlgItem(IDC_CHECK_OPEN_T3000))->GetCheck();
}