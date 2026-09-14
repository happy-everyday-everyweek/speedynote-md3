#include "Md3ColorScheme.h"

// Vendored material-color-utilities (Apache-2.0).
// Include root is thirdparty/material-color-utilities/ so the "cpp/..."
// include prefixes used inside the library resolve.
#include "cpp/cam/hct.h"
#include "cpp/dynamiccolor/material_dynamic_colors.h"
#include "cpp/scheme/scheme_content.h"
#include "cpp/scheme/scheme_expressive.h"
#include "cpp/scheme/scheme_fidelity.h"
#include "cpp/scheme/scheme_fruit_salad.h"
#include "cpp/scheme/scheme_monochrome.h"
#include "cpp/scheme/scheme_neutral.h"
#include "cpp/scheme/scheme_rainbow.h"
#include "cpp/scheme/scheme_tonal_spot.h"
#include "cpp/scheme/scheme_vibrant.h"

namespace Md3 {

using material_color_utilities::Argb;
using material_color_utilities::DynamicScheme;
using material_color_utilities::Hct;
using material_color_utilities::MaterialDynamicColors;

namespace {

inline QColor toColor(Argb argb)
{
    // mc-utilities uses 0xAARRGGBB, same layout as Qt's QRgb.
    return QColor::fromRgba(static_cast<QRgb>(argb));
}

template <typename Scheme>
Md3ColorScheme buildScheme(const Scheme &s)
{
    Md3ColorScheme c;
    c.isDark = s.is_dark;

    c.primary = toColor(MaterialDynamicColors::Primary().GetArgb(s));
    c.onPrimary = toColor(MaterialDynamicColors::OnPrimary().GetArgb(s));
    c.primaryContainer = toColor(MaterialDynamicColors::PrimaryContainer().GetArgb(s));
    c.onPrimaryContainer = toColor(MaterialDynamicColors::OnPrimaryContainer().GetArgb(s));

    c.secondary = toColor(MaterialDynamicColors::Secondary().GetArgb(s));
    c.onSecondary = toColor(MaterialDynamicColors::OnSecondary().GetArgb(s));
    c.secondaryContainer = toColor(MaterialDynamicColors::SecondaryContainer().GetArgb(s));
    c.onSecondaryContainer = toColor(MaterialDynamicColors::OnSecondaryContainer().GetArgb(s));

    c.tertiary = toColor(MaterialDynamicColors::Tertiary().GetArgb(s));
    c.onTertiary = toColor(MaterialDynamicColors::OnTertiary().GetArgb(s));
    c.tertiaryContainer = toColor(MaterialDynamicColors::TertiaryContainer().GetArgb(s));
    c.onTertiaryContainer = toColor(MaterialDynamicColors::OnTertiaryContainer().GetArgb(s));

    c.error = toColor(MaterialDynamicColors::Error().GetArgb(s));
    c.onError = toColor(MaterialDynamicColors::OnError().GetArgb(s));
    c.errorContainer = toColor(MaterialDynamicColors::ErrorContainer().GetArgb(s));
    c.onErrorContainer = toColor(MaterialDynamicColors::OnErrorContainer().GetArgb(s));

    c.surface = toColor(MaterialDynamicColors::Surface().GetArgb(s));
    c.onSurface = toColor(MaterialDynamicColors::OnSurface().GetArgb(s));
    c.surfaceVariant = toColor(MaterialDynamicColors::SurfaceVariant().GetArgb(s));
    c.onSurfaceVariant = toColor(MaterialDynamicColors::OnSurfaceVariant().GetArgb(s));
    c.surfaceDim = toColor(MaterialDynamicColors::SurfaceDim().GetArgb(s));
    c.surfaceBright = toColor(MaterialDynamicColors::SurfaceBright().GetArgb(s));
    c.surfaceContainerLowest = toColor(MaterialDynamicColors::SurfaceContainerLowest().GetArgb(s));
    c.surfaceContainerLow = toColor(MaterialDynamicColors::SurfaceContainerLow().GetArgb(s));
    c.surfaceContainer = toColor(MaterialDynamicColors::SurfaceContainer().GetArgb(s));
    c.surfaceContainerHigh = toColor(MaterialDynamicColors::SurfaceContainerHigh().GetArgb(s));
    c.surfaceContainerHighest = toColor(MaterialDynamicColors::SurfaceContainerHighest().GetArgb(s));

    c.background = toColor(MaterialDynamicColors::Background().GetArgb(s));
    c.onBackground = toColor(MaterialDynamicColors::OnBackground().GetArgb(s));

    c.outline = toColor(MaterialDynamicColors::Outline().GetArgb(s));
    c.outlineVariant = toColor(MaterialDynamicColors::OutlineVariant().GetArgb(s));
    c.shadow = toColor(MaterialDynamicColors::Shadow().GetArgb(s));
    c.scrim = toColor(MaterialDynamicColors::Scrim().GetArgb(s));
    c.inverseSurface = toColor(MaterialDynamicColors::InverseSurface().GetArgb(s));
    c.inverseOnSurface = toColor(MaterialDynamicColors::InverseOnSurface().GetArgb(s));
    c.inversePrimary = toColor(MaterialDynamicColors::InversePrimary().GetArgb(s));

    c.primaryFixed = toColor(MaterialDynamicColors::PrimaryFixed().GetArgb(s));
    c.primaryFixedDim = toColor(MaterialDynamicColors::PrimaryFixedDim().GetArgb(s));
    c.onPrimaryFixed = toColor(MaterialDynamicColors::OnPrimaryFixed().GetArgb(s));
    c.onPrimaryFixedVariant = toColor(MaterialDynamicColors::OnPrimaryFixedVariant().GetArgb(s));
    c.secondaryFixed = toColor(MaterialDynamicColors::SecondaryFixed().GetArgb(s));
    c.secondaryFixedDim = toColor(MaterialDynamicColors::SecondaryFixedDim().GetArgb(s));
    c.onSecondaryFixed = toColor(MaterialDynamicColors::OnSecondaryFixed().GetArgb(s));
    c.onSecondaryFixedVariant = toColor(MaterialDynamicColors::OnSecondaryFixedVariant().GetArgb(s));
    c.tertiaryFixed = toColor(MaterialDynamicColors::TertiaryFixed().GetArgb(s));
    c.tertiaryFixedDim = toColor(MaterialDynamicColors::TertiaryFixedDim().GetArgb(s));
    c.onTertiaryFixed = toColor(MaterialDynamicColors::OnTertiaryFixed().GetArgb(s));
    c.onTertiaryFixedVariant = toColor(MaterialDynamicColors::OnTertiaryFixedVariant().GetArgb(s));

    return c;
}

} // namespace

Md3ColorScheme Md3ColorScheme::fromSeed(const QColor &seed, bool dark, double contrastLevel)
{
    return fromSeedVariant(seed, dark, static_cast<int>(SchemeVariant::TonalSpot), contrastLevel);
}

Md3ColorScheme Md3ColorScheme::fromSeedVariant(const QColor &seed, bool dark,
                                               int variant, double contrastLevel)
{
    const Hct hct(static_cast<Argb>(seed.rgba()));

    switch (variant) {
    case static_cast<int>(SchemeVariant::Vibrant): {
        material_color_utilities::SchemeVibrant s(hct, dark, contrastLevel);
        return buildScheme(s);
    }
    case static_cast<int>(SchemeVariant::Expressive): {
        material_color_utilities::SchemeExpressive s(hct, dark, contrastLevel);
        return buildScheme(s);
    }
    case static_cast<int>(SchemeVariant::Content): {
        material_color_utilities::SchemeContent s(hct, dark, contrastLevel);
        return buildScheme(s);
    }
    case static_cast<int>(SchemeVariant::Fidelity): {
        material_color_utilities::SchemeFidelity s(hct, dark, contrastLevel);
        return buildScheme(s);
    }
    case static_cast<int>(SchemeVariant::Monochrome): {
        material_color_utilities::SchemeMonochrome s(hct, dark, contrastLevel);
        return buildScheme(s);
    }
    case static_cast<int>(SchemeVariant::Neutral): {
        material_color_utilities::SchemeNeutral s(hct, dark, contrastLevel);
        return buildScheme(s);
    }
    case static_cast<int>(SchemeVariant::Rainbow): {
        material_color_utilities::SchemeRainbow s(hct, dark, contrastLevel);
        return buildScheme(s);
    }
    case static_cast<int>(SchemeVariant::FruitSalad): {
        material_color_utilities::SchemeFruitSalad s(hct, dark, contrastLevel);
        return buildScheme(s);
    }
    case static_cast<int>(SchemeVariant::TonalSpot):
    default: {
        material_color_utilities::SchemeTonalSpot s(hct, dark, contrastLevel);
        return buildScheme(s);
    }
    }
}

} // namespace Md3
