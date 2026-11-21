#include "Infrastructure/Logging/Logging.h"
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
#include <QComboBox>
#include <QCheckBox>
#include <QShowEvent>
#include <cmath>

namespace App::Views {
ManagementForm::ManagementForm(const Domain::Records::RecordDefinition& definition, QWidget* parent)
    : ManagementForm("Create " + definition.noun(), definition.resource(), definition.form_fields(), parent) {}

ManagementForm::ManagementForm(const Domain::Records::RecordDefinition& definition, const QJsonObject& record, QWidget* parent)
    : ManagementForm("Edit " + definition.noun(), definition.resource(), definition.form_fields(true), parent, record, true) {}

ManagementForm::ManagementForm(const QString& title,
                               const QString& resource,
                               const QVector<QString>& fields,
                               QWidget* parent,
                               const QJsonObject& record,
                               bool editing)
    : QWidget(parent), resource_(resource), original_record_(record), editing_(editing) {
    if (resource != "account") definition_ = &Domain::Records::RecordDefinition::for_resource(resource);
    auto* content = editing_ ? this : UiStyle::page_content(this);
    auto* layout = new QVBoxLayout(content);
    layout->setContentsMargins(editing_ ? 24 : 0, editing_ ? 24 : 0, editing_ ? 24 : 0, editing_ ? 24 : 0);
    layout->setSpacing(12);
    if (editing_) layout->setAlignment(Qt::AlignTop);
    auto* heading = new QLabel(title, this);
    heading->setObjectName("titleLabel");
    heading->setProperty("role", "title");
    heading->setAlignment(Qt::AlignLeft);
    if (!editing_) layout->addWidget(
        UiStyle::label(resource_ == "account" ? "YOUR PROFILE" : "BUSINESS ESSENTIALS", content, "eyebrow"));
    layout->addWidget(heading);
    const auto description = resource_ == "account"    ? "Keep your personal details up to date."
                             : resource_ == "business" ? "The details behind every invoice you send."
                             : resource_ == "client"   ? "Make room for your next working relationship."
                                                       : "Keep your products and pricing in one place.";
    auto* subtitle = UiStyle::label(description, content, "subtitle");
    subtitle->setWordWrap(true);
    if (!editing_) layout->addWidget(subtitle);
    else subtitle->hide();

    form_layout_ = new QFormLayout;
    form_layout_->setFormAlignment(editing_ ? Qt::AlignTop : Qt::AlignCenter);
    form_layout_->setSpacing(10);
    form_layout_->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    for (const auto& field : fields) {
        if (field == "Country") {
            country_ = new QComboBox(this);
            country_->setObjectName("form_country");
            country_->setProperty("role", "input");
            country_->addItem("Choose a country (optional)", "");
            QStringList countries;
            for (const auto& locale :
                 QLocale::matchingLocales(QLocale::AnyLanguage, QLocale::AnyScript, QLocale::AnyTerritory)) {
                if (locale.territory() != QLocale::AnyTerritory)
                    countries.append(QLocale::territoryToString(locale.territory()));
            }
            countries.removeDuplicates();
            countries.sort(Qt::CaseInsensitive);
            for (const auto& country : countries)
                country_->addItem(country, country);
            auto* label = UiStyle::label("Country (optional):", this);
            label->setBuddy(country_);
            form_layout_->addRow(label, country_);
            continue;
        }
        if (field == "Are You Registered for GST") {
            gst_registered_ = new QCheckBox(this);
            gst_registered_->setObjectName("form_gst_registered");
            gst_registered_->setProperty("role", "input");
            if (editing_) gst_registered_->setChecked(Domain::Business::gst_registered(record));
            auto* label = UiStyle::label("Registered for GST:", this);
            label->setBuddy(gst_registered_);
            form_layout_->addRow(label, gst_registered_);
            continue;
        }
        auto* input = new QLineEdit(this);
        const auto* definition = definition_ ? definition_->field(field) : nullptr;
        input->setObjectName(editing_ && definition ? resource_ + "_edit_" + definition->key.toLower() : "form_input");
        input->setProperty("role", "input");
        input->setPlaceholderText(field);
        if (resource_ == "account" && field == "Password")
            input->setEchoMode(QLineEdit::Password);
        const bool required = field_required(field);
        input->setProperty("required", required);
        auto label_text = definition ? definition->label : field;
        if (definition) {
            input->setMaxLength(definition->maximum_length);
            if (editing_) {
                const auto value = record.value(definition->key);
                using Kind = Domain::Records::FieldKind;
                if (definition->kind == Kind::Quantity || definition->kind == Kind::Money || definition->kind == Kind::Percent)
                    input->setText(QLocale().toString(value.toDouble() * (definition->kind == Kind::Percent ? 100 : 1), 'g', 15));
                else if (definition->kind == Kind::Keywords) {
                    QStringList words;
                    for (const auto& word : value.toArray()) words.append(word.toString());
                    input->setText(words.join(", "));
                } else input->setText(value.toString());
            }
        }
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
        invoice_preferences_ = new InvoicePreferencesForm(content, false);
        if (editing_) invoice_preferences_->set_values(record);
        layout->addWidget(invoice_preferences_);
    }
    if (resource_ == "business") {
        logo_path_ = new QLineEdit(content);
        logo_path_->setObjectName("business_logo_path");
        logo_path_->setProperty("role", "input");
        logo_path_->setReadOnly(true);
        logo_path_->setPlaceholderText("Optional PNG or JPEG, up to 5 MB");
        if (editing_) {
            logo_data_ = QByteArray::fromBase64(record.value("LogoData").toString().toLatin1());
            if (!logo_data_.isEmpty()) logo_path_->setText("Current business logo");
        }
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
            const auto path =
                QFileDialog::getOpenFileName(this, "Choose business logo", {}, "Images (*.png *.jpg *.jpeg)");
            if (!path.isEmpty())
                select_logo(path);
        });
        connect(remove, &QPushButton::clicked, this, [this]() {
            logo_path_->clear();
            logo_data_.clear();
        });
    }

    auto* buttons = new QHBoxLayout;
    auto* save = new QPushButton(editing_ ? "Save changes" : "Save", this);
    auto* back = new QPushButton(editing_ ? "Cancel" : "Back", this);
    save->setObjectName(editing_ ? resource_ + "_edit_save" : "register_button_");
    back->setObjectName(editing_ ? resource_ + "_edit_cancel" : "login_button_");
    UiStyle::button(save, "primary");
    UiStyle::button(back);
    buttons->setSpacing(12);
    buttons->addWidget(back, 1);
    buttons->addWidget(save, 1);
    if (resource_ != "account" && !editing_) {
        const auto noun = resource_ == "stock" ? "stock item" : resource_;
        auto* another = new QPushButton("Save and add new " + noun, this);
        another->setObjectName("save_add_new");
        UiStyle::button(another);
        buttons->addWidget(another, 1);
        connect(another, &QPushButton::clicked, this, [this]() {
            add_another_ = true;
            submit_form();
        });
    }
    layout->addLayout(buttons);

    status_label_ = new QLabel(this);
    if (editing_) status_label_->setObjectName("record_edit_status");
    status_label_->setTextFormat(Qt::PlainText);
    status_label_->setProperty("role", "status");
    status_label_->setWordWrap(true);
    status_label_->setAlignment(Qt::AlignCenter);
    layout->addWidget(status_label_);
    for (auto* input : inputs_)
        connect(input, &QLineEdit::textChanged, this, [this]() { if (!saving_) set_status(""); });

    connect(save, &QPushButton::clicked, this, [this]() {
        add_another_ = false;
        submit_form();
    });
    connect(back, &QPushButton::clicked, this, [this]() { emit navigate_to(Page::Dashboard); });
}

