#pragma once
#include <QPushButton>
#include <QPropertyAnimation>

namespace App {
namespace Widgets {


class AnimatedButton : public QPushButton {
    Q_OBJECT
public:
    explicit AnimatedButton(QWidget* parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    QRect originalGeometry;
    QPropertyAnimation* anim;
};


}
}