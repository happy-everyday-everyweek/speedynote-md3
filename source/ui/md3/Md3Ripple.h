#ifndef MD3_RIPPLE_H
#define MD3_RIPPLE_H

#include <QColor>
#include <QObject>
#include <QPointF>
#include <QRectF>

class QWidget;
class QVariantAnimation;
class QPainter;

namespace Md3 {

/**
 * State layer + press ripple helper for custom-painted MD3 widgets.
 *
 * MD3 defines two interaction layers:
 *  - a state layer (hover 8%, focus 10%, pressed 10% of the "on" color), and
 *  - a ripple that expands from the touch point and fades out on release.
 *
 * Hosting widgets forward enter/leave/press/release events and call
 * paint() from their paintEvent; connect repaintRequested() to update().
 *
 * Usage:
 *   m_ripple = new Ripple(this);
 *   connect(m_ripple, &Ripple::repaintRequested, this, qOverload<>(&QWidget::update));
 *   // in enterEvent: m_ripple->setHovered(true); ...
 *   // in mousePressEvent: m_ripple->press(event->position());
 *   // in paintEvent: m_ripple->paint(painter, rect(), radius);
 */
class Ripple : public QObject {
    Q_OBJECT

public:
    explicit Ripple(QWidget *host, QObject *parent = nullptr);

    /// Color of both the state layer and the ripple (usually an "on-" role).
    void setColor(const QColor &color);
    QColor color() const { return m_color; }

    void setHovered(bool hovered);
    void setFocused(bool focused);

    /// Begin a ripple at `pos` (host-local coordinates).
    void press(const QPointF &pos);
    /// Fade the ripple out after a release.
    void release();
    /// Abort the ripple immediately (e.g. when the widget is disabled).
    void cancel();

    /// Combined opacity of the hover/focus/pressed state layers (0..1).
    qreal stateLayerAlpha() const;

    /// Paint state layer + ripple clipped to `bounds` with `cornerRadius`.
    void paint(QPainter &painter, const QRectF &bounds, qreal cornerRadius);

    bool isRippling() const { return m_fade > 0.0; }

signals:
    /// Emitted whenever the visual state changes; hosts call update().
    void repaintRequested();

private:
    QWidget *m_host = nullptr;
    QColor m_color = QColor(0, 0, 0);

    bool m_hovered = false;
    bool m_focused = false;
    bool m_pressed = false;

    QPointF m_origin;
    qreal m_expand = 0.0; // 0..1 ripple expansion progress
    qreal m_fade = 0.0;   // 0..1 ripple opacity

    QVariantAnimation *m_expandAnim = nullptr;
    QVariantAnimation *m_fadeAnim = nullptr;
};

} // namespace Md3

#endif // MD3_RIPPLE_H
