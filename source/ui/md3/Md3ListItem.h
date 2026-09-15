#ifndef MD3_LIST_ITEM_H
#define MD3_LIST_ITEM_H

#include <QIcon>
#include <QWidget>

#include "Md3Theme.h"

namespace Md3 {

class Ripple;

/**
 * Material Design 3 list item.
 *
 * One/two/three-line rows (56/72/88dp) with an optional leading icon,
 * overline, headline, supporting text, trailing text/icon, selection
 * container (secondary-container when selected) and a press ripple.
 */
class ListItem : public QWidget {
    Q_OBJECT

public:
    enum Lines { OneLine, TwoLine, ThreeLine };

    explicit ListItem(QWidget *parent = nullptr);

    void setOverlineText(const QString &text);
    void setHeadline(const QString &text);
    void setSupportingText(const QString &text);
    void setTrailingText(const QString &text);
    void setLeadingIcon(const QIcon &icon);
    void setTrailingIcon(const QIcon &icon);

    void setSelected(bool selected);
    bool isSelected() const { return m_selected; }

    void setLines(Lines lines);
    Lines lines() const { return m_lines; }

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void clicked();

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
    int effectiveLines() const;
    int heightToken() const;

    QString m_overline;
    QString m_headline;
    QString m_supporting;
    QString m_trailing;
    QIcon m_leadingIcon;
    QIcon m_trailingIcon;
    bool m_selected = false;
    bool m_hover = false;
    bool m_pressed = false;
    Lines m_lines = OneLine;
    Ripple *m_ripple = nullptr;
};

} // namespace Md3

#endif // MD3_LIST_ITEM_H
