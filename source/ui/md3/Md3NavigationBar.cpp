#include "Md3NavigationBar.h"

#include <QMouseEvent>
#include <QPainter>
#include <QVariantAnimation>

#include "Md3IconUtils.h"
#include "Md3Motion.h"
#include "Md3Ripple.h"

namespace Md3 {

namespace {
constexpr qreal kIndicatorWidth = 64.0;
constexpr qreal kIndicatorHeight = 32.0;
constexpr qreal kIconSize = 24.0;
constexpr qreal kRowTop = 12.0;      // indicator/icon row starts here
constexpr qreal kLabelTop = 48.0;    // label row
} // namespace

NavigationBar::NavigationBar(QWidget *parent)
    : QWidget(parent)
{
    initialize();
}

void NavigationBar::initialize()
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setFixedHeight(BarHeight + m_bottomInset);
    setMouseTracking(false);

    m_ripple = new Ripple(this);
    connect(m_ripple, &Ripple::repaintRequested, this, qOverload<>(&QWidget::update));

    m_indicatorAnim = new QVariantAnimation(this);
    m_indicatorAnim->setDuration(Motion::Medium3); // 350ms
    m_indicatorAnim->setEasingCurve(Motion::emphasized());
    connect(m_indicatorAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        m_indicatorX = v.toReal();
        update();
    });

    connect(&Theme::instance(), &Theme::changed, this, [this]() {
        rebuildIcons();
        update();
    });
}

void NavigationBar::addItem(const QIcon &icon, const QString &label)
{
    Item item;
    item.icon = icon;
    item.label = label;
    m_items.append(item);
    rebuildIcons();
    updateGeometry();
    update();
}

void NavigationBar::clearItems()
{
    m_items.clear();
    m_index = 0;
    m_indicatorX = 0.0;
    updateGeometry();
    update();
}

void NavigationBar::rebuildIcons()
{
    const Md3ColorScheme &c = Theme::instance().colors();
    for (Item &item : m_items) {
        if (item.icon.isNull()) {
            continue;
        }
        item.tintedOff = Icons::tinted(item.icon, c.onSurfaceVariant, 24);
        item.tintedOn = Icons::tinted(item.icon, c.onSecondaryContainer, 24);
    }
    if (m_items.isEmpty()) {
        return;
    }
    if (qFuzzyIsNull(m_indicatorX) && m_index >= 0 && m_index < m_items.size()) {
        m_indicatorX = itemCenterX(m_index);
    }
}

void NavigationBar::setCurrentIndex(int index, bool animate)
{
    if (m_items.isEmpty()) {
        return;
    }
    index = qBound(0, index, m_items.size() - 1);
    if (m_index == index) {
        return;
    }
    const int previous = m_index;
    m_index = index;

    const qreal target = itemCenterX(index);
    if (animate && width() > 0) {
        m_indicatorAnim->stop();
        m_indicatorAnim->setStartValue(m_indicatorX > 0.0 ? m_indicatorX : itemCenterX(previous));
        m_indicatorAnim->setEndValue(target);
        m_indicatorAnim->start();
    } else {
        m_indicatorX = target;
        update();
    }

    emit currentChanged(index);
}

void NavigationBar::setBottomInset(int inset)
{
    inset = qMax(0, inset);
    if (m_bottomInset == inset) {
        return;
    }
    m_bottomInset = inset;
    setFixedHeight(BarHeight + m_bottomInset);
    update();
}

QSize NavigationBar::sizeHint() const
{
    const int perItem = 80;
    return QSize(qMax(240, perItem * m_items.size()), BarHeight + m_bottomInset);
}

QSize NavigationBar::minimumSizeHint() const
{
    return sizeHint();
}

QRectF NavigationBar::itemRect(int index) const
{
    if (m_items.isEmpty()) {
        return QRectF();
    }
    const qreal w = static_cast<qreal>(width()) / m_items.size();
    return QRectF(index * w, 0, w, BarHeight);
}

qreal NavigationBar::itemCenterX(int index) const
{
    if (m_items.isEmpty()) {
        return 0.0;
    }
    const qreal w = static_cast<qreal>(width()) / m_items.size();
    return index * w + w / 2.0;
}

