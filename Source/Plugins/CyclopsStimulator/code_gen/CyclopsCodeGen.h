#ifndef CL_CODE_GENERATE_H
#define CL_CODE_GENERATE_H

#include "../CyclopsAPI/CyclopsAPI.h"
#include "../CyclopsCanvas.h"
#include <vector>
#include <map>
#include <bitset>

namespace cyclops{

class CyclopsSignal;
class HookInfo;

namespace code{

typedef std::pair<String, String> PluginCodename_Pair_t;
typedef std::pair<int, String> IdCodename_Pair_t;

typedef std::map<IdCodename_Pair_t, String> IdCodename_SourceMap_t;
typedef std::map<PluginCodename_Pair_t, String> PluginCodename_SourceMap_t;

/**
 * @brief      This collects all user configurations and inputs for the Code
 *             Generation module.
 * @details    An instance of this class is kept by CyclopsProgram, to quickly
 *             determine if there is any change in configuration.
 */
class CyclopsConfig
{
public:
	CyclopsConfig( std::vector<HookInfo*>& hInfos
				 , std::vector<std::bitset<CLSTIM_NUM_PARAMS> >& summaryList);
	CyclopsConfig& operator= (const CyclopsConfig& rhs);
	std::vector<HookInfo*> hookInfos;
	std::vector<std::bitset<CLSTIM_NUM_PARAMS> > summaries;

	//int timeOfBuild;
};

bool operator== (const CyclopsConfig& lhs, const CyclopsConfig& rhs);

class CyclopsProgram
{
public:
	CyclopsProgram(const String& deviceName);
	bool create(CyclopsConfig* config);

	const String device;

protected:
	inline File getFileFromExeDir(const String& pathFromExeDir);
	bool getTemplateJSON(String& templateJSON);

	virtual bool createFromConfig() = 0;
	virtual String getSourceHeader() = 0;
	virtual String getMain() = 0;
	virtual String getMakefile() = 0;
	virtual String getDeviceName() = 0;
	/*
	virtual addPlugin(CyclopsPluginInfo*) = 0;
	virtual removePlugin(CyclopsPluginInfo*) = 0;
	*/
	PluginCodename_SourceMap_t sourceDataArrays;
	IdCodename_SourceMap_t     sourceObjects;
	StringArray globalSrcList;
	
	CyclopsConfig *oldConfig;
};

class CyclopsCodeGenerator
{
public:
	CyclopsCodeGenerator(CyclopsProgram *target_program);
	virtual bool generate(int& genError, CyclopsConfig& config);

protected:
	String oldCode;
	CyclopsProgram *program;
};

} // NAMESPACE cyclops::code
} // NAMESPACE cyclops

#endif