/*
*	Lemonizer -- turns 68K code into lemon script
*	Copyright (C) 2021 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#pragma once

#include "assembly/AssemblyDefinitions.h"


namespace assembly
{
	class Disassembler
	{
	public:
		static bool getStringRepresentation(const AssemblyCode& code, std::string& outInstruction, std::string& outParam1, std::string& outParam2);
	};
}
