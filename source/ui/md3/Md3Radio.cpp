#include "Md3Radio.h"

#include <QFontMetricsF>
#include <QMouseEvent>
#include <QPainter>
#include <QVariantAnimation>

#include "Md3Motion.h"
#include "Md3Ripple.h"

namespace Md3 {

namespace {

constexpr int kTouch = 48;
constexpr qreal kRingSize = 20.0;
constexpr qreal kStateRadius = 20.0;
constexpr int kTextStart = 40;
constexpr qreal kBorderWidth = 2.0;

QColor withAlpha(const QColor &color, qreal alpha)
{
    QColor c = color;
    c.setAlphaF(qBound(0.0, alpha, 1.0));
    return c;
}

} // namespace

RadioButton::RadioButton(QWidget *parent)
    : QAbstractButton(parent)
{
    initialize();
}

RadioButton::RadioButton(const QString &text, QWidget *parent)
    : QAbstractButton(parent)
{
    setText(text);
    initialize();
}

void RadioButton::initialize()
{
    setCheckable(true);
    setAutoExclusive(true);
    setAttribute(Qt::WA_Hover, true);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::StrongFocus);

    m_ripple = new Ripple(this);
    connect(m_ripple, &Ripple::repaintRequested, this, qOverload<>(&QWidget::update));

    m_anim = new QVariantAnimation(this);
    m_anim->setDuration(Motion::Short3); // 150 ms
    m_anim->setEasingCurve(Motion::emphasizedDecelerate());
    connect(m_anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        setProgress(v.toReal());
    });

    connect(&Theme::instance(), &Theme::changed, this, qOverload<>(&QWidget::update));
}

void RadioButton::setProgress(qreal progress)
{
    progress = qBound(0.0, progress, 1.0);
    if (qFuzzyCompare(m_progress, progress)) {
        return;
    }
    m_progress = progress;
    update();
}

void RadioButton::animateTo(bool checked)
{
    m_anim->stop();
    m_anim->setStartValue(m_progress);
    m_anim->setEndValue(checked ? 1.0 : 0.0);
    m_anim->start();
}

QSize RadioButton::sizeHint() const
{
    const QFontMetricsF fm(Theme::instance().font(TypeRole::BodyLarge));
    qreal w = kTouch;
    if (!text().isEmpty()) {
        w = kTextStart + fm.horizontalAdvance(text()) + 8.0;
    }
    return QSize(static_cast<int>(w + 0.5), kTouch);
}

QSize RadioButton::minimumSizeHint() const
{
    return sizeHint();
}

void RadioButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const Md3ColorScheme &c = Theme::instance().colors();
    const qreal t = m_progress;
    const qreal cy = height() / 2.0;
    const QRectF ring(24.0 - kRingSize / 2.0, cy - kRingSize / 2.0, kRingSize, kRingSize);

    // State layer + ripple: 40dp circle centered on the ring.
    if (isEnabled() && m_ripple) {
        const QRectF stateRect(24.0 - kStateRadius, cy - kStateRadius,
                               kStateRadius * 2.0, kStateRadius * 2.0);
        m_ripple->setColor(c.primary);
        m_ripple->paint(p, stateRect, kStateRadius);
    }

    // Container fill: fades from transparent to primary.
    QColor fill = withAlpha(c.primary, t);
    if (!isEnabled()) {
        fill = withAlpha(c.onSurface, t * 0.38);
    }
    if (fill.alphaF() > 0.001) {
        p.setPen(Qt::NoPen);
        p.setBrush(fill);
        p.drawEllipse(ring);
    }

    // Outline: fades out while the container fades in.
    if (t < 1.0) {
        QColor border;
        if (isEnabled()) {
            border = withAlpha(c.onSurfaceVariant, 1.0 - t);
        } else {
            border = withAlpha(c.onSurface, 0.38);
        }
        QPen pen(border, kBorderWidth);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        const qreal inset = kBorderWidth / 2.0;
        p.drawEllipse(ring.adjusted(inset, inset, -inset, -inset));
    }

    // Inner dot: scales in while the container fills.
    if (t > 0.01) {
        const qreal radius = 5.0 * t;
        QColor dot = isEnabled() ? c.onPrimary : c.surface;
        p.setPen(Qt::NoPen);
        p.setBrush(dot);
        p.drawEllipse(ring.center(), radius, radius);
    }

    // Label.
    if (!text().isEmpty()) {
        p.setFont(Theme::instance().font(TypeRole::BodyLarge));
        p.setPen(isEnabled() ? c.onSurface : withAlpha(c.onSurface, 0.38));
        const QRectF textRect(kTextStart, 0.0, width() - kTextStart - 4.0, height());
        p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft | Qt::TextSingleLine, text());
    }
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void RadioButton::enterEvent(QEnterEvent *event)
#else
void RadioButton::enterEvent(QEvent *event)
#endif
{
    if (m_ripple) {
        m_ripple->setHovered(true);
    }
    QAbstractButton::enterEvent(event);
}

void RadioButton::leaveEvent(QEvent *event)
{
    if (m_ripple) {
        m_ripple->setHovered(false);
    }
    QAbstractButton::leaveEvent(event);
}

void RadioButton::mousePressEvent(QMouseEvent *event)
{
    if (isEnabled() && m_ripple && event->button() == Qt::LeftButton) {
        m_ripple->press(QPointF(24.0, height() / 2.0));
    }
    QAbstractButton::mousePressEvent(event);
}

void RadioButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_ripple) {
        m_ripple->release();
    }
    QAbstractButton::mouseReleaseEvent(event);
}

void RadioButton::checkStateSet()
{
    QAbstractButton::checkStateSet();
    animateTo(isChecked());
}

void RadioButton::nextCheckState()
{
    QAbstractButton::nextCheckState();
    animateTo(isChecked());
}

} // namespace Md3
