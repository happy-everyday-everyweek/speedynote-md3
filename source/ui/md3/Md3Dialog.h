#ifndef MD3_DIALOG_H
#define MD3_DIALOG_H

#include <QString>
#include <QStringList>
#include <QWidget>

#include <functional>
#include <vector>

#include "Md3Theme.h"

class QGraphicsOpacityEffect;
class QHBoxLayout;
class QLabel;
class QVariantAnimation;

namespace Md3 {

class Button;

/**
 * Material Design 3 basic dialog.
 *
 * Rendered as an in-window overlay (like BottomSheet) so it follows the
 * host window on device rotation and does not spawn extra top-level windows.
 * Container: surface-container-high, 28dp corners, 24dp padding, 280dp-560dp
 * wide. Scrim at 32% black; fade + rise entrance motion; dismissible via
 * scrim tap or Esc when enabled.
 */
class Dialog : public QWidget {
    Q_OBJECT

public:
    explicit Dialog(QWidget *host);
    ~Dialog() override;

    void setHeadline(const QString &text);
    void setSupportingText(const QString &text);

    /// Append an action button; returns its index (order of addition).
    int addAction(const QString &label, std::function<void()> action = {});
    void clearActions();

    void setDismissible(bool dismissible) { m_dismissible = dismissible; }

    void open();
    void close();
    bool isOpen() const { return m_open; }

    /**
     * Fire-and-forget helper: builds a dialog with the given headline,
     * supporting text and action labels (default "OK"), invokes
     * `onResult(index)` when an action is chosen and deletes itself.
     */
    static Dialog *showMessage(QWidget *host, const QString &headline,
                               const QString &supporting = QString(),
                               const QStringList &actions = QStringList(),
                               std::function<void(int)> onResult = {});

signals:
    void actionTriggered(int index);
    void closed();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    class Panel;

    void initialize();
    void updateGeometryToHost();
    void layoutPanel();
    void animateTo(qreal target, bool closing);
    void updateColors();

    QWidget *m_hostWindow = nullptr;
    Panel *m_panel = nullptr;
    QLabel *m_headlineLabel = nullptr;
    QLabel *m_supportingLabel = nullptr;
    QWidget *m_actionsRow = nullptr;
    QHBoxLayout *m_actionsLayout = nullptr;
    QGraphicsOpacityEffect *m_panelOpacity = nullptr;

    QString m_headline;
    QString m_supporting;

    struct Action {
        QString label;
        std::function<void()> fn;
    };
    std::vector<Action> m_actions;

    bool m_open = false;
    bool m_dismissible = true;
    qreal m_reveal = 0.0;
    QVariantAnimation *m_anim = nullptr;
};

} // namespace Md3

#endif // MD3_DIALOG_H
