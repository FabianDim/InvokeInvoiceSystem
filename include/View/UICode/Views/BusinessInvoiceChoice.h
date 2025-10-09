#pragma once
#include "pch.h"
#include "Utils/FormUtils.h"
#include <QGridLayout>
#include <QLineEdit>
#include <qlabel.h>
#include <QPushButton>
#include <QJsonArray>
#include <vector>
#include <Utils/FormUtils.h>
#include <qdatetime.h>
#include <qmetatype.h>
#include <qnamespace.h>
#include <qboxlayout.h>
#include <qcombobox.h>
#include <qdatetimeedit.h>
#include <qgridlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qwidget.h>
#include <qjsonobject.h>
#include <qpointer.h>
#include <QLayout>
#include "Domain/Stock/StockItem.h"
class StockItem;
class Invoice;
namespace App::Views {
class BusinessInvoieChoice : public QWidget {
    Q_OBJECT
  public:
    BusinessInvoiceChoice(QWidget* parent = nullptr);
    Invoice invoice_;

  private:
    void create_page_layout();
    QLayout* create_item_entry_form();
    QWidget* parent_widget_;
    QWidget* item_form_layout_;

  signals:
    void find_businesses();

    // private slots:
    //   void on_add_item_clicked();
};
} // namespace App::Views