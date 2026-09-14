#ifndef MD3_MENU_H
#define MD3_MENU_H

class QMenu;

namespace Md3 {

/**
 * Material Design 3 menu helpers.
 *
 * Qt menus are still QMenu popups; these helpers restyle them to the MD3
 * menu spec (surface-container container, 4dp corners, 48dp item rhythm,
 * label-large text, 8%/10% state layers) and add the standard fade-in.
 */
namespace Menus {

/// Apply MD3 styling (and the open animation) to a menu. Call once after
/// creating the menu, before adding actions.
void style(QMenu *menu);

/// Just the open animation, for menus that are styled manually.
void animateOpen(QMenu *menu);

} // namespace Menus
} // namespace Md3

#endif // MD3_MENU_H