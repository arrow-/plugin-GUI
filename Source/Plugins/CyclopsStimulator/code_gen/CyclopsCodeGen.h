#ifndef CL_CODE_GENERATE_H
#define CL_CODE_GENERATE_H

#include "../../../../JuceLibraryCode/JuceHeader.h"
#include <vector>
#include <map>
#include <bitset>
#include "../Headers/CoreServicesHeader.h"
#include "../CyclopsAPI/CyclopsAPI.h"
#include "../plugin_manager/CyclopsPluginInfo.h"

#define CLSTIM_NUM_PARAMS  3
#define CLSTIM_MAP_CH      0
#define CLSTIM_MAP_SIG     1
#define CLSTIM_MAP_LED     2

namespace cyclops{

class CyclopsSignal;

namespace code{

typedef std::pair<int, String> IdCodename_Pair_t;

typedef std::map<IdCodename_Pair_t, String> IdCodename_SourceMap_t;

/**
 * @brief      A clone (of sorts) of cyclops::HookInfo to hold hook
 *             configuration, relevant for code-generation.
 * @details    Instances must be created, and all instances must be collected
 *             into an ``std::vector<CyclopsHookConfig*>`` by the CyclopsCanvas and
 *             passed to the code-generator. **This decouples the code-generation
 *             from CyclopsCanvas and it's objects.**
 */
class CyclopsHookConfig
{
public:
	int nodeId, LEDChannel;
	CyclopsPluginInfo* pluginInfo;
	std::vector<int> selectedSignals;

	CyclopsHookConfig( int node_id, int LEDchannel, CyclopsPluginInfo* pInfo
					 , std::vector<int>& selection);
	/*
	CyclopsHookConfig();
	CyclopsHookConfig& operator= (const CyclopsHookConfig& rhs);
	*/
};

// Might require the operator== later, if complicated structure is added to CyclopsHookConfig... */
bool operator== (const CyclopsHookConfig& lhs, const CyclopsHookConfig& rhs);

/**
 * @brief      This collects all user configurations and inputs for the Code
 *             Generation module.
 * @details    An instance of this class is kept by CyclopsProgram, to quickly
 *             determine if there is any change in configuration.
 */
class CyclopsConfig
{
public:
	CyclopsConfig();
	CyclopsConfig( std::vector<CyclopsHookConfig>& hInfos
				 , Array<std::bitset<CLSTIM_NUM_PARAMS> >& summaryList);
	CyclopsConfig& operator= (const CyclopsConfig& rhs);
	std::vector<CyclopsHookConfig> hookInfos;
	Array<std::bitset<CLSTIM_NUM_PARAMS> > summaries;

	//int timeOfBuild;
};

bool operator== (const CyclopsConfig& lhs, const CyclopsConfig& rhs);


/* +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
 * |                               CYCLOPS-PROGRAM                            |
 * +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
 */

/**
 * @brief      The base class for all cyclops programs, for all target devices.
 */
class CyclopsProgram
{
public:
	CyclopsProgram(const String& deviceName);
	virtual ~CyclopsProgram();

	/**
	 * @brief      Generates the source code of the program using templates and
	 *             the CyclopsConfig
	 *
	 * @param[in]  config    The configuration of the canvas
	 * @param      genError  The _Generation Error Code_, depends on what the
	 *                       error was.
	 *                       Code | Description
	 *                       ---- | -----------
	 *                       0    | No Error
	 *                       1    | Could not generate "Sources Header"
	 *                       2    | Could not generate "main"
	 *                       3    | Could not generate Makefile
	 *
	 * @return     ``true`` only if code could be generated.
	 */
	bool create(const CyclopsConfig& config, int& genError);
	
	/**
	 * @brief      Builds the generated source code of the program using the
	 *             Makefile.
	 *
	 * @param      buildError  The _Build Error Code_ is the return value of the
	 *                         ``make`` subprocess.
	 *
	 * @return     ``true`` only if code could be successfully compiled (ie,
	 *             genError == 0).
	 */
	bool build(int &buildError);

	/**
	 * @brief      Flashes the built binary to the arget device.
	 *
	 * @param      flashError    The _Flash Error Code_ is the return value of
	 *                           the _flasher_ subprocess (invoked via
	 *                           ``make``).
	 * @param[in]  canvas_index  The canvas index, used for making proper
	 *                           directory structure to hold temporary files.
	 *
	 * @return     ``true`` only if binary was successully flashed (ie,
	 *             flashError == 0).
	 */
	bool flash(int &flashError, int canvas_index);

	const String device;
	int32 currentHash; /**< The hash value of the _current_ program source code. */
	String sourceHeader,
		   main,
		   makefile;
	bool oldConfigAvailable;

