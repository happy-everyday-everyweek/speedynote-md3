#include "Md3Theme.h"

#include <QApplication>
#include <QGuiApplication>
#include <QPainter>
#include <QPalette>
#include <QSettings>
#include <QStyleHints>
#include <QtMath>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#endif

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

bool Theme::applyDynamicColor()
{
#ifdef Q_OS_ANDROID
    const jint argb = QJniObject::callStaticMethod<jint>(
        "org/speedynote/app/Md3ColorHelper",
        "getSystemAccentColor",
        "()I");
    if (argb != 0) {
        const QColor seed = QColor::fromRgba(static_cast<QRgb>(argb));
        if (seed.isValid() && seed != m_seed) {
            setSeedColor(seed);
            return true;
        }
    }
#endif
    return false;
}

void Theme::followSystemColorScheme()
{
    auto applyCurrentMode = [this]() {
        QSettings settings(QStringLiteral("SpeedyNote"), QStringLiteral("App"));
        const QString mode = settings.value(QStringLiteral("md3/themeMode"),
                                            QStringLiteral("system")).toString();
        if (mode == QLatin1String("dark")) {
            setDarkMode(true);
            return;
        }
        if (mode == QLatin1String("light")) {
            setDarkMode(false);
            return;
        }
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        const auto scheme = QGuiApplication::styleHints()->colorScheme();
        setDarkMode(scheme == Qt::ColorScheme::Dark);
#endif
    };

    applyCurrentMode();

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this,
            [applyCurrentMode](Qt::ColorScheme) { applyCurrentMode(); });
#endif
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

