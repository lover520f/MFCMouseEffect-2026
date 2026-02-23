
// MFCMouseEffectView.cpp: CMFCMouseEffectView 类的实现
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "MFCMouseEffect.h"
#endif

#include "MFCMouseEffectDoc.h"
#include "MFCMouseEffectView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFCMouseEffectView

IMPLEMENT_DYNCREATE(CMFCMouseEffectView, CView)

BEGIN_MESSAGE_MAP(CMFCMouseEffectView, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CMFCMouseEffectView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CMFCMouseEffectView 构造/析构

CMFCMouseEffectView::CMFCMouseEffectView() noexcept
{
	// TODO: 在此处添加构造代码

}

CMFCMouseEffectView::~CMFCMouseEffectView()
{
}

BOOL CMFCMouseEffectView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CMFCMouseEffectView 绘图

void CMFCMouseEffectView::OnDraw(CDC* /*pDC*/)
{
	CMFCMouseEffectDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 在此处为本机数据添加绘制代码
}


// CMFCMouseEffectView 打印


void CMFCMouseEffectView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CMFCMouseEffectView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CMFCMouseEffectView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void CMFCMouseEffectView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}

void CMFCMouseEffectView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CMFCMouseEffectView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CMFCMouseEffectView 诊断

#ifdef _DEBUG
void CMFCMouseEffectView::AssertValid() const
{
	CView::AssertValid();
}

void CMFCMouseEffectView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMFCMouseEffectDoc* CMFCMouseEffectView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMFCMouseEffectDoc)));
	return (CMFCMouseEffectDoc*)m_pDocument;
}
#endif //_DEBUG


// CMFCMouseEffectView 消息处理程序