	static String arduinoPath, /**< Path to arduino installation, read from build Config file. */
		   		  deviceDir, /**< Path to temp directory, to store build files, read from build Config file. */
		   		  arduinoLibPath; /**< Path to Arduino Libraries directory (which includes cyclops library), read from build Config file. */
	static int reconDuration; /**< The amount of time (in milli seconds) to wait for a response right after flashing device. *Recommended 3000 msec**. */

protected:
	inline File getFileFromExeDir(const String& pathFromExeDir);

	/**
	 * @brief      Reads a ``.json`` file as a String, while handling errors
	 *             gracefully.
	 * @details    Does not verify contents, and is potentially an insecure
	 *             function. It is used to read template code for the teensy,
	 *             and is never "executed" by the computer.
	 *
	 *             The only forseeable bug could be a corrupt template being
	 *             used to compile code for teensy, which could either:
	 *             * fail during compilation of the device code, or
	 *             * execute malicious code on the device. This is a scary
	 *               outcome as it could potentially hurt the organism or even
	 *               worse.
	 *
	 * @warning    Make sure the contents of this json file are verified, as
	 *             this is an insecure function.
	 *
	 * @todo Implement a security check to easily verify conformance of
	 * generated device code with goals of optogentic experiments.
	 *
	 * @param[in]  fileName  The file name
	 * @param      json_str  The String which would hold the file contents
	 *
	 * @return     ``true`` if file was found, and contents were successfully read.
	 */
	bool readJSON(String fileName, String& json_str);

	/**
	 * @brief      Gets the template from a JSON file.
	 *
	 * @param      templateJSON  The ``String`` which would hold the JSON as string.
	 *
	 * @return     ``true`` if file was found, and contents were successfully read.
	 */
	bool getTemplateJSON(String& templateJSON);

	/**
	 * @brief      Gets the Cyclops Code Generation configuration from a JSON
	 *             file.
	 * @details    Information about the computer, and other code-generation
	 *             options can be provided via this file.
	 *
	 * @param      configJSON  The ``String`` which would hold the JSON as string.
	 *
	 * @return     ``true`` if file was found, and contents were successfully read.
	 */
	bool getConfig(String& configJSON);

	/**
	 * @brief      Creates the source-code from the _current_ Canvas configuration.
	 *
	 * @return     The _Generation Error Code_.
	 */
	virtual int createFromConfig() = 0;

	/**
	 * @brief      Generates the "Sources Header". Implemented by child classes.
	 * @sa         Teensy32::updateSourceHeader
	 * @return     ``true`` if successful.
	 */
	virtual bool updateSourceHeader() = 0;

	/**
	 * @brief      Generates the "Main Program". Implemented by child classes.
	 * @sa         Teensy32::updateMain
	 * @return     ``true`` if successful.
	 */
	virtual bool updateMain() = 0;

	/**
	 * @brief      Generates the "Makefile". Implemented by child classes.
	 * @sa         Teensy32::updateMakefile
	 * @return     ``true`` if successful.
	 */
	virtual bool updateMakefile() = 0;

	/**
	 * @brief      Retrieves the Source Name from the _global Source List_ using
	 *             the ``sourceListMap``and ``node_id`` to jump to the node's
	 *             _first Source_, and add then ``index`` to that.
	 *
	 * @param[in]  node_id  The CyclopsEditor (node) identifier
	 * @param[in]  index    The index of the ``node_id``'s Source Object
	 *
	 * @return     The name of the Source Object _(as provided in the Cyclops Sub-Plugin).
	 */
	const String& getSourceName(int node_id, int index);

	/**
	 * Each item of this dictionary is the source code of the Data Arrays for _a
	 * Source_ Object.
	 */
	std::map<String, String> sourceDataArrays;
	/**
	 * Each item of this dictionary is the source code of _a Source_ Object.
	 */
	IdCodename_SourceMap_t   sourceObjects;
	/**
	 * @brief      This dictionary maps each hook-id to the position of the
	 *             _first_ Source object (defined for that hook), in the global
	 *             Source List.
	 * @details    Scenario: _User code in sub-plugin that is on Hook-ID = 102
	 *             invokes an API call on Source #3._
	 *
	 *             Which Source object among those in the global Source List is
	 *             being reffered to here? This map can be used to find out,
	 *             since we know the position of the
	 *             "1st Source Object" in the global Source List.
	 *
	 * @note       In the pseudo-code, please note that order of
	 *             ``hookID.sources`` is also known -- it's the order specified
	 *             in the Cyclops SubPlugin code!
	 */
	std::map<int, int> sourceListMap;
	StringArray sourceList;

	CyclopsConfig oldConfig;

	/**
	 * @brief      Initially ``false``, it is set to ``true`` as soon as the
	 *             build Configuration is read, to avoid re-reading it.
	 * @see        getConfig
	 */
	static bool readBuildConfig;
};

} // NAMESPACE cyclops::code
} // NAMESPACE cyclops

#endif