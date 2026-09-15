#include "Md3Dialog.h"

#include <QFontMetrics>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QVBoxLayout>
#include <QVariantAnimation>

#include "Md3Button.h"
#include "Md3Motion.h"

namespace Md3 {

namespace {

constexpr int kMinWidth = 280;
constexpr int kMaxWidth = 560;
constexpr int kPadding = 24;
constexpr qreal kCornerRadius = 28.0;
constexpr qreal kScrimAlpha = 0.32;

} // namespace

class Dialog::Panel : public QWidget {
public:
    explicit Panel(Dialog *dialog)
        : QWidget(dialog)
        , m_dialog(dialog)
    {
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setPen(Qt::NoPen);
        p.setBrush(Theme::instance().colors().surfaceContainerHigh);
        p.drawRoundedRect(QRectF(rect()), kCornerRadius, kCornerRadius);
    }

private:
    Dialog *m_dialog;
};

Dialog::Dialog(QWidget *host)
    : QWidget(host ? host->window() : nullptr)
{
    initialize();
}

Dialog::~Dialog() = default;

void Dialog::initialize()
{
    m_hostWindow = parentWidget();
    if (m_hostWindow) {
        m_hostWindow->installEventFilter(this);
    }

    setAttribute(Qt::WA_StyledBackground, false);
    hide();

    m_panel = new Panel(this);
    auto *layout = new QVBoxLayout(m_panel);
    layout->setContentsMargins(kPadding, kPadding, kPadding, kPadding);
    layout->setSpacing(0);

    m_headlineLabel = new QLabel(m_panel);
    m_headlineLabel->setWordWrap(true);
    m_headlineLabel->setFont(Theme::instance().font(TypeRole::HeadlineSmall));
    layout->addWidget(m_headlineLabel);

    layout->addSpacing(16);

    m_supportingLabel = new QLabel(m_panel);
    m_supportingLabel->setWordWrap(true);
    m_supportingLabel->setFont(Theme::instance().font(TypeRole::BodyMedium));
    m_supportingLabel->hide();
    layout->addWidget(m_supportingLabel);

    layout->addSpacing(24);

    m_actionsRow = new QWidget(m_panel);
    m_actionsLayout = new QHBoxLayout(m_actionsRow);
    m_actionsLayout->setContentsMargins(0, 0, 0, 0);
    m_actionsLayout->setSpacing(8);
    m_actionsLayout->addStretch(1);
    layout->addWidget(m_actionsRow);

    m_panelOpacity = new QGraphicsOpacityEffect(m_panel);
    m_panelOpacity->setOpacity(0.0);
    m_panel->setGraphicsEffect(m_panelOpacity);

    m_anim = new QVariantAnimation(this);
    connect(m_anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        m_reveal = v.toReal();
        layoutPanel();
        update();
    });
    connect(m_anim, &QVariantAnimation::finished, this, [this]() {
        if (!m_open && m_reveal <= 0.0) {
            hide();
            emit closed();
        }
    });

    connect(&Theme::instance(), &Theme::changed, this, [this]() {
        updateColors();
        update();
    });

    updateColors();
    updateGeometryToHost();
}

void Dialog::updateColors()
{
    const Md3ColorScheme &c = Theme::instance().colors();
    if (m_headlineLabel) {
        m_headlineLabel->setStyleSheet(QStringLiteral("color: %1;").arg(c.onSurface.name()));
    }
    if (m_supportingLabel) {
        m_supportingLabel->setStyleSheet(QStringLiteral("color: %1;").arg(c.onSurfaceVariant.name()));
    }
}

void Dialog::setHeadline(const QString &text)
{
    m_headline = text;
    m_headlineLabel->setText(text);
    updateGeometryToHost();
}

void Dialog::setSupportingText(const QString &text)
{
    m_supporting = text;
    m_supportingLabel->setText(text);
    m_supportingLabel->setVisible(!text.isEmpty());
    updateGeometryToHost();
}

int Dialog::addAction(const QString &label, std::function<void()> action)
{
    m_actions.push_back(Action{label, action});
    const int index = static_cast<int>(m_actions.size()) - 1;

    auto *button = new Button(label, Button::Text, m_actionsRow);
    connect(button, &Button::clicked, this, [this, index]() {
        std::function<void()> fn;
        if (index >= 0 && index < static_cast<int>(m_actions.size())) {
            fn = m_actions[static_cast<size_t>(index)].fn;
        }
        close();
        emit actionTriggered(index);
        if (fn) {
            fn();
        }
    });
    m_actionsLayout->addWidget(button);
    updateGeometryToHost();
    return index;
}

