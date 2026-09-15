#include "Md3Checkbox.h"

#include <QFontMetricsF>
#include <QMouseEvent>
#include <QPainter>
#include <QVariantAnimation>

#include "Md3Motion.h"
#include "Md3Ripple.h"

namespace Md3 {

namespace {

constexpr int kBoxSize = 18;
constexpr int kTouch = 48;
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

CheckBox::CheckBox(QWidget *parent)
    : QAbstractButton(parent)
{
    initialize();
}

CheckBox::CheckBox(const QString &text, QWidget *parent)
    : QAbstractButton(parent)
{
    setText(text);
    initialize();
}

void CheckBox::initialize()
{
    setCheckable(true);
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

void CheckBox::setProgress(qreal progress)
{
    progress = qBound(0.0, progress, 1.0);
    if (qFuzzyCompare(m_progress, progress)) {
        return;
    }
    m_progress = progress;
    update();
}

void CheckBox::animateTo(bool checked)
{
    m_anim->stop();
    m_anim->setStartValue(m_progress);
    m_anim->setEndValue(checked ? 1.0 : 0.0);
    m_anim->start();
}

QSize CheckBox::sizeHint() const
{
    const QFontMetricsF fm(Theme::instance().font(TypeRole::BodyLarge));
    qreal w = kTouch;
    if (!text().isEmpty()) {
        w = kTextStart + fm.horizontalAdvance(text()) + 8.0;
    }
    return QSize(static_cast<int>(w + 0.5), kTouch);
}

QSize CheckBox::minimumSizeHint() const
{
    return sizeHint();
}

void CheckBox::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const Md3ColorScheme &c = Theme::instance().colors();
    const qreal t = m_progress;
    const qreal cy = height() / 2.0;
    const QRectF box(15.0, cy - kBoxSize / 2.0, kBoxSize, kBoxSize);

    // State layer + ripple: 40dp circle centered on the box.
    if (isEnabled() && m_ripple) {
        const QRectF stateRect(24.0 - kStateRadius, cy - kStateRadius,
                               kStateRadius * 2.0, kStateRadius * 2.0);
        m_ripple->setColor(c.onSurface);
        m_ripple->paint(p, stateRect, kStateRadius);
    }

    // Container: fades from transparent to primary.
    QColor fill = withAlpha(c.primary, t);
    if (!isEnabled()) {
        fill = withAlpha(c.onSurface, t * 0.38);
    }
    if (fill.alphaF() > 0.001) {
        p.setPen(Qt::NoPen);
        p.setBrush(fill);
        p.drawRoundedRect(box, 2.0, 2.0);
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
        p.drawRoundedRect(box.adjusted(inset, inset, -inset, -inset), 2.0, 2.0);
    }

    // Check mark: two segments drawn in sequence.
    if (t > 0.02) {
        const QPointF p1(box.left() + kBoxSize * 0.27, box.top() + kBoxSize * 0.53);
        const QPointF p2(box.left() + kBoxSize * 0.43, box.top() + kBoxSize * 0.70);
        const QPointF p3(box.left() + kBoxSize * 0.75, box.top() + kBoxSize * 0.31);

        QColor check = isEnabled() ? c.onPrimary : c.surface;
        QPen pen(check, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);

        if (t <= 0.5) {
            const qreal k = t / 0.5;
            p.drawLine(p1, QPointF(p1.x() + (p2.x() - p1.x()) * k,
                                   p1.y() + (p2.y() - p1.y()) * k));
        } else {
            const qreal k = (t - 0.5) / 0.5;
            p.drawLine(p1, p2);
            p.drawLine(p2, QPointF(p2.x() + (p3.x() - p2.x()) * k,
                                   p2.y() + (p3.y() - p2.y()) * k));
        }
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
void CheckBox::enterEvent(QEnterEvent *event)
#else
void CheckBox::enterEvent(QEvent *event)
#endif
{
    if (m_ripple) {
        m_ripple->setHovered(true);
    }
    QAbstractButton::enterEvent(event);
}

void CheckBox::leaveEvent(QEvent *event)
{
    if (m_ripple) {
        m_ripple->setHovered(false);
    }
    QAbstractButton::leaveEvent(event);
}

void CheckBox::mousePressEvent(QMouseEvent *event)
{
    if (isEnabled() && m_ripple && event->button() == Qt::LeftButton) {
        m_ripple->press(QPointF(24.0, height() / 2.0));
    }
    QAbstractButton::mousePressEvent(event);
}

void CheckBox::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_ripple) {
        m_ripple->release();
    }
    QAbstractButton::mouseReleaseEvent(event);
}

void CheckBox::checkStateSet()
{
    QAbstractButton::checkStateSet();
    animateTo(isChecked());
}

void CheckBox::nextCheckState()
{
    QAbstractButton::nextCheckState();
    animateTo(isChecked());
}

} // namespace Md3
