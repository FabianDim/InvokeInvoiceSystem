#pragma once
#include <QJsonObject>
#include <QRegularExpression>

namespace Domain::Invoices {
inline QString default_filename_template() { return "{invoice_number}"; }
inline bool reserved_filename(const QString& name) {
    static const QRegularExpression reserved("^(CON|PRN|AUX|NUL|COM[1-9\\x{00b9}\\x{00b2}\\x{00b3}]|LPT[1-9\\x{00b9}\\x{00b2}\\x{00b3}])(?:\\.|$)", QRegularExpression::CaseInsensitiveOption);
    return reserved.match(name).hasMatch();
}
inline QString filename_error(const QString& name) {
    if (name.trimmed().isEmpty()) return "Enter a PDF file name.";
    if (name.contains(QRegularExpression("[<>:\"/\\\\|?*\\x00-\\x1f]")))
        return "Use a filename without path separators, control characters or reserved characters: < > : \" / \\ | ? *.";
    if (name.endsWith('.') || name.endsWith(' ')) return "A filename cannot end with a dot or space.";
    auto stem = name;
    if (stem.endsWith(".pdf", Qt::CaseInsensitive)) stem.chop(4);
    if (stem.trimmed().isEmpty() || stem == "." || stem == "..") return "Enter a name before .pdf.";
    if (reserved_filename(stem.trimmed())) return "Choose a filename other than a reserved device name such as CON, NUL, COM1 or LPT1.";
    if (stem.size() > 180 || (stem + ".pdf").toUtf8().size() > 255)
        return "Shorten the PDF filename (at most 180 characters and 255 UTF-8 bytes including .pdf).";
    return {};
}
inline QString filename_template_error(const QString& pattern) {
    if (pattern.isEmpty()) return {};
    if (pattern.size() > 200) return "Keep the filename template within 200 characters.";
    if (pattern.contains(QRegularExpression("[<>:\"/\\\\|?*\\x00-\\x1f]")))
        return "Use a filename without path separators or reserved characters.";
    auto literal = pattern;
    literal.remove("{business_name}");
    literal.remove("{invoice_number}");
    literal.remove("{invoice_number_only}");
    if (literal.contains('{') || literal.contains('}')) return "Use the available pills for automatic values.";
    auto example = pattern;
    example.replace("{business_name}", "Business");
    example.replace("{invoice_number}", "INV-000001");
    example.replace("{invoice_number_only}", "000001");
    return filename_error(example);
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
inline QString invoice_filename(QString pattern, const QString& business_name, const QString& number, const QString& prefix = {}) {
    if (pattern.trimmed().isEmpty()) pattern = default_filename_template();
    static const QRegularExpression tokens("\\{(business_name|invoice_number|invoice_number_only)\\}");
    QString result;
    qsizetype previous = 0;
    auto matches = tokens.globalMatch(pattern);
    while (matches.hasNext()) {
        const auto match = matches.next();
        result += pattern.mid(previous, match.capturedStart() - previous);
        auto value = match.captured(1) == "business_name" ? business_name : number;
        if (match.captured(1) == "invoice_number_only" && !prefix.isEmpty() && number.startsWith(prefix))
            value = number.mid(prefix.size());
        result += safe_filename_part(value);
        previous = match.capturedEnd();
    }
    result += pattern.mid(previous);
    result = safe_filename_part(result);
    if (result.endsWith(".pdf", Qt::CaseInsensitive)) result.chop(4);
    result = result.left(180);
    if (!result.isEmpty() && result.back().isHighSurrogate()) result.chop(1);
    while (result.endsWith('.') || result.endsWith(' ')) result.chop(1);
    if (result.isEmpty()) result = "Invoice";
    if (reserved_filename(result)) result.prepend('_');
    while (result.size() > 180 || (result + ".pdf").toUtf8().size() > 255) {
        result.chop(result.back().isLowSurrogate() && result.size() > 1 && result.at(result.size()-2).isHighSurrogate() ? 2 : 1);
    }
    while (result.endsWith('.')) result.chop(1);
    return result;
}
}
