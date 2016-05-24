// YBMfcView.cpp : CYBMfcView ���ʵ��
//

#include "stdafx.h"
// SHARED_HANDLERS ������ʵ��Ԥ��������ͼ������ɸѡ�������
// ATL ��Ŀ�н��ж��壬�����������Ŀ�����ĵ����롣
#ifndef SHARED_HANDLERS
#include "YBMfc.h"
#endif

#include "YBMfcDoc.h"
#include "YBMfcView.h"
#include <string>
#include <complex>
#include <geogisf.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CYBMfcView

IMPLEMENT_DYNCREATE(CYBMfcView, CView)

BEGIN_MESSAGE_MAP(CYBMfcView, CView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_TIMER()
	END_MESSAGE_MAP()

// CYBMfcView ����/����
const int gKEDU = 5;
const int gGap = 200;

CYBMfcView::CYBMfcView()
{
	// TODO: �ڴ˴���ӹ������
	m_iSpeed = 800;
	m_iAlt = 850;
	m_direct = 1;
    m_iPitch = 0;
	m_iBent = 30;

}

CYBMfcView::~CYBMfcView()
{
}

BOOL CYBMfcView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: �ڴ˴�ͨ���޸�
	//  CREATESTRUCT cs ���޸Ĵ��������ʽ

	return CView::PreCreateWindow(cs);
}

// CYBMfcView ����

BOOL CYBMfcView::IsValidSpeed(int iSpeed)
{
	if (iSpeed >= 40 && iSpeed <= 1000)
		return TRUE;
	else
		return FALSE;
}

BOOL CYBMfcView::IsJiou(int iSpeed, int step)
{
	int s = iSpeed / step;
	return s % 2 == 0;
}

BOOL CYBMfcView::IsEndByTen(int iVal)
{
	return iVal % 10 == 0;
}

void CYBMfcView::DrawShortDashLabel(CDC* pDC, int nex, int iCurX, int iYOrg, int kdOffset, int labelOffset)
{
	pDC->LineTo(iCurX, iYOrg);
	if (IsEndByTen(nex))
	{
		pDC->LineTo(iCurX, iYOrg + kdOffset - 5);
		/* draw lable*/
		char strBuf[10];
		itoa(nex / 10, strBuf, 10);
		pDC->TextOut(iCurX - 5, iYOrg - labelOffset, strBuf, strnlen(strBuf, _countof(strBuf)));
	}
	else
	{
		pDC->LineTo(iCurX, iYOrg + kdOffset);
	}
	pDC->MoveTo(iCurX, iYOrg);
}

bool CYBMfcView::DrawDirectionStrip(CDC* pDC, int iCurVal, int iYOrg, int left, int right, int step, int iMode)
{
	int kdOffset;
	int labelOffset;
	int endKdOffset;
	int markOffset;
	int gap = 20;
	if (iMode == 0)
	{
		kdOffset = -gKEDU;
		endKdOffset = kdOffset - 10;
		labelOffset = 25;
		markOffset = 15;
	}
	else
	{
		kdOffset = gKEDU;
		endKdOffset = kdOffset + 10;
		labelOffset = -5;
		markOffset = -15;
	}

	int iXOrg = (left + right) / 2;
	pDC->MoveTo(iXOrg, iYOrg);
	/* darw mark triangle*/
	pDC->LineTo(iXOrg - 5, iYOrg + markOffset);
	pDC->LineTo(iXOrg + 5, iYOrg + markOffset);
	pDC->LineTo(iXOrg, iYOrg);

	char strBuf[10];
	itoa(iCurVal, strBuf, 10);
	pDC->TextOut(iXOrg - 7, iYOrg + labelOffset, strBuf, strnlen(strBuf, _countof(strBuf)));

	int prev = iCurVal / step * step;
	int nex = prev + step;
	
	int iCurX;

	/* draw right strip*/
	pDC->MoveTo(iXOrg, iYOrg);
	iCurX = iXOrg + (nex - iCurVal) * gap / step;
	if(nex>360)
		nex = step;	
	DrawShortDashLabel(pDC, nex, iCurX, iYOrg, kdOffset, labelOffset);
	
	nex = nex + step;
	if (nex > 360)
		nex = step;
	while (iCurX < right)
	{
		iCurX = iCurX + gap;
		if (iCurX > right)
		{
			iCurX = right;
			pDC->LineTo(iCurX, iYOrg);
			pDC->LineTo(iCurX, iYOrg + endKdOffset);
			break;
		}
		DrawShortDashLabel(pDC, nex, iCurX, iYOrg, kdOffset, labelOffset);
		nex = nex + step;
		if (nex > 360)
			nex = step;
	}

	/* draw left strip*/
	iCurX = iXOrg - (iCurVal - prev) * gap / step;
	pDC->MoveTo(iXOrg, iYOrg);
	if (prev == 0)
		prev = 360;
	DrawShortDashLabel(pDC, prev, iCurX, iYOrg, kdOffset, labelOffset);
	
	prev = prev - step;
	if (prev == 0)
		prev = 360;
	while (iCurX > left)
	{
		iCurX = iCurX - gap;
		if (iCurX < left)
		{
			iCurX = left;
			pDC->LineTo(iCurX, iYOrg);
			pDC->LineTo(iCurX, iYOrg + endKdOffset);
			break;
		}
		DrawShortDashLabel(pDC, prev, iCurX, iYOrg, kdOffset, labelOffset);
		prev = prev - step;
		if (prev == 0)
			prev = 360;
	}
	return false;
}

