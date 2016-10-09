#include "Teensy32.h"

namespace cyclops {
namespace code {

bool ProgramTeensy32::fetchTemplates = true;
StringArray ProgramTeensy32::setupTemplates;
StringArray ProgramTeensy32::loopTemplates;
StringArray ProgramTeensy32::sourceHeaderTemplates;
StringArray ProgramTeensy32::makefileTemplates = {
    "VERBOSITY = 1\nVECHO_0 := @true\nVECHO_1 := @echo\nVECHO := $(VECHO_$(VERBOSITY))\n\nTARGET := ",
    "ARDUINOPATH := ",
    "# path location for Arduino libraries\nARDUINO_LIB_PATH := ",
    "\n# The teensy version to use, 30, 31, or LC\nTEENSY := 31\n\n# Set to 24000000, 48000000, or 96000000 to set CPU core speed\nTEENSY_CORE_SPEED := 96000000\n\n# Some libraries will require this to be defined\n# If you define this, you will break the default main.cpp\nARDUINO := 10611\n\n# configurable options\nOPTIONS := -DUSB_SERIAL -DLAYOUT_US_ENGLISH\n\n# directory to build in\nBUILDDIR := $(CURDIR)/build\n\n#************************************************************************\n# Location of Teensyduino utilities, Toolchain, and Arduino Libraries.\n# To use this makefile without Arduino, copy the resources from these\n# locations and edit the pathnames.  The rest of Arduino is not needed.\n#************************************************************************\n\n# path location for Teensy Loader, teensy_post_compile and teensy_reboot\nTOOLSPATH := $(ARDUINOPATH)/hardware/tools\n# path location for teensy_loader_cli\nTEENSY_CLI_PATH := teensy_loader_cli\n\nifeq ($(OS),Windows_NT)\n\t$(error What is Win Dose?)\nelse\n\tUNAME_S = $(shell uname -s)\n\tifeq ($(UNAME_S),Darwin)\n\t\tTOOLSPATH = /Applications/Arduino.app/Contents/Java/hardware/tools/\n\tendif\nendif\n\n# path location for Teensy 3 core\nifeq ($(TEENSY),30)\n\tCOREPATH = $(ARDUINOPATH)/hardware/teensy/avr/cores/teensy\nelse\n\tCOREPATH = $(ARDUINOPATH)/hardware/teensy/avr/cores/teensy3\nendif\n\n# path location for the arm-none-eabi compiler\nCOMPILERPATH := $(TOOLSPATH)/arm/bin\n\n#************************************************************************\n# Settings below this point usually do not need to be edited\n#************************************************************************\n\n# CPPFLAGS = compiler options for C and C++\nCPPFLAGS := -Wall -g -Os -mthumb -ffunction-sections -fdata-sections -nostdlib -MMD $(OPTIONS) -DTEENSYDUINO=130 -DF_CPU=$(TEENSY_CORE_SPEED) -Isrc -I$(COREPATH)\n\n# compiler options for C++ only\nCXXFLAGS := -std=gnu++0x -felide-constructors -fno-exceptions -fno-rtti\n\n# compiler options for C only\nCFLAGS :=\n\n# linker options\nLDFLAGS := -Os -Wl,--gc-sections,--defsym=__rtc_localtime=0 --specs=nano.specs -mthumb\n\n# additional libraries to link\nLIBS := -lm\n\n# compiler options specific to teensy version\nifeq ($(TEENSY), 30)\n\tCPPFLAGS += -D__MK20DX128__ -mcpu=cortex-m4\n\tMCU = mk20dx128\n\tLDSCRIPT = $(COREPATH)/$(MCU).ld\n\tLDFLAGS += -mcpu=cortex-m4 -T$(LDSCRIPT)\nelse\n\tifeq ($(TEENSY), 31)\n\t\tCPPFLAGS += -D__MK20DX256__ -mcpu=cortex-m4\n\t\tMCU = mk20dx256\n\t\tLDSCRIPT = $(COREPATH)/$(MCU).ld\n\t\tLDFLAGS += -mcpu=cortex-m4 -T$(LDSCRIPT)\n\telse\n\t\tifeq ($(TEENSY), LC)\n\t\t\tCPPFLAGS += -D__MKL26Z64__ -mcpu=cortex-m0plus\n\t\t\tMCU = mkl26z64\n\t\t\tLDSCRIPT = $(COREPATH)/$(MCU).ld\n\t\t\tLDFLAGS += -mcpu=cortex-m0plus -T$(LDSCRIPT)\n\t\t\tLIBS += -larm_cortexM0l_math\n\t\telse\n\t\t\t$(error Invalid setting for TEENSY)\n\t\tendif\n\tendif\nendif\n\n# set arduino define if given\nifdef ARDUINO\n\tCPPFLAGS += -DARDUINO=$(ARDUINO)\nelse\n\tCPPFLAGS += -DUSING_MAKEFILE\nendif\n\n# names for the compiler programs\nCC := $(abspath $(COMPILERPATH))/arm-none-eabi-gcc\nCXX := $(abspath $(COMPILERPATH))/arm-none-eabi-g++\nOBJCOPY := $(abspath $(COMPILERPATH))/arm-none-eabi-objcopy\nSIZE := $(abspath $(COMPILERPATH))/arm-none-eabi-size\n\n# automatically create lists of the sources and objects\nLC_FILES := $(wildcard $(ARDUINO_LIB_PATH)/*/*.c)\nLCPP_FILES := $(wildcard $(ARDUINO_LIB_PATH)/*/*.cpp)\nTC_FILES := $(wildcard $(COREPATH)/*.c)\nTCPP_FILES := $(wildcard $(COREPATH)/*.cpp)\nC_FILES := $(wildcard src/*.c)\nCPP_FILES := $(wildcard src/*.cpp)\nINO_FILES := $(wildcard src/*.ino)\n\n# include paths for libraries\nL_INC := $(foreach lib,$(filter %/, $(wildcard $(ARDUINO_LIB_PATH)/*/)), -I$(lib))\n\nCL_SOURCES := $(C_FILES:.c=.o) $(CPP_FILES:.cpp=.o) $(INO_FILES:.ino=.o)\nCORE_SOURCES := $(TC_FILES:.c=.o) $(TCPP_FILES:.cpp=.o)\nLIB_SOURCES := $(LC_FILES:.c=.o) $(LCPP_FILES:.cpp=.o)\n\nCL_OBJS := $(foreach src,$(CL_SOURCES), $(BUILDDIR)/$(src))\nCORE_OBJS := $(foreach src,$(CORE_SOURCES), $(BUILDDIR)/$(src))\nLIB_OBJS := $(foreach src,$(LIB_SOURCES), $(BUILDDIR)/$(src))\nALL_OBJS := $(CL_OBJS) $(CORE_OBJS) $(LIB_OBJS)\n\nall: hex\n\nbuild: $(TARGET).elf\n\nhex: $(TARGET).hex\n\npost_compile: $(TARGET).hex\n\t@$(abspath $(TOOLSPATH))/teensy_post_compile -file=\"$(basename $<)\" -path=$(CURDIR) -tools=\"$(abspath $(TOOLSPATH))\"\n\nreboot:\n\t-@$(abspath $(TOOLSPATH))/teensy_reboot\n\nupload_gui: post_compile reboot\n\nupload:\n\t@echo\n\t$(TEENSY_CLI_PATH)/teensy_loader_cli -mmcu=$(MCU) $(TARGET).hex -s -v\n\n$(BUILDDIR)/%.o: %.c\n\t$(VECHO) \"[CC]\\t$<\"\n\t@mkdir -p \"$(dir $@)\"\n\t@$(CC) $(CPPFLAGS) $(CFLAGS) $(L_INC) -o \"$@\" -c \"$<\"\n\n$(BUILDDIR)/%.o: %.cpp\n\t$(VECHO) \"[CXX]\\t$<\"\n\t@mkdir -p \"$(dir $@)\"\n\t@$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(L_INC) -o \"$@\" -c \"$<\"\n\n$(BUILDDIR)/%.o: %.ino\n\t$(VECHO) \"[CXX]\\t$<\"\n\t@mkdir -p \"$(dir $@)\"\n\t@$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(L_INC) -o \"$@\" -x c++ -include Arduino.h -c \"$<\"\n\ncyclops: $(CL_OBJS)\n\t@echo \"cyclops DONE 0\"\n\ncores: $(CORE_OBJS)\n\t@echo \"teensy-$(TEENSY) core DONE 1\"\n\nlibs: $(LIB_OBJS)\n\t@echo \"Arduino & CL libraries DONE 2\"\n\n$(TARGET).elf: cyclops cores libs $(LDSCRIPT)\n\t$(VECHO) \"[LD]\\t$@\"\n\t@$(CC) $(LDFLAGS) -o \"$@\" $(ALL_OBJS) $(LIBS)\n\n%.hex: %.elf\n\t$(VECHO) \"[HEX]\\t$@\\n\"\n\t@echo \"<SIZE>\"\n\t@$(SIZE) \"$<\"\n\t@$(OBJCOPY) -O ihex -R .eeprom \"$<\" \"$@\"\n\t@echo \"<EZIS>\"\n\t@echo \"HEX DONE 3\"\n\n# compiler generated dependency info\n-include $(OBJS:.o=.d)\n\nclean:\n\t@echo Cleaning...\n\t@rm -rf \"$(BUILDDIR)\"\n\t@rm -f \"$(TARGET).elf\" \"$(TARGET).hex\"\n\n"
};

ProgramTeensy32::ProgramTeensy32() : CyclopsProgram("teensy32")
{
	if (fetchTemplates){
		// read template file, and fetch content as string
		String templateJSON;
		if (getTemplateJSON(templateJSON)){
			// parse the JSON
			var templates;
			Result result = JSON::parse(templateJSON, templates);
			if (result.wasOk()){
				for (auto& svar : *(templates["sourceHeader"].getArray())){
					sourceHeaderTemplates.add(svar.toString());
				}
				for (auto& svar : *(templates["setup"].getArray())){
					setupTemplates.add(svar.toString());
				}
				for (auto& svar : *(templates["loop"].getArray())){
					loopTemplates.add(svar.toString());
				}
				fetchTemplates = false;
				DBG ("[success] Parse Teensy32 JSON template");
				CoreServices::sendStatusMessage("Loaded Teensy32 templates file.");
			}
			else{
				DBG ("[failed] Parse Teemsy32 JSON template :(");
				DBG (result.getErrorMessage());
				jassert(false);
			}
		}
		else{
			CoreServices::sendStatusMessage("Could not load Teensy32 templates file.");
		}
	}
}

int ProgramTeensy32::createFromConfig()
{
	std::cout << "Creating from config ... " << std::flush;
	if (updateSourceHeader()){
		//std::cout << "src-header done" << std::endl;
		if (updateMain()){
			//std::cout << "main done" << std::endl;
			if (updateMakefile()){
				//std::cout << "makefile done" << std::endl;
				return 0;
			}
			else
				return 3;
		}
		else
			return 2;
	}
	else
		return 1;
}

bool ProgramTeensy32::updateSourceHeader()
{
	//DBG (".... making header");
	std::ostringstream result(sourceHeaderTemplates[0].toStdString(), std::ios_base::ate);
	for (int i=0; i < (int)oldConfig.summaries.size(); ++i){
		//DBG (".... hook " << i);
		if (oldConfig.summaries[i].all()){
			const CyclopsHookConfig& hInfo = oldConfig.hookInfos[i];
			const CyclopsPluginInfo* pInfo = hInfo.pluginInfo;
			jassert(pInfo != nullptr);

			for (int i=0; i < pInfo->signalCount; ++i){
				int signalId = hInfo.selectedSignals[i];
				String codeName = pInfo->signalCodeNames[i];

				IdCodename_Pair_t     sourceObjectKey(hInfo.nodeId, codeName);
				const CyclopsSignal& signal = CyclopsSignal::getSignalByIndex(signalId);
				String sourceDataKey = signal.name;

				// making source data arrays
				if (sourceDataArrays.count(sourceDataKey) == 0){
					std::ostringstream vd("uint16_t " + sourceDataKey.toStdString() + "_vd[] = {" + std::to_string(signal.voltage[0]), std::ios_base::ate);
					std::ostringstream htd("uint32_t " + sourceDataKey.toStdString() + "_htd[] = {" + std::to_string(signal.holdTime[0]), std::ios_base::ate);
					for (int i=1; i < signal.size; ++i){
						vd << ", " << signal.voltage[i];
						htd << ", " << signal.holdTime[i];
					}
					vd << "};" << std::endl;
					htd << "};" << std::endl;
					sourceDataArrays[sourceDataKey] = vd.str() + htd.str();

					// add to result, right now.
					result << sourceDataArrays[sourceDataKey] << std::endl;
				}

				// making source objects
				std::ostringstream sobj("cyclops::storedSource ", std::ios_base::ate);
				sobj << codeName << "_" << hInfo.nodeId << " ( " << sourceDataKey.toStdString() << + "_vd," << std::endl;
				sobj << " " << sourceDataKey.toStdString() + "_htd," << std::endl;
				sobj << " " << signal.size << ",\n" << " ";
				switch (pInfo->allInitialMode){
					case (operationMode::LOOPBACK):
						sobj << "cyclops::operationMode::LOOPBACK";
						break;
					case (operationMode::ONE_SHOT):
						sobj << "cyclops::operationMode::ONE_SHOT";
						break;
					case (operationMode::N_SHOT):
						sobj << "cyclops::operationMode::N_SHOT";
						break;
				}
				//std::cout << hInfo.nodeId << "_" << sourceDataKey << " " << (int)(pInfo->allInitialMode) << std::endl;
				sobj << "\n);" << std::endl;
				sourceObjects[sourceObjectKey] = sobj.str();
			}	
		}
	}
	// Adding all source objects (auto-sorted by <nodeId, codeName>)
	for (IdCodename_SourceMap_t::iterator it = sourceObjects.begin(); it != sourceObjects.end(); ++it){
		result << it->second << std::endl;
	}
	//DBG (".... Need to make the global source list");
	
	result << sourceHeaderTemplates[1];
	result << "cyclops::Source* SourceList[] = {";
	
	int count=0;
	sourceListMap.clear();
	for (int i=0; i < (int)oldConfig.summaries.size(); ++i){
		if (oldConfig.summaries[i].all()) {
			const CyclopsHookConfig& hInfo = oldConfig.hookInfos[i];
			const CyclopsPluginInfo* pInfo = hInfo.pluginInfo;
			jassert(pInfo != nullptr);

			sourceListMap[hInfo.nodeId] = count;
			for (auto& codeName : pInfo->signalCodeNames){
				String sobj_name = String(codeName + "_") + String(hInfo.nodeId);
				result << " &" << sobj_name;

				if (count < (int)sourceObjects.size())
					result << ",";
				sourceList.add(sobj_name);
				result << std::endl;
				++count;
			}
		}
	}
	result << "};" << std::endl;
	
	//DBG (".... Register the global list");
	result << sourceHeaderTemplates[2] << "REGISTER_SOURCE_LIST(SourceList, " << sourceObjects.size() << ");" << std::endl;
	result << sourceHeaderTemplates[3];

/*	for (std::map<int, int>::iterator it = sourceListMap.begin(); it != sourceListMap.end(); it++){
		std::cout << it->first << " : " << it->second << std::endl;
	}*/
	
	sourceHeader = String(result.str());
	return true;
}

bool ProgramTeensy32::updateMain()
{
	std::ostringstream result(setupTemplates[0].toStdString(), std::ios_base::ate);
	for (int i=0; i < (int)oldConfig.summaries.size(); ++i){
		if (oldConfig.summaries[i].all()){
			const CyclopsHookConfig& hInfo = oldConfig.hookInfos[i];
			const CyclopsPluginInfo* pInfo = hInfo.pluginInfo;
			jassert(pInfo != nullptr);

			int channel = hInfo.LEDChannel;

			result << "cyclops::Board ch" << channel << " (cyclops::board::CH" << channel << ");" << std::endl;
			result << "cyclops::Waveform w_" << hInfo.nodeId << "_" << channel << " (&ch" << channel << ", &" << getSourceName(hInfo.nodeId, pInfo->initialSignal) << ");" << std::endl << std::endl;
		}
	}
	result << setupTemplates[1] << loopTemplates[0] << setupTemplates[2];
	
	main = String(result.str());
	return true;
}

bool ProgramTeensy32::updateMakefile()
{
	std::ostringstream result(makefileTemplates[0].toStdString(), std::ios_base::ate);
	result << "control\n";
	result << makefileTemplates[1] << arduinoPath << "\n";
	result << makefileTemplates[2] << arduinoLibPath << "\n";
	result << makefileTemplates[3];
	makefile = result.str();
	return true;
}

} // NAMESPACE cyclops::code
} // NAMESPACE cyclops
