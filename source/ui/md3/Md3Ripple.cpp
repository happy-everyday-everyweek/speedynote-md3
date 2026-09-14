#include "Md3Ripple.h"

#include <QLineF>
#include <QPainter>
#include <QPainterPath>
#include <QVariantAnimation>
#include <QWidget>

#include "Md3Motion.h"

namespace Md3 {

namespace {
constexpr qreal kRippleAlpha = 0.12;   // ripple base opacity
constexpr qreal kHoverAlpha = 0.08;    // state layer: hover
constexpr qreal kFocusAlpha = 0.10;    // state layer: focus
constexpr qreal kPressedAlpha = 0.10;  // state layer: pressed
} // namespace

Ripple::Ripple(QWidget *host, QObject *parent)
    : QObject(parent)
    , m_host(host)
{
    m_expandAnim = new QVariantAnimation(this);
    m_expandAnim->setDuration(Motion::Long1); // 450ms spatial expansion
    m_expandAnim->setStartValue(0.0);
    m_expandAnim->setEndValue(1.0);
    m_expandAnim->setEasingCurve(Motion::emphasizedDecelerate());
    connect(m_expandAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        m_expand = v.toReal();
        emit repaintRequested();
    });

    m_fadeAnim = new QVariantAnimation(this);
    m_fadeAnim->setDuration(Motion::Medium2); // 300ms fade out
    m_fadeAnim->setEasingCurve(Motion::standard());
    connect(m_fadeAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        m_fade = v.toReal();
        emit repaintRequested();
    });
}

void Ripple::setColor(const QColor &color)
{
    if (m_color == color) {
        return;
    }
    m_color = color;
    emit repaintRequested();
}

void Ripple::setHovered(bool hovered)
{
    if (m_hovered == hovered) {
        return;
    }
    m_hovered = hovered;
    emit repaintRequested();
}

void Ripple::setFocused(bool focused)
{
    if (m_focused == focused) {
        return;
    }
    m_focused = focused;
    emit repaintRequested();
}

void Ripple::press(const QPointF &pos)
{
    m_origin = pos;
    m_pressed = true;

    m_fadeAnim->stop();
    m_fade = 1.0;

    m_expandAnim->stop();
    m_expand = 0.0;
    m_expandAnim->start();

    emit repaintRequested();
}

void Ripple::release()
{
    if (!m_pressed) {
        return;
    }
    m_pressed = false;

    if (m_fade > 0.0) {
        m_fadeAnim->stop();
        m_fadeAnim->setStartValue(m_fade);
        m_fadeAnim->setEndValue(0.0);
        m_fadeAnim->start();
    }

    emit repaintRequested();
}

void Ripple::cancel()
{
    m_pressed = false;
    m_expandAnim->stop();
    m_fadeAnim->stop();
    m_expand = 0.0;
    m_fade = 0.0;
    emit repaintRequested();
}

qreal Ripple::stateLayerAlpha() const
{
    qreal alpha = 0.0;
    if (m_hovered) {
        alpha += kHoverAlpha;
    }
    if (m_focused) {
        alpha += kFocusAlpha;
    }
    if (m_pressed) {
        alpha += kPressedAlpha;
    }
    return qMin(alpha, 0.16);
}

void Ripple::paint(QPainter &painter, const QRectF &bounds, qreal cornerRadius)
{
    const qreal layerAlpha = stateLayerAlpha();
    if (m_fade <= 0.0 && layerAlpha <= 0.0) {
        return;
    }

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPainterPath clipPath;
    clipPath.addRoundedRect(bounds, cornerRadius, cornerRadius);
    painter.setClipPath(clipPath);
    painter.setPen(Qt::NoPen);

    // --- State layer ------------------------------------------------------
    if (layerAlpha > 0.0) {
        QColor layer = m_color;
        layer.setAlphaF(layerAlpha);
        painter.fillPath(clipPath, layer);
    }

    // --- Ripple ------------------------------------------------------------
    if (m_fade > 0.0) {
        const QPointF center = m_origin.isNull() ? bounds.center() : m_origin;

        qreal maxDist = 0.0;
        maxDist = qMax(maxDist, QLineF(center, bounds.topLeft()).length());
        maxDist = qMax(maxDist, QLineF(center, bounds.topRight()).length());
        maxDist = qMax(maxDist, QLineF(center, bounds.bottomLeft()).length());
        maxDist = qMax(maxDist, QLineF(center, bounds.bottomRight()).length());

        const qreal radius = maxDist * m_expand;
        QColor rippleColor = m_color;
        rippleColor.setAlphaF(kRippleAlpha * m_fade);

        painter.setBrush(rippleColor);
        painter.drawEllipse(center, radius, radius);
    }

    painter.restore();
}

} // namespace Md3
