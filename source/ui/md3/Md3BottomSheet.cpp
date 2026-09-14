#include "Md3BottomSheet.h"

#include <QEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QVariantAnimation>

#include "Md3IconButton.h"
#include "Md3IconUtils.h"
#include "Md3Motion.h"

namespace Md3 {

namespace {

constexpr int kHandleWidth = 32;
constexpr int kHandleHeight = 4;
constexpr int kHandleTop = 10;
constexpr int kTitleHeight = 56;
constexpr qreal kScrimAlpha = 0.32;

class SheetPanel : public QWidget {
public:
    explicit SheetPanel(BottomSheet *sheet)
        : QWidget(sheet)
        , m_sheet(sheet)
    {
        Q_UNUSED(m_sheet)
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);

        const Md3ColorScheme &c = Theme::instance().colors();

        // Rounded top corners only.
        QPainterPath path;
        path.addRoundedRect(QRectF(rect()), 28.0, 28.0);
        QRectF bottom(0, height() - 28.0, width(), 28.0);
        path.addRect(bottom);
        p.fillPath(path.simplified(), c.surfaceContainerLow);

        // Drag handle
        p.setPen(Qt::NoPen);
        QColor handle = c.onSurfaceVariant;
        handle.setAlphaF(0.4);
        p.setBrush(handle);
        const qreal hx = (width() - kHandleWidth) / 2.0;
        p.drawRoundedRect(QRectF(hx, kHandleTop, kHandleWidth, kHandleHeight),
                          kHandleHeight / 2.0, kHandleHeight / 2.0);
    }

private:
    BottomSheet *m_sheet;
};

} // namespace

BottomSheet::BottomSheet(QWidget *host)
    : QWidget(host ? host->window() : nullptr)
{
    initialize();
}

BottomSheet::~BottomSheet() = default;

void BottomSheet::initialize()
{
    m_hostWindow = parentWidget();
    if (m_hostWindow) {
        m_hostWindow->installEventFilter(this);
    }

    setAttribute(Qt::WA_StyledBackground, false);
    hide();

    // --- Panel ---------------------------------------------------------------
    m_panel = new SheetPanel(this);
    m_panel->installEventFilter(this);

    m_titleLabel = new QLabel(m_panel);
    m_titleLabel->hide();

    m_closeButton = new IconButton(m_panel);
    m_closeButton->setVariant(IconButton::Standard);
    m_closeButton->setIcon(Icons::tintedFromResource(QStringLiteral(":/resources/icons/cross.png"),
                                                     Theme::instance().colors().onSurfaceVariant, 24));
    m_closeButton->setToolTip(tr("Close"));
    m_closeButton->hide();
    connect(m_closeButton, &IconButton::clicked, this, [this]() { close(); });

    m_anim = new QVariantAnimation(this);
    connect(m_anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        m_slide = v.toReal();
        layoutPanel();
        update();
    });
    connect(m_anim, &QVariantAnimation::finished, this, [this]() {
        if (!m_open && m_slide <= 0.0) {
            hide();
        }
    });

    connect(&Theme::instance(), &Theme::changed, this, [this]() {
        m_closeButton->setIcon(Icons::tintedFromResource(QStringLiteral(":/resources/icons/cross.png"),
                                                         Theme::instance().colors().onSurfaceVariant, 24));
        update();
    });

    updateGeometryToHost();
}

void BottomSheet::setContentWidget(QWidget *content)
{
    if (m_content == content) {
        return;
    }
    if (m_content) {
        m_content->setParent(nullptr);
    }
    m_content = content;
    if (m_content) {
        m_content->setParent(m_panel);
        m_content->show();
    }
    layoutPanel();
}

void BottomSheet::setTitle(const QString &title)
{
    m_title = title;
    m_titleLabel->setText(title);
    m_titleLabel->setFont(Theme::instance().font(TypeRole::TitleLarge));
    updateGeometryToHost();
}

void BottomSheet::setClosable(bool closable)
{
    m_closable = closable;
    m_closeButton->setVisible(closable);
    updateGeometryToHost();
}

void BottomSheet::setMaxHeightFraction(qreal fraction)
{
    m_maxHeightFraction = qBound(0.3, fraction, 0.95);
    updateGeometryToHost();
}

int BottomSheet::chromeHeight() const
{
    int h = kHandleTop + kHandleHeight + 10;
    if (!m_title.isEmpty()) {
        h += kTitleHeight - 24;
    }
    return h;
}

int BottomSheet::preferredPanelHeight() const
{
    int contentHeight = 0;
    if (m_content) {
        contentHeight = m_content->sizeHint().height();
        if (contentHeight <= 0) {
            contentHeight = m_content->height();
        }
    }
    return chromeHeight() + contentHeight + 16;
}

