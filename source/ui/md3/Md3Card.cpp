#include "Md3Card.h"

#include <QPainter>

namespace Md3 {

Card::Card(QWidget *parent)
    : QFrame(parent)
{
    setFrameShape(QFrame::NoFrame);
    setAttribute(Qt::WA_StyledBackground, false);
    connect(&Theme::instance(), &Theme::changed, this, qOverload<>(&QWidget::update));
}

Card::Card(Variant variant, QWidget *parent)
    : QFrame(parent)
    , m_variant(variant)
{
    setFrameShape(QFrame::NoFrame);
    setAttribute(Qt::WA_StyledBackground, false);
    connect(&Theme::instance(), &Theme::changed, this, qOverload<>(&QWidget::update));
}

void Card::setVariant(Variant variant)
{
    if (m_variant == variant) {
        return;
    }
    m_variant = variant;
    update();
}

void Card::setCornerRadius(qreal radius)
{
    if (qFuzzyCompare(m_radius, radius)) {
        return;
    }
    m_radius = radius;
    update();
}

void Card::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const Md3ColorScheme &c = Theme::instance().colors();
    const QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);

    QColor container = c.surfaceContainerHighest;
    QPen border(Qt::NoPen);
    if (m_variant == Elevated) {
        Theme::paintElevation(p, rect(), m_radius, 1.0, c.shadow);
        container = c.surfaceContainerLow;
    } else if (m_variant == Outlined) {
        container = c.surface;
        border = QPen(c.outlineVariant, 1.0);
    }

    p.setPen(border);
    p.setBrush(container);
    p.drawRoundedRect(r, m_radius, m_radius);
}

} // namespace Md3
