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









class CyclopsProgram
{
public:
	CyclopsProgram(const String& deviceName);
	virtual ~CyclopsProgram();
	bool create(const CyclopsConfig& config, int& genError);
	bool build(int &buildError);

	const String device;
	String arduinoPath;

	int32 currentHash;
	String sourceHeader,
		   main,
		   makefile;
	bool oldConfigAvailable;

protected:
	inline File getFileFromExeDir(const String& pathFromExeDir);

	bool readJSON(String fileName, String& json_str);
	bool getTemplateJSON(String& templateJSON);
	bool getConfig(String& configJSON);

	virtual int createFromConfig() = 0;
	virtual bool updateSourceHeader() = 0;
	virtual bool updateMain() = 0;
	virtual bool updateMakefile() = 0;

	const String& getSourceName(int node_id, int index);
	/*
	virtual addPlugin(CyclopsPluginInfo*) = 0;
	virtual removePlugin(CyclopsPluginInfo*) = 0;
	*/
	std::map<String, String> sourceDataArrays;
	IdCodename_SourceMap_t   sourceObjects;
	std::map<int, int> sourceListMap;
	StringArray sourceList;

	CyclopsConfig oldConfig;

	static String code_gen_config;
};

} // NAMESPACE cyclops::code
} // NAMESPACE cyclops

#endif