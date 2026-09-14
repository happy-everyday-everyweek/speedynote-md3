#ifndef MD3_SWITCH_H
#define MD3_SWITCH_H

#include <QAbstractButton>

#include "Md3Theme.h"

class QVariantAnimation;

namespace Md3 {

class Ripple;

/**
 * Material Design 3 switch.
 *
 * Track 52x32dp with an animated handle (24dp off / 28dp on, with a check
 * icon when selected), 200ms emphasized motion, state layers and a ripple.
 */
class Switch : public QAbstractButton {
    Q_OBJECT
    Q_PROPERTY(qreal progress READ progress WRITE setProgress)

public:
    explicit Switch(QWidget *parent = nullptr);

    qreal progress() const { return m_progress; }
    void setProgress(qreal progress);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void checkStateSet() override;
    void nextCheckState() override;

private:
    void initialize();
    void animateTo(bool checked);

    qreal m_progress = 0.0;
    Ripple *m_ripple = nullptr;
    QVariantAnimation *m_anim = nullptr;
};

} // namespace Md3

#endif // MD3_SWITCH_H