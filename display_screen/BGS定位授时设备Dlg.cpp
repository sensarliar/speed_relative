// BGS定位授时设备Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "BGS定位授时设备.h"
#include "BGS定位授时设备Dlg.h"
#include "math.h"
#include "calc_rel_pos.h"

#include <time.h>
#include <list>
#include <iostream>
#include <numeric>
#include <algorithm>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



//struct GpsState gps;

//extern struct GpsState gps;
/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

#define HELLO_PORT  31241  
#define HELLO_GROUP "224.1.2.3"  
//#define MSGBUFSIZE 256

extern double POS_TAIL2ANT_JIAYOU_X;
extern double POS_TAIL2ANT_JIAYOU_Y;
extern double POS_TAIL2ANT_JIAYOU_Z;
extern double POS_HEAD2ANT_SHOUYOU_X;
extern double POS_HEAD2ANT_SHOUYOU_Y;
extern double POS_HEAD2ANT_SHOUYOU_Z;


SOCKET m_socket,rcvsock;
sockaddr_in local,remote;
int remotelength=0;
int saveFlag = 0;

CString data;

CString  CfgFilePath="";

CString  StrFilePath="";
char *file_name_p = NULL;

unsigned long g_softstate=0;

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBGSDlg dialog

CBGSDlg::CBGSDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBGSDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBGSDlg)
	//state = _T("");
	sockInitFlag = false;
	//m_dingwei = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CBGSDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBGSDlg)
	//X_Text(pDX, IDC_STATE2, m_state);
	//DDX_Text(pDX, IDC_DINGWEI, m_dingwei);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CBGSDlg, CDialog)
	//{{AFX_MSG_MAP(CBGSDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_SOCKET,OnSocket)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBGSDlg message handlers

BOOL CBGSDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	
	int nScree_x = GetSystemMetrics(SM_CXSCREEN);//屏幕宽度
	int nScree_y = GetSystemMetrics(SM_CYSCREEN);//屏幕高度

	//::SetWindowPos(m_hWnd, NULL, -30, -30, nScree_x+40, nScree_y+40, NULL); 
	::SetWindowPos(m_hWnd, NULL, -10, -10, nScree_x+40, nScree_y+40, NULL);



	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//StrFilePath+=".\\保存文件.txt";//创建文件

	CString str_ttime;
	//获取系统时间
	CTime tm;
	tm=CTime::GetCurrentTime();//获取系统日期
//	str_ttime=tm.Format("%Y-%m-%d-%X");
	str_ttime=tm.Format("%Y-%m-%d--%H-%M-%S");


	StrFilePath+=".\\speed_msg";//创建文件
	StrFilePath+= str_ttime;
	StrFilePath+=".txt";

	file_name_p =(LPSTR)(LPCTSTR)StrFilePath;

	ReadINI();// Set small icon
	
	
	// TODO: Add extra initialization here

	//m_b.SetFont(&m_font1);
	m_font.CreatePointFont(100,"Times New Roman");
	m_font1.CreatePointFont(360," 黑体加粗");
	m_font2.CreatePointFont(260," 宋体");
	m_font3.CreatePointFont(350," 粗体");
	m_font4.CreatePointFont(200," 宋体");

	m_bkBrush.CreateSolidBrush(RGB(0,0,0));//黑色画刷

	//初始化为0
	SetDlgItemText(IDC_HOUR,"00");
	SetDlgItemText(IDC_MIN,"00");
	SetDlgItemText(IDC_SEC,"00.00");
	SetDlgItemText(IDC_XIEJU,"0");
	SetDlgItemText(IDC_SPEED,"0");
	SetDlgItemText(IDC_x,"0");
	SetDlgItemText(IDC_y,"0");
	SetDlgItemText(IDC_z,"0");
	SetDlgItemText(IDC_Vx,"0");
	SetDlgItemText(IDC_Vy,"0");
	SetDlgItemText(IDC_Vz,"0");
	SetDlgItemText(IDC_STATE2,"未锁定");
	SetDlgItemText(IDC_ANGLE,"0");

	SetDlgItemText(IDC_Vx2,"0");
	SetDlgItemText(IDC_Vy2,"0");
	SetDlgItemText(IDC_Vz2,"0");

