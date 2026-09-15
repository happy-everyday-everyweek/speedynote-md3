#ifndef MD3_SEGMENTED_BUTTON_H
#define MD3_SEGMENTED_BUTTON_H

#include <QStringList>
#include <QWidget>

#include "Md3Theme.h"

class QVariantAnimation;

namespace Md3 {

class Ripple;

/**
 * Material Design 3 segmented button (single-select).
 *
 * A 40dp tall full-corner outline holding equal-width segments. The
 * selected segment is filled with secondary-container; changing the
 * selection cross-fades old/new fills following the M3 motion spec
 * (200ms, emphasized-decelerate). Outlined segments are separated by
 * 1dp dividers that disappear around the selected segment.
 */
class SegmentedButton : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal selectionProgress READ selectionProgress WRITE setSelectionProgress)

public:
    explicit SegmentedButton(QWidget *parent = nullptr);

    void addSegment(const QString &text);
    void clearSegments();
    int count() const { return m_labels.size(); }

    int currentIndex() const { return m_current; }
    void setCurrentIndex(int index);

    qreal selectionProgress() const { return m_progress; }
    void setSelectionProgress(qreal progress);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void currentChanged(int index);

protected:
    void paintEvent(QPaintEvent *event) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void initialize();
    QRectF segmentRect(int index) const;
    int segmentAt(const QPointF &pos) const;

    QStringList m_labels;
    int m_current = 0;
    int m_previous = -1;
    int m_hover = -1;
    int m_pressed = -1;
    qreal m_progress = 1.0;
    QVariantAnimation *m_anim = nullptr;
    Ripple *m_ripple = nullptr;
};

} // namespace Md3

#endif // MD3_SEGMENTED_BUTTON_H