void BottomSheet::updateGeometryToHost()
{
    if (!m_hostWindow) {
        return;
    }
    setGeometry(m_hostWindow->rect());
    const int maxH = static_cast<int>(m_hostWindow->height() * m_maxHeightFraction);
    const int panelH = qMin(preferredPanelHeight(), maxH);
    m_panel->resize(width(), panelH);
    layoutPanel();
}

void BottomSheet::layoutPanel()
{
    if (!m_panel) {
        return;
    }
    const int panelH = m_panel->height();
    const int baseY = height() - panelH;
    const int y = baseY + static_cast<int>((1.0 - m_slide) * panelH);
    m_panel->move(0, y);

    // Title row + close button
    const int titleX = 16;
    const int titleY = kHandleTop + kHandleHeight + 6;
    m_titleLabel->setVisible(!m_title.isEmpty());
    if (!m_title.isEmpty()) {
        m_titleLabel->setGeometry(titleX, titleY, width() - titleX * 2 - 48, 28);
    }
    if (m_closable) {
        m_closeButton->setGeometry(width() - 52, titleY - 10, 48, 48);
    }

    if (m_content) {
        const int top = chromeHeight();
        m_content->setGeometry(12, top, width() - 24, qMax(0, panelH - top - 12));
    }
}

void BottomSheet::open()
{
    if (m_open) {
        return;
    }
    m_open = true;
    updateGeometryToHost();
    show();
    raise();
    setFocus();
    animateTo(1.0, false);
    emit opened();
}

void BottomSheet::close()
{
    if (!m_open) {
        return;
    }
    m_open = false;
    animateTo(0.0, true);
    emit closed();
}

void BottomSheet::animateTo(qreal target, bool closing)
{
    m_anim->stop();
    m_anim->setDuration(closing ? Motion::Short4 : Motion::Medium4); // 200 / 400 ms
    m_anim->setEasingCurve(closing ? Motion::emphasizedAccelerate() : Motion::emphasizedDecelerate());
    m_anim->setStartValue(m_slide);
    m_anim->setEndValue(target);
    m_anim->start();
}

void BottomSheet::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    QColor scrim = Theme::instance().colors().scrim;
    scrim.setAlphaF(kScrimAlpha * m_slide);
    p.fillRect(rect(), scrim);
}

void BottomSheet::mousePressEvent(QMouseEvent *event)
{
    // Scrim tap: only reachable outside the sheet panel.
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QPointF pos = event->position();
#else
    const QPointF pos = event->pos();
#endif
    if (!m_panel->geometry().contains(pos.toPoint())) {
        if (m_dismissOnScrimTap) {
            close();
        }
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void BottomSheet::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
}

void BottomSheet::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

bool BottomSheet::eventFilter(QObject *watched, QEvent *event)
{
    // Keep the overlay matching the host window size.
    if (watched == m_hostWindow && event->type() == QEvent::Resize) {
        updateGeometryToHost();
        return false;
    }

    if (watched == m_panel) {
        switch (event->type()) {
        case QEvent::MouseButtonPress: {
            auto *me = static_cast<QMouseEvent *>(event);
            if (me->button() != Qt::LeftButton) {
                break;
            }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            const QPointF pos = me->position();
#else
            const QPointF pos = me->pos();
#endif
            // Only the chrome area (above the content) starts a drag.
            if (pos.y() <= chromeHeight()) {
                m_dragging = true;
                m_dragOffset = 0.0;
                m_animStart = m_slide;
                m_anim->stop();
                return true;
            }
            break;
        }
        case QEvent::MouseMove: {
            if (!m_dragging) {
                break;
            }
            auto *me = static_cast<QMouseEvent *>(event);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            m_dragOffset = qMax(0.0, me->position().y());
#else
            m_dragOffset = qMax(0.0, static_cast<qreal>(me->pos().y()));
#endif
            const qreal panelH = qMax(1, m_panel->height());
            m_slide = qBound(0.0, m_animStart - m_dragOffset / panelH * 0.9, 1.0);
            layoutPanel();
            update();
            return true;
        }
        case QEvent::MouseButtonRelease: {
            if (!m_dragging) {
                break;
            }
            m_dragging = false;
            if (m_dragOffset > 96.0) {
                close();
            } else {
                animateTo(1.0, false);
            }
            m_dragOffset = 0.0;
            return true;
        }
        default:
            break;
        }
    }

    return QWidget::eventFilter(watched, event);
}

} // namespace Md3