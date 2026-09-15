#ifndef MD3_TAB_BAR_H
#define MD3_TAB_BAR_H

#include <QStringList>
#include <QWidget>

#include "Md3Theme.h"

class QVariantAnimation;

namespace Md3 {

class Ripple;

/**
 * Material Design 3 primary tabs.
 *
 * A 48dp row of text-only tabs in label-large with an animated 3dp
 * primary indicator. Selection changes slide the indicator to the new
 * tab (250ms, emphasized-decelerate) per the M3 motion spec.
 */
class TabBar : public QWidget {
    Q_OBJECT

public:
    explicit TabBar(QWidget *parent = nullptr);

    void addTab(const QString &text);
    void insertTab(int index, const QString &text);
    void clearTabs();
    int count() const { return m_tabs.size(); }

    int currentIndex() const { return m_current; }
    void setCurrentIndex(int index);

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
    QRectF tabRect(int index) const;
    int tabAt(const QPointF &pos) const;
    void retargetIndicator(bool animate);

    QStringList m_tabs;
    int m_current = 0;
    int m_hover = -1;
    int m_pressed = -1;
    qreal m_prevLeft = 0.0;
    qreal m_prevWidth = 0.0;
    qreal m_targetLeft = 0.0;
    qreal m_targetWidth = 0.0;
    qreal m_progress = 1.0;
    QVariantAnimation *m_anim = nullptr;
    Ripple *m_ripple = nullptr;
};

} // namespace Md3

#endif // MD3_TAB_BAR_H
