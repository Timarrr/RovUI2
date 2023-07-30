#include "rovdataparser.h"
#include "helpers.h"
#include "joystick.h"
#include "qcheckbox.h"
#include "qdialog.h"
#include "qnamespace.h"
#include "qtimer.h"
#include "qwidget.h"
#include "ui_dataparser.h"
#include <cmath>
#include <cstddef>
#include <cstring>
#define stringify_please (x) #x

inline float constrain(float val, float min, float max) {
    return val < min ? min : val > max ? max : val;
}

RovDataParser::RovDataParser(QWidget *parent)
    : QDialog{parent}, ui(new Ui::DataParser), m_control(new RovControl()),
      m_controlMutex(), overrideTelemetryUpdate(new QTimer(this)),
      m_auxControl(new RovAuxControl), m_auxControlMutex(),
      depthReg(DEFAULT_kP, DEFAULT_kI, DEFAULT_kD),
      yawReg(DEFAULT_kP, DEFAULT_kI, DEFAULT_kD),
      rollReg(DEFAULT_kP, DEFAULT_kI, DEFAULT_kD),
      pitchReg(DEFAULT_kP, DEFAULT_kI, DEFAULT_kD) {
    ui->setupUi(this);

    QTimer *auxTimer = new QTimer(this);
    connect(auxTimer, &QTimer::timeout, this,
            &RovDataParser::prepareAuxControl);
    auxTimer->start(64);
    connect(overrideTelemetryUpdate, &QTimer::timeout, this, [this]() {
        processTelemetry(QByteArray());
        prepareControl(Joystick());
    });
    connect(ui->overrideTelemetry, &QCheckBox::clicked, this,
            [this](bool state) {
                if (state) {
                    overrideTelemetryUpdate->start(16);
                } else {
                    overrideTelemetryUpdate->stop();
                }
            });
}

void RovDataParser::setAuxFlags(qint8 val) {
    m_auxControlMutex.lock();
    m_auxControl->auxFlags.rawFlags = val;
    m_auxControlMutex.unlock();
}

void RovDataParser::setDepth(double val) { depthReg.setTarget(val); }

void RovDataParser::setDepthStatus(int status) {
    if (status != 0)
        depthReg.enable();
    else
        depthReg.disable();
}

void RovDataParser::setYaw(double val) { yawReg.setTarget(val); }

void RovDataParser::setYawStatus(int status) {
    if (status != 0)
        yawReg.enable();
    else
        yawReg.disable();
}

void RovDataParser::setRoll(double val) { rollReg.setTarget(val); }

void RovDataParser::setRollStatus(int status) {
    if (status != 0)
        rollReg.enable();
    else
        rollReg.disable();
}

void RovDataParser::setPitch(double val) { pitchReg.setTarget(val); }

void RovDataParser::setPitchStatus(int status) {
    if (status != 0)
        pitchReg.enable();
    else
        pitchReg.disable();
}

void RovDataParser::toggleLight() {
    qDebug() << "toggling light" << Qt::endl;
    m_auxControlMutex.lock();
    m_auxControl->auxFlags.eLight = !m_auxControl->auxFlags.eLight;
    m_auxControlMutex.unlock();
}

void RovDataParser::togglePump() {
    qDebug() << "toggling anal pump" << Qt::endl;
    m_auxControlMutex.lock();
    m_auxControl->auxFlags.ePump = !m_auxControl->auxFlags.ePump;
    m_auxControlMutex.unlock();
}

void RovDataParser::invalidateImuCalibration() {
    qDebug() << "decalibrating IMU" << Qt::endl;
    m_auxControlMutex.lock();
    m_auxControl->auxFlags.imuInvCal = 1;
    m_auxControlMutex.unlock();
}

void RovDataParser::prepareAuxControl() {
    QByteArray  ba;
    QDataStream in(&ba, QIODevice::WriteOnly);
    in.setFloatingPointPrecision(QDataStream::SinglePrecision);
    m_auxControlMutex.lock();
    in << m_auxControl->header;
    in << m_auxControl->auxFlags.rawFlags;
    // in << m_auxControl->dDepth;
    // in << m_auxControl->dYaw;
    // in << m_auxControl->dRoll;
    // in << m_auxControl->dPitch;
    m_auxControl->auxFlags.imuInvCal = 0;
    m_auxControlMutex.unlock();
    emit auxControlReady(QByteArray(ba));
}

