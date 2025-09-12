#include "View/UICode/Views/NewInvoiceCreation.h"
#include <QLineEdit>
#include <qlabel.h>
using namespace App::Views;
NewInvoiceCreation::NewInvoiceCreation(QWidget* parent) {
    parent_widget_ = new QWidget(this);
    form_layout_ = new QWidget(parent_widget_);
    create_page_layout();
}

void NewInvoiceCreation::create_page_layout() {
    QGridLayout* main_form_layout = new QGridLayout(form_layout_);
    main_form_layout->setAlignment(Qt::AlignCenter);

    // create inputs and labels
    QLabel* client_name_label = new QLabel("Client Name:", form_layout_);
    QLineEdit* client_name_input = new QLineEdit(form_layout_);

    main_form_layout->addWidget(client_name_label, 0, 0);
    main_form_layout->addWidget(client_name_input, 0, 1);

    form_layout_->setLayout(main_form_layout);
    this->setLayout(main_form_layout);
}
