#include "CyclopsCodeGen.h"

namespace cyclops{
namespace code{

CyclopsProgram::CyclopsProgram(const String& deviceName) : device(deviceName) 
{
	oldConfig = nullptr;
}

bool CyclopsProgram::create(CyclopsConfig* config)
{
	if (! (*oldConfig == *config)){
		oldConfig = config;
		return createFromConfig();
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
	File templateFile = getFileFromExeDir("cyclops_plugins/code-gen/" + device + ".json");
    std::cout << "Fecthing " << device << " template from `" << templateFile.getFullPathName() << "`\n";
    if (!templateFile.existsAsFile()){
    	/*
    	#if defined(__APPLE__)
	        std::cout << device << " templates not found! Expected @ Builds/Linux/build/cyclops_plugins/code-gen/" << device << ".json";
    	#else
        	std::cout << device << " templates not found! Expected @ Builds/Linux/build/cyclops_plugins/code-gen/" << device << ".json";
    	#endif
    	*/
        std::cout << device << " templates not found! Expected @ Builds/Linux/build/cyclops_plugins/code-gen/" << device << "\n";
        std::cout << "Try re-building the Cyclops sub-plugins.\n" << std::endl;
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
            std::cout << "Error in opening `Builds/Linux/build/cyclops_plugins/code-gen/" << device << "`\nCheck if you have permissions to this file.\n" << std::endl;
            jassert(false);
        }
        inFile.close();
    }
    templateJSON = "";
    return false;
}

















CyclopsConfig::CyclopsConfig( std::vector<HookInfo*>& hInfos
				 			, std::vector<std::bitset<CLSTIM_NUM_PARAMS> >& summaryList):
	hookInfos(hInfos),
	summaries(summaryList)
{
	
}

CyclopsConfig& CyclopsConfig::operator=(const CyclopsConfig& rhs)
{
	hookInfos = rhs.hookInfos;
	summaries = rhs.summaries;
	return *this;
}

bool operator==(const CyclopsConfig& lhs, const CyclopsConfig& rhs)
{
	int s = lhs.hookInfos.size();
	if (s == (int)rhs.hookInfos.size()){
		for (int i=0; i<s; i++){
			if ((lhs.hookInfos[i] == rhs.hookInfos[i]) &&
				(lhs.summaries[i] == rhs.summaries[i]))
				return true;
		}
	}
	return false;
}


CyclopsCodeGenerator::CyclopsCodeGenerator(CyclopsProgram *target_program) : program(target_program)
{

}

bool CyclopsCodeGenerator::generate(int& genError, CyclopsConfig& config)
{
	return true;
}

} // NAMESPACE cyclops::code
} // NAMESPACE cyclops
