// ChildView.cpp

#include "pch.h"
#include "framework.h"
#include "campusHWW.h" 
#include "ChildView.h"

#include <cmath> 
#include <limits> 
#include <algorithm>
#include <queue>
#include <vector>
#include <map> 

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CChildView::CChildView()
{
}

CChildView::~CChildView()
{
}

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CWnd::PreCreateWindow(cs))
        return FALSE;

    cs.dwExStyle |= WS_EX_CLIENTEDGE;
    cs.style &= ~WS_BORDER;
    cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
        ::LoadCursor(nullptr, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1), nullptr);

    return TRUE;
}

BEGIN_MESSAGE_MAP(CChildView, CWnd)
    ON_WM_PAINT()
    ON_WM_RBUTTONDOWN()
    ON_WM_LBUTTONDOWN()
    ON_WM_CREATE()
END_MESSAGE_MAP()


int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    m_distanceMatrix.assign(0, std::vector<double>(0, std::numeric_limits<double>::max()));


    OnInitialUpdate();

    return 0;
}


void CChildView::OnInitialUpdate()
{

    HRESULT hr = m_campusMapImage.Load(L"C:\\Users\\siyeo\\OneDrive\\사진\\Screenshots\\스크린샷 2025-12-03 010203.png");

    if (FAILED(hr))
    {
        // ... (실패 처리)
    }
}

void CChildView::OnPaint()
{
    CPaintDC dc(this);

    if (!m_campusMapImage.IsNull())
    {
        CRect clientRect;
        GetClientRect(&clientRect);
        m_campusMapImage.Draw(dc.GetSafeHdc(), clientRect);
    }

    CPen penBlack(PS_SOLID, 3, RGB(0, 0, 0));
    CBrush brushBlack(RGB(0, 0, 0));
    CPen* pOldPen = dc.SelectObject(&penBlack);
    CBrush* pOldBrush = dc.SelectObject(&brushBlack);

    for (const auto& node : m_nodes)
    {
        dc.Ellipse(node.pos.x - 5, node.pos.y - 5, node.pos.x + 5, node.pos.y + 5);

        dc.SetBkMode(TRANSPARENT);
        dc.SetTextColor(RGB(0, 0, 0));
        CString strID;
        strID.Format(L"%d", node.id);
        dc.TextOutW(node.pos.x + 8, node.pos.y - 8, strID);
    }

    CPen penLine(PS_SOLID, 2, RGB(0, 0, 0));
    dc.SelectObject(&penLine);
    dc.SelectObject(GetStockObject(NULL_BRUSH));

    for (size_t i = 0; i < m_nodes.size(); ++i)
    {
        for (size_t j = i + 1; j < m_nodes.size(); ++j)
        {
            if (m_distanceMatrix[i][j] != std::numeric_limits<double>::max())
            {
                dc.MoveTo(m_nodes[i].pos);
                dc.LineTo(m_nodes[j].pos);

                int keyId1 = std::min((int)i, (int)j);
                int keyId2 = std::max((int)i, (int)j);

                if (m_edgeWeights.count({ keyId1, keyId2 }))
                {
                    double dist = m_edgeWeights.at({ keyId1, keyId2 });
                    CString strDist;
                    strDist.Format(L"%.1f", dist);

                    int midX = (m_nodes[i].pos.x + m_nodes[j].pos.x) / 2;
                    int midY = (m_nodes[i].pos.y + m_nodes[j].pos.y) / 2;

                    dc.SetBkMode(OPAQUE);
                    dc.SetTextColor(RGB(0, 0, 255));

                    dc.TextOutW(midX + 2, midY - 10, strDist);

                    dc.SetBkMode(TRANSPARENT);
                }
            }
        }
    }

    if (m_isPathDrawn && m_shortestPath.size() > 1)
    {
        CPen penShortestPath(PS_SOLID, 4, RGB(255, 0, 0));
        dc.SelectObject(&penShortestPath);

        for (size_t i = 0; i < m_shortestPath.size() - 1; ++i)
        {
            int nodeID1 = m_shortestPath[i];
            int nodeID2 = m_shortestPath[i + 1];

            dc.MoveTo(m_nodes[nodeID1].pos);
            dc.LineTo(m_nodes[nodeID2].pos);
        }

        CRect clientRect;
        GetClientRect(&clientRect);

        dc.SetBkMode(OPAQUE);
        dc.SetTextColor(RGB(255, 0, 0));

        dc.TextOutW(10, clientRect.bottom - 30, m_distanceText);
    }

    dc.SelectObject(pOldPen);
    dc.SelectObject(pOldBrush);
}

