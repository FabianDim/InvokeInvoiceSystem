#pragma once

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }  // forward declaration from the .ui-generated header
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    

private:
    QMenu* fileMenu;
    QAction* newAct;
    QAction* openAct;
    QAction* saveAct;
    void createMenus();
};
