#include "icons.h"
#include <QIcon>

namespace Icons
{
    void initIcons()
    {
        const QString fallbackTheme = QIcon::fallbackThemeName();
        if (fallbackTheme.isEmpty() || fallbackTheme == QLatin1String("hicolor"))
        {
            QIcon::setFallbackThemeName(QStringLiteral("breeze"));
        }
    }
}
