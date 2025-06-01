#pragma once

#include <QList>
#include <QString>
#include <QTextLayout>
#include <utility>

namespace TextLayoutUtils {
QList<std::pair<qreal, QString>> viewItemTextLayout(QTextLayout &textLayout,
                                                    int lineWidth,
                                                    qreal &height);
}
