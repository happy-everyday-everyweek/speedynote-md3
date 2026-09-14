#ifndef MD3_FAB_H
#define MD3_FAB_H

#include <QIcon>
#include <QPushButton>

#include "Md3Theme.h"

namespace Md3 {

class Ripple;

/**
 * Material Design 3 floating action button (FAB).
 *
 * Sizes: Small (40), Medium (56), Large (96). Color roles follow the M3
 * FAB color styles; elevation is level 3 with a press ripple.
 */
class Fab : public QPushButton {
    Q_OBJECT

public:
    enum Size { Small, Medium, Large };
    enum ColorRole { PrimaryContainer, Surface, SecondaryContainer, TertiaryContainer };

    explicit Fab(QWidget *parent = nullptr);

    void setSize(Size size);
    Size size() const { return m_size; }

    void setColorRole(ColorRole role);
    ColorRole colorRole() const { return m_colorRole; }

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

private:
    void initialize();

    int diameter() const;
    int iconToken() const;
    QColor containerColor() const;
    QColor iconColor() const;

    Size m_size = Medium;
    ColorRole m_colorRole = PrimaryContainer;
    Ripple *m_ripple = nullptr;
};

} // namespace Md3

#endif // MD3_FAB_H