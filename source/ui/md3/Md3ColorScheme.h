#ifndef MD3_COLOR_SCHEME_H
#define MD3_COLOR_SCHEME_H

#include <QColor>

namespace Md3 {

/**
 * Material Design 3 color scheme variants.
 *
 * TonalSpot is the M3 default for user-seeded schemes; the others follow
 * the "dynamic color" variants offered by Android 12+ (Material You).
 */
enum class SchemeVariant {
    TonalSpot = 0,
    Vibrant,
    Expressive,
    Content,
    Fidelity,
    Monochrome,
    Neutral,
    Rainbow,
    FruitSalad,
};

/**
 * The full set of M3 color roles for one scheme (light or dark).
 *
 * Values are generated from a seed color through the color utilities
 * library (HCT / tonal palettes), matching what Android produces for
 * Material You, but usable in a Qt application.
 */
struct Md3ColorScheme {
    // Primary
    QColor primary;
    QColor onPrimary;
    QColor primaryContainer;
    QColor onPrimaryContainer;

    // Secondary
    QColor secondary;
    QColor onSecondary;
    QColor secondaryContainer;
    QColor onSecondaryContainer;

    // Tertiary
    QColor tertiary;
    QColor onTertiary;
    QColor tertiaryContainer;
    QColor onTertiaryContainer;

    // Error
    QColor error;
    QColor onError;
    QColor errorContainer;
    QColor onErrorContainer;

    // Surfaces
    QColor surface;
    QColor onSurface;
    QColor surfaceVariant;
    QColor onSurfaceVariant;
    QColor surfaceDim;
    QColor surfaceBright;
    QColor surfaceContainerLowest;
    QColor surfaceContainerLow;
    QColor surfaceContainer;
    QColor surfaceContainerHigh;
    QColor surfaceContainerHighest;

    // Background (legacy roles, still referenced by older components)
    QColor background;
    QColor onBackground;

    // Utility
    QColor outline;
    QColor outlineVariant;
    QColor shadow;
    QColor scrim;
    QColor inverseSurface;
    QColor inverseOnSurface;
    QColor inversePrimary;

    // Fixed accent roles
    QColor primaryFixed;
    QColor primaryFixedDim;
    QColor onPrimaryFixed;
    QColor onPrimaryFixedVariant;
    QColor secondaryFixed;
    QColor secondaryFixedDim;
    QColor onSecondaryFixed;
    QColor onSecondaryFixedVariant;
    QColor tertiaryFixed;
    QColor tertiaryFixedDim;
    QColor onTertiaryFixed;
    QColor onTertiaryFixedVariant;

    bool isDark = false;

    /**
     * Build a scheme from a seed color.
     * @param seed           Seed color (alpha ignored).
     * @param dark           true for the dark scheme.
     * @param contrastLevel  -1.0 .. 1.0 (0 = standard, 1 = high contrast).
     */
    static Md3ColorScheme fromSeed(const QColor &seed, bool dark,
                                   double contrastLevel = 0.0);

    /**
     * Build a scheme for a specific dynamic-color variant.
     */
    static Md3ColorScheme fromSeedVariant(const QColor &seed, bool dark,
                                          int variant, double contrastLevel = 0.0);
};

} // namespace Md3

#endif // MD3_COLOR_SCHEME_H