//////////////////////////////////////////

	b2_hff_init(0, 0, 0, 0);
	/////////////////////////////////////

	
	sockInitFlag = InitSocket();
	
	if (!sockInitFlag)
	{
		
		SetTimer(2,1000,NULL);
	}

	
	return TRUE;  // return TRUE  unless you set the focus to a control

	
}

void CBGSDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CBGSDlg::OnPaint() 
{


	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CBGSDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}



HBRUSH CBGSDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	

	if(pWnd->GetDlgCtrlID()==IDC_TIME||IDC_STATIC_SPEED)
	{
		pDC->SelectObject(&m_font2);
		pDC->SetTextColor(RGB(255,255,255));//绿色
		pDC->SetBkMode(TRANSPARENT);
		//m_font2.Detach();

		//return m_bkBrush;
	}
	
	if(pWnd->GetDlgCtrlID()==IDC_HOUR)
	{
		pDC->SelectObject(&m_font2);
		pDC->SetTextColor(RGB(0,255,0));//绿色
		pDC->SetBkMode(TRANSPARENT);
		//m_font2.Detach();

		//return m_bkBrush;
	}
	
	if(pWnd->GetDlgCtrlID()==IDC_MIN)
	{
		pDC->SelectObject(&m_font2);
		pDC->SetTextColor(RGB(0,255,0));//绿色
		pDC->SetBkMode(TRANSPARENT);
		//m_font2.Detach();

		//return m_bkBrush;
	}

	if(pWnd->GetDlgCtrlID()==IDC_SEC)
	{
		pDC->SelectObject(&m_font2);
		pDC->SetTextColor(RGB(0,255,0));//绿色
		pDC->SetBkMode(TRANSPARENT);
		//m_font2.Detach();

		//return m_bkBrush;
	}


	if(pWnd->GetDlgCtrlID()==IDC_SPEED)
	{
		pDC->SelectObject(&m_font2);
		pDC->SetTextColor(RGB(0,255,0));//绿色
		pDC->SetBkMode(TRANSPARENT);
		//m_font2.Detach();

		//return m_bkBrush;
	}

	if(pWnd->GetDlgCtrlID()==IDC_XIEJU)
	{
		pDC->SelectObject(&m_font2);
		pDC->SetTextColor(RGB(0,255,0));//绿色
		pDC->SetBkMode(TRANSPARENT);
		//m_font2.Detach();

		//return m_bkBrush;
	}
/*	
		if(pWnd->GetDlgCtrlID()==IDC_SPEED2)
	{
		pDC->SelectObject(&m_font2);
		pDC->SetTextColor(RGB(0,255,0));//绿色
		pDC->SetBkMode(TRANSPARENT);
		//m_font2.Detach();

		//return m_bkBrush;
	}
*/
	if(pWnd->GetDlgCtrlID()==IDC_x)
	{
		pDC->SelectObject(&m_font2);
		pDC->SetTextColor(RGB(0,255,0));//绿色
		pDC->SetBkMode(TRANSPARENT);
		//m_font2.Detach();

		//return m_bkBrush;
	}
	
	if(pWnd->GetDlgCtrlID()==IDC_y)
	{
		pDC->SelectObject(&m_font2);
		pDC->SetTextColor(RGB(0,255,0));//绿色
		pDC->SetBkMode(TRANSPARENT);
		//m_font2.Detach();

		//return m_bkBrush;
	}

	if(pWnd->GetDlgCtrlID()==IDC_z)
	{
		pDC->SelectObject(&m_font2);
		pDC->SetTextColor(RGB(0,255,0));//绿色
		pDC->SetBkMode(TRANSPARENT);
		//m_font2.Detach();

		//return m_bkBrush;
	}

	if(pWnd->GetDlgCtrlID()==IDC_Vx)
	{
		pDC->SelectObject(&m_font2);
		pDC->SetTextColor(RGB(0,255,0));//绿色
		pDC->SetBkMode(TRANSPARENT);
		//m_font2.Detach();

		//return m_bkBrush;
	}

	if(pWnd->GetDlgCtrlID()==IDC_Vy)
	{
		pDC->SelectObject(&m_font2);
		pDC->SetTextColor(RGB(0,255,0));//绿色
		pDC->SetBkMode(TRANSPARENT);
		//m_font2.Detach();

		//return m_bkBrush;
	}

	
	if(pWnd->GetDlgCtrlID()==IDC_Vz)
	{
		pDC->SelectObject(&m_font2);
		pDC->SetTextColor(RGB(0,255,0));//绿色
		pDC->SetBkMode(TRANSPARENT);
		//m_font2.Detach();

		//return m_bkBrush;
	}
