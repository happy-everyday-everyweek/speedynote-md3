#ifndef MD3_BOTTOM_SHEET_H
#define MD3_BOTTOM_SHEET_H

#include <QWidget>

#include "Md3Theme.h"

class QLabel;
class QVariantAnimation;

namespace Md3 {

class IconButton;

/**
 * Material Design 3 modal bottom sheet.
 *
 * An in-window overlay: a scrim covers the host window and a sheet panel
 * (28dp top corners, drag handle, optional title row) slides up from the
 * bottom with the M3 emphasized motion. Supports scrim-tap dismissal,
 * drag-to-dismiss on the sheet chrome, Esc, and programmatic open/close.
 */
class BottomSheet : public QWidget {
    Q_OBJECT

public:
    explicit BottomSheet(QWidget *host);
    ~BottomSheet() override;

    /// Reparent `content` into the sheet and size the sheet around it.
    void setContentWidget(QWidget *content);
    QWidget *contentWidget() const { return m_content; }

    void setTitle(const QString &title);
    void setClosable(bool closable);

    /// Maximum height as a fraction of the host window (default 0.85).
    void setMaxHeightFraction(qreal fraction);

    void open();
    void close();
    bool isOpen() const { return m_open; }

    void setDismissOnScrimTap(bool dismiss) { m_dismissOnScrimTap = dismiss; }

signals:
    void opened();
    void closed();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void initialize();
    void updateGeometryToHost();
    void layoutPanel();
    void animateTo(qreal target, bool closing);
    int chromeHeight() const;
    int preferredPanelHeight() const;

    QWidget *m_hostWindow = nullptr;
    QWidget *m_panel = nullptr;
    QWidget *m_content = nullptr;
    QLabel *m_titleLabel = nullptr;
    IconButton *m_closeButton = nullptr;

    QString m_title;
    bool m_closable = false;
    bool m_open = false;
    bool m_dismissOnScrimTap = true;
    qreal m_maxHeightFraction = 0.85;

    qreal m_slide = 0.0; // 0 hidden, 1 shown
    class QVariantAnimation *m_anim = nullptr;

    bool m_dragging = false;
    qreal m_dragOffset = 0.0;
    qreal m_animStart = 0.0;
};

} // namespace Md3

#endif // MD3_BOTTOM_SHEET_H