void RovDataParser::prepareControl(Joystick joy) {
    m_controlMutex.lock();
    // Buttons processing
    m_control->manipulatorOpenClose =
        (joy.buttons.ManipOpen - joy.buttons.ManipClose) * 100;
    m_control->manipulatorRotate =
        (joy.buttons.ManipCCW - joy.buttons.ManipCW) * 100;
    if (m_prevCamSelState == 0 && joy.buttons.CameraSelect != 0)
        m_control->camsel = !m_control->camsel;
    m_prevCamSelState = joy.buttons.CameraSelect;

    // Thrusters processing
    if (ui->overrideControl->checkState() == Qt::Checked) {
        int i = -1;

        m_control->thrusterPower[i++] = ui->thrusterSpinbox1->value();
        m_control->thrusterPower[i++] = ui->thrusterSpinbox2->value();
        m_control->thrusterPower[i++] = ui->thrusterSpinbox3->value();
        m_control->thrusterPower[i++] = ui->thrusterSpinbox4->value();
        m_control->thrusterPower[i++] = ui->thrusterSpinbox5->value();
        m_control->thrusterPower[i++] = ui->thrusterSpinbox6->value();
        m_control->thrusterPower[i++] = ui->thrusterSpinbox7->value();
        m_control->thrusterPower[i++] = ui->thrusterSpinbox8->value();
    } else {
        float x = joy.axes[0] * joy.runtimeASF[0] * joy.baseASF[0] *
                  joy.directions[0] *
                  (m_control->camsel == 1 ? -1 : 1); // left-right
        float y = joy.axes[1] * joy.runtimeASF[1] * joy.baseASF[1] *
                  joy.directions[1] *
                  (m_control->camsel == 1 ? -1 : 1); // forward-backward
        float z = joy.axes[2] * joy.runtimeASF[1] * joy.baseASF[2] *
                  joy.directions[2]; // up-down
        float w = -1 * (joy.axes[3] * joy.runtimeASF[3] * joy.baseASF[3] *
                        joy.directions[3]);
        float d = joy.axes[4] * joy.runtimeASF[4] * joy.baseASF[4] *
                  joy.directions[4];
        float r = joy.axes[5] * joy.runtimeASF[5] * joy.baseASF[5] *
                  joy.directions[5];

        float dReg  = depthReg.eval(m_tele.depth);
        float yReg  = yawReg.eval(m_tele.yaw);
        float rReg  = rollReg.eval(m_tele.roll);
        float pReg  = pitchReg.eval(m_tele.pitch);
        z          += dReg;
        w          += yReg;
        r          += rReg;
        d          += pReg;

        // qDebug() << "dReg: " << dReg << ", yReg: " << yReg << ", rReg: " <<
        // rReg
        //          << ", pReg: " << pReg << "\n";

        // TODO: directions and axes setup
        // Horizontal thrusters
        m_control->thrusterPower[0] =
            constrain(-x - y + z - w - d + r, -100, 100); // lfl
        m_control->thrusterPower[1] =
            constrain(-x + y + z + w - d - r, -100, 100); // lfr
        m_control->thrusterPower[2] =
            constrain(-x - y - z - w + d - r, -100, 100); // hfl
        m_control->thrusterPower[3] =
            constrain(-x + y - z + w + d + r, -100, 100); // hfr
        // Vertical thrusters
        m_control->thrusterPower[4] =
            constrain(-x + y - z - w - d - r, -100, 100); // lbl
        m_control->thrusterPower[5] =
            constrain(-x - y - z + w - d + r, -100, 100); // lbr
        m_control->thrusterPower[6] =
            constrain(-x + y + z - w + d + r, -100, 100); // hbl
        m_control->thrusterPower[7] =
            constrain(-x - y + z + w + d - r, -100, 100); // hbr

        ui->thrusterSpinbox1->setValue(m_control->thrusterPower[0]);
        ui->thrusterSpinbox2->setValue(m_control->thrusterPower[1]);
        ui->thrusterSpinbox3->setValue(m_control->thrusterPower[2]);
        ui->thrusterSpinbox4->setValue(m_control->thrusterPower[3]);
        ui->thrusterSpinbox5->setValue(m_control->thrusterPower[4]);
        ui->thrusterSpinbox6->setValue(m_control->thrusterPower[5]);
        ui->thrusterSpinbox7->setValue(m_control->thrusterPower[6]);
        ui->thrusterSpinbox8->setValue(m_control->thrusterPower[7]);
        ui->progressBarX->setValue(x);
        ui->progressBarY->setValue(y);
        ui->progressBarZ->setValue(z);
        ui->progressBarW->setValue(w);
        ui->progressBarD->setValue(d);
        ui->progressBarR->setValue(r);
    }
    // Hats processing
    m_control->cameraRotationDelta[0] = joy.hats[1];
    m_control->cameraRotationDelta[1] = -joy.hats[0];

    QByteArray ba;

    QDataStream in(&ba, QIODevice::WriteOnly);
    in.setFloatingPointPrecision(QDataStream::SinglePrecision);

    // begin v1
    in << m_control->header;
    in << m_control->version;
    //    in << m_datagram->auxFlags;
    for (int i = 0; i < 10; i++) {
        qint8 t = m_control->thrusterPower[i];
        in << t;
    }
    in << m_control->manipulatorOpenClose;
    in << m_control->manipulatorRotate;
    for (qint8 c : m_control->cameraRotationDelta) {
        in << c;
    }

    in << m_control->camsel;
    m_controlMutex.unlock();
    emit controlReady(QByteArray(ba));
}

