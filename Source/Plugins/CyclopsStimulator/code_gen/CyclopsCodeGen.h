#ifndef CL_CODE_GENERATE_H
#define CL_CODE_GENERATE_H

// Attempting without JUCE objects!
// JUCE headers are included in CyclopsCanvas and CyclopsAPI though...
#include "../CyclopsAPI.h"
#include "../CyclopsCanvas.h"
#include <vector>
#include <map>
#include <bitset>

namespace cyclops{

class CyclopsSignal;
class HookInfo;

namespace code{

typedef std::map<std::string, std::string> ssMap;

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
	static bool operator== (const CyclopsConfig& lhs, const CyclopsSignal& rhs);
	std::vector<HookInfo*> hookInfos;
	std::vector<std::bitset<CLSTIM_NUM_PARAMS> > summaries;

	//int timeOfBuild;
}

class CyclopsProgram
{
public:
	CyclopsProgram();
	virtual bool createFromConfig(CyclopsConfig* config) = 0;
	virtual std::string getSourceHeader() = 0;
	virtual std::string getMain() = 0;
	virtual std::string getMakefile() = 0;
	/*
	virtual addPlugin(CyclopsPluginInfo*) = 0;
	virtual removePlugin(CyclopsPluginInfo*) = 0;
	*/
	ssMap sourceDataArrays,
		  sourceObjects;
	std::vector<std::string> globalSrcList;
private:
	CyclopsConfig oldConfig;
};

template <class program_T>
class CyclopsCodeGenerator
{
public:
	CyclopsCodeGenerator();
	virtual bool generate(int& genError, CyclopsConfig& config);

protected:
	std::string oldCode;
	program_T program;
};

} // NAMESPACE cyclops::code
} // NAMESPACE cyclops

#endif