#pragma once
#include <QJsonObject>
#include <QRegularExpression>

namespace Domain::Invoices {
inline QString default_filename_template() { return "{invoice_number}"; }
inline QString filename_template_error(const QString& pattern) {
    if (pattern.size() > 200) return "Keep the filename template within 200 characters.";
    if (pattern.contains(QRegularExpression("[<>:\"/\\\\|?*\\x00-\\x1f]")))
        return "Use a filename without path separators or reserved characters.";
    auto literal = pattern;
    literal.remove("{business_name}");
    literal.remove("{invoice_number}");
    if (literal.contains('{') || literal.contains('}')) return "Use the Business name and Invoice number pills for automatic values.";
    return {};
}
inline bool valid_filename_setting(const QJsonObject& settings) {
    const auto value = settings.value("InvoiceFilenameTemplate");
    return value.isUndefined() || (value.isString() && filename_template_error(value.toString()).isEmpty());
}
inline QString safe_filename_part(QString value) {
    value = value.trimmed();
    value.replace(QRegularExpression("[<>:\"/\\\\|?*\\x00-\\x1f\\s]+"), "_");
    return value;
}
// Return a stem; the existing export code adds .pdf once.
inline QString invoice_filename(QString pattern, const QString& business_name, const QString& number) {
    if (pattern.trimmed().isEmpty()) pattern = default_filename_template();
    static const QRegularExpression tokens("\\{(business_name|invoice_number)\\}");
    QString result;
    qsizetype previous = 0;
    auto matches = tokens.globalMatch(pattern);
    while (matches.hasNext()) {
        const auto match = matches.next();
        result += pattern.mid(previous, match.capturedStart() - previous);
        result += safe_filename_part(match.captured(1) == "business_name" ? business_name : number);
        previous = match.capturedEnd();
    }
    result += pattern.mid(previous);
    result = safe_filename_part(result);
    if (result.endsWith(".pdf", Qt::CaseInsensitive)) result.chop(4);
    result = result.left(180);
    while (result.endsWith('.') || result.endsWith(' ')) result.chop(1);
    if (result.isEmpty()) result = "Invoice";
    static const QRegularExpression reserved("^(CON|PRN|AUX|NUL|COM[1-9]|LPT[1-9])(?:\\.|$)", QRegularExpression::CaseInsensitiveOption);
    if (reserved.match(result).hasMatch()) result.prepend('_');
    return result;
}
}
