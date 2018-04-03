
// PenPaperUninstall32Dlg.h : header file
//

#pragma once


// CPenPaperUninstall32Dlg dialog
class CPenPaperUninstall32Dlg : public CDialogEx
{
// Construction
public:
	CPenPaperUninstall32Dlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PENPAPERUNINSTALL32_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
};