void Dialog::clearActions()
{
    m_actions.clear();
    const auto buttons = m_actionsRow->findChildren<Button *>(QString(), Qt::FindDirectChildrenOnly);
    for (Button *button : buttons) {
        m_actionsLayout->removeWidget(button);
        button->deleteLater();
    }
}

void Dialog::updateGeometryToHost()
{
    if (!m_hostWindow) {
        return;
    }
    setGeometry(m_hostWindow->rect());

    const QFontMetrics fmH(Theme::instance().font(TypeRole::HeadlineSmall));
    const QFontMetrics fmS(Theme::instance().font(TypeRole::BodyMedium));
    int desired = kMinWidth;
    if (!m_headline.isEmpty()) {
        desired = qMax(desired, fmH.horizontalAdvance(m_headline) + kPadding * 2 + 16);
    }
    if (!m_supporting.isEmpty()) {
        desired = qMax(desired, fmS.horizontalAdvance(m_supporting) + kPadding * 2 + 16);
    }
    if (m_actionsRow && m_actionsRow->layout()) {
        desired = qMax(desired, m_actionsRow->layout()->sizeHint().width() + kPadding * 2);
    }
    const int w = qMin(kMaxWidth, desired);

    m_panel->setFixedWidth(w);
    m_panel->adjustSize();
    int h = m_panel->height();
    const int maxH = qMax(120, static_cast<int>(height() * 0.9));
    h = qMin(h, maxH);
    m_panel->resize(w, qMax(96, h));

    layoutPanel();
}

void Dialog::layoutPanel()
{
    if (!m_panel) {
        return;
    }
    const int w = m_panel->width();
    const int h = m_panel->height();
    const int x = (width() - w) / 2;
    const int y = (height() - h) / 2 + static_cast<int>((1.0 - m_reveal) * 16.0);
    m_panel->move(x, y);
    if (m_panelOpacity) {
        m_panelOpacity->setOpacity(qBound(0.0, m_reveal, 1.0));
    }
}

void Dialog::open()
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
}

void Dialog::close()
{
    if (!m_open) {
        return;
    }
    m_open = false;
    animateTo(0.0, true);
}

void Dialog::animateTo(qreal target, bool closing)
{
    m_anim->stop();
    m_anim->setDuration(closing ? Motion::Short4 : Motion::Medium2); // 200 / 300 ms
    m_anim->setEasingCurve(closing ? Motion::emphasizedAccelerate()
                                   : Motion::emphasizedDecelerate());
    m_anim->setStartValue(m_reveal);
    m_anim->setEndValue(target);
    m_anim->start();
}

void Dialog::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QColor scrim = Theme::instance().colors().scrim;
    scrim.setAlphaF(kScrimAlpha * m_reveal);
    p.fillRect(rect(), scrim);
}

void Dialog::mousePressEvent(QMouseEvent *event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QPointF pos = event->position();
#else
    const QPointF pos = event->pos();
#endif
    if (!m_panel->geometry().contains(pos.toPoint())) {
        if (m_dismissible) {
            close();
        }
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void Dialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        if (m_dismissible) {
            close();
        }
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

bool Dialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_hostWindow && event->type() == QEvent::Resize) {
        updateGeometryToHost();
        return false;
    }
    return QWidget::eventFilter(watched, event);
}

Dialog *Dialog::showMessage(QWidget *host, const QString &headline,
                            const QString &supporting,
                            const QStringList &actions,
                            std::function<void(int)> onResult)
{
    auto *dialog = new Dialog(host);
    dialog->setHeadline(headline);
    if (!supporting.isEmpty()) {
        dialog->setSupportingText(supporting);
    }

    QStringList labels = actions;
    if (labels.isEmpty()) {
        labels << QObject::tr("OK");
    }
    for (int i = 0; i < labels.size(); ++i) {
        const int index = i;
        dialog->addAction(labels.at(i), [onResult, index]() {
            if (onResult) {
                onResult(index);
            }
        });
    }

    QObject::connect(dialog, &Dialog::closed, dialog, &QObject::deleteLater);
    dialog->open();
    return dialog;
}

} // namespace Md3
