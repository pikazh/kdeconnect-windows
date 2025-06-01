#pragma once

#include <QWidget>

#include "core/device.h"
#include "plugin/remotecommandspluginwrapper.h"
#include "ui/pages/BasePage.h"

namespace Ui {
class RemoteCommandsPage;
}

class RemoteCommandsListModel;

class RemoteCommandsPage : public QWidget, public BasePage
{
    Q_OBJECT
public:
    explicit RemoteCommandsPage(Device::Ptr device, QWidget *parent = nullptr);
    virtual ~RemoteCommandsPage() override;

    virtual QString displayName() const override { return tr("Run Commands"); }
    virtual QIcon icon() const override;
    virtual QString id() const override { return QLatin1StringView("Run Commands"); }
    virtual bool apply() override;
    virtual QString helpPage() const override { return ""; }
    virtual bool shouldDisplay() const override;
    virtual void retranslate() override;

protected Q_SLOTS:
    void on_commandsList_activated(const QModelIndex &index);
    void on_editCommandsButton_clicked();

private:
    Ui::RemoteCommandsPage *ui;
    RemoteCommandsListModel *m_remoteCommandsListModel = nullptr;
    RemoteCommandsPluginWrapper *m_remoteCommandPluginWrapper = nullptr;
};
