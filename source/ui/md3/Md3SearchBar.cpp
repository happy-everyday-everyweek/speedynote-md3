#include "Md3SearchBar.h"

#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>

#include "Md3Ripple.h"

namespace Md3 {

namespace {

constexpr int kHeight = 56;
constexpr int kGlyphBox = 24;
constexpr int kLeftPadding = 16;
constexpr int kGlyphGap = 12;
constexpr int kClearZone = 40;

} // namespace

SearchBar::SearchBar(QWidget *parent)
    : QWidget(parent)
{
    initialize();
}

void SearchBar::initialize()
{
    setAttribute(Qt::WA_Hover, true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_edit = new QLineEdit(this);
    m_edit->setFrame(false);
    m_edit->setStyleSheet(QStringLiteral(
        "QLineEdit { background: transparent; border: none; padding: 0; }"));

    m_ripple = new Ripple(this);
    connect(m_ripple, &Ripple::repaintRequested, this, qOverload<>(&QWidget::update));

    connect(m_edit, &QLineEdit::textChanged, this, &SearchBar::textChanged);
    connect(m_edit, &QLineEdit::textChanged, this, qOverload<>(&QWidget::update));
    connect(m_edit, &QLineEdit::textEdited, this, &SearchBar::textEdited);
    connect(m_edit, &QLineEdit::returnPressed, this, &SearchBar::returnPressed);
    connect(&Theme::instance(), &Theme::changed, this, [this]() {
        updateThemeColors();
        update();
    });

    updateThemeColors();
}

void SearchBar::updateThemeColors()
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

void SearchBar::setPlaceholderText(const QString &text)
{
    if (m_edit) {
        m_edit->setPlaceholderText(text);
    }
}

QString SearchBar::placeholderText() const
{
    return m_edit ? m_edit->placeholderText() : QString();
}

void SearchBar::setText(const QString &text)
{
    if (m_edit) {
        m_edit->setText(text);
    }
}

QString SearchBar::text() const
{
    return m_edit ? m_edit->text() : QString();
}

QSize SearchBar::sizeHint() const
{
    return QSize(280, kHeight);
}

QSize SearchBar::minimumSizeHint() const
{
    return QSize(120, kHeight);
}

QRectF SearchBar::clearRect() const
{
    return QRectF(width() - kClearZone, (height() - kClearZone) / 2.0,
                  kClearZone, kClearZone);
}

void SearchBar::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    const int editTop = (height() - 40) / 2;
    const int editLeft = kLeftPadding + kGlyphBox + kGlyphGap;
    m_edit->setGeometry(editLeft, editTop, qMax(0, width() - editLeft - kClearZone), 40);
}

void SearchBar::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const Md3ColorScheme &c = Theme::instance().colors();
    const qreal radius = height() / 2.0;
    const QRectF box = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);

    // Container.
    p.setPen(Qt::NoPen);
    p.setBrush(c.surfaceContainerHigh);
    p.drawRoundedRect(box, radius, radius);

    // Ripple hover state layer.
    if (m_ripple) {
        m_ripple->setColor(c.onSurface);
        m_ripple->paint(p, box, radius);
    }

    // Leading search glyph.
    const qreal gx = kLeftPadding;
    const qreal cy = height() / 2.0;
    QPen glyphPen(c.onSurfaceVariant, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p.setPen(glyphPen);
    p.setBrush(Qt::NoBrush);
    const QRectF lens(gx + 3.0, cy - 8.0, 11.0, 11.0);
    p.drawEllipse(lens);
    p.drawLine(QPointF(gx + 12.0, cy + 1.0), QPointF(gx + 16.0, cy + 5.0));

    // Trailing clear affordance.
    if (m_edit && !m_edit->text().isEmpty()) {
        const QRectF cr = clearRect();
        const QPointF center = cr.center();
        const qreal arm = 5.0;
        p.setPen(QPen(c.onSurfaceVariant, 1.6, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(center.x() - arm, center.y() - arm),
                   QPointF(center.x() + arm, center.y() + arm));
        p.drawLine(QPointF(center.x() - arm, center.y() + arm),
                   QPointF(center.x() + arm, center.y() - arm));
    }
}

void SearchBar::mousePressEvent(QMouseEvent *event)
{
    m_pressedInClear = false;
    if (event->button() == Qt::LeftButton && m_ripple) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        const QPointF pos = event->position();
#else
        const QPointF pos = QPointF(event->pos());
#endif
        const bool onClear = m_edit && !m_edit->text().isEmpty()
                             && clearRect().contains(pos);
        if (onClear) {
            m_pressedInClear = true;
        } else if (!m_edit->geometry().contains(pos.toPoint())) {
            m_ripple->press(pos);
        }
    }
    QWidget::mousePressEvent(event);
}

void SearchBar::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_ripple) {
        m_ripple->release();
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QPointF pos = event->position();
#else
    const QPointF pos = QPointF(event->pos());
#endif
    if (m_pressedInClear && clearRect().contains(pos) && m_edit) {
        m_edit->clear();
        emit cleared();
    }
    m_pressedInClear = false;
    QWidget::mouseReleaseEvent(event);
}

} // namespace Md3
