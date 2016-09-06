#ifndef CL_CODEGEN_TEENSY32
#define CL_CODEGEN_TEENSY32

#include "CyclopsCodeGen.h"
#include <iomanip>
#include <sstream>

namespace cyclops {
namespace code {

class ProgramTeensy32 : protected CyclopsProgram
{
public:
	ProgramTeensy32();
private:
	bool createFromConfig();
	String getSourceHeader();
	String getMain();
	String getMakefile();

	static bool fetchTemplates;
	static StringArray setupTemplates,
					   loopTemplates,
					   sourceHeaderTemplates,
					   makefileTemplates;
};

} // NAMESPACE cyclops::code
} // NAMESPACE cyclops
#endif