#define PI 3.1415926

void BT_3d(double L2,double x1,double y1,double x2,double y2,double *xl,double *yl,double *xr,double *yr)
{
	double L,x,y;
	
	L=sqrt(1.0*(x1-x2)*(x1-x2)+1.0*(y1-y2)*(y1-y2));
	if (L==0) {		  			//��ͬ���,�˴����ᷢ��
		x=0; 	y=0;
	} else {
		x=L2*(x1-x2)/L;		//���Ȳ�ƽ��x0
		y=L2*(y1-y2)/L;
	}
	*xl= -y + x2;				//��ʱ����ת,ƽ�ƻ�x2,(��ߵ�)
	*yl= +x + y2;
	*xr= +y + x2;				//˳ʱ����ת,ƽ�ƻ�x2,(�ұߵ�)
	*yr= -x + y2;
	return;
}

bool CYBMfcView::DrawAttitude(CDC* pDC, int iCurBent,int iCurPitch, int cx, int cy, int r, int iMode)
{
	CPaintDC dc(this);// device context for painting
					  /////// ����CFont::CreateFont(...)����ʵ����д����////////
	CFont myFont;     //�����������
					  //�����߼�����
	myFont.CreateFont(
		26,        //����߶�(��ת���������)=56
		10,        //������(��ת�������߶�)=20
		300,       //������ʾ�Ƕ�=270��
		0,                  //nOrientation=0
		10,                 //�������=10
		FALSE,              //��б��
		FALSE,              //���»���
		FALSE,       //��ɾ����
		DEFAULT_CHARSET,    //ʹ��ȱʡ�ַ���
		OUT_DEFAULT_PRECIS, //ȱʡ�������
		CLIP_DEFAULT_PRECIS,//ȱʡ�ü�����
		DEFAULT_QUALITY,    //nQuality=ȱʡֵ
		DEFAULT_PITCH,      //nPitchAndFamily=ȱʡֵ
		"@system");         //������=@system
	CFont *pOldFont = pDC->SelectObject(&myFont);//ѡ���豸������

    //�Ȼ��¶��Ǳ�������
	int kdL = 14;
	int kdS = 7;
	double dStartAng = -30;
	double dSpanAng = 120;

	double iCenterGap = -dSpanAng/4/3;
	double iSideGap = -dSpanAng/4/2;

	dStartAng += iCurBent/**dSpanAng/60*/;
	double dAng = dStartAng*PI/180;
	double x1 = cos(dAng)*r+cx;
	double y1 = sin(dAng)*r+cy;

    double x2 = cos(dAng)*(r+kdL)+cx;
	double y2 = sin(dAng)*(r+kdL)+cy;
	pDC->MoveTo(x1,y1);
	pDC->LineTo(x2,y2);

	dStartAng += iSideGap;
	dAng = dStartAng*PI/180;
	x1 = cos(dAng)*r+cx;
	y1 = sin(dAng)*r+cy;

    x2 = cos(dAng)*(r+kdS)+cx;
	y2 = sin(dAng)*(r+kdS)+cy;
	pDC->MoveTo(x1,y1);
	pDC->LineTo(x2,y2);

	for(int i=0;i<7;i++){
		dStartAng += iCenterGap;
		dAng = dStartAng*PI/180;
		x1 = cos(dAng)*r+cx;
		y1 = sin(dAng)*r+cy;

		int kd = i%3==0 ? kdL : kdS;

		x2 = cos(dAng)*(r+kd)+cx;
		y2 = sin(dAng)*(r+kd)+cy;
		
		if(i==3)
		{
			double xl,yl,xr,yr;
			BT_3d(kd/2, x1,y1,x2,y2,&xl,&yl,&xr,&yr);
			pDC->MoveTo(x1,y1);
			pDC->LineTo(xl,yl);
			pDC->LineTo(xr,yr);
			pDC->LineTo(x1,y1);
		}
		else{
			pDC->MoveTo(x1,y1);
			pDC->LineTo(x2,y2);
		}
	}

	dStartAng += iSideGap;
	dAng = dStartAng*PI/180;
	x1 = cos(dAng)*r+cx;
	y1 = sin(dAng)*r+cy;

    x2 = cos(dAng)*(r+kdS)+cx;
	y2 = sin(dAng)*(r+kdS)+cy;
	pDC->MoveTo(x1,y1);
	pDC->LineTo(x2,y2);

	dStartAng += iSideGap;
	dAng = dStartAng*PI/180;
	x1 = cos(dAng)*r+cx;
	y1 = sin(dAng)*r+cy;

    x2 = cos(dAng)*(r+kdL)+cx;
	y2 = sin(dAng)*(r+kdL)+cy;
	pDC->MoveTo(x1,y1);
	pDC->LineTo(x2,y2);

	//��������ָ��
	pDC->MoveTo(cx,cy-r);
	pDC->LineTo(cx-5,cy-r+15);
	pDC->LineTo(cx+5,cy-r+15);
	pDC->LineTo(cx,cy-r);

	//���Ʒɻ����λ��
	pDC->MoveTo(cx-2*r/4,cy);
	pDC->LineTo(cx-r/4,cy);
	pDC->LineTo(cx-r/4,cy+15);

	pDC->MoveTo(cx+2*r/4,cy);
	pDC->LineTo(cx+r/4,cy);
	pDC->LineTo(cx+r/4,cy+15);
	
	//���滭�м�ĸ���ָʾ
	irsa::GAffinex affin((double)iCurBent,(double)cx,(double)cy);
	
	int step = 5;
	int prev,nex;
	if(iCurPitch>=0)
	{
		prev = iCurPitch / step * step;
		nex = prev + step;
	}
	else
	{
		nex = iCurPitch / step * step;
		prev = nex - step;
		
	}
	int iCurY = iCurPitch;
	
	int gap = r/5;
	int l = r/2;
	int s = l/2;

	//Draw downside
	iCurY = cy + (iCurPitch - prev) * gap / step;
	
	for(int i=0;i<4;i++){
		double dx1,dy1,dx2,dy2;
		if(IsEndByTen(prev))
		{
			if(prev==0)
			{
				dx1= cx-r;
				dx2=cx+r;
			}
			else
			{
				dx1 = cx-r/4;
				dx2 = cx+r/4;
			}
		}
		else
		{
			dx1 = cx-r/8;
			dx2 = cx+r/8;
		}
		dy1 = iCurY;
		dy2 = iCurY;
		affin.Transform(&dx1,&dy1);
		affin.Transform(&dx2,&dy2);
		pDC->MoveTo(dx1,dy1);
		pDC->LineTo(dx2,dy2);
		if(prev!=0){
			char strBuf[10];
			itoa(abs(prev), strBuf, 10);
			pDC->TextOut(dx1 - 3, dy1, strBuf, strnlen(strBuf, _countof(strBuf)));
		}
		prev -= step;
		iCurY += gap;
	}

	//Draw upside
	iCurY = cy - (nex - iCurPitch) * gap / step;
	for(int i=0;i<4;i++){
		double dx1,dy1,dx2,dy2;
		if(IsEndByTen(nex))
		{
			if(nex==0)
			{
				dx1= cx-r;
				dx2=cx+r;
			}
			else
			{
				dx1 = cx-r/4;
				dx2 = cx+r/4;
			}
		}
		else
		{
			dx1 = cx-r/8;
			dx2 = cx+r/8;
		}
		dy1 = iCurY;
		dy2 = iCurY;
		affin.Transform(&dx1,&dy1);
		affin.Transform(&dx2,&dy2);
		pDC->MoveTo(dx1,dy1);
		pDC->LineTo(dx2,dy2);
		if(nex!=0)
		{
			char strBuf[10];
			itoa(abs(nex), strBuf, 10);
			pDC->TextOut(dx1 - 3, dy1, strBuf, strnlen(strBuf, _countof(strBuf)));
		}
		nex += step;
		iCurY -= gap;
	}

	pDC->SelectObject(pOldFont); //��myFont���豸�����з���
	myFont.DeleteObject();     //ɾ��myFont����
	return true;
}

