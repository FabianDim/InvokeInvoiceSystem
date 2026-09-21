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
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
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
    QWidget* entry_widget_;
    QWidget* saved_fields_;
    QComboBox* item_source_;
    QLineEdit* new_name_;
    QCheckBox* save_stock_;
    QSpinBox* stock_on_hand_;
    QDoubleSpinBox* margin_;
    QLineEdit* unit_;
    QLineEdit* keywords_;
    QLabel* status_label_;
    QPushButton* back_button_;
    bool saving_stock_ = false;
    bool offline_ = false;
    quint64 stock_request_ = 0;
    QString business_id_;
    void append_item(const QJsonObject& item);
    void update_item_source();
    void set_saving_stock(bool saving);

  public:
  signals:
    void find_stock();
    void add_item_to_invoice(const QJsonObject& doc);

    void add_item_list_to_invoice(const QJsonDocument& doc);
    void invoice_navigation(Page page);
    void save_invoice_stock(const QJsonDocument& item, quint64 request_id);

  public slots:
    void populate_stock_list(const QJsonDocument& list);
    void reset_invoice();
    void set_offline(bool offline);
    void pdf_generated(const QString& path);
    void pdf_failed(const QString& message);
    void business_selected(const QJsonObject& business);
    void stock_saved(const QJsonDocument& item, quint64 request_id);
    void stock_save_failed(const QString& message, quint64 request_id);

    // private slots:
    //   void on_add_item_clicked();
};
} // namespace App::Views
