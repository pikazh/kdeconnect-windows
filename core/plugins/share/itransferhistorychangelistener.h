#pragma once

#include "transferhistoryrecord.h"

#include <QVariantHash>
#include <QtTypes>

class ITransferHistoryChangeListener
{
public:
    virtual void onAdded(qint64 id, const TransferHistoryRecord &r) = 0;
    virtual void onRemoved(qint64 id) = 0;
    virtual void onClear() = 0;
};
