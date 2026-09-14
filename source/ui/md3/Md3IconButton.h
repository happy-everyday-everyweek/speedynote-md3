#ifndef MD3_ICON_BUTTON_H
#define MD3_ICON_BUTTON_H

#include <QIcon>
#include <QPushButton>

#include "Md3Theme.h"

namespace Md3 {

class Ripple;

/**
 * Material Design 3 icon button.
 *
 * Variants: Standard (no container), Filled, Tonal, Outlined.
 * Checkable icon buttons follow the M3 toggle behavior of each variant.
 *
 * Geometry: a 48dp touch target with a centered 40dp visual container.
 * The widget paints itself completely; do not apply QSS to it.
 */
class IconButton : public QPushButton {
    Q_OBJECT

public:
    enum Variant { Standard, Filled, Tonal, Outlined };

    explicit IconButton(QWidget *parent = nullptr);

    void setVariant(Variant variant);
    Variant variant() const { return m_variant; }

    /// Icon rendered at `size` logical px (default 24, per spec).
    void setIconSize(int size);

    /// Visual container diameter (default 40; clamped to 32..48).
    void setContainerSize(int size);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    /// Repaint with the current theme (called automatically on change).
    void refresh();

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
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    void initialize();

    QColor containerColor() const;
    QColor borderColor() const;
    QColor iconColor() const;

    Variant m_variant = Standard;
    int m_iconSize = 24;
    int m_containerSize = 40;
    Ripple *m_ripple = nullptr;
};

} // namespace Md3

#endif // MD3_ICON_BUTTON_H