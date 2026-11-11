#pragma once
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVariant>
#include <QVBoxLayout>

namespace App::Views::UiStyle {
// Widget types define shared geometry; dynamic roles select appearance in QSS.
inline void button(QPushButton* widget, const char* role = "secondary", bool dashboard = false) {
    widget->setProperty("role", role);
    widget->setProperty("controlSize", dashboard ? "dashboard" : "standard");
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    widget->setAutoDefault(false);
}

inline QLabel* label(const QString& text, QWidget* parent, const char* role = "field") {
    auto* widget = new QLabel(text, parent);
    widget->setProperty("role", role);
    return widget;
}

inline QWidget* page_content(QWidget* page, int maximum_width = 680) {
    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(24, 24, 24, 24);
    auto* row = new QHBoxLayout;
    auto* content = new QWidget(page);
    content->setMaximumWidth(maximum_width);
    content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    row->addStretch();
    row->addWidget(content, 1);
    row->addStretch();
    root->addStretch();
    root->addLayout(row);
    root->addStretch();
    return content;
}
} // namespace App::Views::UiStyle
