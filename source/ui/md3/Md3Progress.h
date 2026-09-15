#ifndef MD3_PROGRESS_H
#define MD3_PROGRESS_H

#include <QWidget>

#include "Md3Theme.h"

class QVariantAnimation;

namespace Md3 {

/**
 * Material Design 3 linear progress indicator.
 *
 * 4dp rounded track (surface-container-highest) with a primary indicator, in
 * either determinate (setRange/setValue) or indeterminate form (a 1.8s sweep
 * loop).
 */
class LinearProgress : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal phase READ phase WRITE setPhase)

public:
    explicit LinearProgress(QWidget *parent = nullptr);

    void setRange(int minimum, int maximum);
    void setValue(int value);
    int value() const { return m_value; }
    int minimum() const { return m_minimum; }
    int maximum() const { return m_maximum; }

    void setIndeterminate(bool indeterminate);
    bool isIndeterminate() const { return m_indeterminate; }

    qreal phase() const { return m_phase; }
    void setPhase(qreal phase);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    void syncAnimation();
    qreal fraction() const;

    int m_minimum = 0;
    int m_maximum = 100;
    int m_value = 0;
    bool m_indeterminate = true;
    qreal m_phase = 0.0;
    QVariantAnimation *m_anim = nullptr;
};

/**
 * Material Design 3 circular progress indicator.
 *
 * A 4dp stroke ring; determinate mode draws the arc proportional to the
 * value over a surface-container-highest track, indeterminate mode spins a
 * growing/shrinking arc (1.333s linear rotation).
 */
class CircularProgress : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal phase READ phase WRITE setPhase)

public:
    explicit CircularProgress(QWidget *parent = nullptr);

    void setRange(int minimum, int maximum);
    void setValue(int value);
    int value() const { return m_value; }
    int minimum() const { return m_minimum; }
    int maximum() const { return m_maximum; }

    void setIndeterminate(bool indeterminate);
    bool isIndeterminate() const { return m_indeterminate; }

    qreal phase() const { return m_phase; }
    void setPhase(qreal phase);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    void syncAnimation();
    qreal fraction() const;

    int m_minimum = 0;
    int m_maximum = 100;
    int m_value = 0;
    bool m_indeterminate = true;
    qreal m_phase = 0.0;
    QVariantAnimation *m_anim = nullptr;
};

} // namespace Md3

#endif // MD3_PROGRESS_H
