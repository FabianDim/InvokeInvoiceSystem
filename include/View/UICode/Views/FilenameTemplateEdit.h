#pragma once
#include <QApplication>
#include <QDrag>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QPainter>
#include <QPushButton>
#include <QTextEdit>
#include <QTextBlock>
#include <QTextFragment>
#include <QTextImageFormat>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QRegularExpression>

namespace App::Views {
// An ordinary editable text field with atomic inline images for automatic values.
// Storage remains a small, portable text template, never HTML or image data.
class FilenameTemplateEdit : public QTextEdit {
  public:
    static constexpr const char* token_mime = "application/x-invoice-filename-token";
    explicit FilenameTemplateEdit(QWidget* parent = nullptr) : QTextEdit(parent) {
        setObjectName("default_invoice_filename_template");
        setProperty("role", "input");
        setAccessibleName("Automatic PDF filename template");
        setAcceptRichText(false);
        setTabChangesFocus(true);
        setCursorWidth(1);
        viewport()->setCursor(Qt::IBeamCursor);
        setFixedHeight(68);
        setPlaceholderText("Drag a pill here, or type your own filename");
        setStyleSheet("QTextEdit { background: #f8f9f4; color: #20382e; border: 1px solid #c7d4bd; border-radius: 8px; padding: 7px; }");
        register_pill("business_name", "Business name");
        register_pill("invoice_number", "Invoice number");
        register_pill("invoice_number_only", "Invoice number only");
    }
    QString pattern() const {
        QString result;
        for (auto block = document()->begin(); block.isValid(); block = block.next()) {
            if (!result.isEmpty() && block != document()->begin()) result += ' ';
            for (auto it = block.begin(); !it.atEnd(); ++it) {
                const auto fragment = it.fragment();
                if (fragment.charFormat().isImageFormat()) {
                    const auto name = fragment.charFormat().toImageFormat().name();
                    if (known_token(name)) result += "{" + name + "}";
                } else result += fragment.text();
            }
        }
        return result;
    }
    void set_pattern(const QString& pattern) {
        clear();
        register_pill("business_name", "Business name");
        register_pill("invoice_number", "Invoice number");
        register_pill("invoice_number_only", "Invoice number only");
        insert_pattern(pattern);
        document()->clearUndoRedoStacks();
    }
    void insert_token(const QString& token) {
        if (!known_token(token) || !isEnabled()) return;
        auto cursor = textCursor();
        const auto image = pill_format(token);
        cursor.insertImage(image);
        cursor.setCharFormat(QTextCharFormat{});
        setTextCursor(cursor);
        setFocus();
    }
  protected:
    bool canInsertFromMimeData(const QMimeData* source) const override {
        return source->hasFormat(token_mime) || source->hasText() || source->hasHtml();
    }
    void insertFromMimeData(const QMimeData* source) override {
        if (source->hasFormat(token_mime)) {
            insert_token(QString::fromUtf8(source->data(token_mime)));
        } else if (source->hasHtml()) {
            // Preserve our own pills when copying or moving selections, ignoring external images.
            QTextDocument pasted;
            pasted.setHtml(source->html());
            QString pattern;
            for (auto block = pasted.begin(); block.isValid(); block = block.next()) {
                if (block != pasted.begin()) pattern += ' ';
                for (auto it = block.begin(); !it.atEnd(); ++it) {
                    const auto fragment = it.fragment();
                    if (fragment.charFormat().isImageFormat()) {
                        const auto name = fragment.charFormat().toImageFormat().name();
                        if (known_token(name)) pattern += "{" + name + "}";
                    } else pattern += fragment.text();
                }
            }
            insert_pattern(pattern);
        } else insert_pattern(source->text());
    }
    void keyPressEvent(QKeyEvent* event) override {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) { event->accept(); return; }
        QTextEdit::keyPressEvent(event);
    }
  private:
    static bool known_token(const QString& token) { return token == "business_name" || token == "invoice_number" || token == "invoice_number_only"; }
    QTextImageFormat pill_format(const QString& token) const {
        QTextImageFormat format;
        format.setName(token);
        format.setProperty(QTextFormat::ImageAltText, token == "business_name" ? "Business name" :
            token == "invoice_number_only" ? "Invoice number only" : "Invoice number");
        const auto image = document()->resource(QTextDocument::ImageResource, QUrl(token)).value<QImage>();
        const auto size = image.deviceIndependentSize();
        format.setWidth(size.width());
        format.setHeight(size.height());
        format.setVerticalAlignment(QTextCharFormat::AlignMiddle);
        return format;
    }
    void register_pill(const QString& token, const QString& label) {
        QFont font = this->font();
        font.setPointSize(10);
        const QFontMetrics metrics(font);
        const QSize size(metrics.horizontalAdvance(label) + 22, metrics.height() + 10);
        QImage image(size * 2, QImage::Format_ARGB32_Premultiplied);
        image.setDevicePixelRatio(2);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(QColor("#abc2ae"));
        painter.setBrush(QColor("#e0ecde"));
        painter.drawRoundedRect(QRectF(QPointF(1, 1), QSizeF(size.width()-2, size.height()-2)), 12, 12);
        painter.setFont(font);
        painter.setPen(QColor("#264b39"));
        painter.drawText(QRect(QPoint(), size), Qt::AlignCenter, label);
        painter.end();
        document()->addResource(QTextDocument::ImageResource, QUrl(token), image);
    }
    void insert_pattern(QString pattern) {
        pattern.replace(QRegularExpression("[\\r\\n\\t]+"), " ");
        auto cursor = textCursor();
        cursor.beginEditBlock();
        cursor.removeSelectedText();
        static const QRegularExpression tokens("\\{(business_name|invoice_number|invoice_number_only)\\}");
        qsizetype previous = 0;
        auto matches = tokens.globalMatch(pattern);
        while (matches.hasNext()) {
            const auto match = matches.next();
            cursor.insertText(pattern.mid(previous, match.capturedStart() - previous), QTextCharFormat{});
            const auto image = pill_format(match.captured(1));
            cursor.insertImage(image);
            previous = match.capturedEnd();
        }
        cursor.insertText(pattern.mid(previous), QTextCharFormat{});
        cursor.setCharFormat(QTextCharFormat{});
        cursor.endEditBlock();
        setTextCursor(cursor);
    }
};