QString Theme::applicationStyleSheet() const
{
    const Md3ColorScheme &c = m_color;

    auto fade = [](const QColor &color, qreal alpha) {
        QColor cc = color;
        cc.setAlphaF(alpha);
        return QStringLiteral("rgba(%1, %2, %3, %4)")
            .arg(cc.red())
            .arg(cc.green())
            .arg(cc.blue())
            .arg(QString::number(cc.alphaF(), 'f', 3));
    };

    const QString onSurface = c.onSurface.name();
    const QString onSurfaceVariant = c.onSurfaceVariant.name();
    const QString primary = c.primary.name();
    const QString onPrimary = c.onPrimary.name();
    const QString surface = c.surface.name();
    const QString surfaceContainer = c.surfaceContainer.name();
    const QString surfaceContainerHigh = c.surfaceContainerHigh.name();
    const QString surfaceContainerHighest = c.surfaceContainerHighest.name();
    const QString secondaryContainer = c.secondaryContainer.name();
    const QString onSecondaryContainer = c.onSecondaryContainer.name();
    const QString outlineVariant = c.outlineVariant.name();

    const QColor buttonHover = c.secondaryContainer.darker(108);
    const QColor buttonPressed = c.secondaryContainer.darker(116);

    QString qss = baseStyleSheet();

    // Text inputs.
    qss += QStringLiteral(
               "QLineEdit, QPlainTextEdit, QTextEdit, QSpinBox, QDoubleSpinBox, QComboBox {"
               "  background-color: %1;"
               "  color: %2;"
               "  border: 1px solid %3;"
               "  border-radius: 8px;"
               "  padding: 8px 12px;"
               "  selection-background-color: %4;"
               "  selection-color: %5;"
               "}"
               "QLineEdit:focus, QPlainTextEdit:focus, QTextEdit:focus,"
               "QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus {"
               "  border: 2px solid %4;"
               "}"
               "QLineEdit:disabled, QPlainTextEdit:disabled, QTextEdit:disabled,"
               "QComboBox:disabled {"
               "  color: %6;"
               "  background-color: %7;"
               "}"
               "QComboBox::drop-down { border: none; width: 28px; }"
               "QComboBox QAbstractItemView {"
               "  background-color: %8;"
               "  color: %2;"
               "  border: 1px solid %3;"
               "  selection-background-color: %9;"
               "  outline: none;"
               "}")
        .arg(surfaceContainerHigh,
             onSurface,
             outlineVariant,
             primary,
             onPrimary,
             fade(c.onSurface, 0.38),
             fade(c.onSurface, 0.12),
             surfaceContainer,
             fade(c.onSurface, 0.10));

    // Buttons & tool buttons (generic fallback for legacy widgets; the MD3
    // component library paints itself and is unaffected).
    qss += QStringLiteral(
               "QPushButton {"
               "  background-color: %1;"
               "  color: %2;"
               "  border: none;"
               "  border-radius: 20px;"
               "  padding: 8px 20px;"
               "  font-size: 14px;"
               "}"
               "QPushButton:hover { background-color: %3; }"
               "QPushButton:pressed { background-color: %4; }"
               "QPushButton:disabled { color: %5; background-color: %6; }"
               "QPushButton:flat { background-color: transparent; }"
               "QToolButton {"
               "  background: transparent;"
               "  border: none;"
               "  border-radius: 8px;"
               "  padding: 4px;"
               "  color: %7;"
               "}"
               "QToolButton:hover { background-color: %8; }"
               "QToolButton:pressed { background-color: %9; }")
        .arg(secondaryContainer,
             onSecondaryContainer,
             buttonHover.name(),
             buttonPressed.name(),
             fade(c.onSurface, 0.38),
             fade(c.onSurface, 0.12),
             onSurface,
             fade(c.onSurface, 0.08),
             fade(c.onSurface, 0.10));

    // Containers, dialogs and tabs.
    qss += QStringLiteral(
               "QGroupBox {"
               "  border: 1px solid %1;"
               "  border-radius: 12px;"
               "  margin-top: 14px;"
               "  padding-top: 10px;"
               "  color: %2;"
               "}"
               "QGroupBox::title {"
               "  subcontrol-origin: margin;"
               "  left: 12px;"
               "  padding: 0 4px;"
               "  color: %3;"
               "}"
               "QDialog, QMessageBox, QInputDialog { background-color: %4; }"
               "QTabWidget::pane { border: none; }"
               "QTabBar::tab {"
               "  background: transparent;"
               "  color: %5;"
               "  padding: 8px 16px;"
               "  border-radius: 16px;"
               "  margin: 2px 4px;"
               "}"
               "QTabBar::tab:selected {"
               "  background-color: %6;"
               "  color: %7;"
               "}")
        .arg(outlineVariant,
             onSurface,
             primary,
             surface,
             onSurfaceVariant,
             secondaryContainer,
             onSecondaryContainer);

    // Sliders & progress indicators.
    qss += QStringLiteral(
               "QSlider::groove:horizontal { height: 4px; background: %1; border-radius: 2px; }"
               "QSlider::sub-page:horizontal { background: %2; border-radius: 2px; }"
               "QSlider::handle:horizontal {"
               "  background: %2; width: 16px; height: 16px; margin: -6px 0; border-radius: 8px;"
               "}"
               "QSlider::groove:vertical { width: 4px; background: %1; border-radius: 2px; }"
               "QSlider::sub-page:vertical { background: %2; border-radius: 2px; }"
               "QSlider::handle:vertical {"
               "  background: %2; height: 16px; margin: 0 -6px; border-radius: 8px;"
               "}"
               "QProgressBar {"
               "  background-color: %1;"
               "  border: none;"
               "  border-radius: 2px;"
               "  max-height: 4px;"
               "}"
               "QProgressBar::chunk { background-color: %2; border-radius: 2px; }")
        .arg(surfaceContainerHighest,
             primary);

    // Lists, trees, tables.
    qss += QStringLiteral(
               "QTreeView, QListView, QTableView, QListWidget, QTreeWidget, QTableWidget {"
               "  background-color: transparent;"
               "  color: %1;"
               "  border: none;"
               "  outline: none;"
               "  selection-background-color: %2;"
               "  selection-color: %3;"
               "  alternate-background-color: %4;"
               "}"
               "QTreeView::item, QListView::item, QTableView::item,"
               "QListWidget::item, QTreeWidget::item, QTableWidget::item {"
               "  padding: 8px 10px;"
               "  border-radius: 8px;"
               "}"
               "QTreeView::item:hover, QListView::item:hover, QListWidget::item:hover,"
               "QTreeWidget::item:hover { background-color: %5; }"
               "QHeaderView::section {"
               "  background-color: %6;"
               "  color: %7;"
               "  border: none;"
               "  padding: 8px 10px;"
               "}")
        .arg(onSurface,
             secondaryContainer,
             onSecondaryContainer,
             surfaceContainer,
             fade(c.onSurface, 0.06),
             surfaceContainerHigh,
             onSurfaceVariant);

    // Menus and menu bar.
    qss += QStringLiteral(
               "QMenu {"
               "  background-color: %1;"
               "  color: %2;"
               "  border: 1px solid %3;"
               "  border-radius: 12px;"
               "  padding: 8px;"
               "}"
               "QMenu::item {"
               "  padding: 10px 24px 10px 16px;"
               "  border-radius: 8px;"
               "}"
               "QMenu::item:selected { background-color: %4; color: %5; }"
               "QMenu::separator { height: 1px; background: %3; margin: 6px 10px; }"
               "QMenuBar { background-color: %6; color: %2; }"
               "QMenuBar::item { padding: 6px 12px; border-radius: 8px; background: transparent; }"
               "QMenuBar::item:selected { background-color: %4; color: %5; }")
        .arg(surfaceContainer,
             onSurface,
             outlineVariant,
             fade(c.onSurface, 0.10),
             onSurface,
             surfaceContainerHigh);

    // Checkboxes and radio buttons (indicator tinting comes from the palette;
    // this keeps text metrics and spacing aligned with the M3 scale).
    qss += QStringLiteral(
               "QCheckBox, QRadioButton { color: %1; spacing: 8px; padding: 4px 0; }"
               "QCheckBox:disabled, QRadioButton:disabled { color: %2; }")
        .arg(onSurface, fade(c.onSurface, 0.38));

    // Splitter handles and scroll areas.
    qss += QStringLiteral(
               "QSplitter::handle { background-color: %1; }"
               "QSplitter::handle:horizontal { width: 2px; }"
               "QSplitter::handle:vertical { height: 2px; }"
               "QScrollArea { border: none; }")
        .arg(outlineVariant);

    return qss;
}

