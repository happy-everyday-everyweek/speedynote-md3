#ifndef MD3_TOP_APP_BAR_H
#define MD3_TOP_APP_BAR_H

#include <QIcon>
#include <QWidget>

#include "Md3Theme.h"

class QLabel;
class QHBoxLayout;
class QVariantAnimation;

namespace Md3 {

class IconButton;

/**
 * Material Design 3 top app bar (small variant).
 *
 * 64dp bar with an optional navigation icon button, a title
 * (title-large), up to a few trailing icon buttons and the M3
 * on-scroll container-color transition. A top inset can be reserved for
 * system status bars on mobile.
 *
 * The bar paints itself; use the provided setters instead of styling it.
 */
class TopAppBar : public QWidget {
    Q_OBJECT

public:
    explicit TopAppBar(QWidget *parent = nullptr);

    void setTitle(const QString &title);
    QString title() const { return m_title; }

    /// Leading navigation icon (menu / back). Hidden when the icon is null.
    void setNavigationIcon(const QIcon &icon, const QString &tooltip = QString());
    IconButton *navigationButton() const { return m_navButton; }

    /// Add a trailing action icon button; returns the button for wiring.
    IconButton *addAction(const QIcon &icon, const QString &tooltip);

    void setCenterAligned(bool centered);
    bool isCenterAligned() const { return m_centered; }

    /// M3 "on-scroll" color transition (surface -> surfaceContainer).
    void setScrolled(bool scrolled);

    /// Reserve space at the top (status-bar inset on mobile).
    void setTopInset(int inset);
    int topInset() const { return m_topInset; }

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    /// M3 bar height (excluding the top inset).
    static constexpr int BarHeight = 64;

signals:
    void navigationClicked();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void applyTheme();

    QString m_title;
    int m_topInset = 0;
    bool m_centered = false;
    bool m_scrolled = false;

    QWidget *m_content = nullptr;
    QHBoxLayout *m_layout = nullptr;
    IconButton *m_navButton = nullptr;
    QVector<IconButton *> m_actions;

    QVariantAnimation *m_scrollAnim = nullptr;
    qreal m_scrollProgress = 0.0;
};

} // namespace Md3

#endif // MD3_TOP_APP_BAR_H