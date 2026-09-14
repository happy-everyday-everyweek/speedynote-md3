#ifndef MD3_MOTION_H
#define MD3_MOTION_H

#include <QtCore/QEasingCurve>
#include <QtCore/QPointF>
#include <QtGlobal>

/**
 * Material Design 3 motion tokens.
 *
 * Durations and easing curves follow the M3 motion specification:
 *  - Duration tokens: short/medium/long/extra-long (1..4) plus the
 *    expressive spring-based effect/spatial sets.
 *  - Easing tokens: standard, standard-decelerate, standard-accelerate,
 *    emphasized, emphasized-decelerate, emphasized-accelerate, linear.
 *
 * All durations are in milliseconds and are meant to be used verbatim
 * with QPropertyAnimation / QVariantAnimation.
 */
namespace Md3 {
namespace Motion {

// ---------------------------------------------------------------------------
// Duration tokens (ms)
// ---------------------------------------------------------------------------

// Common set (used for component state changes, fades, small moves).
constexpr int Short1 = 50;
constexpr int Short2 = 100;
constexpr int Short3 = 150;
constexpr int Short4 = 200;

// Medium set (used for small expansions, icon switches).
constexpr int Medium1 = 250;
constexpr int Medium2 = 300;
constexpr int Medium3 = 350;
constexpr int Medium4 = 400;

// Long set (used for large expansions, sheet enters).
constexpr int Long1 = 450;
constexpr int Long2 = 500;
constexpr int Long3 = 550;
constexpr int Long4 = 600;

// Extra-long set (used for ambient / background transitions).
constexpr int ExtraLong1 = 700;
constexpr int ExtraLong2 = 800;
constexpr int ExtraLong3 = 900;
constexpr int ExtraLong4 = 1000;

// Expressive (spring-based) token set.
constexpr int SpringFastEffects = 150;
constexpr int SpringDefaultEffects = 200;
constexpr int SpringSlowEffects = 300;
constexpr int SpringFastSpatial = 350;
constexpr int SpringDefaultSpatial = 500;
constexpr int SpringSlowSpatial = 750;

// ---------------------------------------------------------------------------
// Easing curve factories (cubic-bezier control points from the M3 spec)
// ---------------------------------------------------------------------------

inline QEasingCurve bezier(qreal x1, qreal y1, qreal x2, qreal y2)
{
    QEasingCurve curve(QEasingCurve::BezierSpline);
    curve.addCubicBezierSegment(QPointF(x1, y1), QPointF(x2, y2), QPointF(1.0, 1.0));
    return curve;
}

inline QEasingCurve linear() { return QEasingCurve(QEasingCurve::Linear); }

// M3 standard set.
inline QEasingCurve standard()           { return bezier(0.2, 0.0, 0.0, 1.0); }
inline QEasingCurve standardDecelerate() { return bezier(0.0, 0.0, 0.0, 1.0); }
inline QEasingCurve standardAccelerate() { return bezier(0.3, 0.0, 1.0, 1.0); }

// M3 emphasized set.
inline QEasingCurve emphasized()           { return bezier(0.2, 0.0, 0.0, 1.0); }
inline QEasingCurve emphasizedDecelerate() { return bezier(0.05, 0.7, 0.1, 1.0); }
inline QEasingCurve emphasizedAccelerate() { return bezier(0.3, 0.0, 0.8, 0.15); }

// Legacy M3 easing names kept for completeness.
inline QEasingCurve decelerate() { return bezier(0.0, 0.0, 0.2, 1.0); }
inline QEasingCurve accelerate() { return bezier(0.4, 0.0, 1.0, 1.0); }

} // namespace Motion
} // namespace Md3

#endif // MD3_MOTION_H
