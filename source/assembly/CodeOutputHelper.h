/*
*	Lemonizer -- Turns 68K code into lemonscript
*	Copyright (C) 2017-2026 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#pragma once

#include "assembly/AssemblyDefinitions.h"


namespace assembly
{
	class CodeOutputHelper
	{
	public:
		static const std::string& getRegisterName(Register reg);
		static const std::string& getRegisterName(ExtRegister reg);
		static const std::string& getConditionShortname(Condition condition);
		static const std::string& getConditionLongname(Condition condition);
		static const std::string& getSizePostfix(DataType::Size size);
		static const std::string& getDataTypePostfix(const DataType& dataType);
		static std::vector<Register> getRegisterListFromBitmask(uint32 bitmask);
	};
}
