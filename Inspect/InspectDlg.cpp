
// InspectDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "Inspect.h"
#include "InspectDlg.h"
#include "afxdialogex.h"
#include <thread>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CInspectDlg dialog



CInspectDlg::CInspectDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_INSPECT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CInspectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TRE_UIA, m_treUIA);
	DDX_Control(pDX, IDC_LST_ELEMENT_PROP, m_lstElementProp);
}

BEGIN_MESSAGE_MAP(CInspectDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_BUILD_UI_TREE, &CInspectDlg::OnBnClickedBtnBuildUiTree)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TRE_UIA, &CInspectDlg::OnTvnSelchangedTreUia)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CInspectDlg message handlers

BOOL CInspectDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	m_lstElementProp.InsertColumn(0, L"Name", 0, 200);
	m_lstElementProp.InsertColumn(1, L"Value", 0, 500);
	//LONG lStyleEx = GetWindowLongW(m_lstElementProp.GetSafeHwnd(), GWL_EXSTYLE);
	//lStyleEx |= LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT;
	//SetWindowLongW(m_lstElementProp.GetSafeHwnd(), GWL_EXSTYLE, lStyleEx);

	std::thread t(&CInspectDlg::BuildUITreeThread, this);
	t.detach();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CInspectDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CInspectDlg::OnPaint()
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
HCURSOR CInspectDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

int CInspectDlg::WalkerUITree(CUINode* pUINode,
	HTREEITEM hItemParent,
	HTREEITEM hItemPreviousSibling,
	__out HTREEITEM *phItem)
{
	if (nullptr == pUINode)
	{
		return -1;
	}

	// 自已
	CStringW strItem;
	strItem = L"\"";
	strItem += pUINode->m_strName.c_str();
	strItem += L"\" ";
	strItem += pUINode->m_strLocalizedControlType.c_str();

	if (nullptr == hItemParent)
	{
		*phItem = m_treUIA.InsertItem(strItem);
	}
	else
	{
		*phItem = m_treUIA.InsertItem(strItem, hItemParent, hItemPreviousSibling);
	}
	m_treUIA.SetItemData(*phItem, (DWORD_PTR)pUINode);

	CUINode* pChildElement = pUINode->m_pChild;
	HTREEITEM hPreviousSibling = nullptr;
	while (nullptr != pChildElement)
	{
		HTREEITEM hNewItem = nullptr;
		WalkerUITree(pChildElement, *phItem, hPreviousSibling, &hNewItem);
		hPreviousSibling = hNewItem;

		pChildElement = pChildElement->m_pNext;
	}

	return 0;
}

LRESULT OnUIAThreadMsg(HWND, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (UIA_BUILD_UIA == uMsg)
	{
		CInspectDlg* pThis = (CInspectDlg*)wParam;
		CWnd* pButton = pThis->GetDlgItem(IDC_BTN_BUILD_UI_TREE);

		HWND hWndTarget = ::FindWindowW(L"#32770", L"RaiseUIAEvent");
		HWND hWndSharehub = ::FindWindowW(L"Sharehub", L"Sharehub");
		pButton->EnableWindow(FALSE);
		pThis->m_treUIA.DeleteAllItems();
		pThis->m_lstElementProp.DeleteAllItems();

		ULONGLONG dwTime1 = GetTickCount64();
		pThis->m_UIAHelper.Release();
		pThis->m_UIAHelper.Init(hWndSharehub);	// nullptr is Desktop
		pThis->m_UIAHelper.BuildRawTree();

		pThis->m_UIAHelper.RegisterElementStructureChangedEvent(L"home");
		pThis->m_UIAHelper.RegisterNotifyEvent(/*L""*/&(pThis->m_UIAHelper));

		ULONGLONG dwLoadUIATreeTime = GetTickCount64() - dwTime1;

		ULONGLONG dwTime2 = GetTickCount64();
		CUINode* pRootElement = pThis->m_UIAHelper.GetRootUINode();
		HTREEITEM hItem = nullptr;
		pThis->WalkerUITree(pRootElement, nullptr, nullptr, &hItem);
		pThis->m_treUIA.Expand(hItem, TVE_EXPAND);
//		pThis->m_treUIA.SelectItem(hItem);
		ULONGLONG dwInsertTreeControlTime = GetTickCount64() - dwTime2;

		pButton->EnableWindow(TRUE);
	}
	else if (UIA_GET_ELEMENT_PROP == uMsg)
	{
		CInspectDlg* pThis = (CInspectDlg*)wParam;
		CUINode* pUINode = (CUINode*)lParam;
		pUINode->InitProp();
	}

	return 0;
}

int CInspectDlg::BuildUITreeThread()
{
	// 自动化事件必须初始化为多线程
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	MSG msg;
	while (GetMsg(&msg, OnUIAThreadMsg))
	{
	}

	m_UIAHelper.Release();

	CoUninitialize();

	return 0;
}


void CInspectDlg::OnBnClickedBtnBuildUiTree()
{
	PostMsg(UIA_BUILD_UIA, WPARAM(this), 0);
}

void CInspectDlg::OnTvnSelchangedTreUia(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	*pResult = 0;

	HTREEITEM hItem = m_treUIA.GetSelectedItem();
	if (nullptr != hItem)
	{
		CUINode* pUINode = (CUINode*)m_treUIA.GetItemData(hItem);
		_ASSERT(nullptr != pUINode);

		if (!pUINode->m_bInitProp)
		{
			SendMsg(UIA_GET_ELEMENT_PROP, WPARAM(this), LPARAM(pUINode));
		}

		m_lstElementProp.DeleteAllItems();
		int nIndex = m_lstElementProp.InsertItem(0, L"Name");
		m_lstElementProp.SetItemText(nIndex, 1, pUINode->m_strName.c_str());

		nIndex = m_lstElementProp.InsertItem(1, L"AutomationId");
		m_lstElementProp.SetItemText(nIndex, 1, pUINode->m_strAutomationId.c_str());

		nIndex = m_lstElementProp.InsertItem(2, L"ControlType");
		CStringW strControlType;
		strControlType.Format(L"%d", pUINode->m_ControlType);
		m_lstElementProp.SetItemText(nIndex, 1, strControlType);

		nIndex = m_lstElementProp.InsertItem(3, L"LocalizedControlType");
		m_lstElementProp.SetItemText(nIndex, 1, pUINode->m_strLocalizedControlType.c_str());

		nIndex = m_lstElementProp.InsertItem(4, L"ItemType");
		m_lstElementProp.SetItemText(nIndex, 1, pUINode->m_strItemType.c_str());

		nIndex = m_lstElementProp.InsertItem(5, L"ClassName");
		m_lstElementProp.SetItemText(nIndex, 1, pUINode->m_strClassName.c_str());
	}
}

void CInspectDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	SendMsg(WM_QUIT, 0, 0);

	CDialogEx::OnClose();
}
