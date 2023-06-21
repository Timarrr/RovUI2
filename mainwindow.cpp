#include "mainwindow.h"
#include "filetransmitter.h"
#include "joystick.h"
#include "joystickhandler.h"
#include "qcombobox.h"
#include "qhostinfo.h"
#include "qnamespace.h"
#include "qobjectdefs.h"
#include "qpushbutton.h"
#include "qthread.h"
#include "rovcameracapture.h"
#include "ui_mainwindow2.h"
#include "unistd.h"
#include <cstddef>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
      m_cameraCapture(new RovCameraCapture(this)),
      m_tsd(new ThrusterSetupDialog(this)),
      m_joystickHandler(new JoystickHandler(this)),
      m_jsd(new JoystickSetupDialog(this)),
      m_rovCommunication(new RovCommunication(this)),
      m_rovDataParser(new RovDataParser(this)),
      m_datasplines(new RovDataSplines(this)),
      m_rovStatusLabel(new QLabel(this)),
      m_rovStatusIndicator(new LEDIndicator(this)),
      m_filetransmitter(new FileTransmitter()) {

    ui->setupUi(this);
    setWindowTitle("RovUI2 v0.95");
    ui->viewfinder->show();
    m_cameraCapture->setViewfinder(ui->viewfinder);

    setupStatusbar();
    updateStatusbarText("Initialization...");

    createConnections();
    updateTelemetry(RovTelemetry());

    QTimer *statusbarIndicatorsTimer = new QTimer(this);
    connect(statusbarIndicatorsTimer, &QTimer::timeout, this,
            &MainWindow::updateStatusbarIndicators);

    updateStatusbarText("RovUI2 initialization complete ^_^");
    updateStatusbarProgress(100);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::resizeCameraLabel() {
    //    int targetWidth  =
    //    ui->camLabel->heightForWidth(ui->camLabel->width()); int targetHeight
    //    = ui->camLabel->widthForHeight(ui->camLabel->height()); if
    //    (targetHeight >
    //        ui->centralwidget->height() -
    // qreal rate            12) { // if wanted height is larger than the
    // available height
    //        targetHeight =
    //            ui->camLabel->heightForWidth(ui->centralwidget->width() - 12);
    //        targetWidth = ui->camLabel->widthForHeight(targetHeight);
    //        ui->camLabel->setFixedSize(QSize(targetWidth, targetHeight));
    //    }
    //    if (targetWidth >
    //        ui->centralwidget->width() -
    //            12) { // if wanted width is larger than the available width
    //        targetWidth =
    //            ui->camLabel->widthForHeight(ui->centralwidget->height() -
    //            12);
    //        targetHeight = ui->camLabel->heightForWidth(targetWidth);
    //        ui->camLabel->setFixedSize(QSize(targetWidth, targetHeight));
    //    }
    //    ui->camLabel->setFixedSize(QSize(targetWidth, targetHeight));
}

void MainWindow::updateStatusbar() {
    //    ui->camLabel->setPixmap(QPixmap::fromImage(img));
    updateStatusbarText(m_cameraCapture->getRecordInfo());
}

void MainWindow::updateDatasplines(RovTelemetry tele) {
    m_datasplines.data()->addVoltageSample(QPointF(m_vSamples++, tele.voltage));
    m_datasplines.data()->addCurrentSample(QPointF(m_aSamples++, tele.current));
}

void MainWindow::updateStatusbarText(QString status) {
    ui->statusbar->showMessage(status, 0);
}

void MainWindow::updateStatusbarProgress(int progress) {
    QProgressBar *pb = new QProgressBar(ui->statusbar);
    pb->setValue(progress);
    ui->statusbar->addWidget(pb);
}

void MainWindow::updateStatusbarIndicators() {}

void MainWindow::setDesiredDepth(float val) { ui->depthSpinBox->setValue(val); }
void MainWindow::setDesiredYaw(float val) { ui->yawSpinBox->setValue(val); }
void MainWindow::setDesiredRoll(float val) { ui->rollSpinBox->setValue(val); }
void MainWindow::setDesiredPitch(float val) { ui->pitchSpinBox->setValue(val); }

void MainWindow::overrideDepth(bool readonly) {
    ui->depthSpinBox->setReadOnly(readonly);
    ui->depthSpinBox->setEnabled(!readonly);
    ui->depthRegulatorCB->setCheckable(!readonly);
    ui->depthRegulatorCB->setEnabled(!readonly);
}
void MainWindow::overrideYaw(bool readonly) {
    ui->yawSpinBox->setReadOnly(readonly);
    ui->yawSpinBox->setEnabled(!readonly);
    ui->yawRegulatorCB->setCheckable(!readonly);
    ui->yawRegulatorCB->setEnabled(!readonly);
}
void MainWindow::overrideRoll(bool readonly) {
    ui->rollSpinBox->setReadOnly(readonly);
    ui->rollSpinBox->setEnabled(!readonly);
    ui->rollRegulatorCB->setCheckable(!readonly);
    ui->rollRegulatorCB->setEnabled(!readonly);
}
void MainWindow::overridePitch(bool readonly) {
    ui->pitchSpinBox->setReadOnly(readonly);
    ui->pitchSpinBox->setEnabled(!readonly);
    ui->pitchRegulatorCB->setCheckable(!readonly);
    ui->pitchRegulatorCB->setEnabled(!readonly);
}

void MainWindow::updateTelemetry(RovTelemetry tele) {
    ui->teleVersionLabel->setText(QString::number(tele.version));
    if (tele.depth < 3.4E+38)
        ui->teleDepthLabel->setText(QString::number(tele.depth, 'g', 2) +
                                    QString(lastDepth < tele.depth   ? " m ↑"
                                            : lastDepth > tele.depth ? " m ↓"
                                                                     : " m ="));
    else
        ui->teleDepthLabel->setText(QString("[DIS]"));
    lastDepth = tele.depth;
    ui->telePitchLabel->setText(QString::number(tele.pitch, 'f', 2) + " deg");
    ui->teleYawLabel->setText(QString::number(tele.yaw, 'f', 2) + " deg");
    ui->teleRollLabel->setText(QString::number(tele.roll, 'f', 2) + " deg");
    ui->teleTempLabel->setText(
        tele.depth < 3.4E+38 ? QString::number(tele.temp / 100, 'f', 2) + " °C"
                             : "[DIS]");
    ui->teleVoltageLabel->setText(QString::number(tele.voltage, 'f', 2) + " V");
    ui->teleCurrentLabel->setText(
        QString::number(std::min(tele.current * 1000, 25000.0f), 'f', 2) +
        " mA");
    ui->teleCamSelLabel->setText(
        QString((tele.cameraIndex == 0 ? "Front" : "Rear")));
    lastTele = tele;
}

void MainWindow::updateASF(float factor) { emit asfUpdated(factor); }

void MainWindow::createConnections() {
    // Camera capture
    //     connect(m_cameraCapture.data(), SIGNAL(imgProcessed(QImage)), this,
    //             SLOT(updateCameraLabel()));

    // Show setup dialogs
    connect(ui->actionDisplay_thruster_setup_dialog, &QAction::triggered, this,
            [this] { m_tsd->show(); });
    connect(ui->actionDisplay_joystick_setup_dialog, &QAction::triggered, this,
            [this] { m_jsd->show(); });

    // Show graphs
    connect(ui->actionShow_graphs, &QAction::triggered, this,
            [this] { m_datasplines->show(); });
    connect(ui->actionHide_graphs, &QAction::triggered, this,
            [this] { m_datasplines->hide(); });

    // JoystickHandler to:
    // JSD
    connect(m_joystickHandler.data(), SIGNAL(joystickUpdated(Joystick)),
            m_jsd.data(), SLOT(updateUi(Joystick)));
    connect(m_joystickHandler.data(), SIGNAL(joystickChanged(Joystick)),
            m_jsd.data(), SLOT(populateUi(Joystick)));
    // DataParser
    connect(m_joystickHandler.data(), SIGNAL(joystickUpdated(Joystick)),
            m_rovDataParser.data(), SLOT(prepareControl(Joystick)));

    // UI ASF
    connect(m_joystickHandler.data(), &JoystickHandler::joystickUpdated, this,
            [this](Joystick joy) {
                ui->asfLabel->setText(QString::number(joy.runtimeASF[0]));
            });

    // Settings update requests
    connect(m_jsd.data(), SIGNAL(settingsUpdated()), m_joystickHandler.data(),
            SLOT(updateSettings()));

    // Regulators:
    // Depth
    connect(ui->depthSpinBox, SIGNAL(valueChanged(double)),
            m_rovDataParser.data(), SLOT(setDepth(double)));
    connect(ui->depthRegulatorCB, SIGNAL(stateChanged(int)),
            m_rovDataParser.data(), SLOT(setDepthStatus(int)));

    // Yaw
    connect(ui->yawSpinBox, SIGNAL(valueChanged(double)),
            m_rovDataParser.data(), SLOT(setYaw(double)));
    connect(ui->yawRegulatorCB, SIGNAL(stateChanged(int)),
            m_rovDataParser.data(), SLOT(setYawStatus(int)));

    // Roll
    connect(ui->rollSpinBox, SIGNAL(valueChanged(double)),
            m_rovDataParser.data(), SLOT(setRoll(double)));
    connect(ui->rollRegulatorCB, SIGNAL(stateChanged(int)),
            m_rovDataParser.data(), SLOT(setRollStatus(int)));
    // Pitch
    connect(ui->pitchSpinBox, SIGNAL(valueChanged(double)),
            m_rovDataParser.data(), SLOT(setPitch(double)));
    connect(ui->pitchRegulatorCB, SIGNAL(stateChanged(int)),
            m_rovDataParser.data(), SLOT(setPitchStatus(int)));

    // DataParser:
    // Send AuxControl to ROV
    connect(m_rovDataParser.data(), SIGNAL(auxControlReady(QByteArray)),
            m_rovCommunication.data(), SLOT(write(QByteArray)));

    // Send RovControl to ROV
    connect(m_rovDataParser.data(), SIGNAL(controlReady(QByteArray)),
            m_rovCommunication.data(), SLOT(write(QByteArray)));

    // Process recieved RovTelemetry from ROV
    connect(m_rovCommunication.data(), SIGNAL(telemetryReady(QByteArray)),
            m_rovDataParser.data(), SLOT(processTelemetry(QByteArray)));

    // Update UI with telemetry data
    connect(m_rovDataParser.data(), SIGNAL(telemetryProcessed(RovTelemetry)),
            this, SLOT(updateTelemetry(RovTelemetry)));

    // Update DataSplines with telemetry data
    connect(m_rovDataParser.data(), SIGNAL(telemetryProcessed(RovTelemetry)),
            this, SLOT(updateDatasplines(RovTelemetry)));

    // TSD to DataParser
    connect(m_tsd.data(), SIGNAL(overrideStatusChanged(bool)),
            m_rovDataParser.data(), SLOT(enableThrustersOverride(bool)));
    connect(m_tsd.data(), SIGNAL(thrustersOverridden(QList<qint8>)),
            m_rovDataParser.data(), SLOT(setThrustersOverride(QList<qint8>)));
    connect(m_tsd.data(), SIGNAL(overrideStatusChanged(bool)),
            m_rovDataParser.data(), SLOT(enableThrustersOverride(bool)));

    // Files Send/Recieve
    connect(ui->startSeriesButton, &QPushButton::clicked, this, [this]() {
        m_cameraCapture->startRecord();
        ui->sendScreenShotButton->setEnabled(false);
        ui->startSeriesButton->setEnabled(false);
        ui->pauseSeriesButton->setEnabled(true);
        ui->sendSeriesButton->setEnabled(true);
    });
    connect(ui->pauseSeriesButton, &QPushButton::clicked, this, [this]() {
        m_cameraCapture->pauseRecord();
        ui->sendScreenShotButton->setEnabled(false);
        ui->startSeriesButton->setEnabled(true);
        ui->pauseSeriesButton->setEnabled(false);
        ui->sendSeriesButton->setEnabled(true);
    });
    connect(ui->sendSeriesButton, &QPushButton::clicked, this, [this]() {
        m_cameraCapture->stopRecord();
        ui->sendScreenShotButton->setEnabled(true);
        ui->startSeriesButton->setEnabled(true);
        ui->pauseSeriesButton->setEnabled(false);
        ui->sendSeriesButton->setEnabled(false);
    });
    connect(ui->sendScreenShotButton, SIGNAL(clicked()), m_cameraCapture.data(),
            SLOT(screenshot()));

    connect(
        m_cameraCapture.data(), &RovCameraCapture::recordingReady, this,
        [this](QString path) {
            m_filetransmitter->setPath(path);
            m_filetransmitter->setHost(
                m_filetransmitter->clients[ui->hostsComboBox->currentIndex()]);
            m_filetransmitter->sendFile();
        });
    connect(
        m_cameraCapture.data(), &RovCameraCapture::screenshotReady, this,
        [this](QString path) {
            if (m_filetransmitter->clients.size() <= 0)
                return;
            m_filetransmitter->setPath(path);
            m_filetransmitter->setHost(
                m_filetransmitter->clients[ui->hostsComboBox->currentIndex()]);
            m_filetransmitter->sendFile();
        });

    connect(
        m_filetransmitter.data(), &FileTransmitter::newHost, this,
        [this](qint64 index) {
            qDebug() << "Adding host "
                     << m_filetransmitter->clients[index].socket->peerAddress();
            ui->hostsComboBox->addItem(m_filetransmitter->clients[index]
                                           .socket->peerAddress()
                                           .toString());
        });

    connect(m_filetransmitter.data(), &FileTransmitter::removeHost, this,
            [this](qint64 index) {
                qDebug() << "Removing host ";
                ui->hostsComboBox->removeItem(index);
            });

    // Menu actions
    // Light on/off
    connect(ui->actionLight_on_off, SIGNAL(triggered(bool)),
            m_rovDataParser.data(), SLOT(toggleLight()));
    // Pump on/off
    connect(ui->actionPump_off_on, SIGNAL(triggered(bool)),
            m_rovDataParser.data(), SLOT(togglePump()));

    // Camera start/stop
    connect(ui->actionBegin_camera_capture, &QAction::triggered, this,
            [this] { m_cameraCapture.data()->startCapture(); });
    connect(ui->actionEnd_camera_capture, &QAction::triggered, this,
            [this] { m_cameraCapture.data()->stopCapture(); });
    // Regulators:
    // Depth
    connect(ui->actionDepthOff, &QAction::triggered, this, [this](bool) {
        ui->depthRegulatorCB->setCheckState(Qt::Unchecked);
    });
    connect(ui->actionDepthOn, &QAction::triggered, this,
            [this](bool) { ui->depthRegulatorCB->setCheckState(Qt::Checked); });
    connect(ui->actionDepthOnWC, &QAction::triggered, this, [this](bool) {
        ui->depthSpinBox->setValue(lastTele.depth);
        ui->depthRegulatorCB->setCheckState(Qt::Checked);
    });
    // Yaw
    connect(ui->actionYawOff, &QAction::triggered, this,
            [this](bool) { ui->yawRegulatorCB->setCheckState(Qt::Unchecked); });
    connect(ui->actionYawOn, &QAction::triggered, this,
            [this](bool) { ui->yawRegulatorCB->setCheckState(Qt::Checked); });
    connect(ui->actionYawOnWC, &QAction::triggered, this, [this](bool) {
        ui->yawSpinBox->setValue(lastTele.yaw);
        ui->yawRegulatorCB->setCheckState(Qt::Checked);
    });
    // Roll
    connect(ui->actionRollOff, &QAction::triggered, this, [this](bool) {
        ui->rollRegulatorCB->setCheckState(Qt::Unchecked);
    });
    connect(ui->actionRollOn, &QAction::triggered, this,
            [this](bool) { ui->rollRegulatorCB->setCheckState(Qt::Checked); });
    connect(ui->actionRollOnWC, &QAction::triggered, this, [this](bool) {
        ui->rollSpinBox->setValue(lastTele.roll);
        ui->rollRegulatorCB->setCheckState(Qt::Checked);
    });
    // Pitch
    connect(ui->actionPitchOff, &QAction::triggered, this, [this](bool) {
        ui->pitchRegulatorCB->setCheckState(Qt::Unchecked);
    });
    connect(ui->actionPitchOn, &QAction::triggered, this,
            [this](bool) { ui->pitchRegulatorCB->setCheckState(Qt::Checked); });
    connect(ui->actionPitchOnWC, &QAction::triggered, this, [this](bool) {
        ui->pitchSpinBox->setValue(lastTele.pitch);
        ui->pitchRegulatorCB->setCheckState(Qt::Checked);
    });
}

void MainWindow::setupStatusbar() {
    m_rovStatusLabel->setText(" ROV status ");
    ui->statusbar->addPermanentWidget(m_rovStatusLabel.data());
    ui->statusbar->addPermanentWidget(m_rovStatusIndicator.data());
}