bool ManagementForm::field_required(const QString& field) const {
    if (!definition_) return true;
    const auto* entry = definition_->field(field);
    return entry && entry->required;
}

void ManagementForm::submit_form() {
    if (saving_)
        return;
    Infrastructure::Logging::Operation operation(Infrastructure::Logging::ui(), "ManagementForm.submit_form");
    for (int index = 0; index < inputs_.size(); ++index) {
        if (field_required(field_names_[index]) && inputs_[index]->text().trimmed().isEmpty()) {
            qCWarning(Infrastructure::Logging::ui)
                << "event=form.rejected reason=required_field_missing field_index=" << index;
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
    if (!editing_ && (resource_ == "business" || resource_ == "client")) {
        data["Country"] = country_ ? country_->currentData().toString() : QString{};
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
            const bool quantity = QString(field) == "Quantity";
            const double minimum = editing_ && quantity ? -1'000'000 : 0;
            if (!valid || !std::isfinite(value) || value < minimum ||
                (quantity && (value > 1'000'000 || std::floor(value) != value))) {
                qCWarning(Infrastructure::Logging::ui)
                    << "event=form.rejected reason=invalid_stock_number field=" << field;
                set_status(quantity
                               ? (editing_ ? "Stock on hand must be a whole number between -1,000,000 and 1,000,000."
                                           : "Stock on hand must be a whole number from 0 to 1,000,000, or left blank.")
                               : QString("Enter a valid, non-negative %1.").arg(QString(field).toLower()));
                return;
            }
            // Stored margins are fractions; the form accepts percentages.
            data[field] = QString(field) == "Margin" ? value / 100.0 : value;
        }
        if (data.value("Unit").toString().isEmpty())
            data["Unit"] = "each";

    }
    if (resource_ == "business") {
        const auto preferences = invoice_preferences_->values();
        for (auto it = preferences.begin(); it != preferences.end(); ++it) data[it.key()] = it.value();
        data["GSTRegistered"] = gst_registered_ && gst_registered_->isChecked();
        data["Are You Registered for GST"] = data.value("GSTRegistered");
        data["LogoPath"] = logo_path_->text();
        data["LogoData"] = QString::fromLatin1(logo_data_.toBase64());
    }
    if (definition_) {
        auto canonical = definition_->canonical_values(data);
        // Legacy/custom creation forms may omit optional fields.
        for (const auto& field : definition_->fields())
            if (canonical.value(field.key).isUndefined()) canonical[field.key] = "";
        if (definition_->tracks_quantity())
            canonical["ExpectedQuantity"] = editing_ ? original_record_.value("Quantity") : QJsonValue(0);
        try {
            const auto values = definition_->edit_values(canonical);
            if (editing_) {
                data = canonical;
                data["_id"] = original_record_.value("_id");
            } else {
                // Preserve the established creation API field names.
                for (const auto& field : definition_->fields())
                    if (values.value(field.key).isString()) data[field.form_name] = values.value(field.key);
            }
        } catch (const std::exception& error) {
            set_status(QString::fromUtf8(error.what())); return;
        }
    }
    qCDebug(Infrastructure::Logging::ui) << "event=form.validated resource=" << resource_;
    set_saving(true);
    set_status(resource_ == "business" ? "Saving business..." : "Saving...");
    emit submit_resource(resource_, QJsonDocument(data));
}

void ManagementForm::clear_form() {
    if (logo_path_) {
        logo_path_->clear();
        logo_data_.clear();
    }

    for (auto* input : inputs_)
        input->clear();
    if (country_)
        country_->setCurrentIndex(0);
    if (gst_registered_)
        gst_registered_->setChecked(false);
    if (invoice_preferences_) invoice_preferences_->set_values({});
}

void ManagementForm::set_account_details(const QString& first_name, const QString& last_name, const QString& email) {
    if (resource_ != "account" || saving_)
        return;
    const QJsonObject details{{"First name", first_name}, {"Last name", last_name}, {"Email", email}};
    for (int index = 0; index < inputs_.size(); ++index)
        inputs_[index]->setText(details.value(field_names_[index]).toString());
}

void ManagementForm::set_saving(bool saving) {
    saving_ = saving;
    for (auto* button : findChildren<QPushButton*>())
        button->setEnabled(!saving);
    for (auto* input : inputs_)
        input->setEnabled(!saving);
    if (country_)
        country_->setEnabled(!saving);
    if (gst_registered_)
        gst_registered_->setEnabled(!saving);
    if (invoice_preferences_) invoice_preferences_->setEnabled(!saving);
}

void ManagementForm::save_succeeded() {
    if (!saving_)
        return;
    set_saving(false);
    if (editing_) { set_status("Saved."); return; }
    clear_form();
    set_status("");
    if (resource_ != "account" && !add_another_)
        emit navigate_to(Page::Dashboard);
    else
        set_status(resource_ == "account" ? "Account saved." : "Saved. You can add another " + resource_ + ".");
}

void ManagementForm::save_failed(const QString& message) {
    if (!saving_)
        return;
    set_saving(false);
    set_status(message);
}

void ManagementForm::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    if (!saving_)
        set_status("");
}

