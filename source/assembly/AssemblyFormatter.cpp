/*
*	Lemonizer -- turns 68K code into lemon script
*	Copyright (C) 2021 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#include "pch.h"
#include "assembly/AssemblyFormatter.h"
#include "assembly/AssemblyDefinitions.h"
#include "assembly/AssemblyOpcodeTable.h"
#include "assembly/Disassembler.h"
#include "builder/RomContent.h"


namespace assembly
{

	void Formatter::formatCode(const AssemblyCode& code, uint32 address, Output& output)
	{
		// Program code for this opcode
		if (code.mType != assembly::CodeType::INVALID)
		{
			std::string instructionString;
			std::string param1String;
			std::string param2String;
			if (assembly::Disassembler::getStringRepresentation(code, instructionString, param1String, param2String))
			{
				// Add instruction
				output.addToken(instructionString);

				// Add parameters
				if (!param1String.empty())
				{
					output.addToken(Token::TABULATOR, 10);

					if (param1String[0] == '@')
					{
						output.addToken(Token::JUMP_TARGET, param1String.substr(1));
					}
					else
					{
						output.addToken(param1String);
					}

					if (!param2String.empty())
					{
						output.addToken(", " + param2String);
					}
				}
			}
			else
			{
				output.addToken("<?>");
			}
		}
		else
		{
			const uint16 opcode = RomContent::instance().read16(address);
			const assembly::OpcodeType opcodeType = assembly::OpcodeTable::mOpcodeTable[opcode];
			const std::string& opcodeName = assembly::OpcodeTable::getOpcodeNameByType(opcodeType);

			output.addToken("unknown " + opcodeName);
		}
	}

}
