#pragma once
#include "pch.h"
#include <QWidget>
#include <QButtonGroup>
#include <qlayout.h>

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
};
} // namespace Views
} // namespace App