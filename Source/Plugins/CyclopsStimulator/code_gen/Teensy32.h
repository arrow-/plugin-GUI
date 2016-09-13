#ifndef CL_CODEGEN_TEENSY32_H
#define CL_CODEGEN_TEENSY32_H

#include "CyclopsCodeGen.h"
#include <iomanip>
#include <sstream>

namespace cyclops {
namespace code {

class ProgramTeensy32 : public CyclopsProgram
{
public:
	ProgramTeensy32();
private:
	int createFromConfig();
	bool updateSourceHeader();
	bool updateMain();
	bool updateMakefile();

	static bool fetchTemplates;
	static StringArray setupTemplates,
					   loopTemplates,
					   sourceHeaderTemplates,
					   makefileTemplates;
};

} // NAMESPACE cyclops::code
} // NAMESPACE cyclops
#endif