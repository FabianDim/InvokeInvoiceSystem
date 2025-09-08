#pragma once
#include "pch.h"
#include <QWidget>

namespace App {
namespace Views {
class Dashboard : public QWidget {
    Q_OBJECT
  public:
    Dashboard(QWidget* parent = nullptr);
    ~Dashboard() = default;

  private:
    void createPageLayout();
};
} // namespace Views
} // namespace App