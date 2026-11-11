#pragma once
#include <QBuffer>
#include <QImageReader>
#include <QString>

namespace BusinessLogo {
inline constexpr qint64 maximum_bytes = 5 * 1024 * 1024;

// Store a portable PNG in the business record, independently of the source path.
inline bool normalise(const QByteArray& source, QByteArray& png, QString& error) {
    if (source.isEmpty() || source.size() > maximum_bytes) {
        error = "Choose a PNG or JPEG logo no larger than 5 MB.";
        return false;
    }
    QBuffer input;
    input.setData(source);
    input.open(QIODevice::ReadOnly);
    QImageReader reader(&input);
    const auto format = reader.format();
    const auto size = reader.size();
    if ((format != "png" && format != "jpeg" && format != "jpg") || !size.isValid() ||
        size.width() > 8192 || size.height() > 8192) {
        error = "Use a valid PNG or JPEG logo up to 8192 pixels on each side.";
        return false;
    }
    reader.setAutoTransform(true);
    auto image = reader.read();
    if (image.isNull()) {
        error = "The logo could not be read. Choose another PNG or JPEG image.";
        return false;
    }
    if (image.width() > 1024 || image.height() > 1024)
        image = image.scaled(1024, 1024, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    png.clear();
    QBuffer output(&png);
    output.open(QIODevice::WriteOnly);
    if (!image.save(&output, "PNG") || png.size() > maximum_bytes) {
        error = "The logo could not be stored. Choose a smaller image.";
        return false;
    }
    return true;
}
} // namespace BusinessLogo
