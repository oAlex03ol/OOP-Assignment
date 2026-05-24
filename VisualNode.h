#pragma once
#include <QGraphicsItem>
#include <QPainter>
#include <vector>
#include <memory>
#include <algorithm>
#include "FSNode.h"

class CanvasEdge;

class VisualNode : public QGraphicsItem {
private:
    std::shared_ptr<FSNode> fsNode;
    VisualNode* parentNode = nullptr;
    std::vector<VisualNode*> childNodes;
    std::vector<CanvasEdge*> edges;
    bool isHovered = false;

public:
    VisualNode(std::shared_ptr<FSNode> node) : fsNode(node) {
        setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsGeometryChanges);
        setAcceptHoverEvents(true);
    }

    std::shared_ptr<FSNode> getFSNode() const { return fsNode; }
    
    void setParentNode(VisualNode* parent) { parentNode = parent; }
    VisualNode* getParentNode() const { return parentNode; }
    void addChildNode(VisualNode* child) { childNodes.push_back(child); }
    
    void removeChildNode(VisualNode* child) {
        childNodes.erase(std::remove(childNodes.begin(), childNodes.end(), child), childNodes.end());
    }
    const std::vector<VisualNode*>& getChildNodes() const { return childNodes; }

    void addEdge(CanvasEdge* edge) { edges.push_back(edge); }
    void removeEdge(CanvasEdge* edge) { 
        edges.erase(std::remove(edges.begin(), edges.end(), edge), edges.end()); 
    }
    void clearEdges() { edges.clear(); }
    const std::vector<CanvasEdge*>& getEdges() const { return edges; }

    QRectF boundingRect() const override { return QRectF(-76, -31, 152, 62); }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override { isHovered = true; update(); }
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override { isHovered = false; update(); }
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
};