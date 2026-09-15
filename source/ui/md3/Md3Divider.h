#ifndef MD3_DIVIDER_H
#define MD3_DIVIDER_H

#include <QWidget>

#include "Md3Theme.h"

namespace Md3 {

/**
 * Material Design 3 divider: a 1dp rule in the outline-variant color.
 */
class Divider : public QWidget {
    Q_OBJECT

public:
    explicit Divider(QWidget *parent = nullptr);

    void setVertical(bool vertical);
    bool isVertical() const { return m_vertical; }

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    bool m_vertical = false;
};

} // namespace Md3

#endif // MD3_DIVIDER_H