QString Theme::toolbarStyleSheet() const
{
    const Md3ColorScheme &c = m_color;

    auto fade = [](const QColor &color, qreal alpha) {
        QColor cc = color;
        cc.setAlphaF(alpha);
        return QStringLiteral("rgba(%1, %2, %3, %4)")
            .arg(cc.red())
            .arg(cc.green())
            .arg(cc.blue())
            .arg(QString::number(cc.alphaF(), 'f', 3));
    };

    return QStringLiteral(
               "QPushButton#ActionButton, QPushButton#ToggleButton,"
               "QPushButton#ThreeStateButton, QPushButton#ToolButton {"
               "  border: none;"
               "  border-radius: 18px;"
               "  background: transparent;"
               "  padding: 0px;"
               "}"
               "QPushButton#ActionButton:hover, QPushButton#ToggleButton:hover,"
               "QPushButton#ThreeStateButton:hover, QPushButton#ToolButton:hover {"
               "  background: %1;"
               "}"
               "QPushButton#ActionButton:pressed, QPushButton#ToggleButton:pressed,"
               "QPushButton#ThreeStateButton:pressed, QPushButton#ToolButton:pressed {"
               "  background: %2;"
               "}"
               "QPushButton#ToggleButton:checked, QPushButton#ToolButton:checked {"
               "  background: %3;"
               "}"
               "QPushButton#ToggleButton:checked:hover, QPushButton#ToolButton:checked:hover {"
               "  background: %4;"
               "}"
               "QPushButton#ToolButton[inExpandable=\"true\"]:checked,"
               "QPushButton#ToolButton[inExpandable=\"true\"]:checked:hover {"
               "  background: transparent;"
               "  border: none;"
               "}"
               "QPushButton#ThreeStateButton[state=\"0\"] { background: transparent; }"
               "QPushButton#ThreeStateButton[state=\"1\"] { background: %5; }"
               "QPushButton#ThreeStateButton[state=\"1\"]:hover { background: %6; }"
               "QPushButton#ThreeStateButton[state=\"2\"] { background: %3; }"
               "QPushButton#ThreeStateButton[state=\"2\"]:hover { background: %4; }")
        .arg(fade(c.onSurface, 0.08),
             fade(c.onSurface, 0.10),
             c.secondaryContainer.name(),
             c.secondaryContainer.darker(108).name(),
             fade(c.errorContainer, 0.85),
             fade(c.errorContainer, 1.0));
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
