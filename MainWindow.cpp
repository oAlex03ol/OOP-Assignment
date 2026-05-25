#include "MainWindow.h"
#include "CanvasEdge.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QHeaderView>
#include <QApplication> 
#include <QTimer>       
#include <QPropertyAnimation> 
#include <QEasingCurve> 
#include <cstdlib>
#include <algorithm>    

namespace {
bool isValidItemName(const QString& name) {
    return !name.isEmpty() && name != "." && name != ".." &&
           !name.contains('/') && !name.contains('\\');
}
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    resize(1280, 720);
    setWindowTitle("Group 1 - File Explorer");
    
    rootDataNode = std::make_shared<FolderNode>("./Root");
    fs::create_directories(rootDataNode->getPath());

    setupUI();
    setupConnections();

    rootVisualNode = new VisualNode(rootDataNode);
    scene->addItem(rootVisualNode);
    rootVisualNode->setSelected(true);
    selectedVisualNode = rootVisualNode;

    auto createDefaultNode = [this](VisualNode* parentVNode, const std::string& name, bool isFolder) -> VisualNode* {
        if (!parentVNode) return nullptr;
        auto parentFolder = std::dynamic_pointer_cast<FolderNode>(parentVNode->getFSNode());
        if (!parentFolder) return nullptr;

        std::shared_ptr<FSNode> childData;
        if (isFolder) {
            childData = parentFolder->createChildFolder(name);
        } else {
            childData = parentFolder->createChildFile(name);
        }

        VisualNode* childVNode = new VisualNode(childData);
        childVNode->setFlag(QGraphicsItem::ItemIsMovable, isFreeMoveEnabled);
        scene->addItem(childVNode);
        
        childVNode->setParentNode(parentVNode);
        parentVNode->addChildNode(childVNode);

        CanvasEdge* edge = new CanvasEdge(parentVNode, childVNode);
        scene->addItem(edge);
        parentVNode->addEdge(edge);
        childVNode->addEdge(edge);

        return childVNode;
    };

    VisualNode* vFile1 = createDefaultNode(rootVisualNode, "Desktop", true);  
    VisualNode* vFile2 = createDefaultNode(rootVisualNode, "Document", true);  
    VisualNode* vFile3 = createDefaultNode(rootVisualNode, "Picture", true);  
    VisualNode* Download = createDefaultNode(rootVisualNode, "Download", true);  

    if (vFile2) {
        VisualNode* vFile3_child = createDefaultNode(vFile2, "file3", true);     
        VisualNode* vDataTxt = createDefaultNode(vFile2, "data.txt", false); 
    }

    QTimer::singleShot(0, this, &MainWindow::autoLayoutTree);
}

