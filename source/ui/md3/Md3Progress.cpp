#include "Md3Progress.h"

#include <QHideEvent>
#include <QPainter>
#include <QShowEvent>
#include <QVariantAnimation>
#include <QtMath>

namespace Md3 {

namespace {

constexpr int kLinearDuration = 1800; // one sweep cycle
constexpr int kCircularDuration = 1333;
constexpr qreal kStroke = 4.0;

QColor withAlpha(const QColor &color, qreal alpha)
{
    QColor c = color;
    c.setAlphaF(qBound(0.0, alpha, 1.0));
    return c;
}

} // namespace

// ===========================================================================
// LinearProgress
// ===========================================================================

LinearProgress::LinearProgress(QWidget *parent)
    : QWidget(parent)
{
    m_anim = new QVariantAnimation(this);
    m_anim->setDuration(kLinearDuration);
    m_anim->setStartValue(0.0);
    m_anim->setEndValue(1.0);
    m_anim->setLoopCount(-1);
    m_anim->setEasingCurve(QEasingCurve::Linear);
    connect(m_anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        setPhase(v.toReal());
    });

    connect(&Theme::instance(), &Theme::changed, this, qOverload<>(&QWidget::update));
}

void LinearProgress::setRange(int minimum, int maximum)
{
    if (maximum < minimum) {
        maximum = minimum;
    }
    m_minimum = minimum;
    m_maximum = maximum;
    setValue(qBound(m_minimum, m_value, m_maximum));
    update();
}

void LinearProgress::setValue(int value)
{
    value = qBound(m_minimum, value, m_maximum);
    if (m_value == value) {
        return;
    }
    m_value = value;
    update();
}

void LinearProgress::setIndeterminate(bool indeterminate)
{
    if (m_indeterminate == indeterminate) {
        return;
    }
    m_indeterminate = indeterminate;
    syncAnimation();
    update();
}

void LinearProgress::setPhase(qreal phase)
{
    phase = qBound(0.0, phase, 1.0);
    if (qFuzzyCompare(m_phase, phase + 1.0)) {
        return;
    }
    m_phase = phase;
    update();
}

void LinearProgress::syncAnimation()
{
    if (m_indeterminate && isVisible()) {
        if (m_anim->state() != QAbstractAnimation::Running) {
            m_anim->start();
        }
    } else {
        m_anim->stop();
    }
}

qreal LinearProgress::fraction() const
{
    const int span = m_maximum - m_minimum;
    if (span <= 0) {
        return 0.0;
    }
    return (m_value - m_minimum) / static_cast<qreal>(span);
}

QSize LinearProgress::sizeHint() const
{
    return QSize(160, 4);
}

QSize LinearProgress::minimumSizeHint() const
{
    return QSize(80, 4);
}

void LinearProgress::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    syncAnimation();
}

void LinearProgress::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    m_anim->stop();
}

void LinearProgress::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const Md3ColorScheme &c = Theme::instance().colors();
    const qreal h = 4.0;
    const QRectF track(0.0, (height() - h) / 2.0, width(), h);
    const qreal radius = h / 2.0;

    p.setPen(Qt::NoPen);
    p.setBrush(c.surfaceContainerHighest);
    p.drawRoundedRect(track, radius, radius);

    p.setBrush(c.primary);
    if (m_indeterminate) {
        const qreal w = width();
        const qreal barW = w * 0.4;
        const qreal x = m_phase * (w + barW) - barW;
        const qreal left = qMax(0.0, x);
        const qreal right = qMin(w, x + barW);
        if (right > left + 0.5) {
            const qreal bw = right - left;
            const qreal r = qMin(radius, bw / 2.0);
            p.drawRoundedRect(QRectF(left, track.top(), bw, h), r, r);
        }
    } else {
        const qreal w = width() * fraction();
        if (w > 0.5) {
            const qreal r = qMin(radius, w / 2.0);
            p.drawRoundedRect(QRectF(0.0, track.top(), w, h), r, r);
        }
    }
}

// ===========================================================================
// CircularProgress
// ===========================================================================

CircularProgress::CircularProgress(QWidget *parent)
    : QWidget(parent)
{
    m_anim = new QVariantAnimation(this);
    m_anim->setDuration(kCircularDuration);
    m_anim->setStartValue(0.0);
    m_anim->setEndValue(1.0);
    m_anim->setLoopCount(-1);
    m_anim->setEasingCurve(QEasingCurve::Linear);
    connect(m_anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        setPhase(v.toReal());
    });

    connect(&Theme::instance(), &Theme::changed, this, qOverload<>(&QWidget::update));
}

void CircularProgress::setRange(int minimum, int maximum)
{
    if (maximum < minimum) {
        maximum = minimum;
    }
    m_minimum = minimum;
    m_maximum = maximum;
    setValue(qBound(m_minimum, m_value, m_maximum));
    update();
}

void CircularProgress::setValue(int value)
{
    value = qBound(m_minimum, value, m_maximum);
    if (m_value == value) {
        return;
    }
    m_value = value;
    update();
}

void CircularProgress::setIndeterminate(bool indeterminate)
{
    if (m_indeterminate == indeterminate) {
        return;
    }
    m_indeterminate = indeterminate;
    syncAnimation();
    update();
}

void CircularProgress::setPhase(qreal phase)
{
    phase = qBound(0.0, phase, 1.0);
    if (qFuzzyCompare(m_phase, phase + 1.0)) {
        return;
    }
    m_phase = phase;
    update();
}

void CircularProgress::syncAnimation()
{
    if (m_indeterminate && isVisible()) {
        if (m_anim->state() != QAbstractAnimation::Running) {
            m_anim->start();
        }
    } else {
        m_anim->stop();
    }
}

qreal CircularProgress::fraction() const
{
    const int span = m_maximum - m_minimum;
    if (span <= 0) {
        return 0.0;
    }
    return (m_value - m_minimum) / static_cast<qreal>(span);
}

QSize CircularProgress::sizeHint() const
{
    return QSize(48, 48);
}

QSize CircularProgress::minimumSizeHint() const
{
    return QSize(32, 32);
}

void CircularProgress::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    syncAnimation();
}

void CircularProgress::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    m_anim->stop();
}

void CircularProgress::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const Md3ColorScheme &c = Theme::instance().colors();
    const qreal side = qMin(width(), height()) - kStroke;
    if (side <= 0.0) {
        return;
    }
    const QRectF ring((width() - side) / 2.0, (height() - side) / 2.0, side, side);

    if (m_indeterminate) {
        const qreal rotation = m_phase * 360.0;
        const qreal sweep = 30.0 + 240.0 * 0.5 * (1.0 - qCos(2.0 * M_PI * m_phase));
        QPen pen(c.primary, kStroke, Qt::SolidLine, Qt::RoundCap);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        const int startAngle = static_cast<int>((90.0 - rotation) * 16.0);
        const int spanAngle = static_cast<int>(-sweep * 16.0);
        p.drawArc(ring, startAngle, spanAngle);
    } else {
        QPen trackPen(c.surfaceContainerHighest, kStroke, Qt::SolidLine, Qt::RoundCap);
        p.setPen(trackPen);
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(ring);

        const qreal f = fraction();
        if (f > 0.0) {
            QPen pen(c.primary, kStroke, Qt::SolidLine, Qt::RoundCap);
            p.setPen(pen);
            p.drawArc(ring, 90 * 16, static_cast<int>(-360.0 * f * 16.0));
        }
    }
}

} // namespace Md3
