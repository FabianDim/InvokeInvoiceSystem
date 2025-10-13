#include "View/UICode/Views/BusinessInvoiceChoice.h"
using namespace App::Views;
App::Views::BusinessInvoiceChoice::BusinessInvoiceChoice(QWidget* parent) {
    parent_widget_ = new QWidget(this);
    form_layout_ = new QWidget(parent_widget_);
    create_form_layout();
}

void BusinessInvoiceChoice::create_form_layout() {
    QGridLayout* main_form_layout = new QGridLayout(form_layout_);
    main_form_layout->setAlignment(Qt::AlignCenter);

    QLabel* business_label = new QLabel("Choose your business:", form_layout_);

    business_select = new QComboBox(form_layout_);
    business_label->setObjectName("form_label");
    business_select->setObjectName("form_input");

    QPushButton* continue_button = new QPushButton("Continue", form_layout_);
    continue_button->setObjectName("form_button");
    main_form_layout->addWidget(business_label, 0, 0, Qt::AlignRight);
    main_form_layout->addWidget(business_select, 0, 1);
    main_form_layout->addWidget(continue_button, 1, 0, 1, 2, Qt::AlignCenter);

    form_layout_->setLayout(main_form_layout);
    this->setLayout(main_form_layout);
}

void App::Views::BusinessInvoiceChoice::set_business_list(const QJsonArray& list) {
    for (int i = 0; i < list.size(); i++) {
        try {
            QJsonObject obj = list.at(i).toObject();
            QString business_name = obj["name"].toString();
            business_select->addItem(business_name);
        } catch (...) {
            qErrnoWarning("Error parsing business list JSON object");
        }
    }
}

void App::Views::BusinessInvoiceChoice::populate_business_list(const QJsonArray& list) {
    qDebug() << "Populated business list with" << list.size() << "items";
    set_business_list(list);
}