void RovDataParser::processTelemetry(QByteArray datagram) {
    RovTelemetry telemetry = RovTelemetry();
    if (ui->overrideTelemetry->checkState() == Qt::Checked) {
        telemetry.current = ui->curSB->value();
        telemetry.voltage = ui->voltSB->value();
        telemetry.depth   = ui->depthSB->value();
        telemetry.yaw     = ui->yawSB->value();
        telemetry.roll    = ui->rollSB->value();
        telemetry.pitch   = ui->pitchSB->value();
        m_tele            = telemetry;
        emit telemetryProcessed(telemetry);
    } else {
        char buffer[datagram.size()];
        memcpy(buffer, datagram.data(), datagram.size());
        size_t i = 0;

        helpers::read_bytes(buffer, i, telemetry.version);
        helpers::read_bytes(buffer, i, telemetry.depth);
        helpers::read_bytes(buffer, i, telemetry.pitch);
        helpers::read_bytes(buffer, i, telemetry.yaw);
        helpers::read_bytes(buffer, i, telemetry.roll);
        helpers::read_bytes(buffer, i, telemetry.current);
        helpers::read_bytes(buffer, i, telemetry.voltage);
        helpers::read_bytes(buffer, i, telemetry.cameraIndex);
        helpers::read_bytes(buffer, i, telemetry.temp);

        QDataStream out(&datagram, QIODevice::ReadOnly);
        out.setFloatingPointPrecision(QDataStream::SinglePrecision);

        qint8 header = 0x00;
        out >> header;

        out >> telemetry.version;
        out >> telemetry.depth;
        out >> telemetry.pitch;
        out >> telemetry.yaw;
        out >> telemetry.roll;
        out >> telemetry.current;
        out >> telemetry.voltage;
        out >> telemetry.cameraIndex;
        out >> telemetry.temp;

        m_tele = telemetry;
        emit telemetryProcessed(telemetry);
        ui->depthSB->setValue(telemetry.depth);
        ui->yawSB->setValue(telemetry.yaw);
        ui->rollSB->setValue(telemetry.roll);
        ui->pitchSB->setValue(telemetry.pitch);
        ui->voltSB->setValue(telemetry.voltage);
        ui->curSB->setValue(telemetry.current);
    }
}

float FPPDRegulator::eval(float data) {
    if (!enabled)
        return 0;
    float err  = target - data;
    float uP   = kP * err; // kP(error)
    uI        += kI * err;
    float uD =
        kD * ((err - lastError) / (float(eTimer.elapsed()))); // kD(de/dt)

    // Logger::trace("Evaluated regualtor value: " + String(uP + uD) +
    //               " (uP: " + String(uP) + ", uD: " + String(uD) +
    //               ", offset: " + String(offset, 10) + ") \n\r");
    lastError = isnan(err) ? 0 : err;
    eTimer.start();
    lastData = data;

    float u = uP + uD + uI;

    return constrain(u, -100, 100);
}

void FPPDRegulator::enable() {
    lastData  = 0;
    lastError = 0;
    uI        = 0;
    eTimer.start();
    enabled = 1;
}
void FPPDRegulator::disable() { enabled = 0; }
void FPPDRegulator::setTarget(float target) { this->target = target; }
