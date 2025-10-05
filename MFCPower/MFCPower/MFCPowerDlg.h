
// MFCPowerDlg.h: 头文件
//

#pragma once


// CMFCPowerDlg 对话框
class CMFCPowerDlg : public CDialogEx
{
// 构造
public:
	CMFCPowerDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCPOWER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CStatic lbl_power;
	CProgressCtrl prg_power;
	CStatic lbl_btystate;
	CStatic lbl_symstate;
	CStatic lbl_aclflag;
	CButton chk_num;
	CButton chk_caps;
	CButton chk_scroll;
	void GetPowerInfo();
	void OperaKeyLock();
	void SetKeyState(int VirtualKey);
	afx_msg void OnBnClickedCheckCaps();
	afx_msg void OnBnClickedCheckScroll();
	afx_msg void OnBnClickedCheckNum();
};
