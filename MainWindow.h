#pragma once
#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPushButton>
#include <QWheelEvent>
#include <QStackedWidget>
#include <QTableWidget>
#include <QLineEdit>
#include "VisualNode.h"

class CustomView : public QGraphicsView {
protected:
    void wheelEvent(QWheelEvent* event) override {
        double scaleFactor = 1.15;
        if (event->angleDelta().y() < 0) scaleFactor = 1.0 / scaleFactor;
        scale(scaleFactor, scaleFactor);
    }
public:
    CustomView(QGraphicsScene* scene, QWidget* parent = nullptr) : QGraphicsView(scene, parent) {}
};

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    QGraphicsScene* scene;
    CustomView* canvasView;
    QTableWidget* fileTable;        
    QStackedWidget* viewStack;  
    QLineEdit* searchBar;
    
    // ⭐ 將需要操作動畫與狀態的按鈕與容器宣告為成員變數
    QPushButton* btnToggleView;
    QPushButton* btnAutoLayout;
    QPushButton* btnMoveMode; 
    QWidget* canvasActionContainer; // 用來裝 Auto Layout 與 Free Drag 的動畫容器

    VisualNode* selectedVisualNode = nullptr;
    VisualNode* rootVisualNode = nullptr;
    std::shared_ptr<FolderNode> rootDataNode; 

    bool isFreeMoveEnabled = false; 

    void setupUI();
    void setupConnections();
    void deleteNodeRecursively(VisualNode* node);
    void layoutNode(VisualNode* node, int depth, float& currentX);

public:
    explicit MainWindow(QWidget* parent = nullptr); 

private slots:
    void onCreateFolder();
    void onCreateFile();
    void onDeleteItem();
    void onSelectionChanged();
    void onToggleView(); 
    void refreshListView(); 
    void filterListView(const QString& query);
    void autoLayoutTree();       
    void onToggleLayoutMode();   
};