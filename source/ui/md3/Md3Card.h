#ifndef MD3_CARD_H
#define MD3_CARD_H

#include <QFrame>

#include "Md3Theme.h"

namespace Md3 {

/**
 * Material Design 3 card container.
 *
 * Three variants per the M3 spec: elevated (surface-container-low plus a
 * level-1 shadow), filled (surface-container-highest) and outlined
 * (surface plus a 1dp outline-variant border). Default corner radius is
 * the M3 medium shape (12dp).
 */
class Card : public QFrame {
    Q_OBJECT

public:
    enum Variant { Elevated, Filled, Outlined };

    explicit Card(QWidget *parent = nullptr);
    explicit Card(Variant variant, QWidget *parent = nullptr);

    void setVariant(Variant variant);
    Variant variant() const { return m_variant; }

    void setCornerRadius(qreal radius);
    qreal cornerRadius() const { return m_radius; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Variant m_variant = Filled;
    qreal m_radius = Shape::Medium;
};

} // namespace Md3

#endif // MD3_CARD_H
