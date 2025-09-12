#pragma once

#include <QWidget>
#include <QVBoxLayout>
namespace App::Views {
class NewInvoiceCreation : public QWidget {
    Q_OBJECT
  public:
    NewInvoiceCreation(QWidget* parent = nullptr);
    ~NewInvoiceCreation() = default;

  private:
    void create_page_layout();

    QWidget* parent_widget_;
    QWidget* form_layout_;
};
} // namespace App::Views