#include "CyclopsCodeGen.h"


CyclopsProgram::CyclopsProgram() : oldConfig(nullptr);



CyclopsConfig::CyclopsConfig( std::vector<HookInfo*>& hInfos
				 			, std::vector<std::bitset<CLSTIM_NUM_PARAMS> >& summaryList):
	hookInfos(hInfos),
	summaries(summaryList)
{
	
}

bool CyclopsConfig::operator==(CyclopsConfig& lhs, CyclopsConfig& rhs)
{
	int s = lhs.hookInfos.size();
	if (s == rhs.hookInfos.size()){
		for (int i=0; i<s; i++){
			if ((lhs.hookInfos[i] == rhs.hookInfos[i]) &&
				(lhs.summaries[i] == rhs.summaries[i]))
				return true;
		}
	}
	return false;
}


template <class program_T>
CyclopsCodeGenerator::CyclopsCodeGenerator()
{

}

template <class program_T>
bool CyclopsCodeGenerator::generate(int& genError, std::vector<std::bitset<CLSTIM_NUM_PARAMS> >& summaries)
{

}