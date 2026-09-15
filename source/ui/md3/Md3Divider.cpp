#include "Md3Divider.h"

#include <QPainter>

namespace Md3 {

Divider::Divider(QWidget *parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    connect(&Theme::instance(), &Theme::changed, this, qOverload<>(&QWidget::update));
}

void Divider::setVertical(bool vertical)
{
    if (m_vertical == vertical) {
        return;
    }
    m_vertical = vertical;
    setSizePolicy(vertical ? QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding)
                           : QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    updateGeometry();
    update();
}

QSize Divider::sizeHint() const
{
    return m_vertical ? QSize(1, 24) : QSize(24, 1);
}

void Divider::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), Theme::instance().colors().outlineVariant);
}

} // namespace Md3
