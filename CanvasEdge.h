// CanvasEdge.h
// This file define the structure of a "visual edge connecting two VisualNodes".
#ifndef CANVAS_EDGE_H
#define CANVAS_EDGE_H

#include <QGraphicsLineItem>    // The class used to "draw a line" in Qt's Graphics Scene.
#include <QPen>                 // The class used to "setting the pen style" in Qt's Graphics Scene.

class VisualNode; // Forward declaration class

//CanvasEdge inherit from QGraphicsLineItem
class CanvasEdge : public QGraphicsLineItem {
private:
    // Pointer doesn't require a complete class definition
    // Thus, We can forward-declare the class to the compiler without including its full definition.
    VisualNode* source; // Start node
    VisualNode* dest;   // End node

public:
    CanvasEdge(VisualNode* src, VisualNode* dst);   // Define a construct with two pointer src and dst
    // Update the position of this edge (line) based on the positions of the source and dest nodes.
    void trackNodes();
    
    VisualNode* getSource() const { return source; }    // Source's Getter
    VisualNode* getDest() const { return dest; }        // Destination's Getter
};

#endif
