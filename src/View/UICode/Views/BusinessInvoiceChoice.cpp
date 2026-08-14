#include <QAbstractItemView> // Add this include at the top of the file
#include "Application/Business/BusinessRepository.h"
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
    business_select->setDisabled(true);
    continue_button->setDisabled(true);
    business_select->view()->setMinimumWidth(100);
    connect(this, &BusinessInvoiceChoice::finished_loading_biz, this, [this, continue_button]() {
        business_select->setDisabled(false);
        continue_button->setDisabled(false);
    });
    this->setLayout(main_form_layout);

    connect(continue_button, &QPushButton::clicked, this, [this]() {
        emit business_selected();
        emit navigate_to(Page::NewInvoice);
    });

    connect(this, &BusinessInvoiceChoice::business_selected, [this]() {
        const QString id = business_select->currentData().toString();
        try {
            if (rows.contains(id)) {
                const QJsonObject biz = rows[id];
                emit business_chosen(biz);

            } else {
                qWarning() << "Selected business not found in rows map";
            }
        } catch (...) {
            return;
        }
    });
}

void BusinessInvoiceChoice::set_business_list(const QJsonDocument& doc) {
    business_select->clear();
    rows.clear();
    if (!doc.isObject()) {
        qWarning() << "Business list JSON is not object";
        return;
    }

    const QJsonObject root = doc.object();

    struct Row {
        QString name;
        QString id;
    };
    QVector<Row> items;
    items.reserve(root.size());

    for (auto it = root.constBegin(); it != root.constEnd(); ++it) {
        const QString id = it.key();
        const QJsonObject biz = it.value().toObject();
        const QString name = biz.value("BusinessName").toString();

        if (name.isEmpty())
            continue;
        rows.insert(id, biz);
        items.push_back({name, id});
    }

    std::sort(
        items.begin(), items.end(), [](const Row& a, const Row& b) { return a.name.localeAwareCompare(b.name) < 0; });

    for (const auto& r : items) {
        business_select->addItem(r.name, r.id);
    }

    qDebug() << "Added" << items.size() << "businesses to combo box";
    emit finished_loading_biz();
}

void App::Views::BusinessInvoiceChoice::populate_business_list(const QJsonDocument& doc) {
    set_business_list(doc);
}