bool CYBMfcView::DrawVerStrip(CDC* pDC, int iCurVal, int iXOrg, int top, int bottom, int step, int iMode)
{
	int kdOffset;
	int labelOffset;
	int endKdOffset;
	int markOffset;
	int gap = 20;
	if (iMode == 0)
	{
		kdOffset = -gKEDU;
		endKdOffset = kdOffset - 10;
		labelOffset = 35;
		markOffset = 15;
	}
	else
	{
		kdOffset = gKEDU;
		endKdOffset = kdOffset + 10;
		labelOffset = -5;
		markOffset = -15;
	}

	int iYOrg = (top + bottom) / 2;
	pDC->MoveTo(iXOrg, iYOrg);
	/* darw mark triangle*/
	pDC->LineTo(iXOrg + markOffset, iYOrg - 5);
	pDC->LineTo(iXOrg + markOffset, iYOrg + 5);
	pDC->LineTo(iXOrg, iYOrg);

	char strBuf[10];
	itoa(iCurVal, strBuf, 10);
	pDC->TextOut(iXOrg - labelOffset, iYOrg - 7, strBuf, strnlen(strBuf, _countof(strBuf)));

	int prev = iCurVal / step * step;
	int nex = prev + step;
	int iCurY = iCurVal;

	/* draw downside strip*/
	if (iCurY != 40)
	{
		pDC->MoveTo(iXOrg, iYOrg);
		iCurY = iYOrg + (iCurVal - prev) * gap / step;
		pDC->LineTo(iXOrg, iCurY);
		pDC->LineTo(iXOrg + kdOffset, iCurY); /* draw short line*/
		pDC->MoveTo(iXOrg, iCurY);
		prev = prev - step;
		while (prev >= 40 && iCurY < bottom)
		{
			iCurY = iCurY + gap;
			if (iCurY > bottom)
			{
				iCurY = bottom;
				pDC->LineTo(iXOrg, iCurY);
				pDC->LineTo(iXOrg + endKdOffset, iCurY);
				break;
			}
			pDC->LineTo(iXOrg, iCurY);
			pDC->LineTo(iXOrg + kdOffset, iCurY);
			pDC->MoveTo(iXOrg, iCurY);
			if (IsJiou(prev, step))
			{
				/* draw lable*/
				itoa(prev, strBuf, 10);
				pDC->TextOut(iXOrg - labelOffset, iCurY - 7, strBuf, strnlen(strBuf, _countof(strBuf)));
			}
			prev = prev - step;
		}
	}

	/* draw upside strip*/
	iCurY = iYOrg - (nex - iCurVal) * gap / step;
	pDC->MoveTo(iXOrg, iYOrg);
	pDC->LineTo(iXOrg, iCurY);
	pDC->LineTo(iXOrg + kdOffset, iCurY);

	pDC->MoveTo(iXOrg, iCurY);
	nex = nex + step;
	while (iCurY > top)
	{
		iCurY = iCurY - gap;
		if (iCurY < top)
		{
			iCurY = top;
			pDC->LineTo(iXOrg, iCurY);
			pDC->LineTo(iXOrg + endKdOffset, iCurY);
			break;
		}
		pDC->LineTo(iXOrg, iCurY);
		pDC->LineTo(iXOrg + kdOffset, iCurY);
		pDC->MoveTo(iXOrg, iCurY);
		if (IsJiou(nex, step))
		{
			itoa(nex, strBuf, 10);
			pDC->TextOut(iXOrg - labelOffset, iCurY - 7, strBuf, strnlen(strBuf, _countof(strBuf)));
		}
		nex = nex + step;
	}
	return false;
}