/*

	if(pWnd->GetDlgCtrlID()==IDC_Vx2)
	{
		pDC->SelectObject(&m_font2);
		pDC->SetTextColor(RGB(0,255,0));//绿色
		pDC->SetBkMode(TRANSPARENT);
		//m_font2.Detach();

		//return m_bkBrush;
	}

	if(pWnd->GetDlgCtrlID()==IDC_Vy2)
	{
		pDC->SelectObject(&m_font2);
		pDC->SetTextColor(RGB(0,255,0));//绿色
		pDC->SetBkMode(TRANSPARENT);
		//m_font2.Detach();

		//return m_bkBrush;
	}

	
	if(pWnd->GetDlgCtrlID()==IDC_Vz2)
	{
		pDC->SelectObject(&m_font2);
		pDC->SetTextColor(RGB(0,255,0));//绿色
		pDC->SetBkMode(TRANSPARENT);
		//m_font2.Detach();

		//return m_bkBrush;
	}


		if(pWnd->GetDlgCtrlID()==IDC_ANGLE)
	{
		pDC->SelectObject(&m_font2);
		pDC->SetTextColor(RGB(0,255,0));//绿色
		pDC->SetBkMode(TRANSPARENT);
		//m_font2.Detach();

		//return m_bkBrush;
	}

  */
	
	if(pWnd->GetDlgCtrlID()==IDC_STATIC)
	{
		pDC->SelectObject(&m_font1);
		pDC->SetTextColor(RGB(255,255,255));//白色
		pDC->SetBkMode(TRANSPARENT);
		//m_font1.Detach();

		//return m_bkBrush;
	}


	if(pWnd->GetDlgCtrlID()==IDC_XIEJU1)
	{


		pDC->SelectObject(&m_font3);
		pDC->SetTextColor(RGB(255,255,255));//白色
		pDC->SetBkMode(TRANSPARENT);
		//pDC->SetBkColor(RGB(0,0,0));

		 return m_bkBrush;

	}

	if(pWnd->GetDlgCtrlID()==IDC_S)
	{
		pDC->SelectObject(&m_font4);
		pDC->SetTextColor(RGB(0,255,0));//绿色
		pDC->SetBkMode(TRANSPARENT);
		

		//return m_bkBrush;
	}

	if(pWnd->GetDlgCtrlID()==IDC_STATE2)
	{
		pDC->SelectObject(&m_font4);
	
		CString str ;
		GetDlgItem(IDC_DINGWEI)->GetWindowText(str);
		if(str==_T("未锁定"))
		{
			pDC->SetTextColor(RGB(238,99,99));//红色
		}

	
		else if(str==_T("单点锁定"))
		{
			pDC->SetTextColor(RGB(0,255,255));//青色
		}


		else if(str==_T("相对位置锁定"))
		{
			pDC->SetTextColor(RGB(0,255,0));//绿色
		}

		pDC->SetBkMode(TRANSPARENT);
		
	}
	
	
	// TODO: Return a different brush if the default is not desired
	//return hbr;
	return m_bkBrush;
}


