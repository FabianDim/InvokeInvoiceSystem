#pragma once

#include <QWidget>
#include "pch.h"
#include <qboxlayout.h>

namespace App::Views {
class ItemsPage : public QWidget {
    Q_OBJECT
  public:
    explicit ItemsPage(QWidget* parent = nullptr);
    ~ItemsPage() = default;

  private:
    void create_page_layout();
    QWidget* parent_widget_;
    QVBoxLayout* main_layout_;
};
} // namespace App::Views