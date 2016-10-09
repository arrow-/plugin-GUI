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




























bool CyclopsProgram::readBuildConfig = true;
String CyclopsProgram::arduinoPath = "";
String CyclopsProgram::deviceDir = "";
String CyclopsProgram::arduinoLibPath = "";

CyclopsProgram::CyclopsProgram( const String& deviceName) : device(deviceName)
                                                          , currentHash(0)
                                                          , sourceHeader("")
                                                          , main("")
                                                          , makefile("")
                                                          , oldConfigAvailable(false)
{
    if (readBuildConfig == true){
        String config_text;
        jassert (getConfig(config_text) == true);
        var code_gen_config = JSON::parse(config_text);
        arduinoPath    = code_gen_config["arduinoPath"].toString();
        deviceDir      = code_gen_config["deviceDir"].toString();
        arduinoLibPath = code_gen_config["arduinoLibPath"].toString();
    }
}

CyclopsProgram::~CyclopsProgram()
{
    
}

bool CyclopsProgram::create(const CyclopsConfig& config, int& genError)
{
    sourceDataArrays.clear();
    sourceObjects.clear();
    sourceListMap.clear();
    sourceList.clear();

/*    if (oldConfigAvailable){
        DBG ("old config is here :(");
    	if (oldConfig == config){
            DBG ("and old config is same as new!");
            return true;
        }
        DBG ("some things have changed");
    }
    else{
        DBG ("this is the first ever 'generation'...");
        oldConfigAvailable = true;
    }*/

    // If control reaches here:
    // some things have changed (or this is the first time)...
    oldConfig = config;

    genError = createFromConfig();
	if (genError == 0){
        File devdir(deviceDir);
        if (!devdir.exists())
            devdir.createDirectory();
        String srcdir_path = deviceDir+"/src";
        File srcdir(srcdir_path);
        if (!srcdir.exists())
            srcdir.createDirectory();
        // update CyclopsProgram::currentHash!
        currentHash = (sourceHeader + main).hashCode();

        std::ofstream fout;
        fout.open(srcdir_path.toStdString()+"/MySources.h");
        fout << sourceHeader << "\n";
        fout.close();
        fout.open(srcdir_path.toStdString()+"/control.ino");
        fout << "#define __CL_CG_" << device.toUpperCase() << "__\n";
        fout << "#define __CL_CG_PROG_HASH__ " << currentHash << "L\n\n" << main << "\n";
        fout.close();
        fout.open(deviceDir.toStdString()+"/Makefile");
        fout << makefile << "\n";
        fout.close();
        return true;
    }
    else{
        // something went wrong...
        switch (genError){
            case 1: std::cout << sourceHeader << "\n"; break;
            case 2: std::cout << main << "\n"; break;
            case 3: std::cout << makefile << "\n"; break;
        }
    }
	return false;
}

bool CyclopsProgram::build(int& buildError)
{
    String buildCommandBase = "make -C " + deviceDir + " VERBOSITY=0 hex";
    ChildProcess maker;
    std::cout << "Compiling ... " << std::flush;
    maker.start(buildCommandBase, ChildProcess::StreamFlags::wantStdOut|ChildProcess::StreamFlags::wantStdErr);
    /*
    // PROGRESS INDICATION
    char temp[200];
    String buffer;
    int tpos = -1, read = 0, status = 0;
    while (maker.isRunning()){
        int nowRead = maker.readProcessOutput(temp, 200);
        if (nowRead > 0){
            buffer += String(temp);
            do{
                int n_tpos = buffer.indexOf(tpos+1, "DONE ");
                if (n_tpos >= 0){
                    if (nowRead + read - n_tpos > 6){
                        std::cout << "DONE " << buffer[n_tpos+1] << std::endl;
                        status++;
                        tpos = n_tpos;
                    }
                }
                else
                    break;
            } while (1);
            read += nowRead;
        }
    }
    std::cout << buffer << std::endl;
    */
    while (maker.isRunning()){
        // Why are we polling the return code?
        // We could have simply written this:
        //     ```
        //     maker.waitForProcessToFinish(25000);
        //     buildError = maker.getExitCode();
        //     ```
        // Unfortuneately, that would not give us the exit code, since the child
        // process has terminated by the time we are fetching it's exit code.
        buildError = maker.getExitCode();
    }
    String build_output = maker.readAllProcessOutput();
    //DBG ("\n*CL:CODE* ~~~~~~~~~ BUILD OUTPUT ~~~~~~~~~\n" << build_output << "\n -------------------------------------------");
    //std::cout << build_output << std::endl;
    std::cout << " Status (" << buildError << ") ";
    return (buildError == 0);
}

bool CyclopsProgram::flash(int &flashError, int canvas_index)
{
    String buildCommandBase = "make -C " + deviceDir + " VERBOSITY=0 upload";
    //String buildCommandBase = "echo \"FOOBAR\"";
    ChildProcess flasher;
    std::cout << "Flashing ... " << std::flush;
    flasher.start(buildCommandBase, ChildProcess::StreamFlags::wantStdOut|ChildProcess::StreamFlags::wantStdErr);
    while (flasher.isRunning()){
        // Why are we polling the return code?
        // We could have simply written this:
        //     ```
        //     flasher.waitForProcessToFinish(25000);
        //     buildError = flasher.getExitCode();
        //     ```
        // Unfortuneately, that would not give us the exit code, since the child
        // process has terminated by the time we are fetching it's exit code.
        flashError = flasher.getExitCode();
    }
    flasher.readAllProcessOutput();
    std::cout << " Status (" << flashError << ") ";
    flashError = flasher.getExitCode();
    return (flashError == 0);
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

bool CyclopsProgram::readJSON(String fileName, String& json_str)
{
    File targetFile = getFileFromExeDir(fileName);
    if (!targetFile.existsAsFile()){
        /*
        #if defined(__APPLE__)
            std::cout << fileName << " not found! Expected @ Builds/Linux/build/cyclops_plugins/" << fileName << ".json";
        #else
            std::cout << fileName << " not found! Expected @ Builds/Linux/build/cyclops_plugins/" << fileName << ".json";
        #endif
        */
        std::cout << "FAILED!\nExpected @ " << targetFile.getFullPathName() << "\n";
        std::cout << "*CL:code* Try re-building the Cyclops sub-plugins.\n" << std::endl;
        jassert(false);
    }
    else{
        std::ifstream inFile(targetFile.getFullPathName().toStdString());
        if (inFile){
            DBG("Found! " <<"\n");
            std::ostringstream buf;
            buf << inFile.rdbuf();
            json_str = buf.str();
            return true;
        }
        else{
            std::cout << "FAILED!\nError in opening/reading " << targetFile.getFullPathName() << "`\nCheck if you have permissions to this file.\n" << std::endl;
            jassert(false);
        }
        inFile.close();
    }
    json_str = "";
    return false;
}

bool CyclopsProgram::getTemplateJSON(String& templateJSON)
{
    DBG("*CL:code* Fetching " << device << " code templates... ");
	return readJSON("cyclops_plugins/code_gen_templates/" + device + ".json", templateJSON);
}

bool CyclopsProgram::getConfig(String& configJSON)
{
    DBG("Fetching cyclops code-generation configuration file.. ");
    return readJSON("cyclops_plugins/config.json", configJSON);
}

const String& CyclopsProgram::getSourceName(int node_id, int index)
{
    return sourceList[sourceListMap[node_id] + index];
}

} // NAMESPACE cyclops::code
} // NAMESPACE cyclops
