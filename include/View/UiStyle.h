#pragma once
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVariant>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QFrame>

namespace App::Views::UiStyle {
// Widget types define shared geometry; dynamic roles select appearance in QSS.
inline void button(QPushButton* widget, const char* role = "secondary", bool dashboard = false) {
    widget->setProperty("role", role);
    widget->setProperty("controlSize", dashboard ? "dashboard" : "standard");
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    widget->setAutoDefault(false);
    widget->setCursor(Qt::PointingHandCursor);
}

inline QLabel* label(const QString& text, QWidget* parent, const char* role = "field") {
    auto* widget = new QLabel(text, parent);
    widget->setProperty("role", role);
    return widget;
}

inline QWidget* page_content(QWidget* page, int maximum_width = 680, bool panel = true) {
    auto* outer = new QVBoxLayout(page);
    outer->setContentsMargins(0, 0, 0, 0);
    auto* scroll = new QScrollArea(page);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidgetResizable(true);
    auto* canvas = new QWidget;
    canvas->setObjectName("page_canvas");
    auto* root = new QVBoxLayout(canvas);
    root->setSizeConstraint(QLayout::SetMinAndMaxSize);
    root->setContentsMargins(28, 24, 28, 24);
    auto* row = new QHBoxLayout;
    auto* content = new QFrame(canvas);
    content->setObjectName(panel ? "page_card" : "workspace_content");
    content->setMaximumWidth(maximum_width);
    content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    row->addStretch();
    row->addWidget(content, 1);
    row->addStretch();
    root->addStretch();
    root->addLayout(row);
    root->addStretch();
    scroll->setWidget(canvas);
    outer->addWidget(scroll);
    return content;
}
} // namespace App::Views::UiStyle
