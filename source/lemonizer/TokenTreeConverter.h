/*
*	Lemonizer -- Turns 68K code into lemonscript
*	Copyright (C) 2017-2026 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#pragma once

#include "assembly/AssemblyDefinitions.h"

#include <lemon/compiler/Token.h>

namespace lemon
{
	class BinaryOperationToken;
	class ConstantTokenExt;
	class ParenthesisToken;
	enum class Operator : uint8;
}


namespace lemonizer
{
	class Block;

	class TokenTreeConverter
	{
	public:
		static const lemon::DataTypeDefinition* getLemonDataType(const assembly::DataType& dataType);
		static const lemon::DataTypeDefinition* getLemonDataType(const assembly::DataType& dataType, bool isSigned);

		static bool needsParentheses(lemon::TokenPtr<lemon::StatementToken>& tokenPtr, lemon::Operator op, bool isRightSide);
		static lemon::ParenthesisToken& wrapWithParenthesis(lemon::TokenPtr<lemon::StatementToken>& tokenPtr);
		static void putWithOptionalParenthesis(lemon::TokenPtr<lemon::StatementToken>& tokenPtr, lemon::StatementToken& tokenToPut, bool addParenthesis);
		static bool resolveBinaryToPureAssignment(lemon::BinaryOperationToken& bot);

		static lemon::ConstantTokenExt& createConstantToken(lemon::TokenPtr<lemon::StatementToken>& tokenPtr, int64 value, const lemon::DataTypeDefinition* dataType);
		static void createLemonTokenTreeForParameter(lemon::TokenPtr<lemon::StatementToken>& tokenPtr, const assembly::Parameter& param, const assembly::DataType& dataType);
		static void tryConvertToTokenTree(Block& block, size_t pos);

		static void createTokenTreeForCondition(lemon::TokenPtr<lemon::StatementToken>& tokenPtr, assembly::Condition condition, const assembly::AssemblyCode& ac);
		static bool negateCondition(lemon::StatementToken& token);
	};
}
