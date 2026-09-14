#include "Md3IconUtils.h"

#include <QPainter>
#include <QPixmap>

namespace Md3 {
namespace Icons {

QIcon tinted(const QIcon &source, const QColor &color, int size)
{
    if (source.isNull()) {
        return QIcon();
    }

    const QSize target(size, size);
    QPixmap src = source.pixmap(target);
    if (src.isNull()) {
        return QIcon();
    }

    QPixmap out(src.size());
    out.setDevicePixelRatio(src.devicePixelRatio());
    out.fill(Qt::transparent);

    QPainter painter(&out);
    painter.drawPixmap(0, 0, src);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(out.rect(), color);
    painter.end();

    return QIcon(out);
}

QIcon tintedFromResource(const QString &path, const QColor &color, int size)
{
    const QIcon source(path);
    if (source.isNull()) {
        return QIcon();
    }
    return tinted(source, color, size);
}

} // namespace Icons
} // namespace Md3