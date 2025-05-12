#pragma once

#include <QTimer>
#include <QWidget>

#include "device.h"
#include "plugin/mprisremotepluginwrapper.h"
#include "ui/pages/BasePage.h"
#include "ui/widgets/imagewidget.h"

namespace Ui {
class MprisRemotePage;
}

class MprisRemotePage : public QWidget, public BasePage
{
    Q_OBJECT
public:
    explicit MprisRemotePage(Device::Ptr device, QWidget *parent = nullptr);
    virtual ~MprisRemotePage() override;

    virtual QString displayName() const override { return tr("Multimedia control"); }
    virtual QIcon icon() const override;
    virtual QString id() const override { return "Mpris Remote"; }
    virtual bool apply() override;
    virtual QString helpPage() const override { return ""; }
    virtual bool shouldDisplay() const override;
    virtual void retranslate() override;

protected:
    QIcon muteButtonIconFromVolume(int volume);

    void uiUpdateWidgetsState();
    void uiUpdateControlState();
    void uiUpdateTrackInfo();
    void uiUpdatePlayPositionInfo();
    void uiUpdateVolumeInfo();
    void uiUpdatePlayingState();
    void uiUpdatePlayerList();

protected Q_SLOTS:
    void onPluginControlsChanged(QString player);
    void onPluginTrackInfoChanged(QString player);
    void onPluginPositionChanged(QString player);
    void onPluginVolumeChanged(QString player);
    void onPluginPlayingChanged(QString player);
    void onPluginPlayerListChanged();

    void timerUpdatePlayingPosition();

    void on_progressSlider_actionTriggered();
    void on_volumeSlider_actionTriggered();
    void on_muteButton_clicked();
    void on_playButton_clicked();
    void on_previousButton_clicked();
    void on_nextButton_clicked();
    void on_playerListBox_activated(int index);

private:
    Ui::MprisRemotePage *ui;
    ImageWidget *m_albumArtWidget = nullptr;
    MprisRemotePluginWrapper *m_pluginWrapper = nullptr;
    QTimer *m_updatePlayingPosTimer = nullptr;
    int m_lastVolume = 0;
};
