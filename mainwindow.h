#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "camerasettings.h"
#include "instrumentcluster.h"
#include "joystickhandler.h"
#include "joysticksetupdialog.h"
#include "ledindicator.h"
#include "qscopedpointer.h"
#include "qwidget.h"
#include "rovcameracapture.h"
#include "rovcameracommunication.h"
#include "rovcommunication.h"
#include "rovdataparser.h"
#include "rovdatasplines.h"
#include "rovdatatypes.h"
#include "thrustersetupdialog.h"
#include <QDebug>
#include <QMainWindow>
#include <QString>
#include <chrono>

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

/**
 * \brief The MainWindow class is responsible for displaying the main window of
 * the app and tying it all together \todo maybe create another class that
 * manages all the backend and provides data for the MainWindow
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    /**
     * \brief Default constructor
     * \param parent Parent
     */
    MainWindow(QWidget *parent = nullptr);

    /**
     * \brief Default destructor
     */
    ~MainWindow();

  public slots:
    /**
     * \brief Called to change image of the camera label
     */
    void updateStatusbar();

    /**
     * \brief You guessed it
     */
    void updateStatusbarIndicators();

    /**
     * \brief Called to update telemetry data in corresponding dock widget
     * \param tele Telemetry data
     */
    void updateTelemetry(RovTelemetry tele);

    /**
     * \brief Called to update data splines with the telemetry data
     * \param tele Telemetry data
     */
    void updateDatasplines(RovTelemetry tele);

    /**
     * \brief You guessed it
     * \param text Text to update the statusbar with
     */
    void updateStatusbarText(QString text);

    /**
     * \brief You guessed it \b {NI}
     * \param progress Progress 0-100, if < 0 then continous
     * \todo implement
     */
    void updateStatusbarProgress(int progress);
    //    void updateRegulators(QBitArray regulators);

    /**
     * \brief Updates rASFs across all of the program
     * \param thrustFactor ASF to set
     */
    void updateASF(float thrustFactor);

    /**
     * \brief updates dDepth in RovAuxControl
     * \param val Value
     */
    void setDesiredDepth(float val);

    /**
     * \brief updates dYaw in RovAuxControl
     * \param val Value
     */
    void setDesiredYaw(float val);

    /**
     * \brief updates dPitch in RovAuxControl
     * \param val Value
     */
    void setDesiredPitch(float val);

    /**
     * \brief updates dRoll in RovAuxControl
     * \param val Value
     */
    void setDesiredRoll(float val);

    /**
     * \brief Sets depth QDoubleSpinBox to be readonly or rw for aux programlets
     * purpose \param ro If this true then the spinbox blocks user input. Else
     * it allows user input, overriding previous value
     */
    void overrideDepth(bool ro);

    /**
     * \brief Sets yaw QDoubleSpinBox to be readonly or rw for aux programlets
     * purpose \param ro If this true then the spinbox blocks user input. Else
     * it allows user input, overriding previous value
     */
    void overrideYaw(bool ro);

    /**
     * \brief Sets pitch QDoubleSpinBox to be readonly or rw for aux programlets
     * purpose \param ro If this true then the spinbox blocks user input. Else
     * it allows user input, overriding previous value
     */
    void overridePitch(bool ro);

    /**
     * \brief Sets roll QDoubleSpinBox to be readonly or rw for aux programlets
     * purpose \param ro If this true then the spinbox blocks user input. Else
     * it allows user input, overriding previous value
     */
    void overrideRoll(bool ro);
  signals:

    /**
     * \brief Emitted when rASFs are updated either by programlets or user input
     * \param thrustFactor ASF
     * \todo maybe set individual ASFs and akchually implement this
     */
    void asfUpdated(float thrustFactor);

  private:
    /**
     * \brief Connects all modules together
     */
    void createConnections();

    /**
     * \brief Sets up statusbar
     */
    void setupStatusbar();

    /**
     * \brief Helper variable for feeding into m_datasplines
     */
    quint64 m_vSamples = 0;

    /**
     * \brief Helper variable for feeding into m_datasplines
     */
    quint64 m_aSamples = 0;

    /**
     * @brief Helper variable for UI/UX
     *
     */
    float lastDepth = 0.0f;

    /**
     * @brief Last telemetry data, usually no more than 10ms behind
     *
     */
    RovTelemetry lastTele;

    /**
     * \brief Ui object
     */
    Ui::MainWindow *ui;

    /**
     * \brief Camera capture object
     * \see RovCameraCapture
     */
    QScopedPointer<RovCameraCapture> m_cameraCapture;

    /**
     * \brief Joystick handler object
     * \see JoystickHandler
     */
    QScopedPointer<JoystickHandler> m_joystickHandler;

    /**
     * \brief Joystick setup dialog object
     * \see JoystickSetupDialog
     */
    QScopedPointer<JoystickSetupDialog> m_jsd;

    /**
     * \brief Network interface for communicating with the ROV
     * \see RovCommunication
     */
    QScopedPointer<RovCommunication> m_rovCommunication;

    /**
     * \brief ROV data parser object
     * \see RovDataParser
     */
    QScopedPointer<RovDataParser> m_rovDataParser;

    /**
     * \brief Data splines, cute graphs
     * \see RovDataSplines
     */
    QScopedPointer<RovDataSplines> m_datasplines;

    /**
     * \brief Statusbar label, used to display connection status
     */
    QScopedPointer<QLabel> m_rovStatusLabel;

    /**
     * \brief Statusbar indicator, used to display connection status
     */
    QScopedPointer<LEDIndicator> m_rovStatusIndicator;

    QScopedPointer<RovCameraCommunication> m_rovCameraCommunication;
    QScopedPointer<CameraSettings>         m_cameraSettings;
    QScopedPointer<InstrumentWidget>       m_compassWidget;
    QScopedPointer<InstrumentWidget>       m_gyroWidget;
};
#endif // MAINWINDOW_H