void MainWindow::setupUI() {
    QWidget* centralWidget = new QWidget(this);
    centralWidget->setStyleSheet("background-color: #11111b;"); 
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QWidget* controlPanel = new QWidget(this);
    controlPanel->setFixedWidth(260);

    controlPanel->setStyleSheet("background-color: #181825; border-right: 1px solid #313244;");
    
    QVBoxLayout* btnLayout = new QVBoxLayout(controlPanel);
    btnLayout->setContentsMargins(20, 20, 20, 20);
    btnLayout->setSpacing(15);

    btnToggleView = new QPushButton("Switch viewport");

    canvasActionContainer = new QWidget(controlPanel);
    QVBoxLayout* actionLayout = new QVBoxLayout(canvasActionContainer);
    actionLayout->setContentsMargins(0, 0, 0, 0);
    actionLayout->setSpacing(15); 
    actionLayout->setSizeConstraint(QLayout::SetNoConstraint); 
    
    btnAutoLayout = new QPushButton("Automatic layout");
    btnMoveMode   = new QPushButton("Free Drag：OFF");
    
    btnAutoLayout->setMinimumHeight(0);
    btnMoveMode->setMinimumHeight(0);
    canvasActionContainer->setMinimumHeight(0);

    actionLayout->addWidget(btnAutoLayout);
    actionLayout->addWidget(btnMoveMode);

    QPushButton* btnCreateFolder = new QPushButton("New Folder");
    QPushButton* btnCreateFile   = new QPushButton("New File");
    QPushButton* btnRename       = new QPushButton("Rename");
    QPushButton* btnDelete       = new QPushButton("Delete");


    auto makePremiumStyle = [](const QString& colorHex, int r, int g, int b) {
        return QString(
            "QPushButton { background-color: rgba(%2, %3, %4, 0.12); color: %1; border: 1px solid rgba(%2, %3, %4, 0.3); border-radius: 8px; padding: 12px; font-weight: bold; }"
            "QPushButton:hover { background-color: %1; color: #11111b; border: 1px solid %1; }"
        ).arg(colorHex).arg(r).arg(g).arg(b);
    };


    btnToggleView->setStyleSheet(makePremiumStyle("#cba6f7", 203, 166, 247));   
    

    QString catppuccinGreen = makePremiumStyle("#a6e3a1", 166, 227, 161);
    btnAutoLayout->setStyleSheet(catppuccinGreen);    
    btnMoveMode->setStyleSheet(catppuccinGreen);     
    

    QString catppuccinBlue = makePremiumStyle("#89b4fa", 137, 180, 250);
    btnCreateFolder->setStyleSheet(catppuccinBlue); 
    btnCreateFile->setStyleSheet(catppuccinBlue);     
    btnRename->setStyleSheet(catppuccinBlue);
    

    btnDelete->setStyleSheet(makePremiumStyle("#f38ba8", 243, 139, 168));       

    btnLayout->addWidget(btnToggleView);
    btnLayout->addWidget(canvasActionContainer); 
    btnLayout->addWidget(btnCreateFolder);
    btnLayout->addWidget(btnCreateFile);
    btnLayout->addWidget(btnRename);
    btnLayout->addWidget(btnDelete);
    btnLayout->addStretch();


    viewStack = new QStackedWidget(this);

    scene = new QGraphicsScene(this);
    scene->setSceneRect(-5000, -5000, 10000, 10000);
    canvasView = new CustomView(scene, this);
    canvasView->setRenderHint(QPainter::Antialiasing);
    canvasView->setStyleSheet("background-color: transparent; border: none;");
    canvasView->setDragMode(QGraphicsView::ScrollHandDrag);
    canvasView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    canvasView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    viewStack->addWidget(canvasView); 


    QWidget* listContainer = new QWidget(this);
    QVBoxLayout* listLayout = new QVBoxLayout(listContainer);
    listLayout->setContentsMargins(30, 30, 30, 30);
    listLayout->setSpacing(15);

    searchBar = new QLineEdit(this);
    searchBar->setPlaceholderText("🔍 Search files or folders..."); 
    searchBar->setStyleSheet(
        "QLineEdit { background-color: #1e1e2e; color: #cdd6f4; border: 1px solid #313244; border-radius: 6px; padding: 12px; font-size: 14px; font-weight: bold; }"
        "QLineEdit:focus { border: 1px solid #89b4fa; }"
    );

    fileTable = new QTableWidget(this);
    fileTable->setColumnCount(4);
    fileTable->setHorizontalHeaderLabels({"Name", "Type", "Size", "Modified Time"}); 
    fileTable->setStyleSheet(
        "QTableWidget { background-color: #1e1e2e; color: #a6adc8; gridline-color: transparent; border: 1px solid #313244; border-radius: 6px; outline: 0; }"
        "QTableWidget::item { border-bottom: 1px solid #313244; padding: 10px; }"
        "QTableWidget::item:selected { background-color: rgba(137, 180, 250, 0.15); color: #89b4fa; font-weight: bold; }"
        "QHeaderView::section { background-color: #181825; color: #bac2de; padding: 12px; border: none; border-bottom: 1px solid #313244; font-size: 12px; font-weight: bold; text-transform: uppercase; }"
    );
    fileTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    fileTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    fileTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    fileTable->verticalHeader()->setVisible(false);
    fileTable->setShowGrid(false);

    listLayout->addWidget(searchBar);
    listLayout->addWidget(fileTable);
    viewStack->addWidget(listContainer); 

    mainLayout->addWidget(controlPanel);
    mainLayout->addWidget(viewStack);
    setCentralWidget(centralWidget);
    
    connect(btnToggleView, &QPushButton::clicked, this, &MainWindow::onToggleView);
    connect(btnAutoLayout, &QPushButton::clicked, this, &MainWindow::autoLayoutTree);
    connect(btnMoveMode, &QPushButton::clicked, this, &MainWindow::onToggleLayoutMode);
    connect(btnCreateFolder, &QPushButton::clicked, this, &MainWindow::onCreateFolder);
    connect(btnCreateFile, &QPushButton::clicked, this, &MainWindow::onCreateFile);
    connect(btnRename, &QPushButton::clicked, this, &MainWindow::onRenameItem);
    connect(btnDelete, &QPushButton::clicked, this, &MainWindow::onDeleteItem);
}

void MainWindow::setupConnections() {
    connect(scene, &QGraphicsScene::selectionChanged, this, &MainWindow::onSelectionChanged);
    connect(searchBar, &QLineEdit::textChanged, this, &MainWindow::filterListView);
}

