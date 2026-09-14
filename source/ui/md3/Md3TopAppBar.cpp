#include "Md3TopAppBar.h"

#include <QHBoxLayout>
#include <QPainter>
#include <QVariantAnimation>

#include "Md3Button.h"
#include "Md3IconButton.h"
#include "Md3Motion.h"

namespace Md3 {

namespace {

QColor mixColor(const QColor &a, const QColor &b, qreal t)
{
    t = qBound(0.0, t, 1.0);
    return QColor::fromRgbF(a.redF() + (b.redF() - a.redF()) * t,
                            a.greenF() + (b.greenF() - a.greenF()) * t,
                            a.blueF() + (b.blueF() - a.blueF()) * t,
                            a.alphaF() + (b.alphaF() - a.alphaF()) * t);
}

} // namespace

TopAppBar::TopAppBar(QWidget *parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    // --- Leading navigation button ---------------------------------------
    m_navButton = new IconButton(this);
    m_navButton->setVariant(IconButton::Standard);
    m_navButton->setVisible(false);
    connect(m_navButton, &IconButton::clicked, this, &TopAppBar::navigationClicked);
    m_layout->addWidget(m_navButton, 0, Qt::AlignVCenter);

    m_layout->addStretch(1);

    // --- Traffic spacing (actions appended before the right margin) --------
    m_layout->setContentsMargins(4, 0, 4, 0);

    m_scrollAnim = new QVariantAnimation(this);
    m_scrollAnim->setDuration(Motion::Short4); // 200ms
    m_scrollAnim->setEasingCurve(Motion::standard());
    connect(m_scrollAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        m_scrollProgress = v.toReal();
        update();
    });

    connect(&Theme::instance(), &Theme::changed, this, [this]() { applyTheme(); });
    applyTheme();
}

void TopAppBar::setTitle(const QString &title)
{
    if (m_title == title) {
        return;
    }
    m_title = title;
    update();
}

void TopAppBar::setNavigationIcon(const QIcon &icon, const QString &tooltip)
{
    m_navButton->setIcon(icon);
    m_navButton->setToolTip(tooltip);
    m_navButton->setVisible(!icon.isNull());
    update();
}

IconButton *TopAppBar::addAction(const QIcon &icon, const QString &tooltip)
{
    auto *button = new IconButton(this);
    button->setVariant(IconButton::Standard);
    button->setIcon(icon);
    button->setToolTip(tooltip);
    m_actions.append(button);

    // Insert before the trailing stretch/margin: append at the end but
    // keep the layout consistent for arbitrary many actions.
    m_layout->addWidget(button, 0, Qt::AlignVCenter);
    return button;
}

void TopAppBar::setCenterAligned(bool centered)
{
    if (m_centered == centered) {
        return;
    }
    m_centered = centered;
    update();
}

void TopAppBar::setScrolled(bool scrolled)
{
    if (m_scrolled == scrolled) {
        return;
    }
    m_scrolled = scrolled;
    m_scrollAnim->stop();
    m_scrollAnim->setStartValue(m_scrollProgress);
    m_scrollAnim->setEndValue(scrolled ? 1.0 : 0.0);
    m_scrollAnim->start();
}

void TopAppBar::setTopInset(int inset)
{
    inset = qMax(0, inset);
    if (m_topInset == inset) {
        return;
    }
    m_topInset = inset;
    updateGeometry();
    update();
}

QSize TopAppBar::sizeHint() const
{
    return QSize(200, BarHeight + m_topInset);
}

QSize TopAppBar::minimumSizeHint() const
{
    return QSize(120, BarHeight + m_topInset);
}

void TopAppBar::applyTheme()
{
    update();
}

void TopAppBar::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const Md3ColorScheme &c = Theme::instance().colors();

    // --- Container (with the M3 on-scroll color transition) ----------------
    const QColor bg = mixColor(c.surface, c.surfaceContainer, m_scrollProgress);
    p.fillRect(rect(), bg);

    // --- Title -------------------------------------------------------------
    if (m_title.isEmpty()) {
        return;
    }

    const QRect content = rect().adjusted(0, m_topInset, 0, 0);

    // Available width between the navigation button and the first action.
    int left = 16;
    if (m_navButton && m_navButton->isVisible()) {
        left = m_navButton->geometry().right() + 12;
    }
    int right = content.right() - 16;
    if (!m_actions.isEmpty()) {
        right = m_actions.first()->geometry().left() - 12;
    }
    if (right <= left) {
        return;
    }

    const QFont font = Theme::instance().font(TypeRole::TitleLarge);
    p.setFont(font);
    p.setPen(c.onSurface);

    const QRect avail(left, content.top(), right - left, content.height());
    const QString elided = QFontMetrics(font).elidedText(m_title, Qt::ElideRight, avail.width());

    if (m_centered) {
        p.drawText(avail, Qt::AlignCenter, elided);
    } else {
        p.drawText(avail, Qt::AlignVCenter | Qt::AlignLeft, elided);
    }
}

} // namespace Md3