BOOL CBGSDlg::InitSocket()
{

	WSADATA  wsa  ;
	
	::WSAStartup(MAKEWORD(2,0),&wsa) ;	 //WSAStartup函数加载套接字库

	//创建套接字
	
	local.sin_family=AF_INET;//接收端
	local.sin_addr.s_addr=0;
	local.sin_port=htons(Port);	
	
	remote.sin_family = AF_INET; //发送端
	remote.sin_port = htons(INADDR_ANY); 
	remote.sin_addr.s_addr = inet_addr(Ip);
	
	remotelength=sizeof(SOCKADDR);
	if((m_socket=WSASocket(AF_INET,SOCK_DGRAM,0,NULL,0, WSA_FLAG_MULTIPOINT_C_LEAF|WSA_FLAG_MULTIPOINT_D_LEAF| WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) 

	{ 
		AfxMessageBox("socket create failed"); 
		//perror("socket create failed");
		WSACleanup();
		//exit(1);
	}
	bool l = TRUE;
	setsockopt(m_socket,SOL_SOCKET,SO_REUSEADDR,(char *)&l,sizeof(l)); //重用端口设置
	
	
	//设置该套接字缓冲区大小，
	int recvbufsize=sizeof(int);
	int recvbuf=300000;

	setsockopt(m_socket,SOL_SOCKET,SO_RCVBUF,(char FAR *)&recvbuf,recvbufsize);
	
	bind(m_socket,(sockaddr *)&local,sizeof(sockaddr_in));// 绑定本地端口

	
	::WSAAsyncSelect(m_socket,this->m_hWnd,WM_SOCKET,FD_READ) ;
	
	if(( rcvsock = WSAJoinLeaf(m_socket,(SOCKADDR*)&remote,sizeof(remote), NULL,NULL,NULL,NULL, JL_RECEIVER_ONLY)) == INVALID_SOCKET)  
	{
	
		closesocket(m_socket); 
		return false;
	}
	

	return true;

}


int recvcount = 0;

char buf[1024]="";

struct SpeedInfo{
	CString time_ch;
	CString speed_E_ch;
	CString speed_N_ch;
	CString speed_U_ch;
};

list <SpeedInfo> list_speed_sy;
//list <int> list_speed_sy;

struct SpeedInfo speed_sy;
struct SpeedInfo speed_jy;

struct SpeedInfo speed_sy_ok;

int count=0;

int disp_count = 0;

//接收数据
void CBGSDlg::OnSocket(WPARAM wParam ,LPARAM lParam)
{


	CString m_data;

	//recvfrom等待接收源地址的数据
	recvcount = recvfrom(m_socket,buf,1024,0,(struct sockaddr FAR *)&remote,(int FAR *)&remotelength);
	
	if(recvcount<=0)
	{
	
		return;
		
	}

	m_data =buf;
	CString m_time;
	char m_hour[100];
	CString hour;
	CString m_min;
	CString m_sec;

	double  angle,east,north,up,east1,north1,up1,range,speed,speed_v2;

	
	//位置
	CString m_east;
	CString m_north;
	CString m_up;

	//斜距
	CString m_range;

	//相对速度
	CString m_speed;
	CString m_speed_v2;
	


	//速度
	CString m_east1;
	CString m_north1;
	CString m_up1;

	CString m_v_east;
	CString m_v_north;
	CString m_v_up;

	CString m_v_east_v2;
	CString m_v_north_v2;
	CString m_v_up_v2;
	

	//偏航角
	CString m_angle;
	CString m_angle1;

	//CString m_state;

	CString  StrValue;

	CString str;
	
	//找帧头、帧尾
	int nStart=m_data.Find("$GAOMING",0);
	int nStop=m_data.Find(0x0a,0);

	if(nStop>nStart)
	{
	
	data=m_data.Mid(nStart,nStop-nStart);


	//保存接收的原始数据
	StrValue=data;
	OnSoftState(StrValue,0);
	
	UpdateData(false);
	CString buffer[31];
	
	int k=0;
	for(int i=0;i<31;i++)
	{
		k=data.Find(",",0);
		buffer[i]=data.Left(k);
		data=data.Right(data.GetLength()-1-k);
	}
	

	//时间
	m_time=buffer[1];
	hour=m_time.Left(2);//时
	itoa((atoi(hour)+8)%24,m_hour,10);
	
//	SetDlgItemText(IDC_HOUR,m_hour);
	m_min=m_time.Mid(2,2);//分
	
//	SetDlgItemText(IDC_MIN,m_min);
	m_sec=m_time.Mid(4,5);//秒

//	SetDlgItemText(IDC_SEC,m_sec);
	
	//定位状态
	if (buffer[5]=="1")
	SetDlgItemText(IDC_STATE2,"单点锁定");
	//SetDlgItemText(IDC_DINGWEI,"单点锁定");

	else if(buffer[5]=="2")
	SetDlgItemText(IDC_STATE2,"相对位置锁定");
	//SetDlgItemText(IDC_DINGWEI,"相对位置锁定");

	else
	SetDlgItemText(IDC_STATE2,"未锁定");
	//SetDlgItemText(IDC_DINGWEI,"未锁定");
	
	//GetDlgItem(IDC_STATE2)->GetWindowText(str);

	
	speed_sy.time_ch=buffer[1];
	speed_sy.speed_E_ch=buffer[9];
	speed_sy.speed_N_ch=buffer[10];
	speed_sy.speed_U_ch=buffer[11];

	list_speed_sy.push_back(speed_sy);
	if(list_speed_sy.size()>3)
	{
		list_speed_sy.pop_front();
	}

	
	//东向位置
	m_east=buffer[12];
	east=atof(m_east);	
	
	//北向位置
	m_north=buffer[13];
	north=atof(m_north);
	
	//上向位置
	m_up=buffer[14];
	up=atof(m_up);
	
	//偏航角
	m_angle=buffer[15];
	angle=atof(m_angle);
	m_angle1.Format("%.1f",angle);
//	SetDlgItemText(IDC_ANGLE,m_angle1);
	
	//东向速度//0.05s前的相对位置
	m_east1=buffer[16];
	east1=atof(m_east1);
	
	//北向速度
	m_north1=buffer[17];
	north1=atof(m_north1);
	
	//上向速度
	m_up1=buffer[18];
	up1=atof(m_up1);


	speed_jy.time_ch=buffer[19];
	speed_jy.speed_E_ch=buffer[20];
	speed_jy.speed_N_ch=buffer[21];
	speed_jy.speed_U_ch=buffer[22];

	list <SpeedInfo>::iterator m;

	for (m = list_speed_sy.begin(); m != list_speed_sy.end(); ++m)
//		for(int m=0;m<list_speed_sy.size();m++)
	{
		if(!strncmp(speed_jy.time_ch,(*m).time_ch,9))
		{
		//	list_speed_sy._Head
				speed_sy_ok = *m;
		}
	}


	gps.speed_3d_jy.x=atof(speed_sy_ok.speed_E_ch);
	gps.speed_3d_jy.y=atof(speed_sy_ok.speed_N_ch);
	gps.speed_3d_jy.z=atof(speed_sy_ok.speed_U_ch);

	gps.speed_3d_sy.x=atof(speed_jy.speed_E_ch);
	gps.speed_3d_sy.y=atof(speed_jy.speed_N_ch);
	gps.speed_3d_sy.z=atof(speed_jy.speed_U_ch);
	
	}			
				
			gps.rel_ant_pos.x = -east;
			gps.rel_ant_pos.y = -north;
			gps.rel_ant_pos.z = -up;
			
			gps.speed_angle = angle;
			/*
			gps.rel_ant_last_pos.x = -east1;
			gps.rel_ant_last_pos.y = -north1;
			gps.rel_ant_last_pos.z = -up1;
			*/



			calc_enu2xyz_plane_ordinator(&(gps.rel_ant2plane_pos),&(gps.rel_ant_pos));
		
			calc_tail2plane_pos();
			
			calc_tail2head_pos();

		//	calc_rel_speed_method1();

			calc_rel_speed_method2();
//display			
			m_east.Format("%.2f",gps.rel_tail2head_pos.x);
		//	SetDlgItemText(IDC_x, m_east );
			
			m_north.Format("%.2f",gps.rel_tail2head_pos.y);
		//	SetDlgItemText(IDC_y, m_north );
			
			m_up.Format("%.2f",gps.rel_tail2head_pos.z);
		//	SetDlgItemText(IDC_z, m_up );
			
			range=sqrt(gps.rel_tail2head_pos.x*gps.rel_tail2head_pos.x+gps.rel_tail2head_pos.y*gps.rel_tail2head_pos.y+gps.rel_tail2head_pos.z*gps.rel_tail2head_pos.z);
			m_range.Format("%.2f",range);
		//	SetDlgItemText(IDC_XIEJU,m_range);	


			m_v_east.Format("%.2f",gps.rel_speedv2_xyz.x);
		//	SetDlgItemText(IDC_Vx, m_v_east );
			
			m_v_north.Format("%.2f",gps.rel_speedv2_xyz.y);
		//	SetDlgItemText(IDC_Vy, m_v_north );
			
			m_v_up.Format("%.2f",gps.rel_speedv2_xyz.z);
		//	SetDlgItemText(IDC_Vz, m_v_up );

			//speed=sqrt(gps.rel_speed_xyz.x*gps.rel_speed_xyz.x+gps.rel_speed_xyz.y*gps.rel_speed_xyz.y+gps.rel_speed_xyz.z*gps.rel_speed_xyz.z);
			speed=sqrt(gps.rel_speedv2_xyz.x*gps.rel_speedv2_xyz.x+gps.rel_speedv2_xyz.y*gps.rel_speedv2_xyz.y+gps.rel_speedv2_xyz.z*gps.rel_speedv2_xyz.z);
			m_speed.Format("%.2f",speed);
	//		SetDlgItemText(IDC_SPEED,m_speed);
//rel_speed method 2 display
			m_v_east_v2.Format("%.2f",gps.rel_speed_xyz.x);
	//		SetDlgItemText(IDC_Vx2, m_v_east_v2 );
			
			m_v_north_v2.Format("%.2f",gps.rel_speed_xyz.y);
	//		SetDlgItemText(IDC_Vy2, m_v_north_v2 );
			
			m_v_up_v2.Format("%.2f",gps.rel_speed_xyz.z);
	//		SetDlgItemText(IDC_Vz2, m_v_up_v2 );

			speed_v2=sqrt(gps.rel_speed_xyz.x*gps.rel_speed_xyz.x+gps.rel_speed_xyz.y*gps.rel_speed_xyz.y+gps.rel_speed_xyz.z*gps.rel_speed_xyz.z);
			//speed_v2=sqrt(gps.rel_speedv2_xyz.x*gps.rel_speedv2_xyz.x+gps.rel_speedv2_xyz.y*gps.rel_speedv2_xyz.y+gps.rel_speedv2_xyz.z*gps.rel_speedv2_xyz.z);
			m_speed_v2.Format("%.2f",speed_v2);
	//		SetDlgItemText(IDC_SPEED2,m_speed_v2);
			
			disp_count++;
//display 2
	if(disp_count >4)
	{
	SetDlgItemText(IDC_HOUR,m_hour);
	
	SetDlgItemText(IDC_MIN,m_min);

	SetDlgItemText(IDC_SEC,m_sec);

//	SetDlgItemText(IDC_ANGLE,m_angle1);

			SetDlgItemText(IDC_x, m_east );

			SetDlgItemText(IDC_y, m_north );

			SetDlgItemText(IDC_z, m_up );

			SetDlgItemText(IDC_XIEJU,m_range);	

			SetDlgItemText(IDC_Vx, m_v_east );

			SetDlgItemText(IDC_Vy, m_v_north );

			SetDlgItemText(IDC_Vz, m_v_up );

			SetDlgItemText(IDC_SPEED,m_speed);
//rel_speed method 2 display
/*
			SetDlgItemText(IDC_Vx2, m_v_east_v2 );

			SetDlgItemText(IDC_Vy2, m_v_north_v2 );

			SetDlgItemText(IDC_Vz2, m_v_up_v2 );

			SetDlgItemText(IDC_SPEED2,m_speed_v2);
*/
			disp_count =0;
	}


	 //保存处理后的数据
	char str_tx[1024]="$DATA,";
	char *str_p;
	CString text;
	int text_len ;
	
	
	//获取时
	//GetDlgItem(IDC_HOUR)->GetWindowText(text);
	
	str_p=&str_tx[6];

	int i = 0;
	text=m_hour;
	text_len = text.GetLength();
	while(text_len--)
	{
		*str_p=text.GetAt(i);
		str_p++;
		i++;
	}
	*str_p = ',';
	str_p++;

	//获取分
	//GetDlgItem(IDC_MIN)->GetWindowText(text);
	i = 0;
	text= m_min;
	text_len = text.GetLength();
	while(text_len--)
	{
		*str_p=text.GetAt(i);
		str_p++;
		i++;
	}
	*str_p = ',';
	str_p++;
	
	//获取秒
	//GetDlgItem(IDC_SEC)->GetWindowText(text);
	i = 0;
	text=m_sec;
	text_len = text.GetLength();
	while(text_len--)
	{
		*str_p=text.GetAt(i);
		str_p++;
		i++;
	}
	*str_p = ',';
	str_p++;
	
	//获取偏航角
	//GetDlgItem(IDC_ANGLE)->GetWindowText(text);
	i = 0;
	text=m_angle1;
	text_len = text.GetLength();
	while(text_len--)
	{
		*str_p=text.GetAt(i);
		str_p++;
		i++;
	}
	*str_p = ',';
	str_p++;
	
	//获取相对速度
	//GetDlgItem(IDC_SPEED)->GetWindowText(text);
	i = 0;
	text=m_speed;
	text_len = text.GetLength();
	while(text_len--)
	{
		*str_p=text.GetAt(i);
		str_p++;
		i++;
	}
	*str_p = ',';
	str_p++;
	
	//获取东向速度
	//GetDlgItem(IDC_Vx)->GetWindowText(text);
	i = 0;
	text=m_v_east;
	text_len = text.GetLength();
	while(text_len--)
	{
		*str_p=text.GetAt(i);
		str_p++;
		i++;
	}
	*str_p = ',';
	str_p++;

	//获取北向速度
	//GetDlgItem(IDC_Vy)->GetWindowText(text);
	i = 0;
	text=m_v_north;
	text_len = text.GetLength();
	while(text_len--)
	{
		*str_p=text.GetAt(i);
		str_p++;
		i++;
	}
	*str_p = ',';
	str_p++;
	
	//获取上向速度
	//GetDlgItem(IDC_Vz)->GetWindowText(text);
	i = 0;
	text=m_v_up;
	text_len = text.GetLength();
	while(text_len--)
	{
		*str_p=text.GetAt(i);
		str_p++;
		i++;
	}
	*str_p = ',';
	str_p++;
	
	//获取斜距
	//GetDlgItem(IDC_XIEJU)->GetWindowText(text);
	i = 0;
	text=m_range;
	text_len = text.GetLength();
	while(text_len--)
	{
		*str_p=text.GetAt(i);
		str_p++;
		i++;
	}
	*str_p = ',';
	str_p++;
	
	
	//获取东向位置
	//GetDlgItem(IDC_x)->GetWindowText(text);
	i = 0;
	text=m_east;
	text_len = text.GetLength();
	while(text_len--)
	{
		*str_p=text.GetAt(i);
		str_p++;
		i++;
	}
	*str_p = ',';
	str_p++;

	//获取北向位置
	//GetDlgItem(IDC_y)->GetWindowText(text);
	i = 0;
	text=m_north;
	text_len = text.GetLength();
	while(text_len--)
	{
		*str_p=text.GetAt(i);
		str_p++;
		i++;
	}
	*str_p = ',';
	str_p++;

	//获取上向位置
	//GetDlgItem(IDC_z)->GetWindowText(text);
	i = 0;
	text=m_up;
	text_len = text.GetLength();
	while(text_len--)
	{
		*str_p=text.GetAt(i);
		str_p++;
		i++;
	}
	*str_p = ',';
	str_p++;

	//获取定位状态
	GetDlgItem(IDC_DINGWEI)->GetWindowText(text);
	i = 0;
	text_len = text.GetLength();
	while(text_len--)
	{
		*str_p=text.GetAt(i);
		str_p++;
		i++;
	}
	*str_p = ',';
	str_p++;


	//获取相对速度v2
	//GetDlgItem(IDC_SPEED)->GetWindowText(text);
	i = 0;
	text=m_speed_v2;
	text_len = text.GetLength();
	while(text_len--)
	{
		*str_p=text.GetAt(i);
		str_p++;
		i++;
	}
	*str_p = ',';
	str_p++;
	
	//获取东向速度v2
	//GetDlgItem(IDC_Vx)->GetWindowText(text);
	i = 0;
	text=m_v_east_v2;
	text_len = text.GetLength();
	while(text_len--)
	{
		*str_p=text.GetAt(i);
		str_p++;
		i++;
	}
	*str_p = ',';
	str_p++;

	//获取北向速度v2
	//GetDlgItem(IDC_Vy)->GetWindowText(text);
	i = 0;
	text=m_v_north_v2;
	text_len = text.GetLength();
	while(text_len--)
	{
		*str_p=text.GetAt(i);
		str_p++;
		i++;
	}
	*str_p = ',';
	str_p++;
	
	//获取上向速度v2
	//GetDlgItem(IDC_Vz)->GetWindowText(text);
	i = 0;
	text=m_v_up_v2;
	text_len = text.GetLength();
	while(text_len--)
	{
		*str_p=text.GetAt(i);
		str_p++;
		i++;
	}
	*str_p = ',';
	str_p++;


	*str_p = 0x0d;
	str_p++;
	*str_p = 0x0a;
	str_p++;
	*str_p = '\0';

	StrValue=str_tx;
	OnSoftState(StrValue,0);




}


void CBGSDlg::ReadINI()
{
	CString  FilePath;
	FilePath+=".\\配置文件.ini";
	
	CString strSection       = "组播地址";
	CString strSectionKey    = "Multicast_Addr";
	CString strValue       = _T("");

	char inBuf[80];
	GetPrivateProfileString (strSection,strSectionKey, NULL, inBuf, 80, FilePath); 
	Ip=inBuf;
	//SetDlgItemText(IDC_EDIT3,strValue);
	//Ip=strValue;

	strSection       = "组播端口号";
	strSectionKey    = "Multicast_Port";
	strValue       = _T("");
	GetPrivateProfileString (strSection,strSectionKey, NULL, inBuf, 80, FilePath); 
	//strValue=inBuf;
	Port=atoi(inBuf);
	//SetDlgItemInt(IDC_EDIT4,Port);

	strSection       = "坐标零点偏移";
	strSectionKey    = "POS_TAIL2ANT_JIAYOU_X";
	strValue       = _T("");
	GetPrivateProfileString (strSection,strSectionKey, NULL, inBuf, 80, FilePath); 
	//strValue=inBuf;
	POS_TAIL2ANT_JIAYOU_X=atof(inBuf);
//	double testxx = double(int(POS_TAIL2ANT_JIAYOU_X*100))/100;

	strSectionKey    = "POS_TAIL2ANT_JIAYOU_Y";
	strValue       = _T("");
	GetPrivateProfileString (strSection,strSectionKey, NULL, inBuf, 80, FilePath); 
	//strValue=inBuf;
	POS_TAIL2ANT_JIAYOU_Y=atof(inBuf);

	strSectionKey    = "POS_TAIL2ANT_JIAYOU_Z";
	strValue       = _T("");
	GetPrivateProfileString (strSection,strSectionKey, NULL, inBuf, 80, FilePath); 
	//strValue=inBuf;
	POS_TAIL2ANT_JIAYOU_Z=atof(inBuf);

	strSectionKey    = "POS_HEAD2ANT_SHOUYOU_X";
	strValue       = _T("");
	GetPrivateProfileString (strSection,strSectionKey, NULL, inBuf, 80, FilePath); 
	POS_HEAD2ANT_SHOUYOU_X=atof(inBuf);

	strSectionKey    = "POS_HEAD2ANT_SHOUYOU_Y";
	strValue       = _T("");
	GetPrivateProfileString (strSection,strSectionKey, NULL, inBuf, 80, FilePath); 
	POS_HEAD2ANT_SHOUYOU_Y=atof(inBuf);

	strSectionKey    = "POS_HEAD2ANT_SHOUYOU_Z";
	strValue       = _T("");
	GetPrivateProfileString (strSection,strSectionKey, NULL, inBuf, 80, FilePath); 
	POS_HEAD2ANT_SHOUYOU_Z=atof(inBuf);

}

void CBGSDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	switch (nIDEvent)
	{

	case 2:
		sockInitFlag = InitSocket();
		if(sockInitFlag)
		{
			KillTimer(2);//如果初始化成功后，解除定时函数，不再初始化端口
		}
	break;
	}
	CDialog::OnTimer(nIDEvent);
}


extern void write_log(char *file_name,const char *fmt,...);

//保存文件
void CBGSDlg::OnSoftState(CString StrValue,int m)
{
	CString StrSectionKey="";
	StrSectionKey.Format("[%d]",g_softstate/2);

	m=g_softstate%2;

	if(m==0)
	{
		
		//WritePrivateProfileString("数据",StrSectionKey,StrValue,StrFilePath);
	//	write_log("speed_message.log","%s--%s\n",StrSectionKey,StrValue);
		write_log(file_name_p,"%s--%s\n",StrSectionKey,StrValue);
		
	}
	else if(m==1)
	{
		
		//WritePrivateProfileString("数据",StrSectionKey,StrValue,StrFilePath);
	//	write_log("speed_message.log","%s--%s\n",StrSectionKey,StrValue);
		write_log(file_name_p,"%s--%s\n",StrSectionKey,StrValue);
	}

	g_softstate++;


}