void ManagementForm::select_logo(const QString& path) {
    Infrastructure::Logging::Operation operation(Infrastructure::Logging::ui(), "ManagementForm.select_logo");
    if (!logo_path_)
        return;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly) || file.size() > BusinessLogo::maximum_bytes) {
        qCWarning(Infrastructure::Logging::ui) << "event=logo.rejected reason=unreadable_or_oversized";
        set_status("Choose a readable PNG or JPEG logo no larger than 5 MB.");
        return;
    }
    QByteArray png;
    QString error;
    if (!BusinessLogo::normalise(file.read(BusinessLogo::maximum_bytes + 1), png, error)) {
        qCWarning(Infrastructure::Logging::ui) << "event=logo.rejected reason=invalid_image";
        set_status(error);
        return;
    }
    logo_data_ = png;
    qCDebug(Infrastructure::Logging::ui) << "event=logo.normalized bytes=" << png.size();
    logo_path_->setText(path);
    set_status("Logo selected.");
}

void ManagementForm::set_status(const QString& message) {
    status_label_->setText(message);
}
} // namespace App::Views

void App::Views::ManagementForm::set_offline(bool offline) {
    set_saving(false);
    if (country_)
        country_->setCurrentIndex(0);
    if (gst_registered_)
        gst_registered_->setChecked(false);
    if (invoice_preferences_) invoice_preferences_->set_values({});
    for (auto* input : inputs_)
        input->clear();
    if (logo_path_)
        logo_path_->clear();
    logo_data_.clear();
    findChild<QPushButton*>("register_button_")->setText(offline ? "Use in demo" : "Save");
    set_status(offline ? "This record is only kept for the current demo." : "");
}
