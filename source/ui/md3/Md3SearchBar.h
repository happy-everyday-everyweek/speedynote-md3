#ifndef MD3_SEARCH_BAR_H
#define MD3_SEARCH_BAR_H

#include <QWidget>

#include "Md3Theme.h"

class QLineEdit;

namespace Md3 {

class Ripple;

/**
 * Material Design 3 search bar (docked style).
 *
 * A 56dp full-corner container in surface-container-high with a leading
 * search glyph, an embedded QLineEdit and a trailing clear affordance.
 * The embedded editor is reachable through lineEdit() for advanced use.
 */
class SearchBar : public QWidget {
    Q_OBJECT

public:
    explicit SearchBar(QWidget *parent = nullptr);

    void setPlaceholderText(const QString &text);
    QString placeholderText() const;
    void setText(const QString &text);
    QString text() const;
    QLineEdit *lineEdit() const { return m_edit; }

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void textChanged(const QString &text);
    void textEdited(const QString &text);
    void returnPressed();
    void cleared();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void initialize();
    void updateThemeColors();
    QRectF clearRect() const;

    QLineEdit *m_edit = nullptr;
    Ripple *m_ripple = nullptr;
    bool m_pressedInClear = false;
};

} // namespace Md3

#endif // MD3_SEARCH_BAR_H
