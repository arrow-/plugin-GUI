#include "Teensy32.h"

namespace cyclops {
namespace code {

bool ProgramTeensy32::fetchTemplates = true;
StringArray ProgramTeensy32::setupTemplates;
StringArray ProgramTeensy32::loopTemplates;
StringArray ProgramTeensy32::sourceHeaderTemplates;
StringArray ProgramTeensy32::makefileTemplates;

ProgramTeensy32::ProgramTeensy32() : CyclopsProgram("teensy32"){
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
				for (auto& svar : *(templates["makefile"].getArray())){
					makefileTemplates.add(svar.toString());
				}
				fetchTemplates = false;
				DBG ("[success] Parse Teemsy32 JSON template\n");
				CoreServices::sendStatusMessage("Loaded Teensy32 templates file.");
			}
			else{
				DBG ("[failed] Parse Teemsy32 JSON template :(\n");
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
	std::cout << "Creating from config" << std::endl;
	if (updateSourceHeader()){
		std::cout << "src-header done" << std::endl;
		if (updateMain()){
			std::cout << "main done" << std::endl;
			if (updateMakefile())
				return 0;
			else
				return 3;
			std::cout << "make maybe done" << std::endl;
		}
		else
			return 2;
	}
	else
		return 1;
}

bool ProgramTeensy32::updateSourceHeader()
{
	DBG (".... making header");
	std::ostringstream result(sourceHeaderTemplates[0].toStdString(), std::ios_base::ate);
	for (int i=0; i < (int)oldConfig.summaries.size(); ++i){
		DBG (".... hook " << i);
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
	DBG (".... Need to make the global source list");
	
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
	
	std::cout << ".... Register the global list" << std::endl;
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

			result << "cyclops::Board ch" << channel << " (cyclops::Board::CH" << channel << ");" << std::endl;
			result << "cyclops::Waveform w_" << hInfo.nodeId << "_" << channel << " (&ch" << channel << ", &" << getSourceName(hInfo.nodeId, pInfo->initialSignal) << ");" << std::endl << std::endl;
		}
	}
	result << setupTemplates[1] << loopTemplates[0] << setupTemplates[2];
	
	main = String(result.str());
	return true;
}

bool ProgramTeensy32::updateMakefile()
{
	makefile = "";
	return true;
}

} // NAMESPACE cyclops::code
} // NAMESPACE cyclops
