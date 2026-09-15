#include "Md3SegmentedButton.h"

#include <QFontMetricsF>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QVariantAnimation>

#include "Md3Motion.h"
#include "Md3Ripple.h"

namespace Md3 {

namespace {

constexpr int kHeight = 40;
constexpr int kMinSegment = 64;

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

SegmentedButton::SegmentedButton(QWidget *parent)
    : QWidget(parent)
{
    initialize();
}

void SegmentedButton::initialize()
{
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    m_ripple = new Ripple(this);
    connect(m_ripple, &Ripple::repaintRequested, this, qOverload<>(&QWidget::update));

    m_anim = new QVariantAnimation(this);
    m_anim->setDuration(Motion::Short4); // 200 ms, per the M3 segmented spec
    m_anim->setEasingCurve(Motion::emphasizedDecelerate());
    connect(m_anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        setSelectionProgress(v.toReal());
    });

    connect(&Theme::instance(), &Theme::changed, this, qOverload<>(&QWidget::update));
}

void SegmentedButton::addSegment(const QString &text)
{
    m_labels.append(text);
    updateGeometry();
    update();
}

void SegmentedButton::clearSegments()
{
    m_labels.clear();
    m_current = 0;
    m_previous = -1;
    m_hover = -1;
    m_pressed = -1;
    updateGeometry();
    update();
}

void SegmentedButton::setCurrentIndex(int index)
{
    if (index < 0 || index >= m_labels.size() || index == m_current) {
        return;
    }
    m_previous = m_current;
    m_current = index;
    m_anim->stop();
    m_anim->setStartValue(0.0);
    m_anim->setEndValue(1.0);
    m_anim->start();
    update();
    emit currentChanged(index);
}

void SegmentedButton::setSelectionProgress(qreal progress)
{
    progress = qBound(0.0, progress, 1.0);
    if (qFuzzyCompare(m_progress, progress)) {
        return;
    }
    m_progress = progress;
    if (m_progress >= 1.0) {
        m_previous = -1;
    }
    update();
}

QSize SegmentedButton::sizeHint() const
{
    const QFontMetricsF fm(Theme::instance().font(TypeRole::LabelLarge));
    qreal seg = kMinSegment;
    for (const QString &label : m_labels) {
        seg = qMax(seg, fm.horizontalAdvance(label) + 24.0);
    }
    const int n = qMax(1, m_labels.size());
    return QSize(static_cast<int>(seg * n + 2.0 + 0.5), kHeight);
}

QSize SegmentedButton::minimumSizeHint() const
{
    const QFontMetricsF fm(Theme::instance().font(TypeRole::LabelLarge));
    qreal seg = 48.0;
    for (const QString &label : m_labels) {
        seg = qMax(seg, fm.horizontalAdvance(label) + 16.0);
    }
    const int n = qMax(1, m_labels.size());
    return QSize(static_cast<int>(seg * n + 2.0 + 0.5), kHeight);
}

QRectF SegmentedButton::segmentRect(int index) const
{
    const int n = qMax(1, m_labels.size());
    const qreal segW = (width() - 2.0) / n;
    return QRectF(1.0 + index * segW, 1.0, segW, height() - 2.0);
}

int SegmentedButton::segmentAt(const QPointF &pos) const
{
    const int n = m_labels.size();
    if (n <= 0 || !QRectF(rect()).contains(pos)) {
        return -1;
    }
    const qreal segW = (width() - 2.0) / n;
    if (segW <= 0.0) {
        return -1;
    }
    const int idx = static_cast<int>((pos.x() - 1.0) / segW);
    return qBound(0, idx, n - 1);
}

void SegmentedButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const int n = m_labels.size();
    if (n <= 0) {
        return;
    }

    const Md3ColorScheme &c = Theme::instance().colors();
    const qreal radius = height() / 2.0;
    const QRectF outer = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);

    // Track outline.
    p.setPen(QPen(c.outline, 1.0));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(outer, radius, radius);

    // Clip path matching the inside of the outline.
    QPainterPath clip;
    clip.addRoundedRect(outer.adjusted(1.0, 1.0, -1.0, -1.0), radius - 1.0, radius - 1.0);

    // Selected segment fill, cross-fading between the previous and the new
    // segment while the selection animation runs.
    auto paintFill = [&](int index, qreal alpha) {
        if (index < 0 || alpha <= 0.003) {
            return;
        }
        QPainterPath seg;
        seg.addRect(segmentRect(index));
        QColor fill = isEnabled() ? c.secondaryContainer : withAlpha(c.onSurface, 0.12);
        fill.setAlphaF(fill.alphaF() * qBound(0.0, alpha, 1.0));
        p.fillPath(clip.intersected(seg), fill);
    };
    paintFill(m_previous, 1.0 - m_progress);
    paintFill(m_current, m_progress);

    // Hover state layer (8%) on an unselected segment.
    if (isEnabled() && m_hover >= 0 && m_hover != m_current && m_pressed < 0) {
        QPainterPath seg;
        seg.addRect(segmentRect(m_hover));
        p.fillPath(clip.intersected(seg), withAlpha(c.onSurface, 0.08));
    }

    // Dividers between segments, skipped around the selected one.
    if (n > 1) {
        p.setPen(QPen(c.outlineVariant, 1.0));
        for (int i = 1; i < n; ++i) {
            if (i - 1 == m_current || i == m_current) {
                continue;
            }
            const qreal x = segmentRect(i).left();
            p.drawLine(QPointF(x, 1.0 + 8.0), QPointF(x, height() - 1.0 - 8.0));
        }
    }

    // Press ripple (clipped to the track).
    if (isEnabled() && m_ripple) {
        m_ripple->setColor(c.onSurface);
        m_ripple->paint(p, outer, radius);
    }

    // Segment labels.
    p.setFont(Theme::instance().font(TypeRole::LabelLarge));
    for (int i = 0; i < n; ++i) {
        const bool selected = i == m_current;
        QColor text;
        if (!isEnabled()) {
            text = withAlpha(c.onSurface, 0.38);
        } else {
            text = selected ? c.onSecondaryContainer : c.onSurface;
        }
        p.setPen(text);
        p.drawText(segmentRect(i), Qt::AlignCenter, m_labels.at(i));
    }
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void SegmentedButton::enterEvent(QEnterEvent *event)
#else
void SegmentedButton::enterEvent(QEvent *event)
#endif
{
    QWidget::enterEvent(event);
}

void SegmentedButton::leaveEvent(QEvent *event)
{
    if (m_hover != -1) {
        m_hover = -1;
        update();
    }
    QWidget::leaveEvent(event);
}

void SegmentedButton::mouseMoveEvent(QMouseEvent *event)
{
    const int seg = segmentAt(eventPos(event));
    if (seg != m_hover) {
        m_hover = seg;
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void SegmentedButton::mousePressEvent(QMouseEvent *event)
{
    const QPointF pos = eventPos(event);
    m_pressed = segmentAt(pos);
    if (isEnabled() && m_pressed >= 0 && m_ripple) {
        m_ripple->press(pos);
    }
    update();
    QWidget::mousePressEvent(event);
}

void SegmentedButton::mouseReleaseEvent(QMouseEvent *event)
{
    const int seg = segmentAt(eventPos(event));
    if (m_ripple) {
        m_ripple->release();
    }
    if (isEnabled() && seg >= 0 && seg == m_pressed) {
        setCurrentIndex(seg);
    }
    m_pressed = -1;
    update();
    QWidget::mouseReleaseEvent(event);
}

} // namespace Md3
