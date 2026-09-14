#include "Md3Snackbar.h"

#include <QHash>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>
#include <QVariantAnimation>

#include "Md3Motion.h"

namespace Md3 {

namespace {

constexpr int kMargin = 16;       // side margin
constexpr int kMinWidth = 288;
constexpr int kMaxWidth = 560;
constexpr int kPadding = 16;      // content padding
constexpr int kActionGap = 8;
constexpr qreal kSlideDistance = 32.0;

QHash<QWidget *, Snackbar *> &activeSnackbars()
{
    static QHash<QWidget *, Snackbar *> map;
    return map;
}

} // namespace

Snackbar::Snackbar(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setCursor(Qt::ArrowCursor);

    m_anim = new QVariantAnimation(this);
    connect(m_anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        m_slide = v.toReal();
        positionInParent();
        update();
    });
    connect(m_anim, &QVariantAnimation::finished, this, [this]() {
        if (m_closing) {
            deleteLater();
        }
    });

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, [this]() { animateOut(); });
}

Snackbar::~Snackbar()
{
    if (parentWidget() && parentWidget()->window()) {
        activeSnackbars().remove(parentWidget()->window());
    }
}

void Snackbar::showMessage(QWidget *host, const QString &message, const QString &actionText,
                           std::function<void()> action, int durationMs)
{
    if (!host) {
        return;
    }
    QWidget *window = host->window();
    if (!window) {
        return;
    }

    // Replace any snackbar already shown on this window.
    if (Snackbar *existing = activeSnackbars().value(window, nullptr)) {
        existing->hide();
        existing->deleteLater();
        activeSnackbars().remove(window);
    }

    auto *snackbar = new Snackbar(window);
    snackbar->m_message = message;
    snackbar->m_actionText = actionText;
    snackbar->m_action = std::move(action);

    // Compute the preferred size from text metrics.
    const QFont messageFont = Theme::instance().font(TypeRole::BodyMedium);
    const QFontMetrics fm(messageFont);
    const int textMaxWidth = kMaxWidth - kPadding * 2;
    QRect textBounds = fm.boundingRect(QRect(0, 0, textMaxWidth, 0),
                                       Qt::TextWordWrap, message);
    int width = textBounds.width() + kPadding * 2;
    if (!actionText.isEmpty()) {
        const QFontMetrics afm(Theme::instance().font(TypeRole::LabelLarge));
        width += kActionGap + afm.horizontalAdvance(actionText) + 12;
    }
    width = qBound(kMinWidth, width, qMin(kMaxWidth, window->width() - kMargin * 2));
    const int height = qMax(48, textBounds.height() + 20);

    snackbar->resize(width, height);
    snackbar->positionInParent();
    snackbar->show();
    snackbar->raise();

    activeSnackbars().insert(window, snackbar);

    snackbar->animateIn();
    if (durationMs > 0) {
        snackbar->m_timer->start(durationMs);
    }
}

void Snackbar::dismissAll(QWidget *host)
{
    if (!host || !host->window()) {
        return;
    }
    if (Snackbar *existing = activeSnackbars().value(host->window(), nullptr)) {
        existing->animateOut();
    }
}

void Snackbar::animateIn()
{
    m_closing = false;
    m_anim->stop();
    m_anim->setDuration(Motion::Medium2); // 300ms
    m_anim->setEasingCurve(Motion::emphasizedDecelerate());
    m_anim->setStartValue(0.0);
    m_anim->setEndValue(1.0);
    m_anim->start();
}

void Snackbar::animateOut()
{
    if (m_closing) {
        return;
    }
    m_closing = true;
    m_timer->stop();
    m_anim->stop();
    m_anim->setDuration(Motion::Short4); // 200ms
    m_anim->setEasingCurve(Motion::emphasizedAccelerate());
    m_anim->setStartValue(m_slide);
    m_anim->setEndValue(0.0);
    m_anim->start();
}

