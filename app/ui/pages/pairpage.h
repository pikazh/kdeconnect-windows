#pragma once

#include <QWidget>

#include "device.h"
#include "ui/pages/BasePage.h"

namespace Ui {
class PairPage;
}

class PairPage : public QWidget, public BasePage
{
    Q_OBJECT
public:
    explicit PairPage(Device::Ptr device, QWidget *parent = nullptr);
    virtual ~PairPage() override;

    virtual QString displayName() const override { return tr("Device Pair"); }
    virtual QIcon icon() const override;
    virtual QString id() const override { return "Device Pair"; }
    virtual bool apply() override;
    virtual QString helpPage() const override { return ""; }
    virtual bool shouldDisplay() const override;
    virtual void retranslate() override;

protected:
    void uiSetNotPairedState();
    void uiSetPairingRequestedState();
    void uiSetPairingRequestedByPeerState();
    void uiSetPairedState();

protected Q_SLOTS:
    void uiResetPairState();

    void on_pairButton_clicked();
    void on_acceptButton_clicked();
    void on_rejectButton_clicked();

private:
    Ui::PairPage *ui;
    Device::Ptr m_device;
};
