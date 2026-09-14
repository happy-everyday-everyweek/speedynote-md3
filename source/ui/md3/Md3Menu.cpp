#include "Md3Menu.h"

#include <QMenu>
#include <QVariantAnimation>

#include "Md3Motion.h"
#include "Md3Theme.h"

namespace Md3 {
namespace Menus {

namespace {

QString rgba(const QColor &color)
{
    return QStringLiteral("rgba(%1, %2, %3, %4)")
        .arg(color.red())
        .arg(color.green())
        .arg(color.blue())
        .arg(QString::number(color.alphaF(), 'f', 3));
}

QString styleSheet()
{
    const Md3ColorScheme &c = Theme::instance().colors();

    QColor hover = c.onSurface;
    hover.setAlphaF(0.08);
    QColor pressed = c.onSurface;
    pressed.setAlphaF(0.10);
    QColor disabled = c.onSurface;
    disabled.setAlphaF(0.38);

    return QStringLiteral(
               "QMenu {"
               "  background-color: %1;"
               "  border: none;"
               "  border-radius: 4px;"
               "  padding: 8px 0px;"
               "}"
               "QMenu::item {"
               "  background: transparent;"
               "  color: %2;"
               "  padding: 12px 16px;"
               "  margin: 0px 4px;"
               "  border-radius: 4px;"
               "  font-size: 14px;"
               "}"
               "QMenu::item:selected {"
               "  background-color: %3;"
               "}"
               "QMenu::item:pressed {"
               "  background-color: %4;"
               "}"
               "QMenu::item:disabled {"
               "  color: %5;"
               "}"
               "QMenu::separator {"
               "  height: 1px;"
               "  background: %6;"
               "  margin: 8px 12px;"
               "}"
               "QMenu::icon {"
               "  padding-left: 12px;"
               "}")
        .arg(c.surfaceContainer.name(),
             c.onSurface.name(),
             rgba(hover),
             rgba(pressed),
             rgba(disabled),
             c.outlineVariant.name());
}

} // namespace

void style(QMenu *menu)
{
    if (!menu) {
        return;
    }

    // Frameless + translucent so the rounded corners are not clipped by a
    // native window frame (same approach the app used pre-MD3).
    menu->setWindowFlags(menu->windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    menu->setAttribute(Qt::WA_TranslucentBackground);

    menu->setStyleSheet(styleSheet());
    animateOpen(menu);

    // Re-apply when the theme changes (menus are usually short lived, but
    // long-lived overflow menus should follow a theme switch).
    QObject::connect(&Theme::instance(), &Theme::changed, menu, [menu]() {
        menu->setStyleSheet(styleSheet());
    });
}

void animateOpen(QMenu *menu)
{
    if (!menu) {
        return;
    }

    static const char *kHooked = "md3_menu_animated";
    if (menu->property(kHooked).toBool()) {
        return;
    }
    menu->setProperty(kHooked, true);

    auto *anim = new QVariantAnimation(menu);
    anim->setDuration(Motion::Short2); // 100ms fade
    anim->setEasingCurve(Motion::standardDecelerate());
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    QObject::connect(anim, &QVariantAnimation::valueChanged, menu, [menu](const QVariant &v) {
        menu->setWindowOpacity(v.toReal());
    });

    QObject::connect(menu, &QMenu::aboutToShow, menu, [anim]() {
        anim->stop();
        anim->start();
    });
}

} // namespace Menus
} // namespace Md3