void Snackbar::positionInParent()
{
    QWidget *window = parentWidget();
    if (!window) {
        return;
    }
    const int finalY = window->height() - height() - kMargin;
    const int y = finalY + static_cast<int>((1.0 - m_slide) * kSlideDistance);
    move((window->width() - width()) / 2, y);
}

QRect Snackbar::actionRect() const
{
    if (m_actionText.isEmpty()) {
        return QRect();
    }
    const QFontMetrics fm(Theme::instance().font(TypeRole::LabelLarge));
    const int textWidth = fm.horizontalAdvance(m_actionText);
    return QRect(width() - kPadding - textWidth - 12, 0, textWidth + 12, height());
}

void Snackbar::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const Md3ColorScheme &c = Theme::instance().colors();

    QRectF r = rect();
    // Slide-in also fades slightly.
    p.setOpacity(m_slide);
    r.translate(0, (1.0 - m_slide) * 6.0);

    Theme::paintElevation(p, r, 4.0, 3.0, c.shadow);

    p.setPen(Qt::NoPen);
    p.setBrush(c.inverseSurface);
    p.drawRoundedRect(r, 4.0, 4.0);

    // --- Message ---------------------------------------------------------------
    p.setPen(c.inverseOnSurface);
    p.setFont(Theme::instance().font(TypeRole::BodyMedium));

    int textRight = width() - kPadding;
    const QRect ar = actionRect();
    if (!ar.isNull()) {
        textRight = ar.left() - kActionGap;
    }
    const QRect textRect = QRect(kPadding, 0, textRight - kPadding, height());
    p.drawText(textRect, Qt::AlignVCenter | Qt::TextWordWrap, m_message);

    // --- Action ------------------------------------------------------------------
    if (!ar.isNull()) {
        p.setFont(Theme::instance().font(TypeRole::LabelLarge));
        p.setPen(c.inversePrimary);
        p.drawText(ar, Qt::AlignCenter, m_actionText);

        if (m_hovered || m_dragging) {
            QColor hover = c.inversePrimary;
            hover.setAlphaF(0.10);
            p.setBrush(hover);
            QRectF hoverRect = QRectF(ar).adjusted(-8, 8, 8, -8);
            p.drawRoundedRect(hoverRect, hoverRect.height() / 2.0, hoverRect.height() / 2.0);
        }
    }
}

void Snackbar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        return;
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    m_dragStart = event->position();
#else
    m_dragStart = event->pos();
#endif
    m_dragging = true;
    m_dragOffset = 0.0;
    event->accept();
}

void Snackbar::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_dragging) {
        return;
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QPointF pos = event->position();
#else
    const QPointF pos = event->pos();
#endif
    m_dragOffset = qMax(0.0, pos.y() - m_dragStart.y());
    m_slide = qBound(0.35, 1.0 - m_dragOffset / 160.0, 1.0);
    positionInParent();
    update();
}

void Snackbar::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_dragging) {
        return;
    }
    m_dragging = false;

    // Tap on the action?
    const QRect ar = actionRect();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QPointF pos = event->position();
#else
    const QPointF pos = event->pos();
#endif
    if (ar.contains(pos.toPoint()) && m_dragOffset < 8.0) {
        if (m_action) {
            m_action();
        }
        animateOut();
        return;
    }

    if (m_dragOffset > 48.0) {
        animateOut();
    } else {
        // Spring back up.
        m_anim->stop();
        m_anim->setDuration(Motion::Short4);
        m_anim->setEasingCurve(Motion::emphasizedDecelerate());
        m_anim->setStartValue(m_slide);
        m_anim->setEndValue(1.0);
        m_anim->start();
    }
    m_dragOffset = 0.0;
}

void Snackbar::enterEvent(QEnterEvent *event)
{
    m_hovered = true;
    update();
    QWidget::enterEvent(event);
}

void Snackbar::leaveEvent(QEvent *event)
{
    m_hovered = false;
    update();
    QWidget::leaveEvent(event);
}

} // namespace Md3