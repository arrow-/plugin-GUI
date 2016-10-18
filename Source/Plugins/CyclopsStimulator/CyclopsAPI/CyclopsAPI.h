/** @file CyclopsAPI.h
    @brief Defines constants of the CL-RPC Format Specification.
    
    @page cl-rpc-desc Cyclops RPC Format Description

    @tableofcontents

    @section rpc-overview Overview
    There are two kinds of Serial packets, single byte and multi byte.
    The size of a packet is determined by MSB of header and the ``command`` field.

    The teensy will respond to all serial commands that it reads, see the section on @ref return-codes "Return Codes"

    @section S-header-desc Single Byte Packets
    Single byte Headers are distinguished from other headers by the MSB bit. For
    Single byte headers, this bit is always set.
    
    Field       | Bits  | Description
    ----------- | ----- | -----------
    Reserved    | [7]   | Always ``1``.
    ``channel`` | [6:3] | This is a bitmask and determines if "command" is applied on the Channel ``x``
    ``command`` | [2:0] | The command field
    
    @subsection S-cmd-desc Command descriptions
    
    command[2:0] | Name     | Effect                                                                                                                                                                                                   | Invoke in System State                |
    ------------ | -------  | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------- |
    ``000``      | start    | Launch Waveform generation.                                                                                                                                                                              | ``Expt. Active``                      |
    ``001``      | stop     | Pause Waveform generation.                                                                                                                                                                               | ``Expt. Active``                      |
    ``010``      | reset    | Reset selected sources. @attention The system is *not* reset to _initial configuration_!                                                                                                                 | ``Expt. Active``                      |
    ``011``      | swap     | Swap the Cyclops instances of the 2 high ``channel`` bits. @attention Picks the two Lowest Significant bits (in case more than 2 bits are high).                                                         | ``Expt. Active``                      |
    ``100``      | launch   | Launch the experiment main-loop.                                                                                                                                                                         | ``Expt. notActive``                   |
    ``101``      | end      | Stop the experiment main-loop. The teensy will enter a restricted mode, responding only to some Single byte commands: [launch, identity]. For all commands, an error code will be returned.              | ``Expt. Active``                      |
    ``110``      | test     | Tests the selected LED channel (provided it is connected!) with a TEST signal waveform. @attention Tests only (one) channel, the one with the lowest ID, i.e. ``CH0`` is tested if bit-mask is ``1101``. | ``Expt. notActive``                   |
    ``111``      | identity | Send device description.                                                                                                                                                                                 | ``Expt. Active``, ``Expt. notActive`` |
    
    @section M-header-desc Multi Byte Packets
    Packet is formed by concatenating the header with argument bytes. These must be invoked _only when_ "Experiment is Active".

    Field       | Bits  | Description
    ----------- | ----- | -----------
    Reserved    | [7]   | Always ``0``.
    ``channel`` | [6:5] | Command is appplied on Channel ``channel[1:0]``
    ``command`` | [4:0] | The command field.

    @subsection M-cmd-desc Command descriptions

    command[4:0]  | Name               | Size(Bytes) | Effect
    ------------- | ------------------ | ----------- | --------
    ``00000``     | change_source_l    | 2           | Changes Source instance to the one reffered by @ref src-id-sec. Mode is set to ``LOOPBACK``.
    ``00001``     | change_source_o    | 2           | Changes Source instance to the one reffered by @ref src-id-sec. Mode is set to ``ONE_SHOT``.
    ``00010``     | change_source_n    | 3           | Changes Source instance to the one reffered by @ref src-id-sec. Mode is set to ``N_SHOT``. \f$N\f$ is set to @ref shot_cycle "shot_cycle".
    ``00011``     | change_time_period | 5           | Set time period of updates @attention Works only if Source::holdTime is a constant!
    ``00100``     | time_factor        | 5           | Scale Source::holdTime values by this factor. \f$\in [0, \infty)\f$.
    ``00101``     | voltage_factor     | 5           | Scale Source::getVoltage values by this factor. \f$\in [0, \infty)\f$.
    ``00110``     | voltage_offset     | 3           | Add this DC offset level to Source::getVoltage values. \f$\in [0, \infty)\f$.
    ``00111``     | square_on_time     | 5           | Set squareSource pulse "ON" time.
    ``01000``     | square_off_time    | 5           | Set squareSource pulse "OFF" time.
    ``01001``     | square_on_level    | 3           | Set squareSource pulse "ON" voltage.
    ``01010``     | square_off_level   | 3           | Set squareSource pulse "OFF" voltage.
    
    @note       Voltage scaling can also be manually accomplished by the
                tweaking the GAIN knob on Cyclops Front Panel.

    @subsubsection src-id-sec src_id
    Each Source has a unique ID which is internally used by Task. The OE plugin
    might use the number just as a reference. @sa Source::src_id
    
    @subsection M-arg-byte-desc Argument Bytes

    | Command Name       | Argument[0]   | Argument[1]      |
    | ------------------ | ------------- | ---------------- |
    | change_source_l    | uint8  src_id |                  |
    | change_source_o    | uint8  src_id |                  |
    | change_source_n    | uint8  src_id | @anchor shot_cycle uint8 shot_cycle |
    | change_time_period | uint32 val    |                  |
    | time_factor        | float  val    |                  |
    | voltage_factor     | float  val    |                  |
    | voltage_offset     | int16  val    |                  |
    | square_on_time     | uint32 val    |                  |
    | square_off_time    | uint32 val    |                  |
    | square_on_level    | uint16 val    |                  |
    | square_off_level   | uint16 val    |                  |

    @subsection return-codes Error and Success Codes

    These are the _codes_ which will be returned by the teensy when an RPC command is recieved by it.

    For each RPC command,

    * a **Success Code** is returned _only when_, command can be executed in the current system state AND device performed the intended action successfully.
    * an **Error Code** is returned when device failed to execute it OR command _cannot_ be executed in the current system state.

    ``EA`` _stands for_ **Experiment is Active**

    ``notEA`` _stands for_ **Experiment is Inactive**

    @subsubsection RC-success Success Codes

    Success codes never start with first nibble HIGH ``~(0xFX)``.

    | Name     | Upon recieving...                                                  | Action when ``EA``                                            | Action when ``notEA``                                  | Success Code |
    | -------- | ------------------------------------------------------------------ | ------------------------------------------------------------- | ------------------------------------------------------ | ------------ |
    | LAUNCH   | ``SB.launch``                                                      | **No Action**                                                 | **Starts main-loop**, thereby starting the experiment. | ``0x00``     |
    | END      | ``SB.end``                                                         | **Breaks out of mainloop**, resets sources, grounds all LEDs. | **No Action**                                          | ``0x01``     |
    | SB.DONE  | ``SB.*`` (except ``launch``, ``end``, ``identity``, ``test``)      | Suitable action is performed.                                 | **No Action**                                          | ``0x10``     |
    | IDENTITY | ``SB.identity``                                                    | Identification Info is returned.                              | Identification Info is returned.                       | ``0x11``     |
    | TEST     | ``SB.test``                                                        | **No Action**                                                 | A TEST waveform is run on the channel.                 | ``0x12``     |
    | MB.DONE  | ``MB.*``                                                           | Suitable action is performed.                                 | **No Action**                                          | ``0x20``     |
    
    @subsubsection RC-error Error Codes
    
    Error codes generally start with first nibble HIGH ``(0xFX)``

    | Name           | Reason                                                                                                                     | ...while in state   | Code     |
    | -------------- | -------------------------------------------------------------------------------------------------------------------------- | ------------------- | -------- |
    | RPC_FAIL_EA    | ``SB.*`` or ``MB.*`` failed due to either inability to perform task, or task cannot be done when experiment is _Live_.     | only when ``EA``    | ``0xf0`` |
    | RPC_FAIL_notEA | ``SB.*`` or ``MB.*`` failed due to either inability to perform task, or task cannot be done when experiment is _Not Live_. | only when ``notEA`` | ``0xf1`` |

    @author Ananya Bahadur
*/

