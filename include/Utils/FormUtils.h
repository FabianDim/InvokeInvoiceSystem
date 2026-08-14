#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>

struct FormField {
    enum class Type { LineEdit, ComboBox, DateEdit, DirBrowse };

    QString key;
    QString label;
    Type type;

    struct Options {
        QString placeholder;
        bool defaultToday = false;
        bool fromNow = false;
        QString objectName;
        std::function<void(QWidget*)> customise;
    } opt;
};