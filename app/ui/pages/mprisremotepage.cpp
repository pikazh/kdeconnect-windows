#include "mprisremotepage.h"
#include "app_debug.h"
#include "ui_mprisremotepage.h"

#include <QChar>

#define RETRIEVE_THEME_ICON(icon_name) QIcon::fromTheme(QStringLiteral(##icon_name##))

MprisRemotePage::MprisRemotePage(Device::Ptr device, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MprisRemotePage)
    , m_pluginWrapper(new MprisRemotePluginWrapper(device, this))
    , m_updatePlayingPosTimer(new QTimer(this))
{
    m_pluginWrapper->init();
    QObject::connect(m_pluginWrapper,
                     &MprisRemotePluginWrapper::controlsChanged,
                     this,
                     &MprisRemotePage::onPluginControlsChanged);
    QObject::connect(m_pluginWrapper,
                     &MprisRemotePluginWrapper::volumeChanged,
                     this,
                     &MprisRemotePage::onPluginVolumeChanged);
    QObject::connect(m_pluginWrapper,
                     &MprisRemotePluginWrapper::trackInfoChanged,
                     this,
                     &MprisRemotePage::onPluginTrackInfoChanged);
    QObject::connect(m_pluginWrapper,
                     &MprisRemotePluginWrapper::positionChanged,
                     this,
                     &MprisRemotePage::onPluginPositionChanged);
    QObject::connect(m_pluginWrapper,
                     &MprisRemotePluginWrapper::playingChanged,
                     this,
                     &MprisRemotePage::onPluginPlayingChanged);

    QObject::connect(m_pluginWrapper,
                     &MprisRemotePluginWrapper::playerListChanged,
                     this,
                     &MprisRemotePage::onPluginPlayerListChanged);

    QObject::connect(m_pluginWrapper,
                     &MprisRemotePluginWrapper::pluginLoadedChange,
                     this,
                     [this](bool loaded) {
                         if (loaded) {
                             m_pluginWrapper->requestPlayerList();
                         } else {
                             uiUpdateWidgetsState();
                         }
                     });

    m_updatePlayingPosTimer->setInterval(1000);
    m_updatePlayingPosTimer->start();
    QObject::connect(m_updatePlayingPosTimer,
                     SIGNAL(timeout()),
                     this,
                     SLOT(timerUpdatePlayingPosition()));

    ui->setupUi(this);

    m_albumArtWidget = new ImageWidget(this);
    ui->albumArtLayout->addWidget(m_albumArtWidget);

    ui->previousButton->setIcon(RETRIEVE_THEME_ICON("media-skip-backward"));
    ui->nextButton->setIcon(RETRIEVE_THEME_ICON("media-skip-forward"));

    ui->volumeSlider->setRange(0, 100);
    ui->volumeSlider->setSingleStep(5);
    ui->volumeSlider->setPageStep(10);
    ui->volumeSlider->setTracking(false);

    ui->progressSlider->setSingleStep(5);
    ui->progressSlider->setPageStep(10);
    ui->progressSlider->setTracking(false);

    m_pluginWrapper->requestPlayerList();

    uiUpdateWidgetsState();
}

MprisRemotePage::~MprisRemotePage()
{
    delete ui;
}

void MprisRemotePage::uiUpdateWidgetsState()
{
    uiUpdatePlayerList();
    uiUpdateTrackInfo();
    uiUpdateVolumeInfo();
    uiUpdatePlayPositionInfo();
    uiUpdatePlayingState();
    uiUpdateControlState();
}

void MprisRemotePage::uiUpdateControlState()
{
    ui->progressSlider->setEnabled(m_pluginWrapper->canSeek());
}

void MprisRemotePage::uiUpdateTrackInfo()
{
    ui->titleLabel->setText(m_pluginWrapper->title());
    QString artist = m_pluginWrapper->artist();
    QString album = m_pluginWrapper->album();
    if (!artist.isEmpty() && !album.isEmpty()) {
        ui->artistAndAlbumLabel->setText(artist + " - " + album);
    } else if (!artist.isEmpty()) {
        ui->artistAndAlbumLabel->setText(artist);
    } else if (!album.isEmpty()) {
        ui->artistAndAlbumLabel->setText(album);
    } else {
        ui->artistAndAlbumLabel->clear();
    }

    m_albumArtWidget->loadImage(m_pluginWrapper->albumArtData());
    m_albumArtWidget->update();

    int len = m_pluginWrapper->length() / 1000;
    ui->progressSlider->setRange(0, len);
    QChar fillChar('0');
    QString totalProgress = QString(QStringLiteral("%1:%2"))
                                .arg(len / 60, 2, 10, fillChar)
                                .arg(len % 60, 2, 10, fillChar);
    ui->totalProgressLabel->setText(totalProgress);
}

void MprisRemotePage::uiUpdatePlayPositionInfo()
{
    int pos = m_pluginWrapper->position() / 1000;
    ui->progressSlider->setValue(pos);

    auto fillChar = QChar('0');
    QString currentProgress = QString(QStringLiteral("%1:%2"))
                                  .arg(pos / 60, 2, 10, fillChar)
                                  .arg(pos % 60, 2, 10, fillChar);
    ui->currentProgressLabel->setText(currentProgress);
}

