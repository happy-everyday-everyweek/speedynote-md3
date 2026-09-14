#ifndef MD3_THEME_H
#define MD3_THEME_H

#include <QColor>
#include <QFont>
#include <QObject>

#include "Md3ColorScheme.h"

class QApplication;
class QPainter;

namespace Md3 {

/**
 * M3 shape scale (corner radii, in logical px).
 */
namespace Shape {
constexpr qreal None = 0.0;
constexpr qreal ExtraSmall = 4.0;
constexpr qreal Small = 8.0;
constexpr qreal Medium = 12.0;
constexpr qreal Large = 16.0;
constexpr qreal ExtraLarge = 28.0;
} // namespace Shape

/**
 * M3 type scale roles.
 */
enum class TypeRole {
    DisplayLarge, DisplayMedium, DisplaySmall,
    HeadlineLarge, HeadlineMedium, HeadlineSmall,
    TitleLarge, TitleMedium, TitleSmall,
    BodyLarge, BodyMedium, BodySmall,
    LabelLarge, LabelMedium, LabelSmall,
};

/**
 * Application-wide M3 theme.
 *
 * Owns the active color scheme (seed-derived, light/dark), the type scale
 * helpers and a couple of painting utilities. Widgets connect to
 * Theme::changed() to repaint / restyle when the theme or system dark
 * mode changes.
 *
 * This class intentionally has no dependency on the rest of the app so it
 * can also be exercised in isolation (gallery / tests).
 */
class Theme : public QObject {
    Q_OBJECT

public:
    static Theme &instance();

    // --- Configuration ---------------------------------------------------

    /// Rebuild the scheme with an explicit configuration.
    void configure(const QColor &seed, bool dark,
                   SchemeVariant variant = SchemeVariant::TonalSpot,
                   double contrastLevel = 0.0);

    void setDarkMode(bool dark);
    void setSeedColor(const QColor &seed);

    bool isDark() const { return m_color.isDark; }
    const Md3ColorScheme &colors() const { return m_color; }
    QColor seedColor() const { return m_seed; }
    SchemeVariant variant() const { return m_variant; }
    double contrastLevel() const { return m_contrast; }

    // --- Typography -------------------------------------------------------

    /// Font for an M3 type-scale role, based on the app font (or `base`).
    QFont font(TypeRole role, const QFont &base = QFont()) const;

    // --- Application integration ------------------------------------------

    /// Apply the scheme to the QApplication palette (Fusion style friendly).
    void applyToApplication(QApplication *app);

    /// Global stylesheet for generic Qt widgets (tool tip, scroll bars).
    QString baseStyleSheet() const;

    /// App-wide stylesheet: base + MD3 styling for common Qt widgets
    /// (inputs, buttons, tabs, sliders, progress, dialogs).
    QString applicationStyleSheet() const;

    // --- Persistence -------------------------------------------------------

    /// Load seed/variant/contrast from QSettings ("SpeedyNote"/"App").
    void load();
    /// Persist the current configuration.
    void save() const;

    // --- Painting utilities ------------------------------------------------

    /// Soft drop shadow approximating an M3 elevation level (0..5).
    static void paintElevation(QPainter &painter, const QRectF &rect,
                               qreal cornerRadius, double level,
                               const QColor &shadowColor);

signals:
    /// Emitted whenever the scheme has been rebuilt.
    void changed();

private:
    Theme();

    void rebuild();

    QColor m_seed;
    SchemeVariant m_variant = SchemeVariant::TonalSpot;
    double m_contrast = 0.0;
    Md3ColorScheme m_color;
};

} // namespace Md3

#endif // MD3_THEME_H
