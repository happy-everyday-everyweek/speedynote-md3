#ifndef MD3_ICON_UTILS_H
#define MD3_ICON_UTILS_H

#include <QColor>
#include <QIcon>
#include <QString>

namespace Md3 {

/**
 * Icon helpers for the MD3 UI layer.
 *
 * The app ships monochrome PNG glyphs (plus "_reversed" white variants for
 * dark mode). MD3 components need icons tinted with arbitrary color roles,
 * so these helpers re-color a source icon by painting it and applying a
 * SourceIn color fill.
 */
namespace Icons {

/// Re-color an existing icon for use with any M3 color role.
QIcon tinted(const QIcon &source, const QColor &color, int size = 24);

/// Load a resource path (e.g. ":/resources/icons/pen.png") and tint it.
QIcon tintedFromResource(const QString &path, const QColor &color, int size = 24);

} // namespace Icons
} // namespace Md3

#endif // MD3_ICON_UTILS_H