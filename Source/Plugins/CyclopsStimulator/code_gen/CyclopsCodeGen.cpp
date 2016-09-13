#include "CyclopsCodeGen.h"

namespace cyclops{
namespace code{

CyclopsHookConfig::CyclopsHookConfig(int node_id, int LEDchannel, CyclopsPluginInfo* pInfo, std::vector<int>& selection) : nodeId(node_id), LEDChannel(LEDchannel), pluginInfo(pInfo), selectedSignals(selection)
{}

bool operator== (const CyclopsHookConfig& lhs, const CyclopsHookConfig& rhs)
{
    if (   lhs.nodeId == rhs.nodeId
        && lhs.LEDChannel == rhs.LEDChannel
        && lhs.pluginInfo == rhs.pluginInfo
        && lhs.selectedSignals == rhs.selectedSignals)
        return true;
    return false;
}

CyclopsConfig::CyclopsConfig() {}

CyclopsConfig::CyclopsConfig( std::vector<CyclopsHookConfig>& hInfos
                            , Array<std::bitset<CLSTIM_NUM_PARAMS> >& summaryList):
    hookInfos(hInfos),
    summaries(summaryList) {}

CyclopsConfig& CyclopsConfig::operator=(const CyclopsConfig& rhs)
{
    summaries = rhs.summaries;
    hookInfos = rhs.hookInfos;
    return *this;
}

bool operator==(const CyclopsConfig& lhs, const CyclopsConfig& rhs)
{
    int s = lhs.hookInfos.size();
    if (s == (int)rhs.hookInfos.size()){
        if (   lhs.hookInfos == rhs.hookInfos
            && lhs.summaries == rhs.summaries)
            return true;
    }
    return false;
}






























CyclopsProgram::CyclopsProgram(const String& deviceName) : device(deviceName)
                                                         , currentHash(0)
                                                         , sourceHeader("")
                                                         , main("")
                                                         , makefile("")
                                                         , oldConfigAvailable(false)
{

}

CyclopsProgram::~CyclopsProgram()
{
    
}

bool CyclopsProgram::create(const CyclopsConfig& config, int& genError)
{
    std::cout << "Starting program creation" << std::endl;
    if (oldConfigAvailable){
        std::cout << "oldie is here :(" << std::endl;
    	if (oldConfig == config/* && noSignalUpdated*/){
            std::cout << "and oldie is golden!" << std::endl;
            return true;
        }
        std::cout << "some things have changed" << std::endl;
    }
    else{
        std::cout << "(or this is the first time)..." << std::endl;
        oldConfigAvailable = true;
    }

    // If control reaches here:
    // some things have changed (or this is the first time)...
    oldConfig = config;

    std::cout << "oldConfig set" << std::endl;
    genError = createFromConfig();
	if (genError == 0){
        // DBG ("@@@-SRC-@@@\n" << sourceHeader << "\n@@@-MAIN-@@@\n" << main << "\n");
        // update CyclopsProgram::currentHash!
        currentHash = (sourceHeader + main).hashCode64();
    }
    else{
        // something went wrong...
        std::cout << "*CL:code* Failed to generate code for " << device << "\n";
        switch (genError){
            case 1: std::cout << sourceHeader << "\n"; break;
            case 2: std::cout << main << "\n"; break;
            case 3: std::cout << makefile << "\n"; break;
        }
    }
	return false;
}

// SUPPORT APPLE LATER
File CyclopsProgram::getFileFromExeDir(const String& pathFromExeDir)
{   /*
    #if defined(__APPLE__)
        File dir = File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("Application Support/open-ephys");
        if (!dir.isDirectory()) {
            dir.createDirectory();
        }
        return std::move(dir);
    #else
        return File::getSpecialLocation(File::currentExecutableFile).getParentDirectory();
    #endif
    */
	return File::getSpecialLocation(File::currentExecutableFile).getParentDirectory().getChildFile(pathFromExeDir);
}

bool CyclopsProgram::getTemplateJSON(String& templateJSON)
{
	File templateFile = getFileFromExeDir("cyclops_plugins/code_gen_templates/" + device + ".json");
    std::cout << "*CL:code* Fecthing " << device << " template from `" << templateFile.getFullPathName() << "`\n";
    if (!templateFile.existsAsFile()){
    	/*
    	#if defined(__APPLE__)
	        std::cout << device << " templates not found! Expected @ Builds/Linux/build/cyclops_plugins/code-gen/" << device << ".json";
    	#else
        	std::cout << device << " templates not found! Expected @ Builds/Linux/build/cyclops_plugins/code-gen/" << device << ".json";
    	#endif
    	*/
        std::cout << "*CL:code* " << device << " templates not found! Expected @ Builds/Linux/build/cyclops_plugins/code-gen/" << device << "\n";
        std::cout << "*CL:code* Try re-building the Cyclops sub-plugins.\n" << std::endl;
        jassert(false);
    }
    else{
        std::ifstream inFile(templateFile.getFullPathName().toStdString());
        if (inFile){
            DBG("Found " << device << " templates!\n");
            std::ostringstream buf;
            buf << inFile.rdbuf();
            templateJSON = buf.str();
            return true;
        }
        else{
            std::cout << "*CL:code* Error in opening `Builds/Linux/build/cyclops_plugins/code-gen/" << device << "`\nCheck if you have permissions to this file.\n" << std::endl;
            jassert(false);
        }
        inFile.close();
    }
    templateJSON = "";
    return false;
}

const String& CyclopsProgram::getSourceName(int node_id, int index)
{
    return sourceList[sourceListMap[node_id] + index];
}

} // NAMESPACE cyclops::code
} // NAMESPACE cyclops
