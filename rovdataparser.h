#ifndef ROVDATAPARSER_H
#define ROVDATAPARSER_H

#include "joystick.h"
#include "qdialog.h"
#include "qelapsedtimer.h"
#include "qwidget.h"
#include "rovdatatypes.h"
#include <QDataStream>
#include <QDebug>
#include <QMutex>
#include <QObject>
#include <algorithm>
#define DEFAULT_kP 3
#define DEFAULT_kI 0.01
#define DEFAULT_kD 5
/**
 * @brief Floating point PD regulator implementation
 *
 */
class FPPDRegulator {
  public:
    /**
     * @brief Construct a new FPPDRegulator object
     *
     * @param coeffP Proportional coefficient
     * @param coeffD Differential coefficient
     */
    FPPDRegulator(float coeffP, float coeffI, float coeffD)
        : kP(coeffP), kI(coeffI), kD(coeffD), lastError(0.0f), uI(0.0f), target(0.0f){};
    /**
     * @brief Evaluate the regulator expression
     *
     * @param data Data
     * @param target Target
     * @return float Control signal
     */
    float eval(float data);
    void  setTarget(float target);
    void  enable();
    void  disable();

  private:
    bool  enabled = false;
    float kP;
    float kI;
    float kD;

    float lastError;
    float uI;
    float target;

    int lastData;

    QElapsedTimer eTimer;
};

QT_BEGIN_NAMESPACE
namespace Ui {
    class DataParser;
}
QT_END_NAMESPACE

/**
 * \brief The RovDataParser class is responsible for packing control data in
 * QByteArrays and unpacking telemetry data from them.
 */
class RovDataParser : public QDialog {
    Q_OBJECT
  public:
    /**
     * \brief Default constructor
     * \param parent Parent
     */
    explicit RovDataParser(QWidget *parent = nullptr);

  signals:
    /**
     * \brief Emitted when the class is finished processing raw QByteArray
     * \param tele Telemetry object
     */
    void telemetryProcessed(RovTelemetry tele);

    /**
     * \brief Emitted when the class is finished packing RovControl struct into
     * a QByteArray \param ba QByteArray with the control data
     */
    void controlReady(QByteArray ba);

    /**
     * \brief Emitted when the class is finished packing RovAuxControl struct
     * into a QByteArray \param ba QByteArray with the auxiliary data
     */
    void auxControlReady(QByteArray ba);
  public slots:

    /**
     * \brief Called to unpack telemetry data from the QByteArray into the
     * RovTelemetry struct \param ba Datagrem with telemetry data
     */
    void processTelemetry(QByteArray ba);

    /**
     * \brief Called to pack joystick data from Joystick struct and then into
     * QByteArray \param joy Joystick struct with data
     */
    void prepareControl(Joystick joy);

    /**
     * \brief Called to pack the auxiliary control data into QByteArray
     */
    void prepareAuxControl();

    /**
     * \brief Sets RovAuxControl' auxiliary flags
     * \param aFlags Flags
     */
    void setAuxFlags(qint8 aFlags);

    /**
     * \brief Enables or disables depth regulator
     * \param status
     */
    void setDepthStatus(int status);
    /**
     * \brief Sets desired depth
     * \param dVal Desired value
     */
    void setDepth(double dVal);

    /**
     * \brief Enables or disables depth regulator
     * \param status
     */
    void setYawStatus(int status);
    /**
     * \brief Sets desired yaw
     * \param dVal Desired value
     */
    void setYaw(double dVal);

    /**
     * \brief Enables or disables depth regulator
     * \param status
     */
    void setRollStatus(int status);
    /**
     * \brief Sets desired roll
     * \param dVal Desired value
     */
    void setRoll(double dVal);

    /**
     * \brief Enables or disables depth regulator
     * \param status
     */
    void setPitchStatus(int status);
    /**
     * \brief Sets desired pitch
     * \param dVal Desired value
     */
    void setPitch(double dVal);

    void toggleLight();

    void togglePump();

    void invalidateImuCalibration();

  private:
    /**
     * \brief UI
     */
    Ui::DataParser *ui;

    /**
     * \brief Internally-stored RovControl struct
     */
    QScopedPointer<RovControl> m_control;
    /**
     * \brief Mutex for m_control
     */
    QMutex                     m_controlMutex;

    QTimer *overrideTelemetryUpdate;

    /**
     * \brief Previous CamSel state, used to switch cameras on the ROV
     */
    bool m_prevCamSelState;

    /**
     * \brief Internally-stored RovAuxControl struct
     */
    QScopedPointer<RovAuxControl> m_auxControl;

    /**
     * \brief Mutex for m_auxControl
     */
    QMutex m_auxControlMutex;

    RovTelemetry m_tele;

    QMutex m_teleMutex;

    FPPDRegulator depthReg;
    FPPDRegulator yawReg;
    FPPDRegulator rollReg;
    FPPDRegulator pitchReg;
};

#endif // ROVDATAPARSER_H
