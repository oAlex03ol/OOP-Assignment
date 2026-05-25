#pragma once
#include <QGraphicsItem>
#include <QPainter>
#include <vector>
#include <memory>
#include <algorithm>
#include <iostream>
#include "FSNode.h"

class CanvasEdge;

class VisualNode : public QGraphicsItem {
private:
    std::shared_ptr<FSNode> fsNode;         // Interface between data layer and visual layer!
    VisualNode* parentNode = nullptr;       // Only one parentNode
    std::vector<VisualNode*> childNodes;    // Several childNode
    std::vector<CanvasEdge*> edges;         // one edge between two node
    bool isHovered = false;                 // Boolean values ​​used to identify mouse hover states

public:
    // Constructor：Initialize the data model and set the graphical properties.
    VisualNode(std::shared_ptr<FSNode> node) : fsNode(node) {
        // Make the node selectable and enable location change notifications (to update connections).
        setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsGeometryChanges);    // enum flag
        setAcceptHoverEvents(true);
    }

    std::shared_ptr<FSNode> getFSNode() const { return fsNode; }    // Retrieve associated backend data
    
    // Setting and getting the parent node
    void setParentNode(VisualNode* parent) { parentNode = parent; }
    VisualNode* getParentNode() const { return parentNode; }

    // Managing child nodes: adding and removing
    void addChildNode(VisualNode* child) { childNodes.push_back(child); }
    void removeChildNode(VisualNode* child) {
        childNodes.erase(std::remove(childNodes.begin(), childNodes.end(), child), childNodes.end());
    }
    const std::vector<VisualNode*>& getChildNodes() const { return childNodes; }

    // Managing Connecting Edges: Adding, Removing, and Clearing
    void addEdge(CanvasEdge* edge) { edges.push_back(edge); }
    void removeEdge(CanvasEdge* edge) { 
        edges.erase(std::remove(edges.begin(), edges.end(), edge), edges.end()); 
    }
    void clearEdges() { edges.clear(); }
    const std::vector<CanvasEdge*>& getEdges() const { return edges; }

    // Define the extent of the node on the canvas (with the node's center as the origin).
    QRectF boundingRect() const override { return QRectF(-126, -31, 252, 62); }
    // Polymorphism : Draws the nodes on the screen (implemented in .cpp)
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

protected:
    // Mouse event handling: Set isHovered to true and refresh the interface when the mouse hovers over the mouse.
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override { isHovered = true; update(); }
    // Mouse event handling: Set isHovered to false and refresh the interface when the mouse moves away.
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override { isHovered = false; update(); }
    // State change callback: When a node's position changes, it notifies the relevant edges to synchronize and update.
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
};

//virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
