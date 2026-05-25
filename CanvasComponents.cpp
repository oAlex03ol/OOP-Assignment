// CanvasComponents.cpp
// A draggable graph structure composed of "VisualNode" and "CanvasEdge", with real-time connection updates.

#include "VisualNode.h"
#include "CanvasEdge.h"
#include <iostream>

// The implementation of the CanvasEdge class's constructor
// This constructor generates a line between itself and the connected VisualNode
CanvasEdge::CanvasEdge(VisualNode* src, VisualNode* dst) : source(src), dest(dst) {
    // QColor() : https://doc.qt.io/qt-6/qcolor.html#QColor-4
    // QPen()   : https://doc.qt.io/qt-6/qpen.html
    setPen(QPen(QColor(100, 110, 130, 150), 2, Qt::DashLine));  // Setting connected line styles : colors / width / pattern
    setZValue(-1);  // Layer Depth: z-index
    trackNodes();   // Initialize line position
}

// Update the edge position by connecting the current positions of source and destination nodes
void CanvasEdge::trackNodes() {
    if (source && dest) {   // To handle cases where the root or leaf node might be a null pointer.
        setLine(QLineF(source->pos(), dest->pos()));    // Update position of two VisualNode
        //qDebug() << QString::fromStdString(dest->getFSNode()->getName());
        //std::cout << source->getFSNode()->getName() << " " << source->getFSNode()->getName() << std::endl ;
    }
}

// Draw the visual appearance of the node on the scene
void VisualNode::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    //std::cout << painter << std::endl ;
    Q_UNUSED(option); Q_UNUSED(widget); // These are macros used to preventing unused parameter warnings.
    painter->setRenderHint(QPainter::Antialiasing); // Enable anti-aliasing
    
    QFont font("Microsoft JhengHei", 12, QFont::Bold);    // Font, Size, Bold
    painter->setFont(font);
    
    QColor borderColor = fsNode->getType() == NodeType::Folder ? QColor("#00adb5") : QColor("#ff2e63");
    QPen pen(borderColor, isHovered || isSelected() ? 4 : 2);
    painter->setPen(pen);
    
    // Background color changes with selection
    QColor bgColor(26, 26, 36, 230);
    if (isSelected()) {
        bgColor = borderColor.lighter(180); // 180% brighter
        bgColor = borderColor;
        bgColor.setAlpha(80);               // Keep transparent feeling
    }
    painter->setBrush(QBrush(bgColor)); // BackGround
    painter->drawRoundedRect(-120, -25, 240, 50, 8, 8);  // x, y, width, height, radiusX, radiusY

    // For text orientation
    /*std::string prefix = (fsNode->getType() == NodeType::Folder) ? "📁 " : "📄 ";
    QString text = QString::fromStdString(prefix + fsNode->getName());*/

    // For icon orientation
    QPixmap icon(fsNode->getType() == NodeType::Folder ? "../icons/Folder.png" : "../icons/File.png" );
    painter->drawPixmap(-110, -12, 24, 24, icon);
    QString text = QString::fromStdString(fsNode->getName() + " / " + fsNode->getSizeString());
    painter->drawText(QRectF(-80, -20, 190, 40), Qt::AlignVCenter, text);

    /*painter->setPen(QColor("#eeeeee"));
    painter->drawText(QRectF(-65, -20, 130, 40), Qt::AlignCenter, text);*/
}

QVariant VisualNode::itemChange(GraphicsItemChange change, const QVariant& value) {
    if (change == ItemPositionHasChanged) {
        for (CanvasEdge* edge : edges) {
            edge->trackNodes();
        }
    }
    return QGraphicsItem::itemChange(change, value);
}
