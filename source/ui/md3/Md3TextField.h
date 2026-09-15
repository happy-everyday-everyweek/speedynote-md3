#ifndef MD3_TEXT_FIELD_H
#define MD3_TEXT_FIELD_H

#include <QWidget>

#include "Md3Theme.h"

class QLineEdit;
class QVariantAnimation;

namespace Md3 {

/**
 * Material Design 3 text field.
 *
 * Filled (surface-container-highest, 4dp top corners, underline) and
 * outlined (1dp/2dp border) variants, an optional label above the field
 * and supporting/error text below. The focus indicator animates between
 * the idle and focused states following the M3 motion spec.
 */
class TextField : public QWidget {
    Q_OBJECT

public:
    enum Variant { Filled, Outlined };

    explicit TextField(QWidget *parent = nullptr);

    void setVariant(Variant variant);
    Variant variant() const { return m_variant; }

    void setLabel(const QString &text);
    QString label() const { return m_label; }
    void setSupportingText(const QString &text);
    void setErrorText(const QString &text);

    void setText(const QString &text);
    QString text() const;
    QLineEdit *lineEdit() const { return m_edit; }

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void textChanged(const QString &text);
    void editingFinished();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void initialize();
    void updateThemeColors();
    void animateFocus(bool focused);
    void layoutEdit();
    int labelHeight() const;
    int supportingHeight() const;

    QLineEdit *m_edit = nullptr;
    QVariantAnimation *m_anim = nullptr;
    Variant m_variant = Filled;
    QString m_label;
    QString m_supporting;
    QString m_error;
    bool m_focused = false;
    qreal m_focusProgress = 0.0;
};

} // namespace Md3

#endif // MD3_TEXT_FIELD_H
