#ifndef MD3_SLIDER_H
#define MD3_SLIDER_H

#include <QSlider>

#include "Md3Theme.h"

namespace Md3 {

class Ripple;

/**
 * Material Design 3 slider (horizontal).
 *
 * 4dp track: active portion and the 20dp handle are primary, the inactive
 * portion is surface-container-highest. 48dp touch height with state layers
 * and a ripple on the handle. Vertical orientation falls back to the plain
 * QSlider painting.
 */
class Slider : public QSlider {
    Q_OBJECT

public:
    explicit Slider(QWidget *parent = nullptr);
    explicit Slider(Qt::Orientation orientation, QWidget *parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;

private:
    void initialize();
    qreal fraction() const;
    qreal trackLeft() const;
    qreal trackRight() const;
    qreal handleCenter() const;
    int valueFromPos(qreal x) const;

    Ripple *m_ripple = nullptr;
};

} // namespace Md3

#endif // MD3_SLIDER_H
