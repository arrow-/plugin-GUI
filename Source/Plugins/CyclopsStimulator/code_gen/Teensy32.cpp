#include "Teensy32.h"

namespace cyclops {
namespace code {

bool ProgramTeensy32::fetchTemplates = true;

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
			}
			else{
				std::cout << "Failed to parse Teemsy32 JSON template :(\n" << std::endl;
				DBG(result.getErrorMessage());
				jassert(false);
			}
		}
		else{
			CoreServices::sendStatusMessage("Could not load Teensy32 templates file.");
		}
	}
}

bool ProgramTeensy32::createFromConfig()
{
	String sourceHeader = getSourceHeader();
	String mainFile = getMain();
	return true;
}

String ProgramTeensy32::getSourceHeader()
{
	std::ostringstream result(sourceHeaderTemplates[0].toStdString(), std::ios_base::ate);
	for (int i=0; i < (int)oldConfig->summaries.size(); ++i){
		if (oldConfig->summaries[i].all()){
			const HookInfo*          hInfo = oldConfig->hookInfos[i];
			const CyclopsPluginInfo* pInfo = hInfo->pluginInfo;
			int node_id = hInfo->nodeId;

			for (int i=0; i < pInfo->sourceCount; ++i){
				int signalId = hInfo->selectedSignals[i];
				String codeName = pInfo->sourceCodeNames[i];

				IdCodename_Pair_t     sourceObjectKey(node_id, codeName);
				PluginCodename_Pair_t sourceDataKey(pInfo->Name, codeName);
				const CyclopsSignal& signal = CyclopsSignal::getSignalByIndex(signalId);

				// making source data arrays
				if (sourceDataArrays.count(sourceDataKey) == 0){
					std::ostringstream vd("uint16_t " + sourceDataKey.first.toStdString() + "_" + sourceDataKey.second.toStdString() + "_vd[] = {" + std::to_string(signal.voltage[0]), std::ios_base::ate);
					std::ostringstream htd("uint32_t " + sourceDataKey.first.toStdString() + "_" + sourceDataKey.second.toStdString() + "_htd[] = {" + std::to_string(signal.holdTime[0]), std::ios_base::ate);
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
				sobj << codeName << "_" << node_id << " ( " << sourceDataKey.first.toStdString() << "_" << sourceDataKey.second.toStdString() + "_vd," << std::endl;
				sobj << sourceDataKey.first.toStdString() << "_" << sourceDataKey.second.toStdString() + "_htd," << std::endl;
				sobj << signal.size << "," << std::endl << "cyclops::source::LOOPBACK );" << std::endl;
				sourceObjects[sourceObjectKey] = sobj.str();
			}	
		}
	}
	// Adding all source objects (auto-sorted by <nodeId, codeName>)
	for (IdCodename_SourceMap_t::iterator it = sourceObjects.begin(); it != sourceObjects.end(); ++it){
		result << it->second << std::endl;
	}
	// Need to make the global source list
	result << sourceHeaderTemplates[1];
	result << "cyclops::Source* SourceList[] = { &" << sourceObjects.begin()->first.second << "_" << sourceObjects.begin()->first.first;
	IdCodename_SourceMap_t::iterator it = sourceObjects.begin();
	for (++it; it != sourceObjects.end(); ++it){
		result << "," << std::endl << "&" << it->first.second << "_" << String(it->first.first);
	}
	result << " };" << std::endl;
	// Register the global list
	result << sourceHeaderTemplates[2] << "REGISTER_SOURCE_LIST(SourceList, " << sourceObjects.size() << ");" << std::endl;
	result << sourceHeaderTemplates[3];
	return String(result.str());
}

String ProgramTeensy32::getMain()
{
	std::ostringstream result(setupTemplates[0].toStdString(), std::ios_base::ate);
	for (int i=0; i < (int)oldConfig->summaries.size(); ++i){
		if (oldConfig->summaries[i].all()){
			const HookInfo*          hInfo = oldConfig->hookInfos[i];
/*			const CyclopsPluginInfo* pInfo = hInfo->pluginInfo;
			int node_id = hInfo->nodeId;*/
			int channel = hInfo->LEDChannel;

			result << "cyclops::Board ch" << channel << " (cyclops::Board::CH" << channel << ");" << std::endl;
			result << "cyclops::Waveform w" << channel << " (&ch" << channel << ", &????" << ");" << std::endl << std::endl;
		}
	}
	result << setupTemplates[1] << loopTemplates[0] << setupTemplates[2];
	return String(result.str());
}

String ProgramTeensy32::getMakefile()
{
	return "";
}

} // NAMESPACE cyclops::code
} // NAMESPACE cyclops
