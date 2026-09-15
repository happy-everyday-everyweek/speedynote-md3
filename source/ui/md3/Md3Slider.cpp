#include "Md3Slider.h"

#include <QMouseEvent>
#include <QPainter>

#include "Md3Ripple.h"

namespace Md3 {

namespace {

constexpr int kTouch = 48;
constexpr qreal kTrackHeight = 4.0;
constexpr qreal kHandleRadius = 10.0;
constexpr qreal kStateRadius = 20.0;
constexpr qreal kSideMargin = 20.0;

QColor withAlpha(const QColor &color, qreal alpha)
{
    QColor c = color;
    c.setAlphaF(qBound(0.0, alpha, 1.0));
    return c;
}

} // namespace

Slider::Slider(QWidget *parent)
    : QSlider(Qt::Horizontal, parent)
{
    initialize();
}

Slider::Slider(Qt::Orientation orientation, QWidget *parent)
    : QSlider(orientation, parent)
{
    initialize();
}

void Slider::initialize()
{
    setAttribute(Qt::WA_Hover, true);
    setFocusPolicy(Qt::StrongFocus);

    m_ripple = new Ripple(this);
    connect(m_ripple, &Ripple::repaintRequested, this, qOverload<>(&QWidget::update));

    connect(&Theme::instance(), &Theme::changed, this, qOverload<>(&QWidget::update));
}

qreal Slider::fraction() const
{
    const int span = maximum() - minimum();
    if (span <= 0) {
        return 0.0;
    }
    return (value() - minimum()) / static_cast<qreal>(span);
}

qreal Slider::trackLeft() const
{
    return kSideMargin;
}

qreal Slider::trackRight() const
{
    return width() - kSideMargin;
}

qreal Slider::handleCenter() const
{
    const qreal left = trackLeft();
    const qreal right = trackRight();
    return left + (right - left) * fraction();
}

int Slider::valueFromPos(qreal x) const
{
    const qreal span = trackRight() - trackLeft();
    if (span <= 0) {
        return minimum();
    }
    qreal f = (x - trackLeft()) / span;
    f = qBound(0.0, f, 1.0);
    return minimum() + qRound(f * (maximum() - minimum()));
}

QSize Slider::sizeHint() const
{
    return QSize(200, kTouch);
}

QSize Slider::minimumSizeHint() const
{
    return QSize(120, kTouch);
}

void Slider::paintEvent(QPaintEvent *event)
{
    if (orientation() != Qt::Horizontal) {
        QSlider::paintEvent(event);
        return;
    }

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const Md3ColorScheme &c = Theme::instance().colors();
    const qreal cy = height() / 2.0;
    const qreal x0 = trackLeft();
    const qreal x1 = trackRight();
    const qreal hc = handleCenter();

    // State layer + ripple around the handle.
    if (isEnabled() && m_ripple) {
        const QRectF stateRect(hc - kStateRadius, cy - kStateRadius,
                               kStateRadius * 2.0, kStateRadius * 2.0);
        m_ripple->setColor(c.primary);
        m_ripple->paint(p, stateRect, kStateRadius);
    }

    // Inactive track.
    const QRectF track(x0, cy - kTrackHeight / 2.0, x1 - x0, kTrackHeight);
    QColor inactive = isEnabled() ? c.surfaceContainerHighest
                                  : withAlpha(c.onSurface, 0.12);
    p.setPen(Qt::NoPen);
    p.setBrush(inactive);
    p.drawRoundedRect(track, kTrackHeight / 2.0, kTrackHeight / 2.0);

    // Active track.
    if (hc > x0) {
        const QRectF active(x0, cy - kTrackHeight / 2.0, hc - x0, kTrackHeight);
        QColor activeColor = isEnabled() ? c.primary : withAlpha(c.onSurface, 0.38);
        p.setBrush(activeColor);
        p.drawRoundedRect(active, kTrackHeight / 2.0, kTrackHeight / 2.0);
    }

    // Handle.
    QColor handle = isEnabled() ? c.primary : withAlpha(c.onSurface, 0.38);
    p.setBrush(handle);
    p.drawEllipse(QPointF(hc, cy), kHandleRadius, kHandleRadius);
}

void Slider::mousePressEvent(QMouseEvent *event)
{
    if (orientation() != Qt::Horizontal) {
        QSlider::mousePressEvent(event);
        return;
    }
    if (isEnabled() && event->button() == Qt::LeftButton) {
        setSliderDown(true);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        setValue(valueFromPos(event->position().x()));
#else
        setValue(valueFromPos(event->pos().x()));
#endif
        if (m_ripple) {
            m_ripple->press(QPointF(handleCenter(), height() / 2.0));
        }
        event->accept();
        return;
    }
    QSlider::mousePressEvent(event);
}

void Slider::mouseMoveEvent(QMouseEvent *event)
{
    if (orientation() != Qt::Horizontal) {
        QSlider::mouseMoveEvent(event);
        return;
    }
    if (isSliderDown()) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        setValue(valueFromPos(event->position().x()));
#else
        setValue(valueFromPos(event->pos().x()));
#endif
        event->accept();
        return;
    }
    QSlider::mouseMoveEvent(event);
}

void Slider::mouseReleaseEvent(QMouseEvent *event)
{
    if (orientation() != Qt::Horizontal) {
        QSlider::mouseReleaseEvent(event);
        return;
    }
    if (isSliderDown()) {
        setSliderDown(false);
        if (m_ripple) {
            m_ripple->release();
        }
        event->accept();
        return;
    }
    QSlider::mouseReleaseEvent(event);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void Slider::enterEvent(QEnterEvent *event)
#else
void Slider::enterEvent(QEvent *event)
#endif
{
    if (m_ripple) {
        m_ripple->setHovered(true);
    }
    QSlider::enterEvent(event);
}

void Slider::leaveEvent(QEvent *event)
{
    if (m_ripple) {
        m_ripple->setHovered(false);
    }
    QSlider::leaveEvent(event);
}

} // namespace Md3
