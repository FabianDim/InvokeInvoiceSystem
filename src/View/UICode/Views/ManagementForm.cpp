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
#include "Utils/BusinessLogo.h"
#include <QFileDialog>
#include <QFile>
#include <QUrl>

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
    if (resource_ == "business") {
        logo_path_ = new QLineEdit(content);
        logo_path_->setObjectName("business_logo_path");
        logo_path_->setProperty("role", "input");
        logo_path_->setReadOnly(true);
        logo_path_->setPlaceholderText("Optional PNG or JPEG, up to 5 MB");
        auto* browse = new QPushButton("Choose logo", content);
        auto* remove = new QPushButton("Remove", content);
        UiStyle::button(browse);
        UiStyle::button(remove);
        auto* row = new QHBoxLayout;
        row->setSpacing(8);
        row->addWidget(logo_path_, 1);
        row->addWidget(browse);
        row->addWidget(remove);
        form_layout_->addRow(UiStyle::label("Business logo:", content), row);
        connect(browse, &QPushButton::clicked, this, [this]() {
            const auto path = QFileDialog::getOpenFileName(this, "Choose business logo", {}, "Images (*.png *.jpg *.jpeg)");
            if (!path.isEmpty())
                select_logo(path);
        });
        connect(remove, &QPushButton::clicked, this, [this]() {
            logo_path_->clear();
            logo_data_.clear();
        });
    }

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
        if (input->text().trimmed().isEmpty() && input->placeholderText() != "Website") {
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
        const auto website = data.value("Website").toString();
        if (!website.isEmpty()) {
            const auto url = QUrl::fromUserInput(website);
            if (!url.isValid() || url.host().isEmpty() || (url.scheme() != "https" && url.scheme() != "http")) {
                set_status("Enter a valid business website, such as https://example.com.");
                return;
            }
            data["Website"] = url.toString();
        }
        data["LogoPath"] = logo_path_->text();
        data["LogoData"] = QString::fromLatin1(logo_data_.toBase64());
    }
    emit submit_resource(resource_, QJsonDocument(data));
    if (logo_path_) {
        logo_path_->clear();
        logo_data_.clear();
    }

    for (int index = 0; index < inputs_.size(); ++index) {
        inputs_[index]->clear();
        if (!inputs_[index]->text().isEmpty()) {
            inputs_[index]->setText("");
        }
    }
    return;
}

void ManagementForm::select_logo(const QString& path) {
    if (!logo_path_)
        return;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly) || file.size() > BusinessLogo::maximum_bytes) {
        set_status("Choose a readable PNG or JPEG logo no larger than 5 MB.");
        return;
    }
    QByteArray png;
    QString error;
    if (!BusinessLogo::normalise(file.read(BusinessLogo::maximum_bytes + 1), png, error)) {
        set_status(error);
        return;
    }
    logo_data_ = png;
    logo_path_->setText(path);
    set_status("Logo selected.");
}

void ManagementForm::set_status(const QString& message) {
    status_label_->setText(message);
}
} // namespace App::Views
