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
#include "Infrastructure/Enums/RouteEnums.h"
class StockItem;
class Invoice;
namespace App::Views {
class NewInvoiceStock : public QWidget {
    Q_OBJECT
  public:
    NewInvoiceStock(QWidget* parent = nullptr);
    QJsonArray stock_items;
    QMap<QString, QWidget*> invoice_body_form_fields;

  private:
    void create_page_layout();
    QLayout* create_item_entry_form();
    QPushButton* create_invoice_pdf;
    QVector<QLabel*> item_labels_;

  public:
  signals:
    void find_stock();
    void add_item_to_invoice(const QJsonObject& doc);

    void add_item_list_to_invoice(const QJsonDocument& doc);
    void invoice_navigation(Page page);

  public slots:
    void populate_stock_list(const QJsonDocument& list);
    void reset_invoice();

    // private slots:
    //   void on_add_item_clicked();
};
} // namespace App::Views
