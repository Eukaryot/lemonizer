*	Lemonizer -- Turns 68K code into lemonscript
*	Copyright (C) 2017-2026 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#include "pch.h"
#include "lemonizer/LemonizerCode.h"


namespace lemonizer
{
	const assembly::AssemblyCode* CodeAssembly::getAssemblyCode(const Code& code)
	{
		const CodeAssembly* codeAssembly = code.cast<CodeAssembly>();
		return (nullptr != codeAssembly) ? codeAssembly->mAssemblyCode : nullptr;
	}

	const assembly::AssemblyCode* CodeAssembly::getAssemblyCode(const Code& code, assembly::CodeType filterType)
	{
		const assembly::AssemblyCode* ac = getAssemblyCode(code);
		return (nullptr != ac && ac->mType == filterType) ? ac : nullptr;
	}


	lemon::Variable* CodeLemonTokenTree::getRegisterVariable(assembly::Register reg, const assembly::DataType& dataType)
	{
		size_t subIndex = (dataType.isSigned() ? 3 : 0);
		subIndex += ((dataType.mSize == assembly::DataType::Size::SIZE_8) ? 0 : (dataType.mSize == assembly::DataType::Size::SIZE_16) ? 1 : 2);
		const size_t index = (size_t)reg + subIndex * 16;
		return reinterpret_cast<lemon::Variable*>(index);
	}

	void CodeLemonTokenTree::splitRegisterVariable(const lemon::Variable* variable, assembly::Register& outReg, assembly::DataType& outDataType)
	{
		const size_t index = reinterpret_cast<size_t>(variable);
		outReg = (assembly::Register)(index % 16);
		const size_t sizeValue = (index / 16) % 3;
		outDataType.mSize = (sizeValue == 0) ? assembly::DataType::Size::SIZE_8 : (sizeValue == 1) ? assembly::DataType::Size::SIZE_16 : assembly::DataType::Size::SIZE_32;
		outDataType.mSign = (index >= 48) ? assembly::DataType::Sign::SIGNED : assembly::DataType::Sign::UNSIGNED;
	}
}
