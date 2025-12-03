// ChildView.h

#pragma once

#include <vector>
#include <cmath>
#include <limits>
#include <queue>
#include <algorithm>
#include <map> 
#include <atlimage.h> 

struct Node {
    int id;
    CPoint pos;
};

class CChildView : public CWnd
{
public:
    CChildView();
    virtual ~CChildView();

protected:
    std::vector<Node> m_nodes;
    std::vector<std::vector<double>> m_distanceMatrix;
    std::vector<int> m_shortestPath;
    std::map<std::pair<int, int>, double> m_edgeWeights;

    int m_lastSelectedNodeIndex = -1;
    CImage m_campusMapImage;
    CString m_distanceText;
    bool m_isPathDrawn = false;

protected:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    int FindNodeAtPoint(CPoint point, int radius = 10);
    double RunDijkstra(int startNodeId, int endNodeId);

    void OnInitialUpdate();

protected:
    afx_msg void OnPaint();
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

    DECLARE_MESSAGE_MAP()
};