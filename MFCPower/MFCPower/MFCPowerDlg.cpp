
// MFCPowerDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "MFCPower.h"
#include "MFCPowerDlg.h"
#include "afxdialogex.h"

#include <thread>
#include <string>
#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

SYSTEM_POWER_STATUS PowerInfo;
bool GetPowerInfoFlag = true;
std::map<int, CString> BatteryFlagMapping;
std::map<int, CString> ACLineStatusMapping;
std::map<int, CString> SystemStatusFlagMapping;

bool LockFlag = true;
BYTE KeyState[256];

// CMFCPowerDlg 对话框

CMFCPowerDlg::CMFCPowerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCPOWER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCPowerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS_POWER, prg_power);
	DDX_Control(pDX, IDC_STATIC_POWER, lbl_power);
	DDX_Control(pDX, IDC_STATIC_BTYSTATE, lbl_btystate);
	DDX_Control(pDX, IDC_STATIC_SYMSTATE, lbl_symstate);
	DDX_Control(pDX, IDC_STATIC_ACLFLAG, lbl_aclflag);
	DDX_Control(pDX, IDC_CHECK_NUM, chk_num);
	DDX_Control(pDX, IDC_CHECK_CAPS, chk_caps);
	DDX_Control(pDX, IDC_CHECK_SCROLL, chk_scroll);
}

BEGIN_MESSAGE_MAP(CMFCPowerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CHECK_CAPS, &CMFCPowerDlg::OnBnClickedCheckCaps)
	ON_BN_CLICKED(IDC_CHECK_SCROLL, &CMFCPowerDlg::OnBnClickedCheckScroll)
	ON_BN_CLICKED(IDC_CHECK_NUM, &CMFCPowerDlg::OnBnClickedCheckNum)
END_MESSAGE_MAP()

// CMFCPowerDlg 消息处理程序

BOOL CMFCPowerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	// 电池标志映射
	BatteryFlagMapping[1] = L"高-电池容量超过 66 %";
	BatteryFlagMapping[2] = L"低-电池容量小于 33 %";
	BatteryFlagMapping[4] = L"严重-电池容量小于 5 %";
	BatteryFlagMapping[8] = L"Charging";
	BatteryFlagMapping[128] = L"无系统电池";
	BatteryFlagMapping[255] = L"未知状态-无法读取电池标志信息";

	// 充电状态映射
	ACLineStatusMapping[0] = L"未充电";
	ACLineStatusMapping[1] = L"充电中";
	ACLineStatusMapping[255] = L"状态未知";

	// 系统状态标志映射
	SystemStatusFlagMapping[0] = L"节电模式已关闭";
	SystemStatusFlagMapping[1] = L"节电模式已开启";


	SetWindowTheme(prg_power.GetSafeHwnd(), L"", L"");     // 关闭进度条的 Virtual Styles 风格
	prg_power.SetBkColor(RGB(210,210,210));     // 设置进度条的背景色
	// 创建 CMFCPowerDlg::GetPowerInfo 成员函数线程，并运行
	std::thread PowerProc(&CMFCPowerDlg::GetPowerInfo, this);   
	PowerProc.detach();
	
	// 创建 CMFCPowerDlg::OperaKeyLock 成员函数线程，并运行
	std::thread KeyLockProc(&CMFCPowerDlg::OperaKeyLock, this);
	KeyLockProc.detach();
	//LockFlag = false;
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMFCPowerDlg::OnPaint()
{
	if (IsIconic()){
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMFCPowerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMFCPowerDlg::GetPowerInfo() {
	CString pwrtmp;
	while (GetPowerInfoFlag) {
		GetSystemPowerStatus(&PowerInfo);   // 获取电池信息
		prg_power.SetPos(PowerInfo.BatteryLifePercent);   // 设置进度条位置
		double Percent = (PowerInfo.BatteryLifePercent / 100.0);   // 获取电池电量百分比
		// 根据电池电量百分比调节进度条颜色
		bool Flag = Percent < 0.5;
		COLORREF color = Flag ? RGB(255, 510 * Percent, 0) : RGB(510 * (1 - Percent), 255, 0);
		prg_power.SetBarColor(color);
		// 显示电池信息
		pwrtmp.Format(L"当前电量：%d%%", PowerInfo.BatteryLifePercent);
		lbl_power.SetWindowTextW(pwrtmp);
		pwrtmp.Format(L"当前电池状态：%ws", BatteryFlagMapping[PowerInfo.BatteryFlag]);
		lbl_btystate.SetWindowTextW(pwrtmp);
		pwrtmp.Format(L"当前系统状态：%ws", SystemStatusFlagMapping[PowerInfo.SystemStatusFlag]);
		lbl_symstate.SetWindowTextW(pwrtmp);
		pwrtmp.Format(L"当前充电状态：%ws", ACLineStatusMapping[PowerInfo.ACLineStatus]);
		lbl_aclflag.SetWindowTextW(pwrtmp);
		Sleep(1000);
		chk_num.SetCheck(0);
	}
}

// TODO: 在此以下添加控件通知处理程序代码
void CMFCPowerDlg::OperaKeyLock(){
	while (LockFlag) {
		chk_num.SetCheck((int)GetKeyState(VK_NUMLOCK));
		chk_caps.SetCheck((int)GetKeyState(VK_CAPITAL));
		chk_scroll.SetCheck((int)GetKeyState(VK_SCROLL));
		Sleep(100);
	}
}

void CMFCPowerDlg::SetKeyState(int VirtualKey) {
	if (GetKeyboardState(KeyState)) {
		KeyState[VirtualKey] = (BYTE)(!(GetKeyState(VirtualKey) != 0));
		INPUT KeyInfo[2];
		memset(&KeyInfo, 0, sizeof(KeyInfo));
		KeyInfo[0].type = INPUT_KEYBOARD;
		KeyInfo[0].ki.wVk = VirtualKey;
		KeyInfo[1].type = INPUT_KEYBOARD;
		KeyInfo[1].ki.wVk = VirtualKey;
		KeyInfo[1].ki.dwFlags = KEYEVENTF_KEYUP;
		SendInput(2, KeyInfo, sizeof(INPUT));
		SetKeyboardState(KeyState);
	}
}

void CMFCPowerDlg::OnBnClickedCheckNum()
{
	SetKeyState(VK_NUMLOCK);
}

void CMFCPowerDlg::OnBnClickedCheckCaps()
{
	SetKeyState(VK_CAPITAL);
}

void CMFCPowerDlg::OnBnClickedCheckScroll()
{
	SetKeyState(VK_SCROLL);
}


