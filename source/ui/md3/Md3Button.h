#ifndef MD3_BUTTON_H
#define MD3_BUTTON_H

#include <QIcon>
#include <QPushButton>

#include "Md3Theme.h"

namespace Md3 {

class Ripple;

/**
 * Material Design 3 "common" button (label button).
 *
 * Five variants (filled / tonal / outlined / text / elevated) and three
 * sizes, painted per the M3 spec: full-corner pill shape, label-large text,
 * 8%/10% state layers, press ripple and a soft elevation shadow for the
 * elevated variant.
 *
 * The widget paints itself completely; do not apply QSS to it.
 */
class Button : public QPushButton {
    Q_OBJECT

public:
    enum Variant { Filled, Tonal, Outlined, Text, Elevated };
    enum Size { Small, Medium, Large };

    explicit Button(QWidget *parent = nullptr);
    explicit Button(const QString &text, QWidget *parent = nullptr);
    Button(const QString &text, Variant variant, QWidget *parent = nullptr);

    void setVariant(Variant variant);
    Variant variant() const { return m_variant; }

    void setSize(Size size);
    Size size() const { return m_size; }

    /// Optional leading icon, rendered at 18x18 like the M3 spec.
    void setLeadingIcon(const QIcon &icon);

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
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    void initialize();

    int heightToken() const;
    int horizontalPadding() const;
    QColor containerColor() const;
    QColor borderColor() const;      // invalid color = no border
    QColor contentColor() const;

    Variant m_variant = Filled;
    Size m_size = Medium;
    QIcon m_leadingIcon;
    Ripple *m_ripple = nullptr;
};

} // namespace Md3

#endif // MD3_BUTTON_H