#include "Md3ListItem.h"

#include <QFontMetricsF>
#include <QMouseEvent>
#include <QPainter>

#include "Md3Ripple.h"

namespace Md3 {

namespace {

constexpr int kMinHeight = 56;
constexpr int kLeftPadding = 16;
constexpr int kRightPadding = 16;
constexpr int kIconSize = 24;
constexpr int kIconGap = 16;

QColor withAlpha(const QColor &color, qreal alpha)
{
    QColor c = color;
    c.setAlphaF(qBound(0.0, alpha, 1.0));
    return c;
}

} // namespace

ListItem::ListItem(QWidget *parent)
    : QWidget(parent)
{
    initialize();
}

void ListItem::initialize()
{
    setAttribute(Qt::WA_Hover, true);
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_ripple = new Ripple(this);
    connect(m_ripple, &Ripple::repaintRequested, this, qOverload<>(&QWidget::update));
    connect(&Theme::instance(), &Theme::changed, this, qOverload<>(&QWidget::update));
}

void ListItem::setOverlineText(const QString &text)
{
    m_overline = text;
    updateGeometry();
    update();
}

void ListItem::setHeadline(const QString &text)
{
    m_headline = text;
    updateGeometry();
    update();
}

void ListItem::setSupportingText(const QString &text)
{
    m_supporting = text;
    updateGeometry();
    update();
}

void ListItem::setTrailingText(const QString &text)
{
    m_trailing = text;
    updateGeometry();
    update();
}

void ListItem::setLeadingIcon(const QIcon &icon)
{
    m_leadingIcon = icon;
    updateGeometry();
    update();
}

void ListItem::setTrailingIcon(const QIcon &icon)
{
    m_trailingIcon = icon;
    updateGeometry();
    update();
}

void ListItem::setSelected(bool selected)
{
    if (m_selected == selected) {
        return;
    }
    m_selected = selected;
    update();
}

void ListItem::setLines(Lines lines)
{
    if (m_lines == lines) {
        return;
    }
    m_lines = lines;
    updateGeometry();
    update();
}

int ListItem::effectiveLines() const
{
    int l = static_cast<int>(m_lines);
    if (!m_supporting.isEmpty() && l < 1) {
        l = 1;
    }
    if (!m_overline.isEmpty() && l < 1) {
        l = 1;
    }
    return qBound(0, l, 2);
}

int ListItem::heightToken() const
{
    switch (effectiveLines()) {
    case 2:  return 88;
    case 1:  return 72;
    default: return kMinHeight;
    }
}

QSize ListItem::sizeHint() const
{
    const QFontMetricsF fmH(Theme::instance().font(TypeRole::BodyLarge));
    qreal width = kLeftPadding + kRightPadding;
    if (!m_leadingIcon.isNull()) {
        width += kIconSize + kIconGap;
    }
    if (!m_trailing.isEmpty()) {
        width += fmH.horizontalAdvance(m_trailing) + kIconGap;
    }
    if (!m_trailingIcon.isNull()) {
        width += kIconSize + kIconGap;
    }
    width += qMax(fmH.horizontalAdvance(m_headline), qreal(120.0));
    return QSize(static_cast<int>(width + 0.5), heightToken());
}

QSize ListItem::minimumSizeHint() const
{
    return QSize(160, heightToken());
}

void ListItem::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const Md3ColorScheme &c = Theme::instance().colors();

    // Selection container.
    if (m_selected) {
        p.fillRect(rect(), c.secondaryContainer);
    }

    // Hover state layer (8%).
    if (isEnabled() && m_hover && !m_selected) {
        p.fillRect(rect(), withAlpha(c.onSurface, 0.08));
    }

    // Ripple.
    if (isEnabled() && m_ripple) {
        m_ripple->setColor(m_selected ? c.onSecondaryContainer : c.onSurface);
        m_ripple->paint(p, QRectF(rect()), 0.0);
    }

    const QColor primaryText = !isEnabled() ? withAlpha(c.onSurface, 0.38)
                              : (m_selected ? c.onSecondaryContainer : c.onSurface);
    const QColor secondaryText = !isEnabled() ? withAlpha(c.onSurface, 0.38)
                                : (m_selected ? withAlpha(c.onSecondaryContainer, 0.80)
                                              : c.onSurfaceVariant);

