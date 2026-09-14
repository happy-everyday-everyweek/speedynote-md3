#include "Md3Switch.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QVariantAnimation>

#include "Md3Motion.h"
#include "Md3Ripple.h"

namespace Md3 {

namespace {

QColor mixColor(const QColor &a, const QColor &b, qreal t)
{
    t = qBound(0.0, t, 1.0);
    return QColor::fromRgbF(a.redF() + (b.redF() - a.redF()) * t,
                            a.greenF() + (b.greenF() - a.greenF()) * t,
                            a.blueF() + (b.blueF() - a.blueF()) * t,
                            a.alphaF() + (b.alphaF() - a.alphaF()) * t);
}

qreal lerp(qreal a, qreal b, qreal t)
{
    return a + (b - a) * qBound(0.0, t, 1.0);
}

} // namespace

Switch::Switch(QWidget *parent)
    : QAbstractButton(parent)
{
    initialize();
}

void Switch::initialize()
{
    setCheckable(true);
    setAttribute(Qt::WA_Hover, true);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::StrongFocus);

    m_ripple = new Ripple(this);
    connect(m_ripple, &Ripple::repaintRequested, this, qOverload<>(&QWidget::update));

    m_anim = new QVariantAnimation(this);
    m_anim->setDuration(Motion::Short4); // 200ms
    m_anim->setEasingCurve(Motion::emphasizedDecelerate());
    connect(m_anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        setProgress(v.toReal());
    });

    connect(&Theme::instance(), &Theme::changed, this, qOverload<>(&QWidget::update));
}

void Switch::setProgress(qreal progress)
{
    progress = qBound(0.0, progress, 1.0);
    if (qFuzzyCompare(m_progress, progress)) {
        return;
    }
    m_progress = progress;
    update();
}

void Switch::animateTo(bool checked)
{
    m_anim->stop();
    m_anim->setStartValue(m_progress);
    m_anim->setEndValue(checked ? 1.0 : 0.0);
    m_anim->start();
}

QSize Switch::sizeHint() const
{
    // Track 52x32 centered in a 56x40 touch-friendly box (room for the
    // 40dp state-layer circle around the handle).
    return QSize(56, 40);
}

QSize Switch::minimumSizeHint() const
{
    return sizeHint();
}

void Switch::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const Md3ColorScheme &c = Theme::instance().colors();
    const qreal t = m_progress;

    const QRectF box(rect());
    const QRectF track(box.center().x() - 26.0, box.center().y() - 16.0, 52.0, 32.0);
    const qreal trackRadius = 16.0;

    const QColor trackOff = c.surfaceContainerHighest;
    const QColor trackOn = c.primary;
    const QColor outline = c.outline;

    // --- Track ----------------------------------------------------------------
    QColor trackColor = mixColor(trackOff, trackOn, t);
    if (!isEnabled()) {
        QColor disabled = c.onSurface;
        disabled.setAlphaF(0.12);
        trackColor = disabled;
    }
    p.setPen(Qt::NoPen);
    p.setBrush(trackColor);
    p.drawRoundedRect(track, trackRadius, trackRadius);

    if (t < 1.0) {
        // Outline fades out as the switch turns on.
        QColor border = outline;
        border.setAlphaF(border.alphaF() * (1.0 - t));
        QPen pen(border, 1.0);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(track.adjusted(0.5, 0.5, -0.5, -0.5), trackRadius - 0.5, trackRadius - 0.5);
    }

    // --- Handle ----------------------------------------------------------------
    const qreal diameter = lerp(24.0, 28.0, t);
    const qreal centerX = lerp(track.left() + 16.0, track.right() - 16.0, t);
    const qreal centerY = track.center().y();
    const QRectF handleRect(centerX - diameter / 2.0, centerY - diameter / 2.0, diameter, diameter);

    const QColor handleOff = outline;
    const QColor handleOn = c.onPrimary;
    QColor handleColor = mixColor(handleOff, handleOn, t);
    if (!isEnabled()) {
        handleColor = c.surface;
    }
    p.setPen(Qt::NoPen);
    p.setBrush(handleColor);
    p.drawEllipse(handleRect);

    // --- State layer + ripple (40dp circle around the handle) --------------------
    if (isEnabled() && m_ripple) {
        const QRectF stateRect(centerX - 20.0, centerY - 20.0, 40.0, 40.0);
        QColor layerColor = mixColor(c.onSurface, c.primary, t);
        m_ripple->setColor(layerColor);
        m_ripple->paint(p, stateRect, 20.0);
    }

    // --- Check icon when mostly on ------------------------------------------------
    if (t > 0.55) {
        const qreal k = diameter / 28.0;
        QPainterPath check;
        check.moveTo(centerX - 6.5 * k, centerY + 0.5 * k);
        check.lineTo(centerX - 2.0 * k, centerY + 4.5 * k);
        check.lineTo(centerX + 6.5 * k, centerY - 4.5 * k);
        QColor checkColor = isEnabled() ? mixColor(c.onPrimaryContainer, c.primary, 1.0) : c.onSurface;
        QPen pen(checkColor, 2.0 * k, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        p.setOpacity(qBound(0.0, (t - 0.55) / 0.45, 1.0));
        p.drawPath(check);
        p.setOpacity(1.0);
    }
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void Switch::enterEvent(QEnterEvent *event)
#else
void Switch::enterEvent(QEvent *event)
#endif
{
    if (m_ripple) {
        m_ripple->setHovered(true);
    }
    QAbstractButton::enterEvent(event);
}

void Switch::leaveEvent(QEvent *event)
{
    if (m_ripple) {
        m_ripple->setHovered(false);
    }
    QAbstractButton::leaveEvent(event);
}

void Switch::mousePressEvent(QMouseEvent *event)
{
    if (isEnabled() && m_ripple && event->button() == Qt::LeftButton) {
        const QRectF box(rect());
        const qreal centerX = box.center().x() + (m_progress > 0.5 ? 10.0 : -10.0);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        Q_UNUSED(centerX)
        m_ripple->press(event->position());
#else
        Q_UNUSED(centerX)
        m_ripple->press(event->pos());
#endif
    }
    QAbstractButton::mousePressEvent(event);
}

void Switch::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_ripple) {
        m_ripple->release();
    }
    QAbstractButton::mouseReleaseEvent(event);
}

void Switch::checkStateSet()
{
    QAbstractButton::checkStateSet();
    animateTo(isChecked());
}

void Switch::nextCheckState()
{
    QAbstractButton::nextCheckState();
    animateTo(isChecked());
}

} // namespace Md3