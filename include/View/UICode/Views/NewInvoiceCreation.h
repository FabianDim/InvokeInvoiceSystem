#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <qmap.h>
namespace App::Views {
class NewInvoiceCreation : public QWidget {
    Q_OBJECT
  public:
    NewInvoiceCreation(QWidget* parent = nullptr);
    ~NewInvoiceCreation() = default;
    struct FormField {
        enum class Type { LineEdit, ComboBox, DateEdit };

        QString key;
        QString label;
        Type type;

        struct Options {
            QString placeholder;
            bool defaultToday = false;
            bool fromNow = false;
            QString objectName;
            std::function<void(QWidget*)> customise;
        } opt;
    };

  signals:
    void navigate_to(Page page);

  private:
    void create_page_layout();

    void create_form_layout();

    QMap<QString, QWidget*> form_fields_;

    QWidget* parent_widget_;
    QWidget* form_layout_;
};
} // namespace App::Views