#include "Teensy32.h"

bool ProgramTeensy32::fetchTemplates = true;

std::string ProgramTeensy32::sourceHeaderTemplates[] = {"#ifndef CL_MY_SOURCES_H\n#define CL_MY_SOURCES_H\n//\n// This file contains the globals definitions of the Source objects. Just include\n// this file into your main `.ino` script, and you'll get access to the objects\n// here.\n//\n", "/* You must register these sources with the library, by:\n *\n * 1. making a globally scoped array of pointers to the objects.\n * 2. assign the global array to the ``globalSourceList_ptr``\n *\n * Only the registered sources are guaranteed to work when using the RPC,\n * especially when using the OE GUI.\n */\n"};
std::string ProgramTeensy32::setupTemplate = "";
std::string ProgramTeensy32::loopTemplate = "";
std::string ProgramTeensy32::makefileTemplate = "";

ProgramTeensy32::ProgramTeensy32(){
	if (fetchTemplates){
		// getTemplates();
		fetchTemplates = false;
	}
}

bool ProgramTeensy32::createFromPluginInfo(CyclopsConfig* config)
{
	for (auto& summary : config.summaries){
		if (summary.test(CLSTIM_MAP_CH) && summary.test	(CLSTIM_MAP_SIG)){
			// completely configured for code generation
			// LED output port might not be selected
			if (summary.test(CLSTIM_MAP_LED)){
				// LED too has been selected
			}
			else{

			}
		}
		// else don't make code.
	}
}

std::string ProgramTeensy32::getSourceHeader()
{
	
}

std::string ProgramTeensy32::getMain()
{

}

std::string ProgramTeensy32::getMakefile()
{

}
