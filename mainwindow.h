#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QDebug>
#include "math.h"
#include "joystickhandler.h"
#include "joysticksetupdialog.h"
#include "rovcameracapture.h"
#include "rovdatasplines.h"
#include "thrustersetupdialog.h"
#include "rovcommunication.h"
#include "rovdataparser.h"
#include "rovdatatypes.h"

static QString settings_path = QString(getenv("HOME")) + QString("/.config/RovUI2.ini");

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void doUpdateCameraLabel(QImage);
    void doResizeCameraLabel();
    void doUpdateTelemetry(RovTelemetry);
    void updateDatasplines(RovTelemetry);
//    void updateRegulators(QBitArray regulators); //TODO: implement
    void doUpdateThrustFactor(float thrustFactor);

signals:
    void thrustFactorUpdated(float thrustFactor); // TODO: implement

private:
    void createConnections();

    quint8 m_vSamples = 0;
    quint8 m_aSamples = 0;

    Ui::MainWindow *ui;
    QScopedPointer<RovCameraCapture> m_camera;
    QScopedPointer<ThrusterSetupDialog> m_tsd;
    QScopedPointer<JoystickHandler> m_joystick;
    QScopedPointer<JoystickSetupDialog> m_jsd;
    QScopedPointer<RovCommunication> m_communication;
    QScopedPointer<RovDataParser> m_dataparser;
    QScopedPointer<RovDataSplines> m_datasplines;
};
#endif // MAINWINDOW_H