void MainWindow::onToggleLayoutMode() {
    isFreeMoveEnabled = !isFreeMoveEnabled;
    
    if (isFreeMoveEnabled) {
        btnMoveMode->setText("Free Drag：ON");
    } else {
        btnMoveMode->setText("Free Drag：OFF");
        autoLayoutTree(); 
    }

    for (auto* item : scene->items()) {
        if (auto* vNode = dynamic_cast<VisualNode*>(item)) {
            vNode->setFlag(QGraphicsItem::ItemIsMovable, isFreeMoveEnabled);
        }
    }
}

void MainWindow::autoLayoutTree() {
    if (!rootVisualNode) return;
    float currentX = 0.0f;
    layoutNode(rootVisualNode, 0, currentX);

    scene->update(); 
    QApplication::processEvents(); 

    QRectF treeBounds = scene->itemsBoundingRect();
    if(!treeBounds.isEmpty()) {
        scene->setSceneRect(treeBounds.adjusted(-1000, -1000, 1000, 1000));
        
        canvasView->resetTransform();
        
        double baseScale = 1.15; 
        canvasView->scale(baseScale, baseScale);
        canvasView->centerOn(treeBounds.center()); 

        double viewWidth = canvasView->viewport()->width();
        double viewHeight = canvasView->viewport()->height();
        
        if (viewWidth > 50 && viewHeight > 50) {
            double safetyMargin = 180.0 * baseScale; 
            double treeWidth = (treeBounds.width() * baseScale) + safetyMargin;
            double treeHeight = (treeBounds.height() * baseScale) + safetyMargin;

            if (treeWidth > viewWidth || treeHeight > viewHeight) {
                double scaleX = viewWidth / treeWidth;
                double scaleY = viewHeight / treeHeight;
                double idealScale = std::min(scaleX, scaleY);
                canvasView->scale(idealScale, idealScale);
            }
        }
    }
}

void MainWindow::layoutNode(VisualNode* node, int depth, float& currentX) {
    constexpr float nodeSpacing = 280.0f;
    if (node->getChildNodes().empty()) {
        node->setPos(currentX, depth * 120);
        currentX += nodeSpacing;
    } else {
        float startX = currentX;
        for (VisualNode* child : node->getChildNodes()) {
            layoutNode(child, depth + 1, currentX);
        }
        float endX = currentX - nodeSpacing;
        node->setPos((startX + endX) / 2.0f, depth * 120);
    }
}

void MainWindow::onToggleView() {
    if (viewStack->currentIndex() == 0) {
        refreshListView();
        viewStack->setCurrentIndex(1);

        QPropertyAnimation *anim = new QPropertyAnimation(canvasActionContainer, "maximumHeight");
        anim->setDuration(300);
        anim->setStartValue(canvasActionContainer->height());
        anim->setEndValue(0);
        anim->setEasingCurve(QEasingCurve::OutQuart);
        anim->start(QAbstractAnimation::DeleteWhenStopped);

    } else {
        viewStack->setCurrentIndex(0);

        int singleHeight = btnToggleView->height() > 0 ? btnToggleView->height() : 45;
        int targetHeight = (singleHeight * 2) + 15;
        
        QPropertyAnimation *anim = new QPropertyAnimation(canvasActionContainer, "maximumHeight");
        anim->setDuration(300);
        anim->setStartValue(0);
        anim->setEndValue(targetHeight); 
        anim->setEasingCurve(QEasingCurve::OutQuart);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void MainWindow::refreshListView() {
    fileTable->setRowCount(0);
    searchBar->clear();
    int selectedRow = -1;

    auto const addNodeToTable = [this, &selectedRow](auto& self, VisualNode* vNode, int depth) -> void {
        if (!vNode) return;
        auto node = vNode->getFSNode();
        int row = fileTable->rowCount();
        fileTable->insertRow(row);
        if (vNode == selectedVisualNode) {
            selectedRow = row;
        }

        std::string indent = std::string(depth * 4, ' ');
        std::string prefix = (node->getType() == NodeType::Folder) ? "📁 " : "📄 ";
        
        auto* nameItem = new QTableWidgetItem(QString::fromStdString(indent + prefix + node->getName()));
        nameItem->setData(Qt::UserRole, QVariant::fromValue(reinterpret_cast<qlonglong>(vNode)));
        
        fileTable->setItem(row, 0, nameItem);
        fileTable->setItem(row, 1, new QTableWidgetItem(node->getType() == NodeType::Folder ? "Folder" : "File")); 
        fileTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(node->getSizeString()))); 
        fileTable->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(node->getTimeString()))); 

        for (VisualNode* child : vNode->getChildNodes()) {
            self(self, child, depth + 1);
        }
    };

    if (rootVisualNode) {
        addNodeToTable(addNodeToTable, rootVisualNode, 0);
    }

    if (fileTable->rowCount() > 0) {
        if (selectedRow < 0) {
            selectedRow = 0;
        }
        fileTable->setCurrentCell(selectedRow, 0);
        fileTable->selectRow(selectedRow);
    }
}

