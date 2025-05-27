#pragma once

#include "kdeconnectcore_export.h"

#include <QPixmap>

namespace ImageUtil {
KDECONNECTCORE_EXPORT QPixmap combineIcon(const QList<QPixmap> &images);
}