#ifndef OE_CYCLOPS_API_H
#define OE_CYCLOPS_API_H

#include "../../../../JuceLibraryCode/JuceHeader.h"
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "yaml-cpp/yaml.h"

/**
 * @defgroup ns-cyclops cyclops
 * 
 */
namespace cyclops
{

static const int RPC_HEADER_SZ = 1;
static const int RPC_SUCCESSCODE_SZ = 1;
static const int RPC_ERRORCODE_SZ = 1;
static const int RPC_MAX_ARGS  = 4;
/** size of ((identity string) + (RPC success code)) */
static const int RPC_IDENTITY_SUCCESSCODE_SZ = 27;
static const int RPC_IDENTITY_SZ = RPC_IDENTITY_SUCCESSCODE_SZ - RPC_SUCCESSCODE_SZ;

enum class operationMode
{
    LOOPBACK,
    ONE_SHOT,
    N_SHOT
};

enum class sourceType
{
    STORED,
    GENERATED,
    SQUARE
};

class CyclopsDeviceInfo
{
public:
    CyclopsDeviceInfo(const unsigned char* id_str);
    String toString();

    uint8_t libMajor,
            libMinor,
            numWaveforms,
            numChannels,
            channelState[4];
    bool isTeensy,
         isArduino;
    String devName,
           identity;
private:
    void parseUnsignedChar(const unsigned char* id_str);
};

class CyclopsSignal
{
public:
    int type;
    int size;
    std::string name;
    std::vector<int> voltage, holdTime;