void MainWindow::filterListView(const QString& query) {
    for (int i = 0; i < fileTable->rowCount(); ++i) {
        QTableWidgetItem* nameItem = fileTable->item(i, 0);
        if (nameItem) {
            bool match = nameItem->text().contains(query, Qt::CaseInsensitive);
            fileTable->setRowHidden(i, !match);
        }
    }
}

void MainWindow::onSelectionChanged() {
    auto selectedItems = scene->selectedItems();
    selectedVisualNode = selectedItems.isEmpty() ? nullptr : dynamic_cast<VisualNode*>(selectedItems.first());
}

void MainWindow::onCreateFolder() {
    VisualNode* targetNode = nullptr;

    if (viewStack->currentIndex() == 0) {
        targetNode = selectedVisualNode; 
    } else {
        int currentRow = fileTable->currentRow(); 
        if (currentRow >= 0) {
            QTableWidgetItem* nameItem = fileTable->item(currentRow, 0);
            if (nameItem) {
                qlonglong ptrVal = nameItem->data(Qt::UserRole).toLongLong();
                targetNode = reinterpret_cast<VisualNode*>(ptrVal);
            }
        }
    }

    if (!targetNode || targetNode->getFSNode()->getType() != NodeType::Folder) {
        QMessageBox::warning(this, "FAHHHHHHHHHHHHHHHHHH", "Please select a [Folder] node first!");
        return;
    }

    bool ok;
    QString name = QInputDialog::getText(this, "Create new folder", "Please enter a name:                       ", QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty()) return;
    if (!isValidItemName(name)) {
        QMessageBox::warning(this, "Create folder", "Please enter a valid folder name.");
        return;
    }

    auto parentFolder = std::dynamic_pointer_cast<FolderNode>(targetNode->getFSNode());
    if (parentFolder) {
        try {
            if (fs::exists(parentFolder->getPath() / name.toStdString())) {
                QMessageBox::warning(this, "Create folder", "An item with the same name already exists.");
                return;
            }
            auto childData = parentFolder->createChildFolder(name.toStdString());
            VisualNode* childNode = new VisualNode(childData);
            childNode->setFlag(QGraphicsItem::ItemIsMovable, isFreeMoveEnabled);
            scene->addItem(childNode);

            childNode->setParentNode(targetNode);
            targetNode->addChildNode(childNode);

            CanvasEdge* edge = new CanvasEdge(targetNode, childNode);
            scene->addItem(edge);
            targetNode->addEdge(edge);
            childNode->addEdge(edge);

            if (!isFreeMoveEnabled) autoLayoutTree();
            if (viewStack->currentIndex() == 1) refreshListView();
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Create folder failed", e.what());
        }
    }
}

void MainWindow::onCreateFile() {
    VisualNode* targetNode = nullptr;

    if (viewStack->currentIndex() == 0) {
        targetNode = selectedVisualNode; 
    } else {
        int currentRow = fileTable->currentRow(); 
        if (currentRow >= 0) {
            QTableWidgetItem* nameItem = fileTable->item(currentRow, 0);
            if (nameItem) {
                qlonglong ptrVal = nameItem->data(Qt::UserRole).toLongLong();
                targetNode = reinterpret_cast<VisualNode*>(ptrVal);
            }
        }
    }

    if (!targetNode || targetNode->getFSNode()->getType() != NodeType::Folder) {
        QMessageBox::warning(this, "FAHHHHHHHHHHHHHHHHHH", "Please select a [Folder] node first!");
        return;
    }

    bool ok;
    QString name = QInputDialog::getText(this, "Create new file", "Please enter a name:                       ", QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty()) return;
    if (!isValidItemName(name)) {
        QMessageBox::warning(this, "Create file", "Please enter a valid file name.");
        return;
    }

    auto parentFolder = std::dynamic_pointer_cast<FolderNode>(targetNode->getFSNode());
    if (parentFolder) {
        if (fs::exists(parentFolder->getPath() / name.toStdString())) {
            QMessageBox::warning(this, "Create file", "An item with the same name already exists.");
            return;
        }
        int size = QInputDialog::getInt(this, "Set file size", "Size (bytes):", 0, 0, 1073741824, 1, &ok);
        if (!ok) return;
        try {
            auto childData = parentFolder->createChildFile(name.toStdString(), static_cast<uintmax_t>(size));
            VisualNode* childNode = new VisualNode(childData);
            childNode->setFlag(QGraphicsItem::ItemIsMovable, isFreeMoveEnabled);
            scene->addItem(childNode);

            childNode->setParentNode(targetNode);
            targetNode->addChildNode(childNode);

            CanvasEdge* edge = new CanvasEdge(targetNode, childNode);
            scene->addItem(edge);
            targetNode->addEdge(edge);
            childNode->addEdge(edge);

            if (!isFreeMoveEnabled) autoLayoutTree();
            if (viewStack->currentIndex() == 1) refreshListView();
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Create file failed", e.what());
        }
    }
}

