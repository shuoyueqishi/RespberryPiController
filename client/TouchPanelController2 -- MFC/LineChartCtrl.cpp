// LineChartCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "TouchPanelController2.h"
#include "LineChartCtrl.h"
#include <math.h>
#include "MemDC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////////////////////
//Line
CLine::CLine(int iStyle, COLORREF color, int iThick)
{
	m_bDraw = TRUE;
	m_iStyle = iStyle;
	m_color = color;
	m_iThick = iThick;
	InitializeCriticalSection ( & g_cs ) ;
}
CLine::CLine(const CLine &line)
{
	m_bDraw = line.m_bDraw;
	m_iStyle = line.m_iStyle;
	m_color = line.m_color;
	m_iThick = line.m_iThick;
	//
	m_dXValue.clear();
	for (int i = 0; i < line.m_dXValue.size(); i++){
		m_dXValue.push_back(line.m_dXValue[i]);
	}
	m_dYValue.clear();
	for (i = 0; i < line.m_dYValue.size(); i++){
		m_dXValue.push_back(line.m_dYValue[i]);
	}
	InitializeCriticalSection ( & g_cs ) ;
}
CLine& CLine::operator=(const CLine& line)
{
	m_bDraw = line.m_bDraw;
	m_iStyle = line.m_iStyle;
	m_color = line.m_color;
	m_iThick = line.m_iThick;
	//
	m_dXValue.clear();
	for (int i = 0; i < line.m_dXValue.size(); i++){
		m_dXValue.push_back(line.m_dXValue[i]);
	}
	m_dYValue.clear();
	for (i = 0; i < line.m_dYValue.size(); i++){
		m_dXValue.push_back(line.m_dYValue[i]);
	}
	
	return *this;
}
CLine::~CLine()
{
	DeleteCriticalSection ( & g_cs ) ;
}
void CLine::AddPoint(double x, double y)
{
	EnterCriticalSection ( & g_cs ) ;
	m_dXValue.push_back(x);
	m_dYValue.push_back(y);
	LeaveCriticalSection ( & g_cs ) ;
	
}
double CLine::GetPointX(int nIndex)
{
	return m_dXValue[nIndex];
}
double CLine::GetPointY(int nIndex)
{
	return m_dYValue[nIndex];
}
int CLine::GetPointCount()
{
	return (int)m_dXValue.size();
}
void CLine::RemoveAllPoints()
{
	EnterCriticalSection ( & g_cs ) ;
	m_dXValue.clear();
	m_dYValue.clear();
	LeaveCriticalSection ( & g_cs ) ;
}
void CLine::RemoveUselessPoints()
{
	EnterCriticalSection ( & g_cs ) ;
	for(int i=0;i<MAX_POINTS;i++)
	{
		m_dXValue.erase(m_dXValue.begin());
		m_dYValue.erase(m_dYValue.begin());
	}
	LeaveCriticalSection ( & g_cs ) ;
}
/////////////////////////////////////////////////////////////////////////////
// CLineChartCtrl

CLineChartCtrl::CLineChartCtrl()
{
	m_clrBkColor = RGB(0,0,0);
	m_nRate      = 500;
	
	m_iMarginLeft   = 5;
	m_iMarginRight  = 5;
	m_iMarginTop    = 15;
	m_iMarginBottom = 15;
}

CLineChartCtrl::~CLineChartCtrl()
{
}


BEGIN_MESSAGE_MAP(CLineChartCtrl, CStatic)
	//{{AFX_MSG_MAP(CLineChartCtrl)
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLineChartCtrl message handlers



