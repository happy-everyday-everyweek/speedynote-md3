#include "Md3Badge.h"

#include <QEvent>
#include <QFontMetricsF>
#include <QPainter>

namespace Md3 {

namespace {

constexpr int kDotSize = 8;
constexpr int kBadgeHeight = 16;

} // namespace

Badge::Badge(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(&Theme::instance(), &Theme::changed, this, qOverload<>(&QWidget::update));
}

void Badge::setText(const QString &text)
{
    m_text = text;
    if (!text.isEmpty()) {
        m_dot = false;
    }
    updateGeometry();
    adjustSize();
    update();
}

void Badge::setDot(bool dot)
{
    if (m_dot == dot) {
        return;
    }
    m_dot = dot;
    updateGeometry();
    adjustSize();
    update();
}

void Badge::attachTo(QWidget *anchor)
{
    if (m_anchor) {
        m_anchor->removeEventFilter(this);
    }
    m_anchor = anchor;
    if (!anchor) {
        hide();
        return;
    }
    if (anchor->parentWidget() && anchor->parentWidget() != parentWidget()) {
        setParent(anchor->parentWidget());
    }
    anchor->installEventFilter(this);
    adjustSize();
    reposition();
    show();
    raise();
}

QSize Badge::sizeHint() const
{
    if (m_dot) {
        return QSize(kDotSize, kDotSize);
    }
    const QFontMetricsF fm(Theme::instance().font(TypeRole::LabelSmall));
    return QSize(static_cast<int>(qMax(qreal(kBadgeHeight), fm.horizontalAdvance(m_text) + 10.0) + 0.5),
                 kBadgeHeight);
}

QSize Badge::minimumSizeHint() const
{
    return sizeHint();
}

void Badge::reposition()
{
    if (!m_anchor || !parentWidget() || m_anchor->parentWidget() != parentWidget()) {
        return;
    }
    const QPoint topRight = m_anchor->geometry().topRight();
    const int x = topRight.x() - width() + 6;
    const int y = topRight.y() - height() / 2 - 2;
    move(qMax(0, x), qMax(0, y));
}

bool Badge::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_anchor) {
        switch (event->type()) {
        case QEvent::Resize:
        case QEvent::Move:
        case QEvent::Show:
        case QEvent::LayoutRequest:
            reposition();
            break;
        default:
            break;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void Badge::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const Md3ColorScheme &c = Theme::instance().colors();
    p.setPen(Qt::NoPen);
    p.setBrush(c.error);

    if (m_dot) {
        const qreal d = qMin(width(), height());
        p.drawEllipse(QRectF(0.0, (height() - d) / 2.0, d, d));
        return;
    }

    const QRectF box = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    p.drawRoundedRect(box, box.height() / 2.0, box.height() / 2.0);

    p.setPen(c.onError);
    p.setFont(Theme::instance().font(TypeRole::LabelSmall));
    p.drawText(rect(), Qt::AlignCenter, m_text);
}

} // namespace Md3
