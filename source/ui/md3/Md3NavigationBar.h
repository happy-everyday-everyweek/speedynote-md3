#ifndef MD3_NAVIGATION_BAR_H
#define MD3_NAVIGATION_BAR_H

#include <QIcon>
#include <QVector>
#include <QWidget>

#include "Md3Theme.h"

class QVariantAnimation;

namespace Md3 {

class Ripple;

/**
 * Material Design 3 bottom navigation bar.
 *
 * 80dp bar with 3..5 icon+label items, an animated 64x32 active-indicator
 * pill (secondaryContainer), icon/label crossfades and press state layers.
 * Selection changes animate with the M3 emphasized curve.
 *
 * A bottom inset can be reserved for gesture-bar areas on mobile.
 */
class NavigationBar : public QWidget {
    Q_OBJECT

public:
    struct Item {
        QIcon icon;          // base (single color) icon; tinted by the bar
        QString label;
        QIcon tintedOff;     // cache: onSurfaceVariant
        QIcon tintedOn;      // cache: onSecondaryContainer
    };

    explicit NavigationBar(QWidget *parent = nullptr);

    void addItem(const QIcon &icon, const QString &label);
    void clearItems();

    int count() const { return m_items.size(); }
    int currentIndex() const { return m_index; }

    void setCurrentIndex(int index, bool animate = true);

    /// Reserve space at the bottom (navigation-bar inset on mobile).
    void setBottomInset(int inset);
    int bottomInset() const { return m_bottomInset; }

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    static constexpr int BarHeight = 80;

signals:
    void currentChanged(int index);
    void itemClicked(int index);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void initialize();
    void rebuildIcons();
    QRectF itemRect(int index) const;
    qreal itemCenterX(int index) const;

    QVector<Item> m_items;
    int m_index = 0;
    int m_bottomInset = 0;
    qreal m_indicatorX = 0.0;

    class QVariantAnimation *m_indicatorAnim = nullptr;
    Ripple *m_ripple = nullptr;
    int m_pressedIndex = -1;
};

} // namespace Md3

#endif // MD3_NAVIGATION_BAR_H