void MainWindow::onRenameItem() {
    VisualNode* targetNode = nullptr;

    if (viewStack->currentIndex() == 0) {
        targetNode = selectedVisualNode;
    } else {
        int currentRow = fileTable->currentRow();
        if (currentRow >= 0) {
            QTableWidgetItem* nameItem = fileTable->item(currentRow, 0);
            if (nameItem) {
                qlonglong ptrVal = nameItem->data(Qt::UserRole).toLongLong();
                targetNode = reinterpret_cast<VisualNode*>(ptrVal);
            }
        }
    }

    if (!targetNode) {
        QMessageBox::warning(this, "Rename", "Please select an item to rename first.");
        return;
    }
    if (targetNode == rootVisualNode) {
        QMessageBox::warning(this, "Rename", "The root directory cannot be renamed.");
        return;
    }

    bool ok;
    QString oldName = QString::fromStdString(targetNode->getFSNode()->getName());
    QString newName = QInputDialog::getText(this, "Rename", "Please enter a new name:", QLineEdit::Normal, oldName, &ok);
    if (!ok || newName.isEmpty() || newName == oldName) return;
    if (!isValidItemName(newName)) {
        QMessageBox::warning(this, "Rename", "Please enter a valid file or folder name.");
        return;
    }

    try {
        targetNode->getFSNode()->renameTo(newName.toStdString());
        targetNode->update();
        if (viewStack->currentIndex() == 1) refreshListView();
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Rename failed", e.what());
    }
}

void MainWindow::deleteNodeRecursively(VisualNode* node) {
    if (!node) return;
    auto childrenCopy = node->getChildNodes(); 
    for (VisualNode* child : childrenCopy) {
        deleteNodeRecursively(child);
    }
    auto edgesCopy = node->getEdges();
    for (CanvasEdge* edge : edgesCopy) {
        scene->removeItem(edge);
        VisualNode* src = edge->getSource();
        VisualNode* dst = edge->getDest();
        if (src && src != node) src->removeEdge(edge);
        if (dst && dst != node) dst->removeEdge(edge);
        delete edge;
    }
    node->clearEdges();
    if (node->getParentNode()) node->getParentNode()->removeChildNode(node);
    if (selectedVisualNode == node) selectedVisualNode = nullptr;
    scene->removeItem(node);
    delete node;
}

void MainWindow::onDeleteItem() {
    VisualNode* targetNode = nullptr;

    if (viewStack->currentIndex() == 0) {
        targetNode = selectedVisualNode; 
    } else {
        int currentRow = fileTable->currentRow(); 
        if (currentRow >= 0) {
            QTableWidgetItem* nameItem = fileTable->item(currentRow, 0);
            if (nameItem) {
                qlonglong ptrVal = nameItem->data(Qt::UserRole).toLongLong();
                targetNode = reinterpret_cast<VisualNode*>(ptrVal);
            }
        }
    }

    if (!targetNode) {
        QMessageBox::warning(this, "FAHHHHHHHHHHHHHHHHHH", "Please select the item you want to delete first!");
        return;
    }
    if (targetNode == rootVisualNode) {
        QMessageBox::warning(this, "Cannot be deleted!", "Unable to delete root directory!");
        return;
    }

    auto reply = QMessageBox::question(this, "Confirm deletion", "This will simultaneously delete physical files and subfolders on the hard drive. Are you sure?", QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No) return;

    try {
        fs::remove_all(targetNode->getFSNode()->getPath());
        deleteNodeRecursively(targetNode);
        if (!isFreeMoveEnabled) autoLayoutTree();
        if (viewStack->currentIndex() == 1) refreshListView();
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", e.what());
    }
}
