#ifndef MD3_NAVIGATION_RAIL_H
#define MD3_NAVIGATION_RAIL_H

#include <QIcon>
#include <QList>
#include <QWidget>

#include "Md3Theme.h"

namespace Md3 {

class Ripple;

/**
 * Material Design 3 navigation rail.
 *
 * A vertical rail (80dp wide) of icon + label destinations. The selected
 * destination uses a 56x32 secondary-container pill per the M3 spec;
 * items animate and ripple like the navigation bar.
 */
class NavigationRail : public QWidget {
    Q_OBJECT

public:
    struct Item {
        QIcon icon;
        QString label;
    };

    explicit NavigationRail(QWidget *parent = nullptr);

    void addItem(const QIcon &icon, const QString &label);
    void clearItems();
    int count() const { return m_items.size(); }

    int currentIndex() const { return m_current; }
    void setCurrentIndex(int index);

    void setShowLabels(bool show);
    bool showLabels() const { return m_showLabels; }

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void currentChanged(int index);

protected:
    void paintEvent(QPaintEvent *event) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void initialize();
    qreal itemHeight() const;
    QRectF pillRect(int index) const;
    int itemAt(const QPointF &pos) const;

    QList<Item> m_items;
    int m_current = 0;
    int m_hover = -1;
    int m_pressed = -1;
    bool m_showLabels = true;
    Ripple *m_ripple = nullptr;
};

} // namespace Md3

#endif // MD3_NAVIGATION_RAIL_H
