#include "rovdataparser.h"
#include "helpers.h"
#include "qdatetime.h"
#include "qtimer.h"

inline float constrain(float val, float min, float max) {
    return val < min ? min : val > max ? max : val;
}

RovDataParser::RovDataParser(QObject *parent)
    : QObject{parent}, m_thrOvrMutex(), m_thrOvrInvMutex(), m_thrOvrEnable(),
      m_thrOvrEnableMutex(), m_control(new RovControl()), m_controlMutex(),
      m_auxControl(new RovAuxControl), m_auxControlMutex() {
    QTimer *auxTimer = new QTimer(this);
    connect(auxTimer, &QTimer::timeout, this,
            &RovDataParser::prepareAuxControl);
    auxTimer->start(64);
}

void RovDataParser::enableThrustersOverride(bool state) {
    m_thrOvrEnableMutex.lock();
    m_thrOvrEnable = state;
    m_thrOvrEnableMutex.unlock();
}

void RovDataParser::setThrustersOverride(QList<qint8> override) {
    m_thrOvrMutex.lock();
    for (int i = 0; i < override.size(); i++) {

        qDebug() << override[i];
        this->m_thrOvr[i] = override.at(i);
    }
    m_thrOvrMutex.unlock();
}

void RovDataParser::setThrustersOverrideInvert(qint8 override) {
    m_thrOvrInvMutex.lock();
    this->m_thrOvrInv = override;
    m_thrOvrInvMutex.unlock();
}

void RovDataParser::setAuxFlags(qint8 val) {
    m_auxControlMutex.lock();
    m_auxControl->auxFlags.rawFlags = val;
    m_auxControlMutex.unlock();
}

void RovDataParser::setDepth(double val) {
    m_auxControlMutex.lock();
    m_auxControl->dDepth = val;
    m_auxControlMutex.unlock();
}

void RovDataParser::setDepthStatus(int status) {
    qDebug() << "Setting depth to " << status << Qt::endl;
    m_auxControlMutex.lock();
    m_auxControl->auxFlags.eDepth = status != 0;
    m_auxControlMutex.unlock();
}

void RovDataParser::setYaw(double val) {
    qDebug() << "Setting yaw to " << val << Qt::endl;
    m_auxControlMutex.lock();
    m_auxControl->dYaw = val;
    m_auxControlMutex.unlock();
}

void RovDataParser::setYawStatus(int status) {
    m_auxControlMutex.lock();
    m_auxControl->auxFlags.eYaw = status != 0;
    m_auxControlMutex.unlock();
}

void RovDataParser::setRoll(double val) {
    m_auxControlMutex.lock();
    m_auxControl->dRoll = val;
    m_auxControlMutex.unlock();
}

void RovDataParser::setRollStatus(int status) {
    qDebug() << "Setting roll to " << status << Qt::endl;
    m_auxControlMutex.lock();
    m_auxControl->auxFlags.eRoll = status != 0;
    m_auxControlMutex.unlock();
}

void RovDataParser::setPitch(double val) {
    m_auxControlMutex.lock();
    m_auxControl->dPitch = val;
    m_auxControlMutex.unlock();
}

void RovDataParser::setPitchStatus(int status) {
    qDebug() << "Setting pitch to " << status << Qt::endl;
    m_auxControlMutex.lock();
    m_auxControl->auxFlags.ePitch = status != 0;
    m_auxControlMutex.unlock();
}

void RovDataParser::prepareAuxControl() {
    QByteArray ba;
    QDataStream in(&ba, QIODevice::WriteOnly);
    in.setFloatingPointPrecision(QDataStream::SinglePrecision);
    m_auxControlMutex.lock();
    in << m_auxControl->header;
    in << m_auxControl->auxFlags.rawFlags;
    in << m_auxControl->dDepth;
    in << m_auxControl->dYaw;
    in << m_auxControl->dRoll;
    in << m_auxControl->dPitch;
    m_auxControlMutex.unlock();
    emit auxControlReady(QByteArray(ba));
}

