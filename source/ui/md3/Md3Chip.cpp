#include "Md3Chip.h"

#include <QFontMetricsF>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

#include "Md3Ripple.h"

namespace Md3 {

namespace {

constexpr int kHeight = 32;
constexpr int kIconSize = 18;
constexpr int kIconGap = 8;

QColor withAlpha(const QColor &color, qreal alpha)
{
    QColor c = color;
    c.setAlphaF(qBound(0.0, alpha, 1.0));
    return c;
}

} // namespace

Chip::Chip(QWidget *parent)
    : QAbstractButton(parent)
{
    initialize();
}

Chip::Chip(const QString &text, QWidget *parent)
    : QAbstractButton(parent)
{
    setText(text);
    initialize();
}

void Chip::initialize()
{
    setAttribute(Qt::WA_Hover, true);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_ripple = new Ripple(this);
    connect(m_ripple, &Ripple::repaintRequested, this, qOverload<>(&QWidget::update));
    connect(&Theme::instance(), &Theme::changed, this, qOverload<>(&QWidget::update));
}

void Chip::setStyle(Style style)
{
    m_style = style;
    if (style == Filter) {
        setCheckable(true);
    }
    update();
}

void Chip::setLeadingIcon(const QIcon &icon)
{
    m_icon = icon;
    updateGeometry();
    update();
}

QSize Chip::sizeHint() const
{
    const QFontMetricsF fm(Theme::instance().font(TypeRole::LabelLarge));
    const qreal leftPad = hasLeading() ? 8.0 : 16.0;
    const qreal iconPart = hasLeading() ? kIconSize + kIconGap : 0.0;
    const qreal width = leftPad + iconPart + fm.horizontalAdvance(text()) + 16.0;
    return QSize(static_cast<int>(qMax(qreal(48.0), width) + 0.5), kHeight);
}

QSize Chip::minimumSizeHint() const
{
    return sizeHint();
}

void Chip::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const Md3ColorScheme &c = Theme::instance().colors();
    const bool checked = isChecked();
    const qreal radius = 8.0; // M3 small shape
    const QRectF box = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);

    QColor container;
    QColor border;
    QColor content;
    if (!isEnabled()) {
        container = Qt::transparent;
        border = withAlpha(c.onSurface, 0.12);
        content = withAlpha(c.onSurface, 0.38);
    } else if (checked) {
        container = c.secondaryContainer;
        content = c.onSecondaryContainer;
    } else {
        container = Qt::transparent;
        border = c.outline;
        content = c.onSurfaceVariant;
    }

    // Ripple + state layers.
    if (isEnabled() && m_ripple) {
        m_ripple->setColor(c.onSurface);
        m_ripple->paint(p, box, radius);
    }

    if (container.alphaF() > 0.001) {
        p.setPen(Qt::NoPen);
        p.setBrush(container);
        p.drawRoundedRect(box, radius, radius);
    }
    if (border.alphaF() > 0.001) {
        p.setPen(QPen(border, 1.0));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(box, radius, radius);
    }

    const qreal cy = height() / 2.0;
    qreal x = hasLeading() ? 8.0 : 16.0;

    if (hasLeading()) {
        const QRectF iconRect(x, cy - kIconSize / 2.0, kIconSize, kIconSize);
        if (checked) {
            // Check mark replaces the leading icon when selected.
            QPen pen(content, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            p.setPen(pen);
            p.setBrush(Qt::NoBrush);
            p.drawLine(QPointF(iconRect.left() + 3.5, iconRect.top() + 9.5),
                       QPointF(iconRect.left() + 7.5, iconRect.top() + 13.5));
            p.drawLine(QPointF(iconRect.left() + 7.5, iconRect.top() + 13.5),
                       QPointF(iconRect.left() + 14.5, iconRect.top() + 5.5));
        } else if (!m_icon.isNull()) {
            m_icon.paint(&p, iconRect.toRect(), Qt::AlignCenter,
                         isEnabled() ? QIcon::Normal : QIcon::Disabled, QIcon::Off);
        }
        x += kIconSize + kIconGap;
    }

    p.setFont(Theme::instance().font(TypeRole::LabelLarge));
    p.setPen(content);
    const QRectF textRect(x, 0.0, width() - x - 16.0, height());
    p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft | Qt::TextSingleLine, text());
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void Chip::enterEvent(QEnterEvent *event)
#else
void Chip::enterEvent(QEvent *event)
#endif
{
    if (isEnabled() && m_ripple) {
        m_ripple->setHovered(true);
    }
    QAbstractButton::enterEvent(event);
}

void Chip::leaveEvent(QEvent *event)
{
    if (m_ripple) {
        m_ripple->setHovered(false);
    }
    QAbstractButton::leaveEvent(event);
}

void Chip::mousePressEvent(QMouseEvent *event)
{
    if (isEnabled() && m_ripple && event->button() == Qt::LeftButton) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        m_ripple->press(event->position());
#else
        m_ripple->press(QPointF(event->pos()));
#endif
    }
    QAbstractButton::mousePressEvent(event);
}

void Chip::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_ripple) {
        m_ripple->release();
    }
    QAbstractButton::mouseReleaseEvent(event);
}

} // namespace Md3
