#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    QFont font("Microsoft JhengHei", 12);
    QApplication::setFont(font);

    MainWindow window; 
    window.show();
    
    return app.exec();
}