void MprisRemotePage::uiUpdateVolumeInfo()
{
    int volume = m_pluginWrapper->volume();
    ui->volumeSlider->setValue(volume);
    QString volumeToolTip = QString(tr("current volume: %1")).arg(volume);
    ui->volumeSlider->setToolTip(volumeToolTip);

    ui->muteButton->setIcon(muteButtonIconFromVolume(volume));
}

void MprisRemotePage::uiUpdatePlayingState()
{
    auto playButtonIcon = m_pluginWrapper->isPlaying()
                              ? RETRIEVE_THEME_ICON("media-playback-pause")
                              : RETRIEVE_THEME_ICON("media-playback-start");
    ui->playButton->setIcon(playButtonIcon);
}

void MprisRemotePage::uiUpdatePlayerList()
{
    auto playerList = m_pluginWrapper->playerList();
    auto currentPlayer = m_pluginWrapper->player();
    ui->playerListBox->clear();
    ui->playerListBox->addItems(playerList);
    ui->playerListBox->setCurrentText(currentPlayer);
}

void MprisRemotePage::onPluginControlsChanged(QString player)
{
    if (ui->playerListBox->currentText() != player) {
        return;
    }

    uiUpdateControlState();
}

void MprisRemotePage::onPluginTrackInfoChanged(QString player)
{
    if (ui->playerListBox->currentText() != player) {
        return;
    }

    uiUpdateTrackInfo();
}

void MprisRemotePage::onPluginPositionChanged(QString player)
{
    if (ui->playerListBox->currentText() != player) {
        return;
    }

    uiUpdatePlayPositionInfo();
}

void MprisRemotePage::onPluginVolumeChanged(QString player)
{
    if (ui->playerListBox->currentText() != player) {
        return;
    }

    uiUpdateVolumeInfo();
}

void MprisRemotePage::onPluginPlayingChanged(QString player)
{
    if (ui->playerListBox->currentText() != player) {
        return;
    }

    uiUpdatePlayingState();
}

void MprisRemotePage::onPluginPlayerListChanged()
{
    //uiUpdatePlayerList();
    uiUpdateWidgetsState();
}

void MprisRemotePage::timerUpdatePlayingPosition()
{
    if (m_pluginWrapper->isPlaying()) {
        int pos = m_pluginWrapper->position() / 1000;
        ui->progressSlider->setValue(pos);

        auto fillChar = QChar('0');
        QString currentProgress = QString(QStringLiteral("%1:%2"))
                                      .arg(pos / 60, 2, 10, fillChar)
                                      .arg(pos % 60, 2, 10, fillChar);
        ui->currentProgressLabel->setText(currentProgress);
    }
}

void MprisRemotePage::on_progressSlider_actionTriggered()
{
    m_pluginWrapper->setPosition(ui->progressSlider->sliderPosition() * 1000);
}

void MprisRemotePage::on_volumeSlider_actionTriggered()
{
    auto pos = ui->volumeSlider->sliderPosition();
    m_pluginWrapper->setVolume(pos);
}

void MprisRemotePage::on_muteButton_clicked()
{
    int volume = m_pluginWrapper->volume();
    if (volume != 0) {
        m_lastVolume = volume;
        m_pluginWrapper->setVolume(0);
    } else {
        m_pluginWrapper->setVolume(m_lastVolume <= 0 ? 50 : m_lastVolume);
    }
}

void MprisRemotePage::on_playButton_clicked()
{
    m_pluginWrapper->sendAction("PlayPause");
}

void MprisRemotePage::on_previousButton_clicked()
{
    m_pluginWrapper->sendAction("Previous");
}

void MprisRemotePage::on_nextButton_clicked()
{
    m_pluginWrapper->sendAction("Next");
}

void MprisRemotePage::on_playerListBox_activated(int index)
{
    auto selectPlayer = ui->playerListBox->itemText(index);
    if (!selectPlayer.isEmpty()) {
        m_pluginWrapper->setPlayer(selectPlayer);
        uiUpdateWidgetsState();
    }
}

QIcon MprisRemotePage::icon() const
{
    return QIcon::fromTheme(QStringLiteral("media-playback-start"));
}

bool MprisRemotePage::apply()
{
    return true;
}

bool MprisRemotePage::shouldDisplay() const
{
    return m_pluginWrapper->isPluginLoaded();
}

void MprisRemotePage::retranslate()
{
    ui->retranslateUi(this);
}

QIcon MprisRemotePage::muteButtonIconFromVolume(int volume)
{
    if (volume <= 0) {
        return RETRIEVE_THEME_ICON("audio-volume-muted");
    } else if (volume <= 25) {
        return RETRIEVE_THEME_ICON("audio-volume-low");
    } else if (volume <= 75) {
        return RETRIEVE_THEME_ICON("audio-volume-medium");
    } else {
        return RETRIEVE_THEME_ICON("audio-volume-high");
    }
}
