#include "Md3TabBar.h"

#include <QFontMetricsF>
#include <QMouseEvent>
#include <QPainter>
#include <QVariantAnimation>

#include "Md3Motion.h"
#include "Md3Ripple.h"

namespace Md3 {

namespace {

constexpr int kHeight = 48;
constexpr int kIndicatorHeight = 3;
constexpr int kTabPadding = 16;
constexpr qreal kIndicatorInset = 16.0;

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

TabBar::TabBar(QWidget *parent)
    : QWidget(parent)
{
    initialize();
}

void TabBar::initialize()
{
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_ripple = new Ripple(this);
    connect(m_ripple, &Ripple::repaintRequested, this, qOverload<>(&QWidget::update));

    m_anim = new QVariantAnimation(this);
    m_anim->setDuration(Motion::Medium1); // 250 ms
    m_anim->setEasingCurve(Motion::emphasizedDecelerate());
    connect(m_anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        m_progress = qBound(0.0, v.toReal(), 1.0);
        update();
    });

    connect(&Theme::instance(), &Theme::changed, this, qOverload<>(&QWidget::update));
}

void TabBar::addTab(const QString &text)
{
    m_tabs.append(text);
    retargetIndicator(false);
    updateGeometry();
    update();
}

void TabBar::insertTab(int index, const QString &text)
{
    m_tabs.insert(qBound(0, index, m_tabs.size()), text);
    retargetIndicator(false);
    updateGeometry();
    update();
}

void TabBar::clearTabs()
{
    m_tabs.clear();
    m_current = 0;
    m_hover = -1;
    m_pressed = -1;
    retargetIndicator(false);
    updateGeometry();
    update();
}

void TabBar::setCurrentIndex(int index)
{
    if (index < 0 || index >= m_tabs.size() || index == m_current) {
        return;
    }
    m_prevLeft = m_targetLeft;
    m_prevWidth = m_targetWidth;
    m_current = index;
    retargetIndicator(true);
    emit currentChanged(index);
}

void TabBar::retargetIndicator(bool animate)
{
    if (m_tabs.isEmpty()) {
        m_targetLeft = 0.0;
        m_targetWidth = 0.0;
        m_progress = 1.0;
        return;
    }
    const QRectF r = tabRect(m_current);
    const qreal inset = qMin(kIndicatorInset, r.width() / 4.0);
    m_targetLeft = r.left() + inset;
    m_targetWidth = qMax(qreal(0.0), r.width() - inset * 2.0);
    if (!animate) {
        m_prevLeft = m_targetLeft;
        m_prevWidth = m_targetWidth;
        m_progress = 1.0;
        update();
        return;
    }
    m_anim->stop();
    m_anim->setStartValue(0.0);
    m_anim->setEndValue(1.0);
    m_anim->start();
}

QRectF TabBar::tabRect(int index) const
{
    const QFontMetricsF fm(Theme::instance().font(TypeRole::LabelLarge));
    qreal x = 0.0;
    for (int i = 0; i < m_tabs.size(); ++i) {
        const qreal w = fm.horizontalAdvance(m_tabs.at(i)) + kTabPadding * 2.0;
        if (i == index) {
            return QRectF(x, 0.0, w, height());
        }
        x += w;
    }
    return QRectF();
}

int TabBar::tabAt(const QPointF &pos) const
{
    if (m_tabs.isEmpty() || !QRectF(rect()).contains(pos)) {
        return -1;
    }
    const QFontMetricsF fm(Theme::instance().font(TypeRole::LabelLarge));
    qreal x = 0.0;
    for (int i = 0; i < m_tabs.size(); ++i) {
        const qreal w = fm.horizontalAdvance(m_tabs.at(i)) + kTabPadding * 2.0;
        if (pos.x() >= x && pos.x() < x + w) {
            return i;
        }
        x += w;
    }
    return -1;
}

QSize TabBar::sizeHint() const
{
    const QFontMetricsF fm(Theme::instance().font(TypeRole::LabelLarge));
    qreal width = 0.0;
    for (const QString &label : m_tabs) {
        width += fm.horizontalAdvance(label) + kTabPadding * 2.0;
    }
    return QSize(static_cast<int>(width + 0.5), kHeight);
}

QSize TabBar::minimumSizeHint() const
{
    return sizeHint();
}

void TabBar::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    if (m_tabs.isEmpty()) {
        return;
    }

    const Md3ColorScheme &c = Theme::instance().colors();

    // Divider under the tab row.
    p.fillRect(QRectF(0.0, height() - 1.0, width(), 1.0), c.outlineVariant);

    // Hover state layer on the hovered tab.
    if (isEnabled() && m_hover >= 0 && m_pressed < 0) {
        const QRectF hr = tabRect(m_hover);
        p.fillRect(hr.adjusted(0.0, 4.0, 0.0, -5.0), withAlpha(c.onSurface, 0.08));
    }

    // Ripple on the pressed tab.
    if (isEnabled() && m_ripple && m_pressed >= 0) {
        m_ripple->setColor(c.onSurface);
        m_ripple->paint(p, tabRect(m_pressed).adjusted(0.0, 4.0, 0.0, -5.0), 0.0);
    }

    // Labels.
    p.setFont(Theme::instance().font(TypeRole::LabelLarge));
    for (int i = 0; i < m_tabs.size(); ++i) {
        const bool selected = i == m_current;
        QColor color = !isEnabled() ? withAlpha(c.onSurface, 0.38)
                       : (selected ? c.primary : c.onSurfaceVariant);
        p.setPen(color);
        p.drawText(tabRect(i), Qt::AlignCenter, m_tabs.at(i));
    }

    // Animated indicator.
    const qreal t = m_progress;
    const qreal left = m_prevLeft + (m_targetLeft - m_prevLeft) * t;
    const qreal w = m_prevWidth + (m_targetWidth - m_prevWidth) * t;
    if (w > 0.5) {
        const QRectF ind(left, height() - 1.0 - kIndicatorHeight, w, kIndicatorHeight);
        p.setPen(Qt::NoPen);
        p.setBrush(c.primary);
        p.drawRoundedRect(ind, 1.5, 1.5);
    }
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void TabBar::enterEvent(QEnterEvent *event)
#else
void TabBar::enterEvent(QEvent *event)
#endif
{
    QWidget::enterEvent(event);
}

void TabBar::leaveEvent(QEvent *event)
{
    if (m_hover != -1) {
        m_hover = -1;
        update();
    }
    QWidget::leaveEvent(event);
}

void TabBar::mouseMoveEvent(QMouseEvent *event)
{
    const int tab = tabAt(eventPos(event));
    if (tab != m_hover) {
        m_hover = tab;
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void TabBar::mousePressEvent(QMouseEvent *event)
{
    const QPointF pos = eventPos(event);
    m_pressed = tabAt(pos);
    if (isEnabled() && m_pressed >= 0 && m_ripple) {
        m_ripple->press(pos);
    }
    update();
    QWidget::mousePressEvent(event);
}

void TabBar::mouseReleaseEvent(QMouseEvent *event)
{
    const int tab = tabAt(eventPos(event));
    if (m_ripple) {
        m_ripple->release();
    }
    if (isEnabled() && tab >= 0 && tab == m_pressed) {
        setCurrentIndex(tab);
    }
    m_pressed = -1;
    update();
    QWidget::mouseReleaseEvent(event);
}

} // namespace Md3
