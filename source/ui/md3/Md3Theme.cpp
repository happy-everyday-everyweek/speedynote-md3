#include "Md3Theme.h"

#include <QApplication>
#include <QPainter>
#include <QPalette>
#include <QSettings>
#include <QtMath>

namespace Md3 {

namespace {
const char *kDefaultSeed = "#316882"; // SpeedyNote brand accent
constexpr int kDefaultVariant = static_cast<int>(SchemeVariant::TonalSpot);
} // namespace

Theme &Theme::instance()
{
    static Theme inst;
    return inst;
}

Theme::Theme()
    : QObject(nullptr)
{
    m_seed = QColor(kDefaultSeed);
    rebuild();
}

void Theme::configure(const QColor &seed, bool dark, SchemeVariant variant, double contrastLevel)
{
    m_seed = seed.isValid() ? seed : QColor(kDefaultSeed);
    m_variant = variant;
    m_contrast = contrastLevel;
    m_color = Md3ColorScheme::fromSeedVariant(m_seed, dark, static_cast<int>(m_variant), m_contrast);
    emit changed();
}

void Theme::setDarkMode(bool dark)
{
    if (m_color.isDark == dark) {
        return;
    }
    m_color = Md3ColorScheme::fromSeedVariant(m_seed, dark, static_cast<int>(m_variant), m_contrast);
    emit changed();
}

void Theme::setSeedColor(const QColor &seed)
{
    if (!seed.isValid() || seed == m_seed) {
        return;
    }
    m_seed = seed;
    m_color = Md3ColorScheme::fromSeedVariant(m_seed, m_color.isDark, static_cast<int>(m_variant), m_contrast);
    emit changed();
}

void Theme::rebuild()
{
    m_color = Md3ColorScheme::fromSeedVariant(m_seed, m_color.isDark, static_cast<int>(m_variant), m_contrast);
}

QFont Theme::font(TypeRole role, const QFont &base) const
{
    QFont f = base.family().isEmpty() ? QApplication::font() : base;

    int pixelSize = 14;
    QFont::Weight weight = QFont::Normal;

    switch (role) {
    case TypeRole::DisplayLarge:   pixelSize = 57; break;
    case TypeRole::DisplayMedium:  pixelSize = 45; break;
    case TypeRole::DisplaySmall:   pixelSize = 36; break;
    case TypeRole::HeadlineLarge:  pixelSize = 32; break;
    case TypeRole::HeadlineMedium: pixelSize = 28; break;
    case TypeRole::HeadlineSmall:  pixelSize = 24; break;
    case TypeRole::TitleLarge:     pixelSize = 22; break;
    case TypeRole::TitleMedium:    pixelSize = 16; weight = QFont::Medium; break;
    case TypeRole::TitleSmall:     pixelSize = 14; weight = QFont::Medium; break;
    case TypeRole::BodyLarge:      pixelSize = 16; break;
    case TypeRole::BodyMedium:     pixelSize = 14; break;
    case TypeRole::BodySmall:      pixelSize = 12; break;
    case TypeRole::LabelLarge:     pixelSize = 14; weight = QFont::Medium; break;
    case TypeRole::LabelMedium:    pixelSize = 12; weight = QFont::Medium; break;
    case TypeRole::LabelSmall:     pixelSize = 11; weight = QFont::Medium; break;
    }

    f.setPixelSize(pixelSize);
    f.setWeight(weight);
    return f;
}

void Theme::applyToApplication(QApplication *app)
{
    if (!app) {
        return;
    }

    const Md3ColorScheme &c = m_color;

    QPalette p;
    p.setColor(QPalette::Window, c.surface);
    p.setColor(QPalette::WindowText, c.onSurface);
    p.setColor(QPalette::Base, c.surfaceContainerHigh);
    p.setColor(QPalette::AlternateBase, c.surfaceContainer);
    p.setColor(QPalette::Text, c.onSurface);
    p.setColor(QPalette::ToolTipBase, c.inverseSurface);
    p.setColor(QPalette::ToolTipText, c.inverseOnSurface);
    p.setColor(QPalette::Button, c.surfaceContainerHigh);
    p.setColor(QPalette::ButtonText, c.onSurface);
    p.setColor(QPalette::BrightText, c.error);
    p.setColor(QPalette::Link, c.primary);
    p.setColor(QPalette::LinkVisited, c.tertiary);
    p.setColor(QPalette::Highlight, c.primary);
    p.setColor(QPalette::HighlightedText, c.onPrimary);
    p.setColor(QPalette::PlaceholderText, c.onSurfaceVariant);

    // Disabled states: onSurface at 38% / 12% (M3 opacity rules).
    QColor disabledText = c.onSurface;
    disabledText.setAlphaF(0.38);
    QColor disabledFill = c.onSurface;
    disabledFill.setAlphaF(0.12);

    p.setColor(QPalette::Disabled, QPalette::WindowText, disabledText);
    p.setColor(QPalette::Disabled, QPalette::Text, disabledText);
    p.setColor(QPalette::Disabled, QPalette::ButtonText, disabledText);
    p.setColor(QPalette::Disabled, QPalette::Base, disabledFill);
    p.setColor(QPalette::Disabled, QPalette::Button, disabledFill);
    p.setColor(QPalette::Disabled, QPalette::Highlight, disabledFill);

    app->setPalette(p);
}

QString Theme::baseStyleSheet() const
{
    const Md3ColorScheme &c = m_color;

    const QString tooltip =
        QStringLiteral(
            "QToolTip {"
            "  background-color: %1;"
            "  color: %2;"
            "  border: none;"
            "  border-radius: 4px;"
            "  padding: 6px 10px;"
            "}")
            .arg(c.inverseSurface.name(), c.inverseOnSurface.name());

    const QString scrollbars =
        QStringLiteral(
            "QScrollBar:vertical { background: transparent; width: 8px; margin: 2px; }"
            "QScrollBar::handle:vertical { background: %1; border-radius: 4px; min-height: 40px; }"
            "QScrollBar::handle:vertical:hover { background: %2; }"
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical,"
            "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; height: 0px; }"
            "QScrollBar:horizontal { background: transparent; height: 8px; margin: 2px; }"
            "QScrollBar::handle:horizontal { background: %1; border-radius: 4px; min-width: 40px; }"
            "QScrollBar::handle:horizontal:hover { background: %2; }"
            "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal,"
            "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background: none; width: 0px; }")
            .arg(c.onSurfaceVariant.name( QColor::HexArgb ), c.onSurface.name());

    return tooltip + scrollbars;
}

void Theme::load()
{
    QSettings settings(QStringLiteral("SpeedyNote"), QStringLiteral("App"));

    const QString seedName = settings.value(QStringLiteral("md3/seedColor"),
                                            QString::fromLatin1(kDefaultSeed)).toString();
    QColor seed(seedName);
    if (!seed.isValid()) {
        seed = QColor(kDefaultSeed);
    }

    int variant = settings.value(QStringLiteral("md3/schemeVariant"), kDefaultVariant).toInt();
    variant = qBound(0, variant, static_cast<int>(SchemeVariant::FruitSalad));

    const double contrast = settings.value(QStringLiteral("md3/contrastLevel"), 0.0).toDouble();

    m_seed = seed;
    m_variant = static_cast<SchemeVariant>(variant);
    m_contrast = qBound(-1.0, contrast, 1.0);

    rebuild();
}

void Theme::save() const
{
    QSettings settings(QStringLiteral("SpeedyNote"), QStringLiteral("App"));
    settings.setValue(QStringLiteral("md3/seedColor"), m_seed.name());
    settings.setValue(QStringLiteral("md3/schemeVariant"), static_cast<int>(m_variant));
    settings.setValue(QStringLiteral("md3/contrastLevel"), m_contrast);
}

void Theme::paintElevation(QPainter &painter, const QRectF &rect,
                           qreal cornerRadius, double level, const QColor &shadowColor)
{
    const int lv = qBound(0, static_cast<int>(qRound(level)), 5);
    if (lv == 0) {
        return;
    }

    // Shadow geometry per level: key shadow dy + blur budget, plus alpha.
    struct ElevationSpec { qreal dy; int blur; qreal alpha; };
    static const ElevationSpec table[6] = {
        {0.0, 0, 0.0},
        {1.0, 3, 0.18},
        {1.0, 4, 0.20},
        {2.0, 6, 0.22},
        {2.0, 8, 0.24},
        {3.0, 10, 0.26},
    };
    const ElevationSpec &spec = table[lv];

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(Qt::NoBrush);

    // Fake gaussian blur by stacking expanding strokes with falling alpha.
    const int steps = qMax(2, spec.blur);
    for (int i = steps; i >= 1; --i) {
        const qreal t = static_cast<qreal>(i) / steps;
        QColor col = shadowColor;
        col.setAlphaF(spec.alpha * t * 0.30);
        QPen pen(col);
        pen.setWidthF(1.6);
        painter.setPen(pen);

        const qreal expand = i * 0.5;
        QRectF r = rect.adjusted(-expand, -expand + spec.dy, expand, expand + spec.dy);
        const qreal rad = cornerRadius > 0.0 ? cornerRadius + expand * 0.6 : 0.0;
        painter.drawRoundedRect(r, rad, rad);
    }

    painter.restore();
}

} // namespace Md3
