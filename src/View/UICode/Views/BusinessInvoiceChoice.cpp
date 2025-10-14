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

    this->setLayout(main_form_layout);
}

void App::Views::BusinessInvoiceChoice::set_business_list(const QJsonDocument& doc) {
    business_select->clear();

    struct Row {
        QString id;
        QString name;
    };
    QVector<Row> rows;

    if (doc.isObject()) {
        const QJsonObject obj = doc.object();
        for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
            const QString id = it.key();
            const QJsonObject biz = it.value().toObject();
            const QString name = biz.value("BusinessName").toString();
        }
    } else if (doc.isArray()) {
        const QJsonArray arr = doc.array();
        for (const QJsonValue& v : arr) {
            const QJsonObject biz = v.toObject();
            const QString name = biz.value("BusinessName").toString();
        }
    } else {
        qWarning() << "Business list JSON is neither object nor array";
        return;
    }

    // Optional: sort by name for a nicer UX
    std::sort(
        rows.begin(), rows.end(), [](const Row& a, const Row& b) { return a.name.localeAwareCompare(b.name) < 0; });

    // Add to combo; stash the ID in item data
    for (const auto& r : rows) {
        business_select->addItem(r.name, r.id);
    }

    qDebug() << "Added" << rows.size() << "businesses to combo box";
}

void App::Views::BusinessInvoiceChoice::populate_business_list(const QJsonDocument& doc) {
    qDebug().noquote() << "Populating business list with data:\n" << doc.toJson(QJsonDocument::Indented);
    set_business_list(doc);
}