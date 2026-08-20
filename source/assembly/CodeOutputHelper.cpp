/*
*	Lemonizer -- Turns 68K code into lemonscript
*	Copyright (C) 2017-2026 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#include "pch.h"
#include "assembly/CodeOutputHelper.h"


namespace assembly
{

	const std::string& CodeOutputHelper::getRegisterName(Register reg)
	{
		return getRegisterName((ExtRegister)reg);
	}

	const std::string& CodeOutputHelper::getRegisterName(ExtRegister reg)
	{
		static const std::string registerNames[] = { "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "" };
		return registerNames[(size_t)reg];
	}

	const std::string& CodeOutputHelper::getConditionShortname(Condition condition)
	{
		static const std::string shortnames[] = { "b", "ae", "ov", "!ov", "!eq", "eq", "neg", "!neg", "l", "ge", "a", "be", "g", "le", "" };
		return shortnames[(size_t)condition];
	}

	const std::string& CodeOutputHelper::getConditionLongname(Condition condition)
	{
		static const std::string longnames[] = { "below.u", "aboveEqual.u", "overflow", "!overflow", "!equal", "equal", "negative", "!negative",
												 "less.s", "greaterEqual.s", "above.u", "belowEqual.u", "greater.s", "lessEqual.s", "" };
		return longnames[(size_t)condition];
	}

	const std::string& CodeOutputHelper::getSizePostfix(DataType::Size size)
	{
		static const std::string postfix[5] = { "", ".8", ".16", "", ".32" };
		return postfix[(size_t)size];
	}

	const std::string& CodeOutputHelper::getDataTypePostfix(const DataType& dataType)
	{
		static const std::string postfixSigned[5]   = { "", ".s8", ".s16", "", ".s32" };
		static const std::string postfixUnsigned[5] = { "", ".u8", ".u16", "", "" };
		return dataType.isSigned() ? postfixSigned[(size_t)dataType.mSize] : postfixUnsigned[(size_t)dataType.mSize];
	}

	std::vector<Register> CodeOutputHelper::getRegisterListFromBitmask(uint32 bitmask)
	{
		std::vector<Register> output;
		const bool reverseTraversal = (bitmask & 0x80000000) != 0;
		for (uint32 i = 0; i < 16; ++i)
		{
			if ((bitmask >> i) & 0x01)
			{
				output.push_back((Register)(reverseTraversal ? (15-i) : i));
			}
		}
		return output;
	}

}