void RovDataParser::prepareControl(Joystick rc) {
    m_thrOvrEnableMutex.lock();
    bool state = m_thrOvrEnable;
    m_thrOvrEnableMutex.unlock();

    m_controlMutex.lock();
    // Buttons processing
    m_control->manipulatorOpenClose =
        rc.buttons.ManipOpen -
        rc.buttons.ManipClose; // 0th is open, 1st is close
    m_control->manipulatorRotate =
        rc.buttons.ManipCCW - rc.buttons.ManipCW; // 2nd is CCW, 3rd is CW
    if (m_prevCamSelState == 0 && rc.buttons.CameraSelect != 0)
        m_control->camsel = !m_control->camsel;
    m_prevCamSelState = rc.buttons.CameraSelect;

    // Thrusters processing
    if (state) {
        m_thrOvrMutex.lock();
        m_thrOvrInvMutex.lock();
        for (int i = 0; i < 8; i++) {
            m_control->thrusterPower[i] =
                m_thrOvr[i] * (BIT_CHECK(m_thrOvrInv, i) ? -1 : 1);
        }
        m_thrOvrMutex.unlock();
        m_thrOvrInvMutex.unlock();
    } else {
        qint8 x = rc.axes[0] * rc.runtimeASF[0] * rc.baseASF[0] *
                  rc.directions[0]; // left-right
        qint8 y = rc.axes[1] * rc.runtimeASF[1] * rc.baseASF[1] *
                  rc.directions[1]; // forward-backward
        qint8 z = rc.axes[2] * rc.runtimeASF[2] * rc.baseASF[2] *
                  rc.directions[2]; // up-down
        qint8 w = rc.axes[3] * rc.runtimeASF[3] * rc.baseASF[3] *
                  rc.directions[3]; // rotation
        qint8 d = rc.axes[4] * rc.runtimeASF[4] * rc.baseASF[4] *
                  rc.directions[4]; // pitch
        qint8 r = rc.axes[5] * rc.runtimeASF[5] * rc.baseASF[5] *
                  rc.directions[5]; // roll

        // TODO: directions and axes setup
        // Horizontal thrusters
        m_control->thrusterPower[0] = constrain(x - y - w, -100, 100);
        m_control->thrusterPower[1] = constrain(x + y + w, -100, 100);
        m_control->thrusterPower[2] = constrain(x + y*.5 - w, -100, 100);
        m_control->thrusterPower[3] = constrain(x - y*.5 + w, -100, 100);
        // Vertical thrusters
        m_control->thrusterPower[4] = constrain(z + d + r, -100, 100);
        m_control->thrusterPower[5] = constrain(z + d - r, -100, 100);
        m_control->thrusterPower[6] = constrain(z - d + r, -100, 100);
        m_control->thrusterPower[7] = constrain(z - d - r, -100, 100);
    }
    // Hats processing
    m_control->cameraRotationDelta[0] = rc.hats[1];
    m_control->cameraRotationDelta[1] = rc.hats[0];

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
    QDataStream out(&datagram, QIODevice::ReadOnly);
    out.setFloatingPointPrecision(QDataStream::SinglePrecision);

    qint8 header = 0x00;
    out >> header;
    if (header == RovTelemetry::header) {

        out >> telemetry.version;
        out >> telemetry.depth;
        out >> telemetry.pitch;
        out >> telemetry.yaw;
        out >> telemetry.roll;
        out >> telemetry.current;
        out >> telemetry.voltage;
        out >> telemetry.cameraIndex;
        out >> telemetry.temperature;

    } else {
        qDebug() << "Wrong header (" << QString::number(header, 2) << " vs "
                 << QString::number(RovTelemetry::header, 2) << "\n";
    }

    emit telemetryProcessed(telemetry);
}