void NavigationBar::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const Md3ColorScheme &c = Theme::instance().colors();

    // --- Container -----------------------------------------------------------
    p.fillRect(rect(), c.surfaceContainer);

    if (m_items.isEmpty()) {
        return;
    }

    const qreal itemWidth = static_cast<qreal>(width()) / m_items.size();

    // --- Active indicator pill ------------------------------------------------
    const QRectF pill(m_indicatorX - kIndicatorWidth / 2.0,
                      kRowTop + (kIndicatorHeight - kIndicatorHeight) / 2.0,
                      kIndicatorWidth, kIndicatorHeight);
    p.setPen(Qt::NoPen);
    p.setBrush(c.secondaryContainer);
    p.drawRoundedRect(pill, kIndicatorHeight / 2.0, kIndicatorHeight / 2.0);

    // --- Items -----------------------------------------------------------------
    for (int i = 0; i < m_items.size(); ++i) {
        const Item &item = m_items[i];

        // Activity factor: 1 for the selected item, fading with distance.
        qreal activity = 1.0 - qAbs(m_indicatorX - itemCenterX(i)) / (itemWidth * 0.6);
        activity = qBound(0.0, activity, 1.0);

        const qreal iconCenterY = kRowTop + kIndicatorHeight / 2.0;
        const QRectF iconRect(itemCenterX(i) - kIconSize / 2.0,
                              iconCenterY - kIconSize / 2.0, kIconSize, kIconSize);

        if (!item.tintedOff.isNull()) {
            item.tintedOff.paint(&p, iconRect.toRect(), Qt::AlignCenter, QIcon::Normal, QIcon::Off);
        }
        if (activity > 0.0 && !item.tintedOn.isNull()) {
            p.setOpacity(activity);
            item.tintedOn.paint(&p, iconRect.toRect(), Qt::AlignCenter, QIcon::Normal, QIcon::Off);
            p.setOpacity(1.0);
        }

        // --- Label ---------------------------------------------------------
        const QFont font = Theme::instance().font(TypeRole::LabelMedium);
        p.setFont(font);
        const QRectF labelRect(itemRect(i).left(), kLabelTop, itemWidth, BarHeight - kLabelTop - 8);

        p.setPen(c.onSurfaceVariant);
        p.drawText(labelRect, Qt::AlignHCenter | Qt::AlignTop, item.label);
        if (activity > 0.0) {
            p.setPen(c.onSurface);
            p.setOpacity(activity);
            p.drawText(labelRect, Qt::AlignHCenter | Qt::AlignTop, item.label);
            p.setOpacity(1.0);
        }
    }

    // --- Press state layer / ripple --------------------------------------------
    if (m_pressedIndex >= 0 && m_ripple) {
        const QRectF target(itemCenterX(m_pressedIndex) - kIndicatorWidth / 2.0,
                            kRowTop, kIndicatorWidth, kIndicatorHeight);
        m_ripple->setColor(c.onSurface);
        m_ripple->paint(p, target, kIndicatorHeight / 2.0);
    }
}

void NavigationBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton || m_items.isEmpty()) {
        return;
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QPointF pos = event->position();
#else
    const QPointF pos = event->pos();
#endif
    m_pressedIndex = qBound(0, static_cast<int>(pos.x() / (width() / m_items.size())),
                            m_items.size() - 1);
    if (m_ripple) {
        m_ripple->press(QPointF(itemCenterX(m_pressedIndex), kRowTop + kIndicatorHeight / 2.0));
    }
    update();
    event->accept();
}

void NavigationBar::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_items.isEmpty()) {
        return;
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QPointF pos = event->position();
#else
    const QPointF pos = event->pos();
#endif
    const int released = qBound(0, static_cast<int>(pos.x() / (width() / m_items.size())),
                                m_items.size() - 1);
    if (m_ripple) {
        m_ripple->release();
    }
    if (released == m_pressedIndex) {
        emit itemClicked(released);
        setCurrentIndex(released, true);
    }
    m_pressedIndex = -1;
    update();
}

} // namespace Md3