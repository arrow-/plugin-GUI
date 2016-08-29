#ifndef CL_CODEGEN_TEENSY32
#define CL_CODEGEN_TEENSY32

#include "../CyclopsCodeGen.h"
#include <iomanip>
#include <sstream>

namespace cyclops {
namespace code {

class ProgramTeensy32 : CyclopsProgram
{
public:
	ProgramTeensy32();
	bool createFromPluginInfo(std::vector<CyclopsPluginInfo*>& pluginInfoList);
	std::string getSourceHeader();
	std::string getMain();
	std::string getMakefile();
private:
	static bool fetchTemplates;
	static std::string setupTemplate,
					   loopTemplate,
					   makefileTemplate;
};

} // NAMESPACE cyclops::code
} // NAMESPACE cyclops
#endif