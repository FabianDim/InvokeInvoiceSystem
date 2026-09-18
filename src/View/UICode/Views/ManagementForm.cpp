#include "View/UICode/Views/ManagementForm.h"
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include "View/UiStyle.h"

namespace App::Views {
ManagementForm::ManagementForm(const QString& title,
                               const QString& resource,
                               const QVector<QString>& fields,
                               QWidget* parent)
    : QWidget(parent), resource_(resource) {
    auto* content = UiStyle::page_content(this);
    auto* layout = new QVBoxLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);
    auto* heading = new QLabel(title, this);
    heading->setObjectName("titleLabel");
    heading->setProperty("role", "title");
    heading->setAlignment(Qt::AlignCenter);
    layout->addWidget(heading);

    form_layout_ = new QFormLayout;
    form_layout_->setFormAlignment(Qt::AlignCenter);
    form_layout_->setSpacing(10);
    form_layout_->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    for (const auto& field : fields) {
        auto* input = new QLineEdit(this);
        input->setObjectName("form_input");
        input->setProperty("role", "input");
        input->setPlaceholderText(field);
        auto* label = new QLabel(field + ":", this);
        label->setObjectName("form_label");
        label->setProperty("role", "field");
        form_layout_->addRow(label, input);
        inputs_.append(input);
        field_names_.append(field);
    }
    layout->addLayout(form_layout_);

    auto* buttons = new QHBoxLayout;
    auto* save = new QPushButton("Save", this);
    auto* back = new QPushButton("Back", this);
    save->setObjectName("register_button_");
    back->setObjectName("login_button_");
    UiStyle::button(save, "primary");
    UiStyle::button(back);
    buttons->setSpacing(12);
    buttons->addWidget(back, 1);
    buttons->addWidget(save, 1);
    layout->addLayout(buttons);

    status_label_ = new QLabel(this);
    status_label_->setProperty("role", "status");
    status_label_->setWordWrap(true);
    status_label_->setAlignment(Qt::AlignCenter);
    layout->addWidget(status_label_);

    connect(save, &QPushButton::clicked, this, &ManagementForm::submit_form);
    connect(back, &QPushButton::clicked, this, [this]() { emit navigate_to(Page::Dashboard); });
}

void ManagementForm::submit_form() {
    for (auto* input : inputs_) {
        if (input->text().trimmed().isEmpty()) {
            status_label_->setText("Please complete every field.");
            return;
        }
    }
    QJsonObject data;
    for (int index = 0; index < inputs_.size(); ++index) {
        const QString key = field_names_[index];
        data[key] = inputs_[index]->text().trimmed();
    }
    if (resource_ == "business") {
        const QString address =
            data.value("Street address").toString().trimmed() + ", " + data.value("City").toString().trimmed() + ", " +
            data.value("State or province").toString().trimmed() + ", " + data.value("Country").toString().trimmed() +
            " " + data.value("Postcode").toString().trimmed();
        data["Address"] = address;
    }
    emit submit_resource(resource_, QJsonDocument(data));

    for (int index = 0; index < inputs_.size(); ++index) {
        inputs_[index]->clear();
        if (!inputs_[index]->text().isEmpty()) {
            inputs_[index]->setText("");
        }
    }
    return;
}

void ManagementForm::set_status(const QString& message) {
    status_label_->setText(message);
}
} // namespace App::Views
