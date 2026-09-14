#include "Md3Button.h"

#include <QFontMetricsF>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

#include "Md3Ripple.h"

namespace Md3 {

namespace {
constexpr int kIconSize = 18;
constexpr int kIconGap = 8;
} // namespace

Button::Button(QWidget *parent)
    : QPushButton(parent)
{
    initialize();
}

Button::Button(const QString &text, QWidget *parent)
    : QPushButton(text, parent)
{
    initialize();
}

Button::Button(const QString &text, Variant variant, QWidget *parent)
    : QPushButton(text, parent)
    , m_variant(variant)
{
    initialize();
}

void Button::initialize()
{
    setAttribute(Qt::WA_Hover, true);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::StrongFocus);

    m_ripple = new Ripple(this);
    connect(m_ripple, &Ripple::repaintRequested, this, qOverload<>(&QWidget::update));
    connect(&Theme::instance(), &Theme::changed, this, qOverload<>(&QWidget::update));
}

void Button::setVariant(Variant variant)
{
    if (m_variant == variant) {
        return;
    }
    m_variant = variant;
    updateGeometry();
    update();
}

void Button::setSize(Size size)
{
    if (m_size == size) {
        return;
    }
    m_size = size;
    updateGeometry();
    update();
}

void Button::setLeadingIcon(const QIcon &icon)
{
    m_leadingIcon = icon;
    updateGeometry();
    update();
}

int Button::heightToken() const
{
    switch (m_size) {
    case Small:  return 32;
    case Large:  return 56;
    case Medium:
    default:     return 40;
    }
}

int Button::horizontalPadding() const
{
    // M3: 24dp padding without icon, 16dp with a leading icon.
    return m_leadingIcon.isNull() ? 24 : 16;
}

QSize Button::sizeHint() const
{
    const QFontMetricsF fm(Theme::instance().font(TypeRole::LabelLarge));
    qreal width = fm.horizontalAdvance(text()) + horizontalPadding() * 2.0;
    if (!m_leadingIcon.isNull()) {
        width += kIconSize + kIconGap;
    }
    const int h = heightToken();
    return QSize(qMax(qreal(64), width), h);
}

QSize Button::minimumSizeHint() const
{
    return sizeHint();
}

QColor Button::containerColor() const
{
    const Md3ColorScheme &c = Theme::instance().colors();
    if (!isEnabled()) {
        if (m_variant == Text || m_variant == Outlined) {
            return Qt::transparent;
        }
        QColor disabled = c.onSurface;
        disabled.setAlphaF(0.12);
        return disabled;
    }

    switch (m_variant) {
    case Filled:   return c.primary;
    case Tonal:    return c.secondaryContainer;
    case Outlined: return Qt::transparent;
    case Text:     return Qt::transparent;
    case Elevated: return c.surfaceContainerLow;
    }
    return c.primary;
}

QColor Button::borderColor() const
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
    return c.outline;
}

QColor Button::contentColor() const
{
    const Md3ColorScheme &c = Theme::instance().colors();
    if (!isEnabled()) {
        QColor disabled = c.onSurface;
        disabled.setAlphaF(0.38);
        return disabled;
    }

    switch (m_variant) {
    case Filled:   return c.onPrimary;
    case Tonal:    return c.onSecondaryContainer;
    case Outlined: return c.primary;
    case Text:     return c.primary;
    case Elevated: return c.primary;
    }
    return c.primary;
}

void Button::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const Md3ColorScheme &c = Theme::instance().colors();

    // Elevated buttons leave a tiny margin so the soft shadow is visible.
    const bool elevated = (m_variant == Elevated);
    const qreal pad = elevated ? 2.0 : 0.0;
    const QRectF r = QRectF(rect()).adjusted(pad, pad, -pad, -pad);
    const qreal radius = r.height() / 2.0;

    // --- Elevation shadow --------------------------------------------------
    if (elevated && isEnabled()) {
        Theme::paintElevation(p, r, radius, 1.0, c.shadow);
    }

    // --- Container ----------------------------------------------------------
    const QColor bg = containerColor();
    if (bg.alpha() > 0) {
        p.setPen(Qt::NoPen);
        p.setBrush(bg);
        p.drawRoundedRect(r, radius, radius);
    }

    // --- Outline -------------------------------------------------------------
    const QColor border = borderColor();
    if (border.isValid()) {
        QPen pen(border, 1.0);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        QRectF br = r.adjusted(0.5, 0.5, -0.5, -0.5);
        p.drawRoundedRect(br, radius, radius);
    }

    // --- State layer + ripple ------------------------------------------------
    if (isEnabled() && m_ripple) {
        // Content-ish state layer color: use the content color as the base,
        // falling back to onSurface for containers with light content.
        QColor layer = contentColor();
        if (m_variant == Text || m_variant == Outlined || m_variant == Elevated) {
            layer = c.primary;
        }
        m_ripple->setColor(layer);
        m_ripple->paint(p, r, radius);
    }

    // --- Content: icon + label ----------------------------------------------
    const QFont font = Theme::instance().font(TypeRole::LabelLarge);
    p.setFont(font);
    const QFontMetricsF fm(font);

    const bool hasIcon = !m_leadingIcon.isNull();
    const qreal textWidth = fm.horizontalAdvance(text());
    const qreal contentWidth = textWidth + (hasIcon ? kIconSize + kIconGap : 0.0);
    qreal x = r.center().x() - contentWidth / 2.0;

    const QColor fg = contentColor();
    p.setPen(fg);

    if (hasIcon) {
        const QRectF iconRect(x, r.center().y() - kIconSize / 2.0, kIconSize, kIconSize);
        m_leadingIcon.paint(&p, iconRect.toRect(), Qt::AlignCenter,
                            isEnabled() ? QIcon::Normal : QIcon::Disabled, QIcon::Off);
        x += kIconSize + kIconGap;
    }

    QRectF textRect(x, r.top(), fm.horizontalAdvance(text()), r.height());
    p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft | Qt::TextSingleLine, text());
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void Button::enterEvent(QEnterEvent *event)
#else
void Button::enterEvent(QEvent *event)
#endif
{
    if (m_ripple) {
        m_ripple->setHovered(true);
    }
    QPushButton::enterEvent(event);
}

void Button::leaveEvent(QEvent *event)
{
    if (m_ripple) {
        m_ripple->setHovered(false);
    }
    QPushButton::leaveEvent(event);
}

void Button::mousePressEvent(QMouseEvent *event)
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

void Button::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_ripple) {
        m_ripple->release();
    }
    QPushButton::mouseReleaseEvent(event);
}

void Button::focusInEvent(QFocusEvent *event)
{
    if (m_ripple) {
        m_ripple->setFocused(true);
    }
    QPushButton::focusInEvent(event);
}

void Button::focusOutEvent(QFocusEvent *event)
{
    if (m_ripple) {
        m_ripple->setFocused(false);
    }
    QPushButton::focusOutEvent(event);
}

} // namespace Md3