#ifndef MD3_CHIP_H
#define MD3_CHIP_H

#include <QAbstractButton>
#include <QIcon>

#include "Md3Theme.h"

namespace Md3 {

class Ripple;

/**
 * Material Design 3 chip (assist / filter / input / suggestion).
 *
 * 32dp tall, 8dp corners, 1dp outline. Filter-style chips toggle: when
 * checked the container turns secondary-container, the outline disappears
 * and a check mark replaces the leading icon (M3 spec).
 */
class Chip : public QAbstractButton {
    Q_OBJECT

public:
    enum Style { Assist, Filter, Input, Suggestion };

    explicit Chip(QWidget *parent = nullptr);
    explicit Chip(const QString &text, QWidget *parent = nullptr);

    void setStyle(Style style);
    Style style() const { return m_style; }
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

private:
    void initialize();
    bool hasLeading() const { return !m_icon.isNull() || isChecked(); }

    Style m_style = Assist;
    QIcon m_icon;
    Ripple *m_ripple = nullptr;
};

} // namespace Md3

#endif // MD3_CHIP_H
