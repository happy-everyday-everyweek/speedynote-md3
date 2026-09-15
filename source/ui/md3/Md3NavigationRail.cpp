#include "Md3NavigationRail.h"

#include <QMouseEvent>
#include <QPainter>

#include "Md3Ripple.h"

namespace Md3 {

namespace {

constexpr qreal kRailWidth = 80.0;
constexpr qreal kItemFull = 72.0;
constexpr qreal kItemIconOnly = 56.0;
constexpr qreal kPillWidth = 56.0;
constexpr qreal kPillHeight = 32.0;
constexpr qreal kIconSize = 24.0;

QColor withAlpha(const QColor &color, qreal alpha)
{
    QColor c = color;
    c.setAlphaF(qBound(0.0, alpha, 1.0));
    return c;
}

inline QPointF eventPos(const QMouseEvent *event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return event->position();
#else
    return QPointF(event->pos());
#endif
}

} // namespace

NavigationRail::NavigationRail(QWidget *parent)
    : QWidget(parent)
{
    initialize();
}

void NavigationRail::initialize()
{
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);

    m_ripple = new Ripple(this);
    connect(m_ripple, &Ripple::repaintRequested, this, qOverload<>(&QWidget::update));
    connect(&Theme::instance(), &Theme::changed, this, qOverload<>(&QWidget::update));
}

void NavigationRail::addItem(const QIcon &icon, const QString &label)
{
    Item item;
    item.icon = icon;
    item.label = label;
    m_items.append(item);
    updateGeometry();
    update();
}

void NavigationRail::clearItems()
{
    m_items.clear();
    m_current = 0;
    m_hover = -1;
    m_pressed = -1;
    updateGeometry();
    update();
}

void NavigationRail::setCurrentIndex(int index)
{
    if (index < 0 || index >= m_items.size() || index == m_current) {
        return;
    }
    m_current = index;
    update();
    emit currentChanged(index);
}

void NavigationRail::setShowLabels(bool show)
{
    if (m_showLabels == show) {
        return;
    }
    m_showLabels = show;
    updateGeometry();
    update();
}

qreal NavigationRail::itemHeight() const
{
    return m_showLabels ? kItemFull : kItemIconOnly;
}

QRectF NavigationRail::pillRect(int index) const
{
    const qreal ih = itemHeight();
    const qreal top = index * ih + (m_showLabels ? 8.0 : (ih - kPillHeight) / 2.0);
    return QRectF((width() - kPillWidth) / 2.0, top, kPillWidth, kPillHeight);
}

int NavigationRail::itemAt(const QPointF &pos) const
{
    if (m_items.isEmpty() || !QRectF(rect()).contains(pos)) {
        return -1;
    }
    const int idx = static_cast<int>(pos.y() / itemHeight());
    return qBound(0, idx, m_items.size() - 1);
}

QSize NavigationRail::sizeHint() const
{
    return QSize(static_cast<int>(kRailWidth), static_cast<int>(m_items.size() * itemHeight()));
}

QSize NavigationRail::minimumSizeHint() const
{
    return sizeHint();
}

void NavigationRail::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const Md3ColorScheme &c = Theme::instance().colors();

    for (int i = 0; i < m_items.size(); ++i) {
        const bool selected = i == m_current;
        const QRectF pill = pillRect(i);

        // Selected pill / hover state layer.
        if (selected) {
            p.setPen(Qt::NoPen);
            p.setBrush(c.secondaryContainer);
            p.drawRoundedRect(pill, kPillHeight / 2.0, kPillHeight / 2.0);
        } else if (isEnabled() && i == m_hover && m_pressed < 0) {
            p.setPen(Qt::NoPen);
            p.setBrush(withAlpha(c.onSurface, 0.08));
            p.drawRoundedRect(pill, kPillHeight / 2.0, kPillHeight / 2.0);
        }

        // Ripple on the pressed item.
        if (isEnabled() && m_ripple && i == m_pressed) {
            m_ripple->setColor(c.onSurface);
            m_ripple->paint(p, pill, kPillHeight / 2.0);
        }

        // Icon.
        const QRectF iconRect(pill.center().x() - kIconSize / 2.0,
                              pill.center().y() - kIconSize / 2.0,
                              kIconSize, kIconSize);
        const QColor iconColor = !isEnabled() ? withAlpha(c.onSurface, 0.38)
                                 : (selected ? c.onSecondaryContainer : c.onSurfaceVariant);
        if (!m_items.at(i).icon.isNull()) {
            m_items.at(i).icon.paint(&p, iconRect.toRect(), Qt::AlignCenter,
                                     isEnabled() ? QIcon::Normal : QIcon::Disabled,
                                     selected ? QIcon::On : QIcon::Off);
        } else {
            QPen pen(iconColor, 1.6);
            p.setPen(pen);
            p.setBrush(Qt::NoBrush);
            p.drawRoundedRect(iconRect.adjusted(4.0, 4.0, -4.0, -4.0), 4.0, 4.0);
        }

        // Label.
        if (m_showLabels && !m_items.at(i).label.isEmpty()) {
            p.setFont(Theme::instance().font(TypeRole::LabelMedium));
            p.setPen(selected ? c.onSurface : c.onSurfaceVariant);
            p.drawText(QRectF(4.0, pill.bottom() + 2.0, width() - 8.0, 16.0),
                       Qt::AlignHCenter | Qt::AlignTop | Qt::TextSingleLine,
                       m_items.at(i).label);
        }
    }
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void NavigationRail::enterEvent(QEnterEvent *event)
#else
void NavigationRail::enterEvent(QEvent *event)
#endif
{
    QWidget::enterEvent(event);
}

void NavigationRail::leaveEvent(QEvent *event)
{
    if (m_hover != -1) {
        m_hover = -1;
        update();
    }
    QWidget::leaveEvent(event);
}

void NavigationRail::mouseMoveEvent(QMouseEvent *event)
{
    const int item = itemAt(eventPos(event));
    if (item != m_hover) {
        m_hover = item;
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void NavigationRail::mousePressEvent(QMouseEvent *event)
{
    const QPointF pos = eventPos(event);
    m_pressed = itemAt(pos);
    if (isEnabled() && m_pressed >= 0 && m_ripple) {
        m_ripple->press(pos);
    }
    update();
    QWidget::mousePressEvent(event);
}

void NavigationRail::mouseReleaseEvent(QMouseEvent *event)
{
    const int item = itemAt(eventPos(event));
    if (m_ripple) {
        m_ripple->release();
    }
    if (isEnabled() && item >= 0 && item == m_pressed) {
        setCurrentIndex(item);
    }
    m_pressed = -1;
    update();
    QWidget::mouseReleaseEvent(event);
}

} // namespace Md3
