#include "Md3TextField.h"

#include <QEvent>
#include <QFontMetricsF>
#include <QLineEdit>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QVariantAnimation>

#include "Md3Motion.h"

namespace Md3 {

namespace {

constexpr int kFieldHeight = 56;
constexpr int kLabelLine = 20;
constexpr int kSupportLine = 20;
constexpr int kCorner = 4;

QColor withAlpha(const QColor &color, qreal alpha)
{
    QColor c = color;
    c.setAlphaF(qBound(0.0, alpha, 1.0));
    return c;
}

QColor mixColor(const QColor &a, const QColor &b, qreal t)
{
    return QColor::fromRgbF(a.redF() + (b.redF() - a.redF()) * t,
                            a.greenF() + (b.greenF() - a.greenF()) * t,
                            a.blueF() + (b.blueF() - a.blueF()) * t,
                            a.alphaF() + (b.alphaF() - a.alphaF()) * t);
}

} // namespace

TextField::TextField(QWidget *parent)
    : QWidget(parent)
{
    initialize();
}

void TextField::initialize()
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_edit = new QLineEdit(this);
    m_edit->setFrame(false);
    m_edit->setStyleSheet(QStringLiteral(
        "QLineEdit { background: transparent; border: none; padding: 0; }"));
    m_edit->installEventFilter(this);

    m_anim = new QVariantAnimation(this);
    m_anim->setDuration(Motion::Short3); // 150 ms
    m_anim->setEasingCurve(Motion::standard());
    connect(m_anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        m_focusProgress = qBound(0.0, v.toReal(), 1.0);
        update();
    });

    connect(m_edit, &QLineEdit::textChanged, this, &TextField::textChanged);
    connect(m_edit, &QLineEdit::editingFinished, this, &TextField::editingFinished);
    connect(&Theme::instance(), &Theme::changed, this, [this]() {
        updateThemeColors();
        update();
    });

    updateThemeColors();
}

void TextField::updateThemeColors()
{
    if (!m_edit) {
        return;
    }
    m_edit->setFont(Theme::instance().font(TypeRole::BodyLarge));
    const Md3ColorScheme &c = Theme::instance().colors();
    QPalette pal = m_edit->palette();
    pal.setColor(QPalette::Text, c.onSurface);
    pal.setColor(QPalette::PlaceholderText, c.onSurfaceVariant);
    m_edit->setPalette(pal);
}

void TextField::setVariant(Variant variant)
{
    if (m_variant == variant) {
        return;
    }
    m_variant = variant;
    update();
}

void TextField::setLabel(const QString &text)
{
    m_label = text;
    updateGeometry();
    layoutEdit();
    update();
}

void TextField::setSupportingText(const QString &text)
{
    m_supporting = text;
    updateGeometry();
    layoutEdit();
    update();
}

void TextField::setErrorText(const QString &text)
{
    m_error = text;
    updateGeometry();
    layoutEdit();
    update();
}

void TextField::setText(const QString &text)
{
    if (m_edit) {
        m_edit->setText(text);
    }
}

QString TextField::text() const
{
    return m_edit ? m_edit->text() : QString();
}

int TextField::labelHeight() const
{
    return m_label.isEmpty() ? 0 : kLabelLine;
}

int TextField::supportingHeight() const
{
    return (m_error.isEmpty() && m_supporting.isEmpty()) ? 0 : kSupportLine;
}

QSize TextField::sizeHint() const
{
    return QSize(240, labelHeight() + kFieldHeight + supportingHeight());
}

QSize TextField::minimumSizeHint() const
{
    return QSize(120, labelHeight() + kFieldHeight + supportingHeight());
}

void TextField::animateFocus(bool focused)
{
    m_focused = focused;
    m_anim->stop();
    m_anim->setStartValue(m_focusProgress);
    m_anim->setEndValue(focused ? 1.0 : 0.0);
    m_anim->start();
    update();
}

void TextField::layoutEdit()
{
    if (!m_edit) {
        return;
    }
    const int top = labelHeight();
    m_edit->setGeometry(16, top + (kFieldHeight - 40) / 2, qMax(0, width() - 32), 40);
}

void TextField::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    layoutEdit();
}

bool TextField::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_edit) {
        if (event->type() == QEvent::FocusIn) {
            animateFocus(true);
        } else if (event->type() == QEvent::FocusOut) {
            animateFocus(false);
        }
    }
    return QWidget::eventFilter(watched, event);
}

void TextField::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const Md3ColorScheme &c = Theme::instance().colors();
    const int lh = labelHeight();
    const int sh = supportingHeight();
    const QRectF field(0.0, lh, width(), kFieldHeight);
    const qreal t = m_focusProgress;

    const bool error = !m_error.isEmpty();
    const QColor accent = error ? c.error : c.primary;

    // Label above the field.
    if (!m_label.isEmpty()) {
        p.setFont(Theme::instance().font(TypeRole::LabelSmall));
        p.setPen(mixColor(c.onSurfaceVariant, error ? c.error : c.primary, t));
        p.drawText(QRectF(0.0, 0.0, width(), lh),
                   Qt::AlignVCenter | Qt::AlignLeft | Qt::TextSingleLine, m_label);
    }

    if (m_variant == Filled) {
        // Container with 4dp top corners.
        QPainterPath path;
        path.moveTo(field.left(), field.bottom());
        path.lineTo(field.left(), field.top() + kCorner);
        path.quadTo(field.left(), field.top(), field.left() + kCorner, field.top());
        path.lineTo(field.right() - kCorner, field.top());
        path.quadTo(field.right(), field.top(), field.right(), field.top() + kCorner);
        path.lineTo(field.right(), field.bottom());
        path.closeSubpath();
        p.fillPath(path, c.surfaceContainerHighest);

        // Underline: 1dp idle -> 2dp focused (animated).
        const qreal lw = 1.0 + t;
        const int lwInt = static_cast<int>(lw + 0.5);
        p.fillRect(QRectF(field.left(), field.bottom() - lw, field.width(), lw),
                   mixColor(error ? c.error : c.outline, accent, t));
        Q_UNUSED(lwInt);
    } else {
        // Outlined: 1dp border growing to 2dp in the accent color.
        const qreal penW = qMin(field.width(), field.height());
        Q_UNUSED(penW);
        const qreal lw = 1.0 + t;
        p.setPen(QPen(mixColor(c.outline, accent, t), lw));
        p.setBrush(Qt::NoBrush);
        const qreal inset = lw / 2.0;
        p.drawRoundedRect(field.adjusted(inset, inset, -inset, -inset),
                          kCorner + 2.0, kCorner + 2.0);
    }

    // Supporting / error text below the field.
    if (sh > 0) {
        const QString text = error ? m_error : m_supporting;
        p.setFont(Theme::instance().font(TypeRole::BodySmall));
        p.setPen(error ? c.error : c.onSurfaceVariant);
        p.drawText(QRectF(16.0, field.bottom(), width() - 32.0, sh),
                   Qt::AlignVCenter | Qt::AlignLeft | Qt::TextSingleLine, text);
    }
}

} // namespace Md3
