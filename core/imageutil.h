#pragma once

#include "kdeconnectcore_export.h"

#include <QPixmap>

namespace ImageUtil {
KDECONNECTCORE_EXPORT QPixmap combineImage(const QList<QPixmap> &images);
}
