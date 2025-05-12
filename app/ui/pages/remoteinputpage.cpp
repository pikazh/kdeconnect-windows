#include "remoteinputpage.h"
#include "app_debug.h"
#include "ui_remoteinputpage.h"

#include <QAction>
#include <QCursor>
#include <QKeySequence>

RemoteInputPage::RemoteInputPage(Device::Ptr device, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RemoteInputPage)
    , m_remoteKBPluginWrapper(new RemoteKeyboardPluginWrapper(device, this))
    , m_remoteMousePadPluginWrapper(new RemoteMousePadPluginWrapper(device, this))
{
    m_remoteKBPluginWrapper->init();
    m_remoteMousePadPluginWrapper->init();

    QObject::connect(m_remoteKBPluginWrapper,
                     &RemoteKeyboardPluginWrapper::remoteStateChanged,
                     this,
                     &RemoteInputPage::onPluginRemoteStateChanged);

    QObject::connect(m_remoteMousePadPluginWrapper,
                     &RemoteMousePadPluginWrapper::pluginLoadedChange,
                     this,
                     [this](bool loaded) { lockMouse(false); });

    ui->setupUi(this);
    ui->lockMouseButton->setIcon(QIcon::fromTheme(QStringLiteral("input-mouse")));
    ui->realtimeInputEdit->installEventFilter(this);

    QAction *unlockAct = new QAction(this);
    QObject::connect(unlockAct, &QAction::triggered, this, [this]() { this->lockMouse(false); });
    unlockAct->setShortcut(QKeySequence(Qt::ALT | Qt::Key_X));
    this->addAction(unlockAct);

    uiSetKeyboardControlsVisible(m_remoteKBPluginWrapper->remoteState());
    uiSetRemoteMouseTipsState();
}

RemoteInputPage::~RemoteInputPage()
{
    lockMouse(false);
    delete ui;
}

QIcon RemoteInputPage::icon() const
{
    return QIcon::fromTheme(QStringLiteral("edit-select"));
}

bool RemoteInputPage::apply()
{
    return true;
}

bool RemoteInputPage::shouldDisplay() const
{
    return m_remoteKBPluginWrapper->isPluginLoaded();
}

void RemoteInputPage::retranslate()
{
    ui->retranslateUi(this);
}

void RemoteInputPage::mouseMoveEvent(QMouseEvent *event)
{
    if (m_mouseLocked) {
        event->accept();
        auto movedPos = QCursor::pos() - m_originalMousePos;
        QCursor::setPos(m_originalMousePos);

        //qDebug(KDECONNECT_APP) << "mouse moved. x:" << movedPos.x() << "y:" << movedPos.y();
        m_remoteMousePadPluginWrapper->moveCursor(movedPos);
    }
}

void RemoteInputPage::mousePressEvent(QMouseEvent *event)
{
    if (m_mouseLocked) {
        event->accept();
        //qDebug(KDECONNECT_APP) << "mouse pressed. ";

        auto btns = event->buttons();
        if ((btns & (Qt::LeftButton | Qt::RightButton)) == (Qt::LeftButton | Qt::RightButton)) {
            // unlock mouse
            QMetaObject::invokeMethod(this, "lockMouse", Qt::QueuedConnection, false);
            return;
        }

        auto btn = event->button();
        if (btn & Qt::LeftButton) {
            m_remoteMousePadPluginWrapper->sendSingleHold();
        } else if (btn & Qt::MiddleButton) {
            m_remoteMousePadPluginWrapper->sendMiddleClick();
        } else if (btn & Qt::RightButton) {
            m_remoteMousePadPluginWrapper->sendRightClick();
        }
    }
}

void RemoteInputPage::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_mouseLocked) {
        event->accept();
        //qDebug(KDECONNECT_APP) << "mouse release. ";
        m_remoteMousePadPluginWrapper->sendSingleRelease();
    }
}

void RemoteInputPage::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (m_mouseLocked) {
        event->accept();
        //qDebug(KDECONNECT_APP) << "mouse dbclick. ";
        m_remoteMousePadPluginWrapper->sendSingleHold();
    }
}

void RemoteInputPage::wheelEvent(QWheelEvent *event)
{
    if (m_mouseLocked) {
        event->accept();
        auto wheelPos = event->angleDelta();
        //qDebug(KDECONNECT_APP) << "mouse wheel. x:" << wheelPos.x() << "y:" << wheelPos.y();
        m_remoteMousePadPluginWrapper->sendScroll(wheelPos.x(), wheelPos.y());
    }
}

bool RemoteInputPage::eventFilter(QObject *obj, QEvent *event)
{
    if (obj != nullptr && obj == ui->realtimeInputEdit) {
        if (event->type() == QEvent::KeyPress
            && handleKeyPressedEventForRealTimeInputEdit(static_cast<QKeyEvent *>(event))) {
            return true;

        } else if (event->type() == QEvent::InputMethod
                   && handleInputMethodEventForRealTimeInputEdit(
                       static_cast<QInputMethodEvent *>(event))) {
            return true;
        }
    }

    return QWidget::eventFilter(obj, event);
}

bool RemoteInputPage::handleInputMethodEventForRealTimeInputEdit(QInputMethodEvent *evt)
{
    if (!evt->commitString().isEmpty()) {
        m_remoteKBPluginWrapper->sendQKeyEvent(0, Qt::NoModifier, evt->commitString());
    }

    return false;
}

bool RemoteInputPage::handleKeyPressedEventForRealTimeInputEdit(QKeyEvent *evt)
{
    m_remoteKBPluginWrapper->sendQKeyEvent(evt->key(), evt->modifiers(), evt->text());
    // prevent the input edit from losting focus
    if (evt->key() == Qt::Key_Tab || evt->key() == Qt::Key_Backtab) {
        return true;
    }
    return false;
}

void RemoteInputPage::uiSetKeyboardControlsVisible(bool visible)
{
    ui->remoteKeyboardTipsLabel->setVisible(visible);
    ui->realtimeInputEdit->setVisible(visible);
}

void RemoteInputPage::uiSetRemoteMouseTipsState()
{
    ui->lockMouseButton->setVisible(!m_mouseLocked);
    ui->lockTipsLabel->setVisible(!m_mouseLocked);
    ui->unlockTipsLabel->setVisible(m_mouseLocked);
}

void RemoteInputPage::lockMouse(bool lock)
{
    if (lock != m_mouseLocked) {
        m_mouseLocked = lock;

        if (lock) {
            m_originalMousePos = QCursor::pos();
            QGuiApplication::setOverrideCursor(QCursor(Qt::BlankCursor));
            this->setMouseTracking(true);
        } else {
            QGuiApplication::restoreOverrideCursor();
            this->setMouseTracking(false);
        }

        uiSetRemoteMouseTipsState();
    }
}

void RemoteInputPage::on_lockMouseButton_clicked()
{
    lockMouse(true);
}

void RemoteInputPage::onPluginRemoteStateChanged()
{
    uiSetKeyboardControlsVisible(m_remoteKBPluginWrapper->remoteState());
}
