#include "View/UICode/Views/ManagementForm.h"
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace App::Views {
ManagementForm::ManagementForm(const QString& title, const QVector<QString>& fields, QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    auto* heading = new QLabel(title, this);
    heading->setObjectName("titleLabel");
    heading->setAlignment(Qt::AlignCenter);
    layout->addWidget(heading);

    form_layout_ = new QFormLayout;
    form_layout_->setFormAlignment(Qt::AlignCenter);
    for (const auto& field : fields) {
        auto* input = new QLineEdit(this);
        input->setObjectName("form_input");
        input->setPlaceholderText(field);
        auto* label = new QLabel(field + ":", this);
        label->setObjectName("form_label");
        form_layout_->addRow(label, input);
        inputs_.append(input);
    }
    layout->addLayout(form_layout_);

    auto* buttons = new QHBoxLayout;
    auto* save = new QPushButton("Save", this);
    auto* back = new QPushButton("Back", this);
    save->setObjectName("register_button_");
    back->setObjectName("login_button_");
    buttons->addWidget(save);
    buttons->addWidget(back);
    layout->addLayout(buttons);

    status_label_ = new QLabel(this);
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
    status_label_->setText("Saved successfully.");
}
} // namespace App::Views
