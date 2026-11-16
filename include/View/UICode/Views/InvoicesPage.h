#pragma once
#include <QWidget>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDeadlineTimer>
#include "Infrastructure/Enums/RouteEnums.h"
class QTableWidget;
class QPushButton;
class QLabel;

namespace App::Views {
class InvoicesPage : public QWidget {
    Q_OBJECT
  public:
    explicit InvoicesPage(QWidget* parent = nullptr);
  public slots:
    void business_selected(const QJsonObject& business);
    void load_invoices();
    void populate_invoices(const QJsonDocument& document);
    void set_error(const QString& message);
    void payment_updated(const QString& id, bool paid);
    void export_finished(const QString& path);
    void export_wait(int seconds);
  signals:
    void navigate_to(Page page);
    void invoices_requested(int offset);
    void payment_requested(const QString& id, bool paid);
    void export_requested(const QString& id, const QString& path);
  private:
    QString business_id_;
    QJsonArray records_;
    int offset_ = 0;
    bool more_ = false;
    bool busy_ = false;
    QDeadlineTimer wait_{0};
    QTableWidget* table_;
    QLabel* status_;
    QLabel* detail_;
    QLabel* selection_;
    QPushButton* refresh_;
    QPushButton* previous_;
    QPushButton* next_;
    QPushButton* payment_;
    QPushButton* export_;
    void update_actions();
    void render();
    void request_page(int offset);
    QJsonObject selected() const;
};
}
