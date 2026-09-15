#ifndef MD3_CHECKBOX_H
#define MD3_CHECKBOX_H

#include <QAbstractButton>

#include "Md3Theme.h"

class QVariantAnimation;

namespace Md3 {

class Ripple;

/**
 * Material Design 3 checkbox.
 *
 * 18dp box inside a 48dp touch target, a 2dp outline that fades into the
 * primary container, an animated check mark (150ms emphasized-decelerate),
 * 8%/10% state layers and a press ripple. An optional label is rendered
 * with the body-large type role.
 */
class CheckBox : public QAbstractButton {
    Q_OBJECT
    Q_PROPERTY(qreal progress READ progress WRITE setProgress)

public:
    explicit CheckBox(QWidget *parent = nullptr);
    explicit CheckBox(const QString &text, QWidget *parent = nullptr);

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

#endif // MD3_CHECKBOX_H
