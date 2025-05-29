#pragma once

#include <QDialog>

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

protected Q_SLOTS:
    void saveConfig();

private:
    Ui::AppSettingsDialog *ui;
};