void CYBMfcView::OnDraw(CDC* pDC)
{
	CYBMfcDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	HDC hdc = pDC->GetSafeHdc();

	HFONT hFont;
	RECT rect;
	hFont = CreateFont(36, 20, 250, 0, FW_DONTCARE, FALSE, TRUE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
		CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Times New Roman"));
	SelectObject(hdc, hFont);

	//Sets the coordinates for the rectangle in which the text is to be formatted.
	SetRect(&rect, 100, 200, 900, 800);
	SetTextColor(hdc, RGB(0, 128, 0));
	char strBuf[] = "Drawing Text with Times New Roman";
	pDC->TextOut(100, 200, strBuf, strnlen(strBuf, _countof(strBuf)));

	//pDC->DrawText(TEXT("Drawing Text with Times New Roman"), -1, &rect, DT_NOCLIP);


//	RECT rec;
//	GetClientRect(&rec);
//	int iWidth = rec.right - rec.left;
//	int iHeight = rec.bottom - rec.top;
//
//    int top = gGap;
//	int bottom = iHeight - gGap;
//
//	int step = 10;
//
//	DrawVerStrip(pDC, m_iSpeed, 200, top, bottom, step, 0);
//	step = 100;
//	DrawVerStrip(pDC, m_iAlt, 600, top, bottom, step, 1);
//
//	DrawDirectionStrip(pDC, m_direct, 500, 200, 600, 5, 0);
//
//	DrawAttitude(pDC, m_iBent,m_iPitch,400, 300, 200, 0);
	// TODO: �ڴ˴�Ϊ����������ӻ��ƴ���
}

void CYBMfcView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CYBMfcView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CYBMfcView ���

#ifdef _DEBUG
void CYBMfcView::AssertValid() const
{
	CView::AssertValid();
}

void CYBMfcView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CYBMfcDoc* CYBMfcView::GetDocument() const // �ǵ��԰汾��������
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CYBMfcDoc)));
	return (CYBMfcDoc*)m_pDocument;
}
#endif //_DEBUG



// CYBMfcView ��Ϣ�������


void CYBMfcView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	switch (nIDEvent)
	{
	case 1:
		m_iSpeed += 1;
		break;
	case 2:
		m_iAlt += 10;
		break;
	case 3:
		m_direct += 1;
		if (m_direct > 360)
			m_direct = 1;
		break;
	case 4:
		m_iBent += 1;
		if (m_iBent > 60)
			m_iBent = -60;
		break;
	case 5:
		m_iPitch += 1;
		if (m_iPitch > 25)
			m_iPitch = -25;
		break;
	}
	Invalidate();
	CView::OnTimer(nIDEvent);
}


void CYBMfcView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	// TODO: �ڴ����ר�ô����/����û���
	SetTimer(1, 50,NULL);
	SetTimer(2, 50,NULL);
	SetTimer(3,50,NULL);
	//SetTimer(4,50,NULL);
	SetTimer(5,50,NULL);
}
