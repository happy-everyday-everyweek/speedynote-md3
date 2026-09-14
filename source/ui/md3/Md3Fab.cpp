#include "Md3Fab.h"

#include <QMouseEvent>
#include <QPainter>

#include "Md3Ripple.h"

namespace Md3 {

Fab::Fab(QWidget *parent)
    : QPushButton(parent)
{
    initialize();
}

void Fab::initialize()
{
    setAttribute(Qt::WA_Hover, true);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::StrongFocus);
    setFlat(true);

    m_ripple = new Ripple(this);
    connect(m_ripple, &Ripple::repaintRequested, this, qOverload<>(&QWidget::update));
    connect(&Theme::instance(), &Theme::changed, this, qOverload<>(&QWidget::update));
}

void Fab::setSize(Size size)
{
    if (m_size == size) {
        return;
    }
    m_size = size;
    updateGeometry();
    update();
}

void Fab::setColorRole(ColorRole role)
{
    if (m_colorRole == role) {
        return;
    }
    m_colorRole = role;
    update();
}

int Fab::diameter() const
{
    switch (m_size) {
    case Small:  return 40;
    case Large:  return 96;
    case Medium:
    default:     return 56;
    }
}

int Fab::iconToken() const
{
    return m_size == Large ? 36 : 24;
}

QSize Fab::sizeHint() const
{
    const int d = diameter();
    // Leave a margin so the elevation shadow is visible.
    return QSize(d + 8, d + 8);
}

QSize Fab::minimumSizeHint() const
{
    return sizeHint();
}

QColor Fab::containerColor() const
{
    const Md3ColorScheme &c = Theme::instance().colors();
    if (!isEnabled()) {
        QColor disabled = c.onSurface;
        disabled.setAlphaF(0.12);
        return disabled;
    }

    switch (m_colorRole) {
    case PrimaryContainer:   return c.primaryContainer;
    case Surface:            return c.surfaceContainerHigh;
    case SecondaryContainer: return c.secondaryContainer;
    case TertiaryContainer:  return c.tertiaryContainer;
    }
    return c.primaryContainer;
}

QColor Fab::iconColor() const
{
    const Md3ColorScheme &c = Theme::instance().colors();
    if (!isEnabled()) {
        QColor disabled = c.onSurface;
        disabled.setAlphaF(0.38);
        return disabled;
    }

    switch (m_colorRole) {
    case PrimaryContainer:   return c.onPrimaryContainer;
    case Surface:            return c.primary;
    case SecondaryContainer: return c.onSecondaryContainer;
    case TertiaryContainer:  return c.onTertiaryContainer;
    }
    return c.onPrimaryContainer;
}

void Fab::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const Md3ColorScheme &c = Theme::instance().colors();
    const qreal pad = 3.0; // room for the shadow
    const QRectF r = QRectF(rect()).adjusted(pad, pad, -pad, -pad);
    const qreal radius = r.width() / 2.0;

    // --- Elevation (level 3 per spec) ---------------------------------------
    if (isEnabled()) {
        Theme::paintElevation(p, r, radius, 3.0, c.shadow);
    }

    // --- Container ------------------------------------------------------------
    p.setPen(Qt::NoPen);
    p.setBrush(containerColor());
    p.drawEllipse(r);

    // --- State layer + ripple ----------------------------------------------------
    if (isEnabled() && m_ripple) {
        m_ripple->setColor(iconColor());
        m_ripple->paint(p, r, radius);
    }

    // --- Icon ----------------------------------------------------------------------
    if (!icon().isNull()) {
        const int is = iconToken();
        const QRect iconRect(static_cast<int>(r.center().x() - is / 2.0),
                             static_cast<int>(r.center().y() - is / 2.0), is, is);
        icon().paint(&p, iconRect, Qt::AlignCenter,
                     isEnabled() ? QIcon::Normal : QIcon::Disabled, QIcon::Off);
    }
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void Fab::enterEvent(QEnterEvent *event)
#else
void Fab::enterEvent(QEvent *event)
#endif
{
    if (m_ripple) {
        m_ripple->setHovered(true);
    }
    QPushButton::enterEvent(event);
}

void Fab::leaveEvent(QEvent *event)
{
    if (m_ripple) {
        m_ripple->setHovered(false);
    }
    QPushButton::leaveEvent(event);
}

void Fab::mousePressEvent(QMouseEvent *event)
{
    if (isEnabled() && m_ripple && event->button() == Qt::LeftButton) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        m_ripple->press(event->position() - QPointF(3, 3) + QPointF(0, 0));
#else
        m_ripple->press(event->pos() - QPoint(3, 3));
#endif
    }
    QPushButton::mousePressEvent(event);
}

void Fab::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_ripple) {
        m_ripple->release();
    }
    QPushButton::mouseReleaseEvent(event);
}

} // namespace Md3