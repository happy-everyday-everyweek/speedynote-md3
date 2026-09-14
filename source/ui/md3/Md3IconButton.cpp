#include "Md3IconButton.h"

#include <QMouseEvent>
#include <QPainter>

#include "Md3Ripple.h"

namespace Md3 {

IconButton::IconButton(QWidget *parent)
    : QPushButton(parent)
{
    initialize();
}

void IconButton::initialize()
{
    setAttribute(Qt::WA_Hover, true);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::StrongFocus);
    setFlat(true);

    m_ripple = new Ripple(this);
    connect(m_ripple, &Ripple::repaintRequested, this, qOverload<>(&QWidget::update));
    connect(&Theme::instance(), &Theme::changed, this, [this]() { refresh(); });
}

void IconButton::setVariant(Variant variant)
{
    if (m_variant == variant) {
        return;
    }
    m_variant = variant;
    update();
}

void IconButton::setIconSize(int size)
{
    size = qBound(16, size, 32);
    if (m_iconSize == size) {
        return;
    }
    m_iconSize = size;
    update();
}

void IconButton::setContainerSize(int size)
{
    size = qBound(32, size, 48);
    if (m_containerSize == size) {
        return;
    }
    m_containerSize = size;
    updateGeometry();
    update();
}

void IconButton::refresh()
{
    update();
}

QSize IconButton::sizeHint() const
{
    // M3 minimum touch target is 48dp; the visual container is centered.
    return QSize(qMax(48, m_containerSize + 8), qMax(48, m_containerSize + 8));
}

QSize IconButton::minimumSizeHint() const
{
    return sizeHint();
}

QColor IconButton::containerColor() const
{
    const Md3ColorScheme &c = Theme::instance().colors();
    const bool on = isChecked();

    if (!isEnabled()) {
        if (m_variant == Standard || m_variant == Outlined) {
            return Qt::transparent;
        }
        QColor disabled = c.onSurface;
        disabled.setAlphaF(0.12);
        return disabled;
    }

    switch (m_variant) {
    case Standard:
        return Qt::transparent;
    case Filled:
        return on ? c.primary : c.surfaceContainerHighest;
    case Tonal:
        return on ? c.secondaryContainer : c.surfaceContainerHighest;
    case Outlined:
        return on ? c.inverseSurface : Qt::transparent;
    }
    return Qt::transparent;
}

QColor IconButton::borderColor() const
{
    if (m_variant != Outlined) {
        return QColor();
    }
    const Md3ColorScheme &c = Theme::instance().colors();
    if (!isEnabled()) {
        QColor disabled = c.onSurface;
        disabled.setAlphaF(0.12);
        return disabled;
    }
    return isChecked() ? QColor(Qt::transparent) : c.outline;
}

QColor IconButton::iconColor() const
{
    const Md3ColorScheme &c = Theme::instance().colors();
    const bool on = isChecked();

    if (!isEnabled()) {
        QColor disabled = c.onSurface;
        disabled.setAlphaF(0.38);
        return disabled;
    }

    switch (m_variant) {
    case Standard:
        return on ? c.primary : c.onSurfaceVariant;
    case Filled:
        return on ? c.onPrimary : c.primary;
    case Tonal:
        return on ? c.onSecondaryContainer : c.onSurfaceVariant;
    case Outlined:
        return on ? c.inverseOnSurface : c.onSurfaceVariant;
    }
    return c.onSurfaceVariant;
}

void IconButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const QRectF full(rect());
    const qreal d = m_containerSize;
    const QRectF container(full.center().x() - d / 2.0, full.center().y() - d / 2.0, d, d);
    const qreal radius = d / 2.0;

    // --- Container -----------------------------------------------------------
    const QColor bg = containerColor();
    if (bg.alpha() > 0) {
        p.setPen(Qt::NoPen);
        p.setBrush(bg);
        p.drawEllipse(container);
    }

    // --- Outline --------------------------------------------------------------
    const QColor border = borderColor();
    if (border.isValid()) {
        QPen pen(border, 1.0);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(container.adjusted(0.5, 0.5, -0.5, -0.5));
    }

    // --- State layer + ripple ---------------------------------------------------
    if (isEnabled() && m_ripple) {
        m_ripple->setColor(iconColor());
        m_ripple->paint(p, container, radius);
    }

    // --- Icon -------------------------------------------------------------------
    if (!icon().isNull()) {
        const QRect iconRect(static_cast<int>(full.center().x() - m_iconSize / 2.0),
                             static_cast<int>(full.center().y() - m_iconSize / 2.0),
                             m_iconSize, m_iconSize);
        icon().paint(&p, iconRect, Qt::AlignCenter,
                     isEnabled() ? QIcon::Normal : QIcon::Disabled, QIcon::Off);
    }
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void IconButton::enterEvent(QEnterEvent *event)
#else
void IconButton::enterEvent(QEvent *event)
#endif
{
    if (m_ripple) {
        m_ripple->setHovered(true);
    }
    QPushButton::enterEvent(event);
}

void IconButton::leaveEvent(QEvent *event)
{
    if (m_ripple) {
        m_ripple->setHovered(false);
    }
    QPushButton::leaveEvent(event);
}

void IconButton::mousePressEvent(QMouseEvent *event)
{
    if (isEnabled() && m_ripple && event->button() == Qt::LeftButton) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        m_ripple->press(event->position());
#else
        m_ripple->press(event->pos());
#endif
    }
    QPushButton::mousePressEvent(event);
}

void IconButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_ripple) {
        m_ripple->release();
    }
    QPushButton::mouseReleaseEvent(event);
}

void IconButton::checkStateSet()
{
    QPushButton::checkStateSet();
    update();
}

void IconButton::nextCheckState()
{
    QPushButton::nextCheckState();
    update();
}

void IconButton::focusInEvent(QFocusEvent *event)
{
    if (m_ripple) {
        m_ripple->setFocused(true);
    }
    QPushButton::focusInEvent(event);
}

void IconButton::focusOutEvent(QFocusEvent *event)
{
    if (m_ripple) {
        m_ripple->setFocused(false);
    }
    QPushButton::focusOutEvent(event);
}

} // namespace Md3