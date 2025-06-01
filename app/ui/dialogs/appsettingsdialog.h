#pragma once

#include <QDialog>
#include <QEvent>

namespace Ui {
class AppSettingsDialog;
}

class AppSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AppSettingsDialog(QWidget *parent = nullptr);
    virtual ~AppSettingsDialog() override;

protected:
    void loadConfig();
    virtual void changeEvent(QEvent *evt) override;

protected Q_SLOTS:
    void saveConfig();

private:
    Ui::AppSettingsDialog *ui;
};