int CChildView::FindNodeAtPoint(CPoint point, int radius)
{
    for (size_t i = 0; i < m_nodes.size(); ++i)
    {
        double distanceSq = std::pow(m_nodes[i].pos.x - point.x, 2) +
            std::pow(m_nodes[i].pos.y - point.y, 2);

        if (distanceSq <= radius * radius)
        {
            return (int)i;
        }
    }
    return -1;
}

void CChildView::OnRButtonDown(UINT nFlags, CPoint point)
{
    if (nFlags & MK_CONTROL)
    {
        Node newNode;
        newNode.id = (int)m_nodes.size();
        newNode.pos = point;
        m_nodes.push_back(newNode);

        size_t newSize = m_nodes.size();
        m_distanceMatrix.resize(newSize);
        for (auto& row : m_distanceMatrix) {
            row.resize(newSize, std::numeric_limits<double>::max());
        }

        Invalidate();
    }
    else
    {
        CWnd::OnRButtonDown(nFlags, point);
    }
}

void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
    int clickedNodeIndex = FindNodeAtPoint(point);

    if ((nFlags & MK_CONTROL) && (nFlags & MK_SHIFT) && (clickedNodeIndex != -1))
    {
        if (m_lastSelectedNodeIndex == -1)
        {
            m_lastSelectedNodeIndex = clickedNodeIndex;
            m_isPathDrawn = false;
            Invalidate();
        }
        else if (m_lastSelectedNodeIndex != clickedNodeIndex)
        {
            int startNodeId = m_lastSelectedNodeIndex;
            int endNodeId = clickedNodeIndex;

            double totalDistance = RunDijkstra(startNodeId, endNodeId);

            m_distanceText.Format(L"최단 거리: %.2f 픽셀", totalDistance);
            m_isPathDrawn = true;
            m_lastSelectedNodeIndex = -1;
            Invalidate();
        }
        CWnd::OnLButtonDown(nFlags, point);
        return;
    }


    if (clickedNodeIndex != -1)
    {
        if (nFlags & MK_CONTROL)
        {
            m_isPathDrawn = false;
            if (m_lastSelectedNodeIndex == -1)
            {
                m_lastSelectedNodeIndex = clickedNodeIndex;
            }
            else if (m_lastSelectedNodeIndex != clickedNodeIndex)
            {
                int id1 = m_lastSelectedNodeIndex;
                int id2 = clickedNodeIndex;

                double distance = std::sqrt(
                    std::pow(m_nodes[id1].pos.x - m_nodes[id2].pos.x, 2) +
                    std::pow(m_nodes[id1].pos.y - m_nodes[id2].pos.y, 2)
                );

                int keyId1 = std::min(id1, id2);
                int keyId2 = std::max(id1, id2);

                m_edgeWeights[{keyId1, keyId2}] = distance;

                m_distanceMatrix[id1][id2] = distance;
                m_distanceMatrix[id2][id1] = distance;

                m_lastSelectedNodeIndex = -1;
                Invalidate();
            }
        }
    }
    else
    {
        m_lastSelectedNodeIndex = -1;
        m_isPathDrawn = false;
        Invalidate();
    }

    CWnd::OnLButtonDown(nFlags, point);
}

double CChildView::RunDijkstra(int startNodeId, int endNodeId)
{
    if (m_nodes.empty()) return 0.0;

    int n = (int)m_nodes.size();

    std::vector<double> dist(n, std::numeric_limits<double>::max());
    std::vector<int> prev(n, -1);

    using PDI = std::pair<double, int>;

    std::priority_queue<PDI, std::vector<PDI>, std::greater<PDI>> pq;

    dist[startNodeId] = 0.0;
    pq.push({ 0.0, startNodeId });

    while (!pq.empty())
    {
        double d = pq.top().first;
        int u = pq.top().second;
        pq.pop();

        if (d > dist[u]) continue;

        for (int v = 0; v < n; ++v)
        {
            double weight = m_distanceMatrix[u][v];

            if (weight != std::numeric_limits<double>::max())
            {
                if (dist[u] + weight < dist[v])
                {
                    dist[v] = dist[u] + weight;
                    prev[v] = u;
                    pq.push({ dist[v], v });
                }
            }
        }
    }

    m_shortestPath.clear();

    if (dist[endNodeId] == std::numeric_limits<double>::max())
    {
        m_distanceText = L"경로를 찾을 수 없습니다.";
        return 0.0;
    }

    int curr = endNodeId;
    while (curr != -1)
    {
        m_shortestPath.push_back(curr);
        if (curr == startNodeId) break;
        curr = prev[curr];
    }

    std::reverse(m_shortestPath.begin(), m_shortestPath.end());

    return dist[endNodeId];
}