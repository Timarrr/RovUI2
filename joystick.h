#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <QString>
#include <SDL_joystick.h>
#include <map>
/**
 * \namespace JoystickNames is used to standardize names and designations of the
 * axes and buttons and whatever
 */
namespace JoystickHelpers {
    /**
     * \brief Map of the axes' indices and their names
     * \image html rovAxes.png
     */
    enum axes { AxisX = 0, AxisY, AxisZ, AxisW, AxisD, AxisR };

    /**
     * \brief The buttons enum is used to ease access to buttons in Joystick
     * struct
     *
     * Button designations:\n
     * [0]: Open manipulator,\n
     * [1]: Close manipulator,\n
     * [2]: Rotate manipulator CW,\n
     * [3]: Rotate manipulator CCW,\n
     * [4]: Set ASFs to 1,\n
     * [5]: Set ASFs to 0.75,\n
     * [6]: Set ASFs to 0.5,\n
     * [7]: Set ASFs to 0.25,\n
     * [8]: Camera Select\n
     * [9-15]: Not used yet\n
     *
     * \see Joystick
     */
    enum buttons {
        ManipOpen = 0,
        ManipClose,
        ManipCW,
        ManipCCW,
        ASFFast,
        ASFMedium,
        ASFSlow,
        ASFUltraSlow,
        CameraSelect,
        Button10,
        Button11,
        Button12,
        Button13,
        Button14,
        Button15,
        Button16
    };

    /**
     * \var axesNames
     * \brief test
     */
    static QString axesNames[6] = {"AxisX", "AxisY", "AxisZ",
                                   "AxisW", "AxisD", "AxisR"};

    static QString buttonsNames[16] = {
        "ManipOpen", "ManipClose", "ManipCW",  "ManipCCW",
        "ASF-F",     "ASF-M",      "ASF-S",    "ASF-US",
        "CamSel",    "Button10",   "Button11", "Button12",
        "Button13",  "Button14",   "Button15", "Button16"};

    static QString hatsNames[4] = {"CamServoFront", "CamServoRear", "Hat3",
                                   "Hat4"};
} // namespace JoystickHelpers

/**
 * \brief The Joystick struct is used to help ordering of the data got from the
 * joystick \todo maybe split into JoystickData with axes, buttons and hats data
 * and JoystickInfo with name and other data describing the physical layout of
 * the joystick
 */
struct Joystick {

    /**
     * \brief Axes data from the joystick
     * \see JoystickNames::axes
     */
    float axes[6];

    /**
     * \brief Previous axes data from the joystick
     * \see axesNames_map
     */
    float axes_last[6];

    /**
     * \brief Mapping of the axes according to JoystickNames::axes to the
     * joystick axes \see axesNames_map
     */
    int8_t axes_id[6];

    /**
     * \brief runtimeASF - runtime Axes Scale Factors
     *
     * Axes' data is multiplied by corresponding ASFs to get final axis value
     * which then is used to calculate thrusters' power \see baseASF \see
     * JoystickNames::axes
     */
    float runtimeASF[6];

    /**
     * \brief baseASF - base Axes Scale Factors
     *
     * Axes' data is multiplied by corresponding ASFs to get final axis value
     * which then is used to calculate thrusters' power \see runtimeASF \see
     * JoystickNames::axes
     */
    float baseASF[6];

    /**
     * \brief Directions of the axes, used to change direction of the
     * corresponding axis
     */
    int8_t directions[6];

    /**
     * \var buttons
     * \brief Buttons of the joystick, compacted into uint16_t
     *
     * Mapped according to JoystickNames::buttons
     * \see JoystickNames::buttons
     */
    /**
     * \union _joystickButtons
     * \brief Buttons of the joystick, compacted into uint16_t
     *
     * Mapped according to JoystickNames::buttons
     * \see JoystickNames::buttons
     */
    union _joystickButtons {
        /**
         * \brief Raw buttons data
         */
        uint16_t rawData;
        struct {
            int8_t ManipOpen    : 1;
            int8_t ManipClose   : 1;
            int8_t ManipCW      : 1;
            int8_t ManipCCW     : 1;
            int8_t ASFFast      : 1;
            int8_t ASFMedium    : 1;
            int8_t ASFSlow      : 1;
            int8_t ASFUltraSlow : 1;
            int8_t CameraSelect : 1;
            int8_t __res        : 6;
        };
    } buttons;

    /**
     * \brief Mapping of the buttons according to buttonsNames to the joystick
     * buttons \see JoystickNames::buttons
     */
    int8_t buttons_id[16];

    /**
     * \brief Hats' data from the joystick
     */
    int8_t hats[4];

    /**
     * \brief Mapping of the hats according to hatsNames to the joystick hats
     */
    int8_t hats_id[4];

    /**
     * \brief Determines which axis of the hat is used
     *
     *
     * Value  | Meaning
     * -------| -------------
     * true   | Horizontal axis is used (left is -1, right is +1)
     * false  | Vertical axis is used (down is -1, up is +1)
     */
    bool hats_hor[4];

    /**
     * \brief Name of the joystick, used for settings
     */
    QString joystickName;

    /**
     * \brief Number of axes>
     */
    int8_t numAxes;

    /**
     * \brief Number of buttons
     */
    int8_t numButtons;

    /**
     * \brief Number of hats
     */
    int8_t numHats;

    /**
     * \brief Default constructor
     */
    Joystick() {
        for (int i = 0; i < 6; i++) {
            this->axes[i]       = 0;
            this->axes_last[i]  = 0;
            this->axes_id[i]    = 0;
            this->runtimeASF[i] = 1;
            this->baseASF[i]    = 1;
            this->directions[i] = 1;
        }
        this->buttons.rawData = 0;
        for (int i = 0; i < 16; i++) {
            buttons_id[i] = 0;
        }
        for (int i = 0; i < 4; i++) {
            this->hats[i]    = 0;
            this->hats_id[i] = 0;
        }
        this->numAxes      = 0;
        this->numButtons   = 0;
        this->numHats      = 0;
        this->joystickName = QString();
    }

    /**
     * \brief Copy-constructor
     * \param j Joystick
     */
    Joystick(Joystick *j) {
        for (int i = 0; i < 6; i++) {
            this->axes[i]       = j->axes[i];
            this->axes_last[i]  = j->axes_last[i];
            this->axes_id[i]    = j->axes_id[i];
            this->runtimeASF[i] = j->runtimeASF[i];
            this->baseASF[i]    = j->baseASF[i];
            this->directions[i] = j->directions[i];
        }
        for (int i = 0; i < 4; ++i) {
            this->hats[i]    = j->hats[i];
            this->hats_id[i] = j->hats_id[i];
        }
        this->buttons = j->buttons;
        for (int i = 0; i < 16; i++) {
            this->buttons_id[i] = j->buttons_id[i];
        }
        this->numAxes      = j->numAxes;
        this->numButtons   = j->numButtons;
        this->numHats      = j->numHats;
        this->joystickName = QString(j->joystickName);
    }
};

#endif // JOYSTICK_H