    static OwnedArray<CyclopsSignal> signals;

    static void readSignals(std::ifstream& inFile);
    static const CyclopsSignal& getSignalByIndex(int index);
private:
    static bool isPrepared;
};

enum singleByteCommands
{
    CL_SB_START     = 0,
    CL_SB_STOP      = 1,
    CL_SB_RESET     = 2,
    CL_SB_SWAP      = 3,
    CL_SB_LAUNCH    = 4,
    CL_SB_END       = 5,
    CL_SB_TEST      = 6,
    CL_SB_IDENTITY  = 7
};

enum multiByteCommands
{
    CHANGE_SOURCE_LOOP = 0,
    CHANGE_SOURCE_ONE  = 1,
    CHANGE_SOURCE_N    = 2,
    CHANGE_TIME_PERIOD = 3,
    TIME_FACTOR        = 4,
    VOLTAGE_FACTOR     = 5,
    VOLTAGE_OFFSET     = 6,
    SQUARE_ON_TIME     = 7,
    SQUARE_OFF_TIME    = 8,
    SQUARE_ON_LEVEL    = 9,
    SQUARE_OFF_LEVEL   = 10,
};

enum multiByteLength
{
    CHANGE_SOURCE_LOOP_LEN = 2,
    CHANGE_SOURCE_ONE_LEN  = 2,
    CHANGE_SOURCE_N_LEN    = 3,
    CHANGE_TIME_PERIOD_LEN = 5,
    TIME_FACTOR_LEN        = 5,
    VOLTAGE_FACTOR_LEN     = 5,
    VOLTAGE_OFFSET_LEN     = 3,
    SQUARE_ON_TIME_LEN     = 5,
    SQUARE_OFF_TIME_LEN    = 5,
    SQUARE_ON_LEVEL_LEN    = 3,
    SQUARE_OFF_LEVEL_LEN   = 3
};

enum returnCode
{
    CL_RC_LAUNCH = 0x00,
    CL_RC_END = 0x01,
    CL_RC_SBDONE = 0x10,
    CL_RC_TEST = 0x12,
    CL_RC_MBDONE = 0x20,
    CL_RC_IDENTITY = 0x11,
    CL_RC_EA_FAIL = 0xf0,
    CL_RC_NEA_FAIL = 0xf1,
    CL_RC_UNKNOWN = 0xff
};

/**
 * @defgroup cyclops-api
 * 
 * @warning
 * Most functions in this namespace assume that the various arguments are going to be valid, ie
 * there is absolutely no error checking. In the worst case, malformed packets might invoke
 * undesired action on the Teensy!
 */
namespace api{

struct CyclopsRPC
{
    uint8_t message[RPC_HEADER_SZ + RPC_MAX_ARGS];
    int length;
};

/*
  +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
  |                                 SINGLE BYTE COMMANDS                          |
  +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
*/

/**
 * @brief      Start (aka reset) the selected Channel Signals.
 * @note       Invoke during ``ExperimentActive``
 * @ingroup    cyclops-api
 */
bool start (CyclopsRPC *rpc, const int *channels, int channelCount);

/**
 * @brief      Pause (aka FREEZE) the selected Channel Signals.
 * @note       Invoke during ``ExperimentActive``
 * @ingroup    cyclops-api
 */
bool stop (CyclopsRPC *rpc, const int *channels, int channelCount);

/**
 * @brief      Reset (aka start) the selected Channel Signals.
 * @note       Invoke during ``ExperimentActive``
 * @ingroup    cyclops-api
 */
bool reset (CyclopsRPC *rpc, const int *channels, int channelCount);

/**
 * @brief      Swap the physical cyclops channel of the two selected Channel
 *             Signals.
 * @note       Invoke during ``ExperimentActive``
 * @ingroup    cyclops-api
 */
bool swap (CyclopsRPC *rpc, int c1, int c2);

/**
 * @brief      Launches the experiment on the teensy.
 * @note       Invoke during ``not ExperimentActive``
 * @ingroup    cyclops-api
 */
bool launch (CyclopsRPC *rpc);

/**
 * @brief      Ends the experiment on the teensy, which can be relaunched.
 * @note       Invoke during ``ExperimentActive``
 * @ingroup    cyclops-api
 */
bool end (CyclopsRPC *rpc);

/**
 * @brief      Tests the given channel LED, by driving a TEST Signal on th LED.
 * @note       Invoke during ``not ExperimentActive``
 * @ingroup    cyclops-api
 */
bool test(CyclopsRPC *rpc, int channel);

/**
 * @brief      Query the Cyclops Board for connected channels, firmware version
 *             etc.
 * @note       Invoke during ``not ExperimentActive``, ``ExperimentActive``
 * @attention  This should not be invoked when aquisition is active (the teensy can
 *             handle it, but the GUI cannot).
 * @ingroup    cyclops-api
 */
bool identify (CyclopsRPC *rpc);

/*
  +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
  |                                 MULTI BYTE COMMANDS                           |
  +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
*/


/**
 * @brief      Change the ``source`` object for the selected channel, in LOOPBACK mode.
 * @ingroup    cyclops-api
 * @param[in]  srcID    The source identifier on the teensy.
 */
bool change_source_loop (CyclopsRPC *rpc, int channel, int srcID);

/**
 * @brief      Change the ``source`` object for the selected channel, in ONE_SHOT mode.
 * @ingroup    cyclops-api
 * @param[in]  srcID    The source identifier on the teensy.
 */
bool change_source_one_shot (CyclopsRPC *rpc, int channel, int srcID);

/**
 * @brief      Change the ``source`` object for the selected channel, in N_SHOT
 *             mode.
 * @ingroup    cyclops-api
 * @param[in]  srcID       The source identifier on the teensy.
 * @param[in]  shot_cycle  The shot cycle (1 by default)
 */
bool change_source_n_shot (CyclopsRPC *rpc, int channel, int srcID, int shot_cycle = 1);

/**
 * @brief      Set a new period of update for ``generatedSource``.
 * @note       Works only if the active source on the selected channel is of type ``generatedSource``
 *             AND was been configured with a ``const`` holdTime.
 * @ingroup    cyclops-api
 */
bool change_time_period (CyclopsRPC *rpc, int channel, uint32_t new_period);

/**
 * @brief      Set the time-scale factor for the source on the selected channel.
 * @ingroup    cyclops-api
 */
bool time_factor (CyclopsRPC *rpc, int channel, float tFactor);

/**
 * @brief      Set the voltage-scale factor for the source on the selected channel.
 * @ingroup    cyclops-api
 */
bool voltage_factor (CyclopsRPC *rpc, int channel, float vFactor);

/**
 * @brief      Set the DC voltage offset for the source on the selected channel.
 * @note       This number is a signed integer!
 * @ingroup    cyclops-api
 */
bool voltage_offset (CyclopsRPC *rpc, int channel, int16_t vOffset);

/**
 * @brief      Set the "ON" time for the square wave.
 * @note       Works only if the active source on the selected channel is of type ``squareSource``!
 * @ingroup    cyclops-api
 */
bool square_on_time (CyclopsRPC *rpc, int channel, uint32_t onTime);

/**
 * @brief      Set the "OFF" time for the square wave.
 * @note       Works only if the active source on the selected channel is of type ``squareSource``!
 * @ingroup    cyclops-api
 */
bool square_off_time (CyclopsRPC *rpc, int channel, uint32_t offTime);

/**
 * @brief      Set the "ON" voltage level for the square wave.
 * @note       Works only if the active source on the selected channel is of type ``squareSource``!
 * @ingroup    cyclops-api
 */
bool square_on_level (CyclopsRPC *rpc, int channel, uint16_t onLevel);

/**
 * @brief      Set the "OFF" voltage level for the square wave.
 * @note       Works only if the active source on the selected channel is of type ``squareSource``!
 * @ingroup    cyclops-api
 */
bool square_off_level (CyclopsRPC *rpc, int channel, uint16_t offLevel);

} // NAMESPACE cyclops-api
} // NAMESPACE cyclops

#endif