void CLineChartCtrl::DrawAll(CDC* pDC)
{
	DrawBackground(pDC);
	DrawAxises(pDC);
	DrawLines(pDC);
}
void CLineChartCtrl::DrawBackground(CDC* pDC)
{
	pDC->FillSolidRect ( & m_rectCtrl , m_clrBkColor ) ;

}
void CLineChartCtrl::DrawAxises(CDC* pDC)
{
	//
	
	//Draw Y axis
	if(m_axisY.m_bDraw)
	{
		CPen pen(m_axisY.m_iStyle, m_axisY.m_iThick, m_axisY.m_color);
		CPen *oldPen = pDC->SelectObject(&pen);
		//Draw border of plot
		pDC->MoveTo(m_rectPlot.left,   m_rectPlot.top);  
		pDC->LineTo(m_rectPlot.right, m_rectPlot.top);  
		pDC->LineTo(m_rectPlot.right, m_rectPlot.bottom);  
		pDC->LineTo(m_rectPlot.left,   m_rectPlot.bottom);  
		pDC->LineTo(m_rectPlot.left,   m_rectPlot.top); 
		//
		int iGridCount = 5;
		int iGrid =(int)floor((m_axisY.m_dMaxValue-m_axisY.m_dMinValue)/iGridCount);
		if(iGrid>0)
		{
			CString str;
			int textHeight =16;
			pDC->SetTextColor(m_axisY.m_color);
			
			int y0=(int)m_axisY.m_dMinValue; 
			for(int i=0;i<iGridCount+1;i++)
			{
				int y = (int)(m_rectPlot.bottom - (y0+i*iGrid-m_axisY.m_dMinValue)/m_axisY.m_dValuePerPixel);
				
				str.Format(_T("%d"), y0+i*iGrid);
				pDC->DrawText(str, CRect(m_rectAxisY.left, y-textHeight/2,m_rectAxisY.right-5,y+textHeight/2), DT_RIGHT|DT_BOTTOM);

				//
				if(m_axisY.m_bGrid)
				{
					pDC->MoveTo(m_rectPlot.left,y);
					pDC->LineTo(m_rectPlot.right,y);
				}

				//TRACE("y0=%d,y=%d,iGrid=%d\n",y0,y,iGrid);
			}
		}
		
		pDC->SelectObject(oldPen);
	}
	
	//Draw X axis
	if(m_axisX.m_bDraw)
	{

		if(m_axisX.m_bGrid)
		{

		}

	}
}
void CLineChartCtrl::DrawLines(CDC *pDC)
{
	for(int i=0;i<GetLineCount();i++)
	{
		CLine& line = GetLine(i);
		//
		if(!line.m_bDraw)continue;

		int iPointCount = line.GetPointCount();
		if(iPointCount<=0)return;

		CPen pen(line.m_iStyle, line.m_iThick, line.m_color);
		CPen *oldPen = pDC->SelectObject(&pen);
		//
		int dataPointStart=0;
		if(iPointCount>m_rectPlot.Width())
		{
			dataPointStart = iPointCount-m_rectPlot.Width();
		}
		if(iPointCount>MAX_POINTS)
		{
			dataPointStart = iPointCount-MAX_POINTS;
		}
		for(int k=dataPointStart;k<iPointCount;k++)
		{
			
			int x = m_rectPlot.right-(iPointCount-k); //a pixel represents one point
			int y = (int)(m_rectPlot.bottom - ((line.GetPointY(k)-m_axisY.m_dMinValue)/m_axisY.m_dValuePerPixel));
	
			m_ptInLine[k-dataPointStart].x = x;
			m_ptInLine[k-dataPointStart].y = y;
			//TRACE("x =%d,y =%d\n",x,y);
		}

		pDC->Polyline(m_ptInLine, iPointCount-dataPointStart);
		pDC->SelectObject(oldPen);

		TRACE("point count =%d\n",iPointCount);
		//handle CArray overflow
		if(iPointCount>2*MAX_POINTS)
		{
			line.RemoveUselessPoints();
			TRACE("Too many points, removing...\n");
		}

	}
}
//
void CLineChartCtrl::Start()
{
	if(m_nRate > 0)
	{
		SetTimer(1,m_nRate,NULL);
	}
}
void CLineChartCtrl::Stop()
{
	KillTimer(1);
}

//
void CLineChartCtrl::CalcLayout()
{
	GetClientRect(&m_rectCtrl);
	//
	int rectAxisYWidth = 50;
	m_rectAxisY.left = m_rectCtrl.left + m_iMarginLeft;
	m_rectAxisY.right = m_rectAxisY.left + rectAxisYWidth;
	m_rectAxisY.top   = m_rectCtrl.top + m_iMarginTop;
	m_rectAxisY.bottom = m_rectCtrl.bottom - m_iMarginBottom;

	//
	m_rectAxisX.left = m_rectAxisY.right;
	m_rectAxisX.right = m_rectCtrl.right-m_iMarginRight;
	m_rectAxisX.bottom = m_rectCtrl.bottom - m_iMarginBottom;
	m_rectAxisX.top   = m_rectAxisX.bottom -20;
	

	//
	m_rectPlot.left   = m_rectAxisY.right;
	m_rectPlot.right  = m_rectCtrl.right-m_iMarginRight;
	m_rectPlot.top    = m_rectCtrl.top + m_iMarginTop;
	m_rectPlot.bottom = m_rectAxisX.top;

	//
	m_axisY.m_dValuePerPixel   = (m_axisY.m_dMaxValue- m_axisY.m_dMinValue) / m_rectPlot.Height();
	
	TRACE("m_axisY.m_dValuePerPixel=%.1f\n",m_axisY.m_dValuePerPixel);

}
void CLineChartCtrl::AddLine(int iStyle, COLORREF color, int iThick)
{
	m_lines.push_back(CLine(iStyle,color,iThick));
}
CLine& CLineChartCtrl::GetLine(int nIndex)
{
	return m_lines[(nIndex)];
}
int CLineChartCtrl::GetLineCount()
{
	return (int)m_lines.size();
}
//
void CLineChartCtrl::SetRate(int nRate)
{
	m_nRate = nRate;
	
}
void CLineChartCtrl::SetBkColor(COLORREF clrBkColor)
{
	m_clrBkColor = clrBkColor;
}

//
CAxis& CLineChartCtrl::GetAxisY()
{
	return m_axisY;
}
CAxis& CLineChartCtrl::GetAxisX()
{
	return m_axisX;
}

void CLineChartCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CStatic::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	CalcLayout();
	Invalidate();
}

void CLineChartCtrl::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	Invalidate(TRUE);
	CStatic::OnTimer(nIDEvent);
}

void CLineChartCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	CRect rectCtrl;
	GetClientRect (&rectCtrl);
	
	CMemDC memDC(&dc, &rectCtrl);
	DrawAll(&memDC);
	// Do not call CStatic::OnPaint() for painting messages
}

BOOL CLineChartCtrl::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	
	//return CStatic::OnEraseBkgnd(pDC);
	return FALSE;
}