class FilenameTokenPill : public QPushButton {
  public:
    FilenameTokenPill(const QString& label, const QString& token, FilenameTemplateEdit* editor, QWidget* parent)
        : QPushButton(label, parent), token_(token) {
        setObjectName("filename_token_" + token);
        setCursor(Qt::PointingHandCursor);
        setToolTip("Drag into the filename, or click to insert at the cursor.");
        setStyleSheet("QPushButton { background: #e0ecde; color: #264b39; border: 1px solid #abc2ae; border-radius: 14px; padding: 5px 12px; min-height: 18px; } QPushButton:hover { background: #cddfc9; } QPushButton:focus { border: 2px solid #376f50; }");
        connect(this, &QPushButton::clicked, editor, [editor, token]() { editor->insert_token(token); });
    }
  protected:
    void mousePressEvent(QMouseEvent* event) override { origin_ = event->position().toPoint(); QPushButton::mousePressEvent(event); }
    void mouseMoveEvent(QMouseEvent* event) override {
        if (!(event->buttons() & Qt::LeftButton) ||
            (event->position().toPoint() - origin_).manhattanLength() < QApplication::startDragDistance()) {
            QPushButton::mouseMoveEvent(event); return;
        }
        QDrag drag(this);
        auto* mime = new QMimeData;
        mime->setData(FilenameTemplateEdit::token_mime, token_.toUtf8());
        mime->setText("{" + token_ + "}");
        drag.setMimeData(mime);
        drag.setPixmap(grab());
        setDown(false);
        drag.exec(Qt::CopyAction);
    }
  private:
    QString token_;
    QPoint origin_;
};
}
