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
#include "Domain/Invoices/Invoice.h"
class StockItem;
class Invoice;
namespace App::Views {
class BusinessInvoiceChoice : public QWidget {
    Q_OBJECT
  public:
    BusinessInvoiceChoice(QWidget* parent = nullptr);
    Invoice invoice_;

  private:
    void create_form_layout();
    QWidget* parent_widget_;
    QWidget* form_layout_;
    QComboBox* business_select;
    void set_business_list(const QJsonDocument& list);
  signals:
    void find_businesses();

  public slots:
    void populate_business_list(const QJsonDocument& list);

    // private slots:
    //   void on_add_item_clicked();
};
} // namespace App::Views