    qreal textX = kLeftPadding;
    if (!m_leadingIcon.isNull()) {
        const QRectF iconRect(textX, (height() - kIconSize) / 2.0, kIconSize, kIconSize);
        m_leadingIcon.paint(&p, iconRect.toRect(), Qt::AlignCenter,
                            isEnabled() ? QIcon::Normal : QIcon::Disabled, QIcon::Off);
        textX += kIconSize + kIconGap;
    }

    qreal textRight = width() - kRightPadding;
    if (!m_trailingIcon.isNull()) {
        const QRectF iconRect(textRight - kIconSize, (height() - kIconSize) / 2.0,
                              kIconSize, kIconSize);
        m_trailingIcon.paint(&p, iconRect.toRect(), Qt::AlignCenter,
                             isEnabled() ? QIcon::Normal : QIcon::Disabled, QIcon::Off);
        textRight -= kIconSize + kIconGap;
    }
    if (!m_trailing.isEmpty()) {
        const QFontMetricsF fmT(Theme::instance().font(TypeRole::LabelSmall));
        const qreal w = fmT.horizontalAdvance(m_trailing);
        p.setFont(Theme::instance().font(TypeRole::LabelSmall));
        p.setPen(secondaryText);
        p.drawText(QRectF(textRight - w, 0.0, w, height()),
                   Qt::AlignVCenter | Qt::AlignRight | Qt::TextSingleLine, m_trailing);
        textRight -= w + kIconGap;
    }

    const QRectF textRect(textX, 0.0, qMax(qreal(0.0), textRight - textX), height());

    const QFontMetricsF fmO(Theme::instance().font(TypeRole::LabelSmall));
    const QFontMetricsF fmH(Theme::instance().font(TypeRole::BodyLarge));
    const QFontMetricsF fmS(Theme::instance().font(TypeRole::BodyMedium));
    const qreal oH = m_overline.isEmpty() ? 0.0 : fmO.height();
    const qreal hH = m_headline.isEmpty() ? 0.0 : fmH.height();
    const qreal sH = m_supporting.isEmpty() ? 0.0 : fmS.height();
    qreal ty = (height() - (oH + hH + sH)) / 2.0;

    if (!m_overline.isEmpty()) {
        p.setFont(Theme::instance().font(TypeRole::LabelSmall));
        p.setPen(secondaryText);
        p.drawText(QRectF(textRect.left(), ty, textRect.width(), oH),
                   Qt::AlignVCenter | Qt::AlignLeft | Qt::TextSingleLine, m_overline);
        ty += oH;
    }
    if (!m_headline.isEmpty()) {
        p.setFont(Theme::instance().font(TypeRole::BodyLarge));
        p.setPen(primaryText);
        p.drawText(QRectF(textRect.left(), ty, textRect.width(), hH),
                   Qt::AlignVCenter | Qt::AlignLeft | Qt::TextSingleLine, m_headline);
        ty += hH;
    }
    if (!m_supporting.isEmpty()) {
        p.setFont(Theme::instance().font(TypeRole::BodyMedium));
        p.setPen(secondaryText);
        p.drawText(QRectF(textRect.left(), ty, textRect.width(), sH),
                   Qt::AlignVCenter | Qt::AlignLeft | Qt::TextSingleLine, m_supporting);
    }
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void ListItem::enterEvent(QEnterEvent *event)
#else
void ListItem::enterEvent(QEvent *event)
#endif
{
    m_hover = true;
    if (m_ripple) {
        m_ripple->setHovered(true);
    }
    update();
    QWidget::enterEvent(event);
}

void ListItem::leaveEvent(QEvent *event)
{
    m_hover = false;
    if (m_ripple) {
        m_ripple->setHovered(false);
    }
    update();
    QWidget::leaveEvent(event);
}

void ListItem::mousePressEvent(QMouseEvent *event)
{
    if (isEnabled() && m_ripple && event->button() == Qt::LeftButton) {
        m_pressed = true;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        m_ripple->press(event->position());
#else
        m_ripple->press(QPointF(event->pos()));
#endif
    }
    update();
    QWidget::mousePressEvent(event);
}

void ListItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_ripple) {
        m_ripple->release();
    }
    const bool inside = rect().contains(event->pos());
    if (isEnabled() && m_pressed && inside) {
        emit clicked();
    }
    m_pressed = false;
    update();
    QWidget::mouseReleaseEvent(event);
}

} // namespace Md3
