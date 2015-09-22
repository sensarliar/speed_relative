// BGS定位授时设备Dlg.h : header file
//


#if !defined(AFX_BGSDLG_H__F360AAC6_096E_497C_9231_221AC701F4C1__INCLUDED_)
#define AFX_BGSDLG_H__F360AAC6_096E_497C_9231_221AC701F4C1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/////////////////////////////////////////////////////////////////////////////
// CBGSDlg dialog

class CBGSDlg : public CDialog
{
// Construction
public:
	//void OnSoftState(CString StrValue,CString CfgValue,int m);
	void OnSoftState(CString StrValue,int m);

	void ReadINI();


	bool sockInitFlag ; // 网络连接成功的标记，false 失败，true 成功
	
	BOOL InitSocket();
	
	CBGSDlg(CWnd* pParent = NULL);	// standard constructor
	bool timeExe;
	
	
	CFont font;
	CFont m_font;
	CFont m_font1;
	CFont m_font2;
	CBrush m_bkBrush;
	CFont m_font3;
	CFont m_font4;

	CString strBuffer;
	CString str_buffer;

	CString Ip;
	int Port;
	CFile file;


// Dialog Data
	//{{AFX_DATA(CBGSDlg)
	enum { IDD = IDD_BGS_DIALOG };
	CString	m_state;
	//CString	m_dingwei;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBGSDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CBGSDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSocket(WPARAM wParam ,LPARAM lParam) ;
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BGSDLG_H__F360AAC6_096E_497C_9231_221AC701F4C1__INCLUDED_)
