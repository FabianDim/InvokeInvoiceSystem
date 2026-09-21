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
#include <QLocale>
#include <cmath>

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
        const bool required = field_required(field);
        input->setProperty("required", required);
        auto label_text = resource_ == "stock" && field == "Quantity" ? QString("Stock on hand") : field;
        auto* label = new QLabel(label_text + (required ? ":" : " (optional):"), this);
        label->setBuddy(input);
        if (resource_ == "stock" && (field == "Quantity" || field == "Margin"))
            input->setToolTip("Leave blank to use 0.");
        if (resource_ == "stock" && field == "Unit")
            input->setToolTip("Leave blank to use each.");
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
        form_layout_->addRow(UiStyle::label("Business logo (optional):", content), row);
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

bool ManagementForm::field_required(const QString& field) const {
    if (resource_ == "business")
        return field == "Business name";
    if (resource_ == "client")
        return field == "Name";
    if (resource_ == "stock")
        return field == "Name" || field == "Price";
    return true;
}

void ManagementForm::submit_form() {
    for (int index = 0; index < inputs_.size(); ++index) {
        if (field_required(field_names_[index]) && inputs_[index]->text().trimmed().isEmpty()) {
            status_label_->setText("Please enter " + field_names_[index].toLower() + ".");
            inputs_[index]->setFocus();
            return;
        }
    }
    QJsonObject data;
    for (int index = 0; index < inputs_.size(); ++index) {
        const QString key = field_names_[index];
        data[key] = inputs_[index]->text().trimmed();
    }
    if (resource_ == "business" || resource_ == "client") {
        QStringList address;
        for (const auto& field : {"Street address", "City", "State or province", "Country", "Postcode"}) {
            const auto part = data.value(field).toString();
            if (!part.isEmpty())
                address.append(part);
        }
        data["Address"] = address.join(", ");
    }
    if (resource_ == "stock") {
        // Send numbers as JSON numbers in both modes; MongoDB does not parse JSON strings as numbers.
        for (const auto& field : {"Quantity", "Price", "Margin"}) {
            const auto text = data.value(field).toString();
            bool valid = true;
            const double value = text.isEmpty() ? 0 : QLocale().toDouble(text, &valid);
            if (!valid || !std::isfinite(value) || value < 0 ||
                (QString(field) == "Quantity" && (value > 1'000'000 || std::floor(value) != value))) {
                set_status(QString(field) == "Quantity"
                    ? "Stock on hand must be a whole number from 0 to 1,000,000, or left blank."
                    : QString("Enter a valid, non-negative %1.").arg(QString(field).toLower()));
                return;
            }
            data[field] = value;
        }
        if (data.value("Unit").toString().isEmpty())
            data["Unit"] = "each";
    }
    if (resource_ == "business") {
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

void App::Views::ManagementForm::set_offline(bool offline) {
    for (auto* input : inputs_)
        input->clear();
    if (logo_path_)
        logo_path_->clear();
    logo_data_.clear();
    findChild<QPushButton*>("register_button_")->setText(offline ? "Use in demo" : "Save");
    set_status(offline ? "This record is only kept for the current demo." : "");
}
