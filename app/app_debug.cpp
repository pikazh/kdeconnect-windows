#include "app_debug.h"

#ifndef QT_NO_DEBUG
Q_LOGGING_CATEGORY(KDECONNECT_APP, "kdeconnect.app")
#else
Q_LOGGING_CATEGORY(KDECONNECT_APP, "kdeconnect.app", QtWarningMsg)
#endif
