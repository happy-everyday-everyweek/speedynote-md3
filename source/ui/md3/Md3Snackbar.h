#ifndef MD3_SNACKBAR_H
#define MD3_SNACKBAR_H

#include <QWidget>
#include <functional>

#include "Md3Theme.h"

namespace Md3 {

/**
 * Material Design 3 snackbar.
 *
 * Bottom-anchored transient message with an optional action, painted with
 * inverse-surface colors, slide-up/down motion and swipe-down dismissal.
 * Shown as an overlay on top of a host window.
 */
class Snackbar : public QWidget {
    Q_OBJECT

public:
    /**
     * Show a snackbar on top of `host` (the widget's window is used as the
     * overlay parent). A previous snackbar on the same window is dismissed.
     * @param durationMs auto-dismiss delay; pass 0 to keep it until dismissed.
     */
    static void show(QWidget *host, const QString &message,
                     const QString &actionText = QString(),
                     std::function<void()> action = {},
                     int durationMs = 4000);

    static void dismissAll(QWidget *host);

    ~Snackbar() override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    Snackbar(QWidget *parent);

    void animateIn();
    void animateOut();
    void positionInParent();
    QRect actionRect() const;

    QString m_message;
    QString m_actionText;
    std::function<void()> m_action;

    qreal m_slide = 0.0;   // 0 hidden below, 1 fully shown
    bool m_closing = false;
    bool m_hovered = false;

    class QVariantAnimation *m_anim = nullptr;
    class QTimer *m_timer = nullptr;

    QPointF m_dragStart;
    bool m_dragging = false;
    qreal m_dragOffset = 0.0;
};

} // namespace Md3

#endif // MD3_SNACKBAR_H