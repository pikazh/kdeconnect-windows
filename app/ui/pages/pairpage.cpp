#include "pairpage.h"
#include "ui_pairpage.h"

#include <QFont>
#include <QIcon>
#include <QStyle>

#define RETRIEVE_THEME_ICON(icon_name) QIcon::fromTheme(QStringLiteral(##icon_name##))

PairPage::PairPage(Device::Ptr device, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PairPage)
    , m_device(device)

{
    ui->setupUi(this);
    ui->acceptButton->setIcon(RETRIEVE_THEME_ICON("dialog-ok-apply"));
    ui->rejectButton->setIcon(RETRIEVE_THEME_ICON("dialog-cancel"));
    ui->pairButton->setIcon(RETRIEVE_THEME_ICON("network-connect"));

    auto notPairedTipsFont = ui->notPairedTipsLabel->font();
    notPairedTipsFont.setPointSize(notPairedTipsFont.pointSize() + 2);
    ui->notPairedTipsLabel->setFont(notPairedTipsFont);

    auto pairingTitleFont = ui->pairingTitleLabel->font();
    pairingTitleFont.setPointSize(pairingTitleFont.pointSize() + 2);
    ui->pairingTitleLabel->setFont(pairingTitleFont);
    ui->pairingTitleLabel->setStyleSheet("color: gray");

    auto pairingKeyFont = ui->pairingKeyLabel->font();
    pairingKeyFont.setPointSize(pairingKeyFont.pointSize() + 2);
    ui->pairingKeyLabel->setFont(pairingKeyFont);
    ui->pairingKeyLabel->setStyleSheet("color: gray");

    QObject::connect(m_device.get(),
                     SIGNAL(pairStateChanged(PairState)),
                     this,
                     SLOT(uiResetPairState()));
    uiResetPairState();
}

QIcon PairPage::icon() const
{
    return QIcon::fromTheme(QStringLiteral("network-connect"));
}

bool PairPage::apply()
{
    return true;
}

bool PairPage::shouldDisplay() const
{
    return true;
}

void PairPage::retranslate()
{
    ui->retranslateUi(this);
}

void PairPage::uiSetNotPairedState()
{
    ui->notPairedTipsLabel->show();
    ui->pairButton->show();

    ui->acceptButton->hide();
    ui->rejectButton->hide();

    ui->pairingTitleLabel->hide();
    ui->pairingKeyLabel->hide();
    ui->waitingPeerTipsLabel->hide();
}

void PairPage::uiSetPairingRequestedState()
{
    ui->pairingTitleLabel->show();
    ui->pairingKeyLabel->show();
    auto key = QString(tr("Key: %1")).arg(m_device->verificationKey());
    ui->pairingKeyLabel->setText(key);
    ui->waitingPeerTipsLabel->show();

    ui->acceptButton->hide();
    ui->rejectButton->hide();

    ui->notPairedTipsLabel->hide();
    ui->pairButton->hide();
}

void PairPage::uiSetPairingRequestedByPeerState()
{
    ui->pairingTitleLabel->show();
    ui->pairingKeyLabel->show();
    auto key = QString(tr("Key: %1")).arg(m_device->verificationKey());
    ui->pairingKeyLabel->setText(key);
    ui->waitingPeerTipsLabel->hide();

    ui->acceptButton->show();
    ui->rejectButton->show();

    ui->notPairedTipsLabel->hide();
    ui->pairButton->hide();
}

void PairPage::uiSetPairedState()
{
    ui->notPairedTipsLabel->hide();
    ui->pairButton->hide();

    ui->acceptButton->hide();
    ui->rejectButton->hide();

    ui->pairingTitleLabel->hide();
    ui->pairingKeyLabel->hide();
    ui->waitingPeerTipsLabel->hide();
}

void PairPage::on_pairButton_clicked()
{
    auto pairState = m_device->pairState();
    switch (pairState) {
    case Device::PairState::NotPaired:
        m_device->requestPairing();
        break;
    case Device::PairState::Requested:
        break;
    case Device::PairState::RequestedByPeer:
        break;
    case Device::PairState::Paired:
        break;
    }
}

void PairPage::on_acceptButton_clicked()
{
    auto pairState = m_device->pairState();
    switch (pairState) {
    case Device::PairState::NotPaired:
        break;
    case Device::PairState::Requested:
        break;
    case Device::PairState::RequestedByPeer:
        m_device->acceptPairing();
        break;
    case Device::PairState::Paired:
        break;
    }
}

void PairPage::on_rejectButton_clicked()
{
    auto pairState = m_device->pairState();
    switch (pairState) {
    case Device::PairState::NotPaired:
        break;
    case Device::PairState::Requested:
        break;
    case Device::PairState::RequestedByPeer:
        m_device->cancelPairing();
        break;
    case Device::PairState::Paired:
        break;
    }
}

void PairPage::uiResetPairState()
{
    auto pairState = m_device->pairState();
    switch (pairState) {
    case Device::PairState::NotPaired:
        uiSetNotPairedState();
        break;
    case Device::PairState::Requested:
        uiSetPairingRequestedState();
        break;
    case Device::PairState::RequestedByPeer:
        uiSetPairingRequestedByPeerState();
        break;
    case Device::PairState::Paired:
        uiSetPairedState();
        break;
    }
}
