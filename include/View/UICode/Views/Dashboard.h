#pragma once
#include "pch.h"
#include <QWidget>
#include <QButtonGroup>
#include <qlayout.h>
#include <Enums/RouteEnums.h>
#include <qpushbutton.h>

namespace App {
namespace Views {
class Dashboard : public QWidget {
    Q_OBJECT
  public:
    Dashboard(QWidget* parent = nullptr);
    ~Dashboard() = default;

  private:
    void create_page_layout();
    QButtonGroup* button_group_;
    QHBoxLayout* main_layout_;
    void button_connections();
    QPushButton* invoice_button;

  signals:
    void dash_navigation(Page page);
};
} // namespace Views
} // namespace App