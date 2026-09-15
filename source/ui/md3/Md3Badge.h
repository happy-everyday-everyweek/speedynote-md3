#ifndef MD3_BADGE_H
#define MD3_BADGE_H

#include <QPointer>
#include <QWidget>

#include "Md3Theme.h"

namespace Md3 {

/**
 * Material Design 3 badge.
 *
 * Small (8dp dot) and labelled (16dp tall, error container) forms. Use
 * attachTo() to pin the badge to the top-right corner of an anchor
 * widget; the badge then follows the anchor's geometry automatically.
 */
class Badge : public QWidget {
    Q_OBJECT

public:
    explicit Badge(QWidget *parent = nullptr);

    void setText(const QString &text);
    QString text() const { return m_text; }
    void setDot(bool dot);
    bool isDot() const { return m_dot; }

    void attachTo(QWidget *anchor);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void reposition();

    QString m_text;
    bool m_dot = true;
    QPointer<QWidget> m_anchor;
};

} // namespace Md3

#endif // MD3_BADGE_H
