/*
*	Lemonizer -- Turns 68K code into lemonscript
*	Copyright (C) 2017-2026 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#include "pch.h"
#include "lemonizer/TokenTreeConverter.h"
#include "lemonizer/LemonizerCode.h"
#include "builder/KnowledgeBase.h"


namespace lemonizer
{
	namespace detail
	{

		// Note: This function is not unused at the moment
		assembly::DataType getDataTypeForConstant(int32 value)
		{
			if (value >= 0)
			{
				if (value <= 0xff)
				{
					return assembly::DataType::u8;
				}
				else if (value <= 0xffff)
				{
					return assembly::DataType::u16;
				}
				else
				{
					return assembly::DataType::u32;
				}
			}
			else
			{
				return assembly::DataType::s32;
			}
		}

		assembly::DataType getDataTypeForConstantSigned(int32 value)
		{
			if (value >= 0)
			{
				if (value <= 0x7f)
				{
					return assembly::DataType::s8;
				}
				else if (value <= 0x7fff)
				{
					return assembly::DataType::s16;
				}
				else
				{
					return assembly::DataType::s32;
				}
			}
			else
			{
				return assembly::DataType::s32;
			}
		}

		bool isBitwiseOperator(lemon::Operator op)
		{
			switch (op)
			{
				case lemon::Operator::ASSIGN_AND:
				case lemon::Operator::ASSIGN_OR:
				case lemon::Operator::ASSIGN_XOR:
				case lemon::Operator::ASSIGN_SHIFT_LEFT:
				case lemon::Operator::ASSIGN_SHIFT_RIGHT:
				case lemon::Operator::BINARY_AND:
				case lemon::Operator::BINARY_OR:
				case lemon::Operator::BINARY_XOR:
				case lemon::Operator::BINARY_SHIFT_LEFT:
				case lemon::Operator::BINARY_SHIFT_RIGHT:
				case lemon::Operator::UNARY_BITNOT:
					return true;
			}
			return false;
		}

		bool isLogicalOperator(lemon::Operator op)
		{
			switch (op)
			{
				case lemon::Operator::LOGICAL_AND:
				case lemon::Operator::LOGICAL_OR:
					return true;
			}
			return false;
		}

		bool tryReplaceWithDefine(lemon::TokenPtr<lemon::StatementToken>& tokenPtr, const KnowledgeBase::DefineInfo* defineInfoSigned, const KnowledgeBase::DefineInfo* defineInfoUnsigned, const assembly::DataType& dataType)
		{
			// Replace the token (usually a memory access) with either of the defines, depending on which is available and what sign might be required
			if (nullptr == defineInfoUnsigned && nullptr == defineInfoSigned)
				return false;

			const KnowledgeBase::DefineInfo* defineInfo = nullptr;
			switch (dataType.mSign)
			{
				default:
				case assembly::DataType::Sign::UNSIGNED:
				{
					defineInfo = (nullptr != defineInfoUnsigned) ? defineInfoUnsigned : defineInfoSigned;
					break;
				}

				case assembly::DataType::Sign::SIGNED:
				{
					defineInfo = (nullptr != defineInfoSigned) ? defineInfoSigned : defineInfoUnsigned;
					break;
				}
			}

			const bool castRequired = (defineInfo == defineInfoSigned && dataType.mSign == assembly::DataType::Sign::UNSIGNED) || (defineInfo == defineInfoUnsigned && dataType.mSign == assembly::DataType::Sign::SIGNED);
			if (castRequired)
			{
				lemon::ValueCastToken& valueCastToken = tokenPtr.create<lemon::ValueCastToken>();
				valueCastToken.mDataType = TokenTreeConverter::getLemonDataType(dataType);

				lemon::IdentifierToken& identifierToken = valueCastToken.mArgument.create<lemon::IdentifierToken>();
				identifierToken.mName = defineInfo->mName;
				identifierToken.mDataType = TokenTreeConverter::getLemonDataType(dataType, (nullptr != defineInfoSigned));
			}
			else
			{
				lemon::IdentifierToken& identifierToken = tokenPtr.create<lemon::IdentifierToken>();
				identifierToken.mName = defineInfo->mName;
				identifierToken.mDataType = TokenTreeConverter::getLemonDataType(dataType, (nullptr != defineInfoSigned));
			}

			return true;
		}

		lemon::Operator getOperationForCondition(assembly::Condition condition, assembly::DataType::Sign& outTypeSign, bool comparingToZero)
		{
			enum class OperatorSign
			{
				INVALID,
				UNSIGNED,
				SIGNED,
				BOTH,
				NEGATIVE
			};

			struct OperatorTranslation
			{
				lemon::Operator mOperator;
				OperatorSign mOperatorSign;
			};

			static const OperatorTranslation operatorTranslations[] =
			{
				{ lemon::Operator::COMPARE_LESS,			 OperatorSign::UNSIGNED	},		// CS == "below.u"
				{ lemon::Operator::COMPARE_GREATER_OR_EQUAL, OperatorSign::UNSIGNED	},		// CC == "aboveEqual.u"
				{ lemon::Operator::_INVALID,				 OperatorSign::INVALID	},		// VS == "overflow"
				{ lemon::Operator::_INVALID,				 OperatorSign::INVALID	},		// VC == "!overflow"
				{ lemon::Operator::COMPARE_NOT_EQUAL,		 OperatorSign::BOTH		},		// NE == "!equal"
				{ lemon::Operator::COMPARE_EQUAL,			 OperatorSign::BOTH		},		// EQ == "equal"
				{ lemon::Operator::COMPARE_LESS,			 OperatorSign::NEGATIVE	},		// MI == "negative"
				{ lemon::Operator::COMPARE_GREATER_OR_EQUAL, OperatorSign::NEGATIVE	},		// PL == "!negative"
				{ lemon::Operator::COMPARE_LESS,			 OperatorSign::SIGNED	},		// LT == "less.s"
				{ lemon::Operator::COMPARE_GREATER_OR_EQUAL, OperatorSign::SIGNED	},		// GE == "greaterEqual.s"
				{ lemon::Operator::COMPARE_GREATER,			 OperatorSign::UNSIGNED	},		// HI == "above.u"
				{ lemon::Operator::COMPARE_LESS_OR_EQUAL,	 OperatorSign::UNSIGNED	},		// LS == "belowEqual.u"
				{ lemon::Operator::COMPARE_GREATER,			 OperatorSign::SIGNED	},		// GT == "greater.s"
				{ lemon::Operator::COMPARE_LESS_OR_EQUAL,	 OperatorSign::SIGNED	},		// LE == "lessEqual.s"
				{ lemon::Operator::_INVALID,				 OperatorSign::INVALID	}		// NONE
			};

			const OperatorTranslation& translation = operatorTranslations[(size_t)condition];

			switch (translation.mOperatorSign)
			{
				case OperatorSign::UNSIGNED:
					outTypeSign = assembly::DataType::Sign::UNSIGNED;
					break;

				case OperatorSign::SIGNED:
					outTypeSign = assembly::DataType::Sign::SIGNED;
					break;

				case OperatorSign::BOTH:
					outTypeSign = assembly::DataType::Sign::UNSPECIFIED;
					break;

				case OperatorSign::NEGATIVE:
					// When comparing to zero, the value has to be interpreted as signed
					outTypeSign = comparingToZero ? assembly::DataType::Sign::SIGNED : assembly::DataType::Sign::UNSIGNED;
					break;

				default:
					return lemon::Operator::_INVALID;
			}

			return translation.mOperator;
		}

	}	// namespace detail



	const lemon::DataTypeDefinition* TokenTreeConverter::getLemonDataType(const assembly::DataType& dataType, bool isSigned)
	{
		switch (dataType.mSize)
		{
			case assembly::DataType::Size::SIZE_8:   return isSigned ? &lemon::PredefinedDataTypes::INT_8  : &lemon::PredefinedDataTypes::UINT_8;
			case assembly::DataType::Size::SIZE_16:  return isSigned ? &lemon::PredefinedDataTypes::INT_16 : &lemon::PredefinedDataTypes::UINT_16;
			case assembly::DataType::Size::SIZE_32:  return isSigned ? &lemon::PredefinedDataTypes::INT_32 : &lemon::PredefinedDataTypes::UINT_32;
		}
		return &lemon::PredefinedDataTypes::UINT_32;
	}

	bool TokenTreeConverter::needsParentheses(lemon::TokenPtr<lemon::StatementToken>& tokenPtr, lemon::Operator op, bool isRightSide)
	{
		if (tokenPtr->isA<lemon::BinaryOperationToken>())
		{
			const lemon::Operator innerOperator = tokenPtr->as<lemon::BinaryOperationToken>().mOperator;
			const uint8 prio1 = lemon::OperatorHelper::getOperatorPriority(innerOperator);
			const uint8 prio2 = lemon::OperatorHelper::getOperatorPriority(op);

			// Lower values mean higher priority here, so this actually means that we need parentheses if the inner has a higher priority
			if (prio1 > prio2)
			{
				return true;
			}
			else if (prio1 < prio2)
			{
				// Add parentheses if a bitwise operator is involved, just for the sake of clarity
				if (detail::isBitwiseOperator(innerOperator) || detail::isBitwiseOperator(op))
					return true;

				// The same is true if both are logical operators, as their priorities can be confusing
				if (detail::isLogicalOperator(innerOperator) && detail::isLogicalOperator(op))
					return true;
			}
		}
		return false;
	}

	const lemon::DataTypeDefinition* TokenTreeConverter::getLemonDataType(const assembly::DataType& dataType)
	{
		return getLemonDataType(dataType, dataType.isSigned());
	}

	lemon::ConstantTokenExt& TokenTreeConverter::createConstantToken(lemon::TokenPtr<lemon::StatementToken>& tokenPtr, int64 value, const lemon::DataTypeDefinition* dataType)
	{
		lemon::ConstantTokenExt& constantToken = tokenPtr.create<lemon::ConstantTokenExt>();
		constantToken.mValue.set(value);
		constantToken.mDataType = dataType;
		return constantToken;
	}

	lemon::ParenthesisToken& TokenTreeConverter::wrapWithParenthesis(lemon::TokenPtr<lemon::StatementToken>& tokenPtr)
	{
		lemon::TokenPtr<lemon::StatementToken> tokenToWrap = tokenPtr;
		lemon::ParenthesisToken& pt = tokenPtr.create<lemon::ParenthesisToken>();
		pt.mContent.add(*tokenToWrap);
		pt.mParenthesisType = lemon::ParenthesisType::PARENTHESIS;
		pt.mDataType = tokenToWrap->mDataType;
		return pt;
	}

	void TokenTreeConverter::putWithOptionalParenthesis(lemon::TokenPtr<lemon::StatementToken>& tokenPtr, lemon::StatementToken& tokenToPut, bool addParenthesis)
	{
		if (addParenthesis)
		{
			lemon::ParenthesisToken& pt1 = tokenPtr.create<lemon::ParenthesisToken>();
			pt1.mContent.add(tokenToPut);
			pt1.mDataType = tokenPtr->mDataType;
		}
		else
		{
			tokenPtr = tokenToPut;
		}
	}

	bool TokenTreeConverter::resolveBinaryToPureAssignment(lemon::BinaryOperationToken& bot)
	{
		const lemon::Operator binaryOperator = lemon::OperatorHelper::getBinaryForAssign(bot.mOperator);
		if (binaryOperator == lemon::Operator::_INVALID)
			return false;

		// Add a new (inner) binary operation on the right side
		lemon::TokenPtr<lemon::StatementToken> newToken;
		lemon::BinaryOperationToken& innerBot = newToken.create<lemon::BinaryOperationToken>();
		innerBot.mDataType = bot.mLeft->mDataType;
		innerBot.mOperator = binaryOperator;

		// The left side of the inner operation is just the original left side once again
		innerBot.mLeft = bot.mLeft;

		// The right side of the inner operation is the old right side, with added parenthesis if needed
		const bool needsParentheses = TokenTreeConverter::needsParentheses(bot.mRight, binaryOperator, true);
		if (needsParentheses)
		{
			lemon::ParenthesisToken& pt = innerBot.mRight.create<lemon::ParenthesisToken>();
			pt.mContent.add(*bot.mRight);
			pt.mParenthesisType = lemon::ParenthesisType::PARENTHESIS;
			pt.mDataType = bot.mRight->mDataType;
		}
		else
		{
			innerBot.mRight = bot.mRight;
		}

		// Assign as new right side of the outer binary operation
		bot.mOperator = lemon::Operator::ASSIGN;
		bot.mRight = newToken;
		return true;
	}

	void TokenTreeConverter::createLemonTokenTreeForParameter(lemon::TokenPtr<lemon::StatementToken>& tokenPtr, const assembly::Parameter& param, const assembly::DataType& dataType)
	{
		switch (param.mType)
		{
			case assembly::Parameter::Type::REGISTER:
			{
				if (param.mIsMemory)
				{
					lemon::MemoryAccessToken& memoryAccessToken = tokenPtr.create<lemon::MemoryAccessToken>();
					memoryAccessToken.mDataType = getLemonDataType(dataType);

					if (param.mRegister.mPreDecrement != 0)
					{
						// As address, create something like "A0-=1"
						lemon::BinaryOperationToken& bot = memoryAccessToken.mAddress.create<lemon::BinaryOperationToken>();
						bot.mOperator = lemon::Operator::ASSIGN_MINUS;

						lemon::VariableToken& variableToken = bot.mLeft.create<lemon::VariableToken>();
						variableToken.mVariable = CodeLemonTokenTree::getRegisterVariable(param.mRegister.mRegister, assembly::DataType::u32);
						variableToken.mDataType = &lemon::PredefinedDataTypes::UINT_32;

						lemon::ConstantTokenExt& ct = createConstantToken(bot.mRight, param.mRegister.mPreDecrement, &lemon::PredefinedDataTypes::UINT_8);
						ct.mOutputAsDecimal = true;
					}
					else if (param.mRegister.mPostIncrement != 0)
					{
						// As address, create something like "(A0+=1)-1"
						lemon::BinaryOperationToken& bot = memoryAccessToken.mAddress.create<lemon::BinaryOperationToken>();
						bot.mOperator = lemon::Operator::BINARY_MINUS;

						lemon::ParenthesisToken& left = bot.mLeft.create<lemon::ParenthesisToken>();
						left.mParenthesisType = lemon::ParenthesisType::PARENTHESIS;
						left.mDataType = &lemon::PredefinedDataTypes::UINT_32;

						{
							lemon::BinaryOperationToken& inner = left.mContent.create<lemon::BinaryOperationToken>();
							inner.mOperator = lemon::Operator::ASSIGN_PLUS;

							lemon::VariableToken& variableToken = inner.mLeft.create<lemon::VariableToken>();
							variableToken.mVariable = CodeLemonTokenTree::getRegisterVariable(param.mRegister.mRegister, assembly::DataType::u32);
							variableToken.mDataType = &lemon::PredefinedDataTypes::UINT_32;

							lemon::ConstantTokenExt& ct = createConstantToken(inner.mRight, param.mRegister.mPostIncrement, &lemon::PredefinedDataTypes::UINT_8);
							ct.mOutputAsDecimal = true;
						}

						lemon::ConstantTokenExt& ct = createConstantToken(bot.mRight, param.mRegister.mPostIncrement, &lemon::PredefinedDataTypes::UINT_8);
						ct.mOutputAsDecimal = true;
					}
					else
					{
						lemon::VariableToken& variableToken = memoryAccessToken.mAddress.create<lemon::VariableToken>();
						variableToken.mVariable = CodeLemonTokenTree::getRegisterVariable(param.mRegister.mRegister, assembly::DataType::u32);
						variableToken.mDataType = &lemon::PredefinedDataTypes::UINT_32;
					}
				}
				else
				{
					lemon::VariableToken& variableToken = tokenPtr.create<lemon::VariableToken>();
					variableToken.mVariable = CodeLemonTokenTree::getRegisterVariable(param.mRegister.mRegister, dataType);
					variableToken.mDataType = getLemonDataType(dataType);
				}
				break;
			}

			case assembly::Parameter::Type::CONSTANT:
			{
				if (param.mIsMemory)
				{
					const uint32 bytes = dataType.getSizeInBytes();
					const KnowledgeBase::DefineInfo* defineInfoSigned = KnowledgeBase::instance().getDefineInfo(KnowledgeBase::DefineKey(param.mConstant.mValue, bytes, true));
					const KnowledgeBase::DefineInfo* defineInfoUnsigned = KnowledgeBase::instance().getDefineInfo(KnowledgeBase::DefineKey(param.mConstant.mValue, bytes, false));

					if (!detail::tryReplaceWithDefine(tokenPtr, defineInfoSigned, defineInfoUnsigned, dataType))
					{
						// Output as normal memory access
						lemon::MemoryAccessToken& memoryAccessToken = tokenPtr.create<lemon::MemoryAccessToken>();
						memoryAccessToken.mDataType = getLemonDataType(dataType);

						lemon::ConstantTokenExt& ct = createConstantToken(memoryAccessToken.mAddress, param.mConstant.mValue, &lemon::PredefinedDataTypes::UINT_32);
						ct.mOutputAsAddress = true;
					}
				}
				else
				{
					lemon::ConstantTokenExt& ct = createConstantToken(tokenPtr, param.mConstant.mValue, getLemonDataType(dataType));
					ct.mOutputAsDecimal = false;
					ct.mOutputAsAddress = false;
				}
				break;
			}

			case assembly::Parameter::Type::COMBINED:
			{
				if (param.mIsMemory)
				{
					if (param.mCombined.mRegister2 == assembly::ExtRegister::NONE)
					{
						const uint32 bytes = dataType.getSizeInBytes();
						const KnowledgeBase::DefineInfo* defineInfoSigned = KnowledgeBase::instance().getDefineInfo(KnowledgeBase::DefineKey(param.mCombined.mRegister1, param.mCombined.mDisplacement, bytes, true));
						const KnowledgeBase::DefineInfo* defineInfoUnsigned = KnowledgeBase::instance().getDefineInfo(KnowledgeBase::DefineKey(param.mCombined.mRegister1, param.mCombined.mDisplacement, bytes, false));

						if (detail::tryReplaceWithDefine(tokenPtr, defineInfoSigned, defineInfoUnsigned, dataType))
						{
							// Done already
							break;
						}
					}
				}

				lemon::TokenPtr<lemon::StatementToken>* currentParent = &tokenPtr;
				if (param.mIsMemory)
				{
					// Output as normal memory access
					lemon::MemoryAccessToken& memoryAccessToken = currentParent->create<lemon::MemoryAccessToken>();
					memoryAccessToken.mDataType = getLemonDataType(dataType);
					currentParent = &memoryAccessToken.mAddress;
				}
				else
				{
					// Note that this case is a bit obscure... it is usually not supposed to happen,
					// but specifically for CODE_LOAD_EA, we set "param.mIsMemory" to false, which can lead to this case here
				}

				if (param.mCombined.mRegister2 != assembly::ExtRegister::NONE)
				{
					// Something with two registers, like "u8[A0 + D0.s16 + 0x18]" instead of just "u8[A0 + 0x18]"
					lemon::BinaryOperationToken& binaryOperationToken = currentParent->create<lemon::BinaryOperationToken>();
					binaryOperationToken.mOperator = lemon::Operator::BINARY_PLUS;
					binaryOperationToken.mDataType = &lemon::PredefinedDataTypes::UINT_32;
					currentParent = &binaryOperationToken.mRight;

					lemon::VariableToken& variableToken = binaryOperationToken.mLeft.create<lemon::VariableToken>();
					variableToken.mVariable = CodeLemonTokenTree::getRegisterVariable((assembly::Register)param.mCombined.mRegister2, assembly::DataType::u32);
					variableToken.mDataType = &lemon::PredefinedDataTypes::UINT_32;
				}

				if (param.mCombined.mDisplacement == 0)
				{
					// Constant is zero
					//  -> This mostly happens with a second register, e.g. something like "u8[A0 + D0.s16]"
					const assembly::DataType dataType(param.mCombined.mSizeOfRegister1, param.mCombined.mSizeOfRegister1 == assembly::DataType::Size::SIZE_32 ? assembly::DataType::Sign::UNSIGNED : assembly::DataType::Sign::SIGNED);
					lemon::VariableToken& variableToken = currentParent->create<lemon::VariableToken>();
					variableToken.mVariable = CodeLemonTokenTree::getRegisterVariable(param.mCombined.mRegister1, dataType);
					variableToken.mDataType = getLemonDataType(dataType);
				}
				else if (param.mCombined.mSizeOfRegister1 == assembly::DataType::Size::SIZE_32)
				{
					// Something like "u8[A0 + 0x18]"
					lemon::BinaryOperationToken& binaryOperationToken = currentParent->create<lemon::BinaryOperationToken>();
					binaryOperationToken.mOperator = lemon::Operator::BINARY_PLUS;
					binaryOperationToken.mDataType = &lemon::PredefinedDataTypes::UINT_32;

					lemon::VariableToken& variableToken = binaryOperationToken.mLeft.create<lemon::VariableToken>();
					variableToken.mVariable = CodeLemonTokenTree::getRegisterVariable(param.mCombined.mRegister1, assembly::DataType::u32);
					variableToken.mDataType = &lemon::PredefinedDataTypes::UINT_32;

					int32 value = param.mCombined.mDisplacement;
					if (value < 0)
					{
						// Change something in the form "u8[A0 + 0xfffffe80]" to "u8[A0 - 0x180]"
						binaryOperationToken.mOperator = lemon::Operator::BINARY_MINUS;
						value = -value;
					}
					createConstantToken(binaryOperationToken.mRight, value, getLemonDataType(detail::getDataTypeForConstantSigned(value)));
				}
				else if (param.mCombined.mRegister2 != assembly::ExtRegister::NONE)
				{
					// Something like "u8[A0 + D0.s16 + 0x18]"
					lemon::BinaryOperationToken& binaryOperationToken = currentParent->create<lemon::BinaryOperationToken>();
					binaryOperationToken.mOperator = lemon::Operator::BINARY_PLUS;
					binaryOperationToken.mDataType = &lemon::PredefinedDataTypes::UINT_32;

					const assembly::DataType dataType(param.mCombined.mSizeOfRegister1, assembly::DataType::Sign::SIGNED);
					lemon::VariableToken& variableToken = binaryOperationToken.mLeft.create<lemon::VariableToken>();
					variableToken.mVariable = CodeLemonTokenTree::getRegisterVariable(param.mCombined.mRegister1, dataType);
					variableToken.mDataType = getLemonDataType(dataType);

					int32 value = param.mCombined.mDisplacement;
					if (value < 0)
					{
						// Change something in the form "u8[A0 + D0.s16 + 0xfffffe80]" to "u8[A0 + D0.s16 - 0x180]"
						binaryOperationToken.mOperator = lemon::Operator::BINARY_MINUS;
						value = -value;
					}
					createConstantToken(binaryOperationToken.mRight, value, getLemonDataType(detail::getDataTypeForConstantSigned(value)));
				}
				else
				{
					// Something like "u8[0xffff8000 + D0.s16]"
					lemon::BinaryOperationToken& binaryOperationToken = currentParent->create<lemon::BinaryOperationToken>();
					binaryOperationToken.mOperator = lemon::Operator::BINARY_PLUS;
					binaryOperationToken.mDataType = &lemon::PredefinedDataTypes::UINT_32;

					lemon::ConstantTokenExt& ct = createConstantToken(binaryOperationToken.mLeft, param.mCombined.mDisplacement, &lemon::PredefinedDataTypes::UINT_32);
					ct.mOutputAsAddress = true;

					const assembly::DataType dataType(param.mCombined.mSizeOfRegister1, assembly::DataType::Sign::SIGNED);
					lemon::VariableToken& variableToken = binaryOperationToken.mRight.create<lemon::VariableToken>();
					variableToken.mVariable = CodeLemonTokenTree::getRegisterVariable(param.mCombined.mRegister1, dataType);
					variableToken.mDataType = getLemonDataType(dataType);
				}
				break;
			}
		}
	}

	void TokenTreeConverter::tryConvertToTokenTree(Block& block, size_t pos)
	{
		const CodeAssembly* codeAsm = block[pos].cast<CodeAssembly>();
		if (nullptr == codeAsm || nullptr == codeAsm->mAssemblyCode)
			return;

		const assembly::AssemblyCode& code = *codeAsm->mAssemblyCode;
		switch (code.mType)
		{
			case assembly::CodeType::CODE_CLEAR:
			{
				// TODO: Get rid of these restrictions
				const bool canHandleDest = (code.mParamDest.isRegister() && code.mParamDest.mRegister.mPreDecrement == 0 && code.mParamDest.mRegister.mPostIncrement == 0);

				if (canHandleDest && !code.mDataType.isSigned())
				{
					CodeLemonTokenTree& tokenTree = genericmanager::Manager<Code>::create<CodeLemonTokenTree>();
					lemon::BinaryOperationToken& bot = tokenTree.mRoot.create<lemon::BinaryOperationToken>();
					bot.mOperator = lemon::Operator::ASSIGN;

					createLemonTokenTreeForParameter(bot.mLeft, code.mParamDest, code.mDataType);
					createConstantToken(bot.mRight, 0, &lemon::PredefinedDataTypes::UINT_32);

					tokenTree.mLines.swap(block[pos].mLines);
					block.replace(tokenTree, pos);
				}
				break;
			}

			case assembly::CodeType::CODE_MOVE:
			{
				// TODO: Get rid of these restrictions
				const bool canHandleDest = (code.mParamDest.isRegister() && code.mParamDest.mRegister.mPreDecrement == 0 && code.mParamDest.mRegister.mPostIncrement == 0) || code.mParamDest.isConstant() || code.mParamDest.isCombinedMemory();
				const bool canHandleSource = (code.mParamSource.isRegister() && code.mParamSource.mRegister.mPreDecrement == 0 && code.mParamSource.mRegister.mPostIncrement == 0) || code.mParamSource.isConstant() || code.mParamSource.isCombinedMemory();

				if (canHandleDest && canHandleSource && !code.mDataType.isSigned())
				{
					CodeLemonTokenTree& tokenTree = genericmanager::Manager<Code>::create<CodeLemonTokenTree>();
					lemon::BinaryOperationToken& bot = tokenTree.mRoot.create<lemon::BinaryOperationToken>();
					bot.mOperator = lemon::Operator::ASSIGN;

					createLemonTokenTreeForParameter(bot.mLeft, code.mParamDest, code.mDataType);
					createLemonTokenTreeForParameter(bot.mRight, code.mParamSource, code.mDataType);

					tokenTree.mLines.swap(block[pos].mLines);
					block.replace(tokenTree, pos);
				}
				break;
			}

			case assembly::CodeType::CODE_LOAD_EA:
			{
				// TODO: Get rid of these restrictions
				const bool canHandleDest = (code.mParamDest.isRegister() && code.mParamDest.mRegister.mPreDecrement == 0 && code.mParamDest.mRegister.mPostIncrement == 0) || code.mParamDest.isConstant() || code.mParamDest.isCombinedMemory();

				if (canHandleDest)
				{
					CodeLemonTokenTree& tokenTree = genericmanager::Manager<Code>::create<CodeLemonTokenTree>();
					lemon::BinaryOperationToken& bot = tokenTree.mRoot.create<lemon::BinaryOperationToken>();
					bot.mOperator = lemon::Operator::ASSIGN;

					createLemonTokenTreeForParameter(bot.mLeft, code.mParamDest, code.mDataType);

					// Ignore memory access in param
					assembly::Parameter paramSource = code.mParamSource;
					paramSource.mIsMemory = false;
					createLemonTokenTreeForParameter(bot.mRight, paramSource, code.mDataType);

					// If right side is an address, output it as such
					if (code.mParamDest.isPureRegister() && code.mParamDest.mRegister.mRegister >= assembly::Register::A0 && code.mParamDest.mRegister.mRegister <= assembly::Register::A7)
					{
						lemon::ConstantTokenExt* ct = bot.mRight->cast<lemon::ConstantTokenExt>();
						if (nullptr != ct)
						{
							ct->mOutputAsAddress = true;
						}
					}

					tokenTree.mLines.swap(block[pos].mLines);
					block.replace(tokenTree, pos);
				}
				break;
			}

			case assembly::CodeType::CODE_SHIFT_LEFT:
			case assembly::CodeType::CODE_SHIFT_RIGHT:
			{
				// Accept logical (i.e. unsigned) and arithmentic (i.e. signed) shifts, but no rotates
				if (code.mShiftType != assembly::ShiftType::LOGICAL && code.mShiftType != assembly::ShiftType::ARITHMETIC)
					break;

				[[fallthrough]];
			}

			case assembly::CodeType::CODE_ADD:
			case assembly::CodeType::CODE_SUB:
				// Do not handle CODE_MUL, because of its different intermediate and result type and required castings (though that would be possible to handle)
				// Do not handle CODE_DIV, as it performs two things at once - divide and modulo
			case assembly::CodeType::CODE_AND:
			case assembly::CodeType::CODE_OR:
			case assembly::CodeType::CODE_XOR:
			{
				// TODO: Get rid of these restrictions
				const bool canHandleDest = (code.mParamDest.isPureRegister() && code.mParamDest.mRegister.mPreDecrement == 0 && code.mParamDest.mRegister.mPostIncrement == 0);
				const bool canHandleSource = (code.mParamSource.isRegister() && code.mParamSource.mRegister.mPreDecrement == 0 && code.mParamSource.mRegister.mPostIncrement == 0) || code.mParamSource.isConstant() || code.mParamSource.isCombinedMemory();
				if (!canHandleDest || !canHandleSource)
					break;

				assembly::DataType destType = code.mDataType;
				assembly::DataType sourceType = code.mDataType;

				// Signed data types could require some special handling
				if (code.mDataType.isSigned())
				{
					if (code.mType == assembly::CodeType::CODE_ADD || code.mType == assembly::CodeType::CODE_SUB)
					{
						// This is the case of signed extension, but that doesn't require anything special here
						if (code.mDataType.mSize == assembly::DataType::Size::SIZE_16)
						{
							destType.mSize = assembly::DataType::Size::SIZE_32;
							destType.mSign = assembly::DataType::Sign::UNSIGNED;
						}
						else
							break;
					}
					else if (code.mType == assembly::CodeType::CODE_MUL)
					{
						// TODO: Signed multiplication is a bit messy...
						break;
					}
				}

				CodeLemonTokenTree& tokenTree = genericmanager::Manager<Code>::create<CodeLemonTokenTree>();

				if ((code.mType == assembly::CodeType::CODE_ADD || code.mType == assembly::CodeType::CODE_SUB) &&
					code.mParamSource.isConstantValue() && code.mParamSource.mConstant.mValue == 1)
				{
					// Special handling for +1 and -1
					lemon::UnaryOperationToken& uot = tokenTree.mRoot.create<lemon::UnaryOperationToken>();
					uot.mOperator = (code.mType == assembly::CodeType::CODE_SUB) ? lemon::Operator::UNARY_DECREMENT : lemon::Operator::UNARY_INCREMENT;
					uot.mDataType = getLemonDataType(code.mDataType);
					createLemonTokenTreeForParameter(uot.mArgument, code.mParamDest, code.mDataType);
				}
				else
				{
					lemon::BinaryOperationToken& bot = tokenTree.mRoot.create<lemon::BinaryOperationToken>();
					bool isBitwiseOperation = false;
					switch (code.mType)
					{
						case assembly::CodeType::CODE_ADD:			bot.mOperator = lemon::Operator::ASSIGN_PLUS;		 break;
						case assembly::CodeType::CODE_SUB:			bot.mOperator = lemon::Operator::ASSIGN_MINUS;		 break;
						case assembly::CodeType::CODE_MUL:			bot.mOperator = lemon::Operator::ASSIGN_MULTIPLY;	 break;
						case assembly::CodeType::CODE_DIV:			bot.mOperator = lemon::Operator::ASSIGN_DIVIDE;		 break;
						case assembly::CodeType::CODE_AND:			bot.mOperator = lemon::Operator::ASSIGN_AND;		 isBitwiseOperation = true;  break;
						case assembly::CodeType::CODE_OR:			bot.mOperator = lemon::Operator::ASSIGN_OR;			 isBitwiseOperation = true;  break;
						case assembly::CodeType::CODE_XOR:			bot.mOperator = lemon::Operator::ASSIGN_XOR;		 isBitwiseOperation = true;  break;
						case assembly::CodeType::CODE_SHIFT_LEFT:	bot.mOperator = lemon::Operator::ASSIGN_SHIFT_LEFT;	 sourceType = assembly::DataType::u8;  break;
						case assembly::CodeType::CODE_SHIFT_RIGHT:	bot.mOperator = lemon::Operator::ASSIGN_SHIFT_RIGHT; sourceType = assembly::DataType::u8;  break;
					}
					bot.mDataType = getLemonDataType(code.mDataType);

					createLemonTokenTreeForParameter(bot.mLeft, code.mParamDest, destType);
					createLemonTokenTreeForParameter(bot.mRight, code.mParamSource, sourceType);

					// Output small constant numbers on the right side as decimal
					if (!isBitwiseOperation)
					{
						lemon::ConstantTokenExt* ct = bot.mRight->cast<lemon::ConstantTokenExt>();
						if (nullptr != ct)
						{
							ct->mOutputAsDecimal = (ct->mValue.get<uint64>() <= 9);
						}
					}
				}

				tokenTree.mLines.swap(block[pos].mLines);
				block.replace(tokenTree, pos);
				break;
			}

			case assembly::CodeType::CODE_NEG:
			{
				CodeLemonTokenTree& tokenTree = genericmanager::Manager<Code>::create<CodeLemonTokenTree>();

				lemon::BinaryOperationToken& bot = tokenTree.mRoot.create<lemon::BinaryOperationToken>();
				bot.mOperator = lemon::Operator::ASSIGN;
				createLemonTokenTreeForParameter(bot.mLeft, code.mParamDest, code.mDataType);

				lemon::UnaryOperationToken& uot = bot.mRight.create<lemon::UnaryOperationToken>();
				uot.mOperator = lemon::Operator::BINARY_MINUS;
				uot.mDataType = getLemonDataType(code.mDataType);
				createLemonTokenTreeForParameter(uot.mArgument, code.mParamDest, code.mDataType);

				tokenTree.mLines.swap(block[pos].mLines);
				block.replace(tokenTree, pos);
				break;
			}

			case assembly::CodeType::CODE_EXTEND_SIGNED:
			{
				// TODO: Get rid of these restrictions
				const bool canHandleDest = (code.mParamDest.isPureRegister() && code.mParamDest.mRegister.mPreDecrement == 0 && code.mParamDest.mRegister.mPostIncrement == 0);

				if (canHandleDest)
				{
					CodeLemonTokenTree& tokenTree = genericmanager::Manager<Code>::create<CodeLemonTokenTree>();
					lemon::BinaryOperationToken& bot = tokenTree.mRoot.create<lemon::BinaryOperationToken>();
					bot.mOperator = lemon::Operator::ASSIGN;

					createLemonTokenTreeForParameter(bot.mLeft, code.mParamDest, code.mDataType);
					createLemonTokenTreeForParameter(bot.mRight, code.mParamDest, assembly::DataType((assembly::DataType::Size)((int)code.mDataType.mSize / 2), assembly::DataType::Sign::SIGNED));

					tokenTree.mLines.swap(block[pos].mLines);
					block.replace(tokenTree, pos);
				}
				break;
			}
		}
	}

	void TokenTreeConverter::createTokenTreeForCondition(lemon::TokenPtr<lemon::StatementToken>& tokenPtr, assembly::Condition condition, const assembly::AssemblyCode& ac)
	{
		assembly::DataType dataType = ac.mDataType;

		switch (ac.mType)
		{
			case assembly::CodeType::CODE_CMP:
			{
				// Output as "dst < source" with fitting operator
				const lemon::Operator op = detail::getOperationForCondition(condition, dataType.mSign, false);

				lemon::BinaryOperationToken& bot = tokenPtr.create<lemon::BinaryOperationToken>();
				bot.mOperator = op;
				bot.mDataType = &lemon::PredefinedDataTypes::BOOL;

				createLemonTokenTreeForParameter(bot.mLeft, ac.mParamDest, dataType);
				createLemonTokenTreeForParameter(bot.mRight, ac.mParamSource, dataType);

				// If right side is an address, output it as such
				if (ac.mParamDest.isPureRegister() && ac.mParamDest.mRegister.mRegister >= assembly::Register::A0 && ac.mParamDest.mRegister.mRegister <= assembly::Register::A7)
				{
					lemon::ConstantTokenExt* ct = bot.mRight->cast<lemon::ConstantTokenExt>();
					if (nullptr != ct)
					{
						ct->mOutputAsAddress = true;
					}
				}
				break;
			}

			case assembly::CodeType::CODE_TEST:
			case assembly::CodeType::CODE_MOVE:
			case assembly::CodeType::CODE_ADD:
			case assembly::CodeType::CODE_SUB:
			case assembly::CodeType::CODE_AND:
			case assembly::CodeType::CODE_OR:
			case assembly::CodeType::CODE_XOR:
			case assembly::CodeType::CODE_SHIFT_LEFT:
			case assembly::CodeType::CODE_SHIFT_RIGHT:
			case assembly::CodeType::CODE_EXTEND_SIGNED:
			{
				// Currently only supported are checks that can be done with the result only
				//  -> Another use-case is carry flag checks (which get wrong output as "aboveEqual.u", "below.u"), but these can't easily be handled
				switch (condition)
				{
					case assembly::Condition::EQ:
					case assembly::Condition::NE:
					case assembly::Condition::MI:
					case assembly::Condition::PL:
					case assembly::Condition::LT:
					case assembly::Condition::GE:
					case assembly::Condition::HI:
					case assembly::Condition::LS:
					case assembly::Condition::GT:
					case assembly::Condition::LE:
					{
						// Output as "dst >= 0" with fitting operator
						const lemon::Operator op = detail::getOperationForCondition(condition, dataType.mSign, true);

						lemon::BinaryOperationToken& bot = tokenPtr.create<lemon::BinaryOperationToken>();
						bot.mOperator = op;
						bot.mDataType = &lemon::PredefinedDataTypes::BOOL;

						createLemonTokenTreeForParameter(bot.mLeft, ac.mParamDest, dataType);
						createConstantToken(bot.mRight, 0, getLemonDataType(dataType));
						break;
					}

					case assembly::Condition::CC:
					case assembly::Condition::CS:
					{
						// Only handle subtraction for now
						if (ac.mType == assembly::CodeType::CODE_SUB)
						{
							// And also, ensure that only looking at the sign of the result actually gives us the right condition
							//  -> We can only really ensure this if the subtracted part is a constant not too large
							if (ac.mParamSource.isConstant() && ac.mParamSource.mConstant.mValue < ((uint32)1 << (dataType.getSizeInBits() - 1)))
							{
								// Output as "dst >= 0" with operator ">=" or "<"
								dataType.mSign = assembly::DataType::Sign::SIGNED;

								lemon::BinaryOperationToken& bot = tokenPtr.create<lemon::BinaryOperationToken>();
								bot.mOperator = (condition == assembly::Condition::CC) ? lemon::Operator::COMPARE_GREATER_OR_EQUAL : lemon::Operator::COMPARE_LESS;
								bot.mDataType = &lemon::PredefinedDataTypes::BOOL;

								createLemonTokenTreeForParameter(bot.mLeft, ac.mParamDest, dataType);
								createConstantToken(bot.mRight, 0, getLemonDataType(dataType));
							}
						}

						// Can't handle other cases, they will get output as "!carryFlag()" or "carryFlag()"
						break;
					}
				}
				break;
			}

			case assembly::CodeType::CODE_NOT:
			{
				if (condition == assembly::Condition::EQ || condition == assembly::Condition::NE)
				{
					// Output as "dst != 0" with operator "==" or "!="
					lemon::BinaryOperationToken& bot = tokenPtr.create<lemon::BinaryOperationToken>();
					bot.mOperator = (condition == assembly::Condition::EQ) ? lemon::Operator::COMPARE_EQUAL : lemon::Operator::COMPARE_NOT_EQUAL;
					bot.mDataType = &lemon::PredefinedDataTypes::BOOL;

					createLemonTokenTreeForParameter(bot.mLeft, ac.mParamDest, dataType);
					createConstantToken(bot.mRight, 0, getLemonDataType(dataType));
				}
				break;
			}

			case assembly::CodeType::CODE_SET_BIT:
			case assembly::CodeType::CODE_CLEAR_BIT:
			{
				// Can't handle the codegen variant
				if (ac.mCodeGenData == 0)
					break;
				[[fallthrough]];
			}

			case assembly::CodeType::CODE_TEST_BIT:
			{
				if (condition == assembly::Condition::EQ || condition == assembly::Condition::NE)
				{
					// TODO: We could handle other source parameter types as well
					if (ac.mParamSource.isConstantValue())
					{
						// Output as "(dst & src) == 0" with operator "==" or "!="
						lemon::BinaryOperationToken& bot = tokenPtr.create<lemon::BinaryOperationToken>();
						bot.mOperator = (condition == assembly::Condition::EQ) ? lemon::Operator::COMPARE_EQUAL : lemon::Operator::COMPARE_NOT_EQUAL;
						bot.mDataType = &lemon::PredefinedDataTypes::BOOL;

						createConstantToken(bot.mRight, 0, getLemonDataType(dataType));

						lemon::ParenthesisToken& pt = bot.mLeft.create<lemon::ParenthesisToken>();
						pt.mParenthesisType = lemon::ParenthesisType::PARENTHESIS;
						pt.mDataType = getLemonDataType(dataType);

						lemon::BinaryOperationToken& bot2 = pt.mContent.create<lemon::BinaryOperationToken>();
						bot2.mOperator = lemon::Operator::BINARY_AND;
						bot2.mDataType = getLemonDataType(dataType);

						createLemonTokenTreeForParameter(bot2.mLeft, ac.mParamDest, dataType);

						const uint32 bitValue = 1 << ac.mParamSource.mConstant.mValue;

						// Output u32 constant as u16, if it's small enough
						bool enforceDataTypeSize = false;
						if (dataType.mSize == assembly::DataType::Size::SIZE_32 && bitValue <= 0xffff)
						{
							dataType.mSize = assembly::DataType::Size::SIZE_16;
							enforceDataTypeSize = true;
						}

						lemon::ConstantTokenExt& ct = createConstantToken(bot2.mRight, bitValue, getLemonDataType(dataType));
						ct.mOutputWithDataTypeSize = enforceDataTypeSize;
					}
				}
				break;
			}
		}
	}

	bool TokenTreeConverter::negateCondition(lemon::StatementToken& token)
	{
		switch (token.getType())
		{
			case lemon::BinaryOperationToken::TYPE:
			{
				lemon::BinaryOperationToken& bot = token.as<lemon::BinaryOperationToken>();
				switch (bot.mOperator)
				{
					case lemon::Operator::COMPARE_EQUAL:			bot.mOperator = lemon::Operator::COMPARE_NOT_EQUAL;  return true;
					case lemon::Operator::COMPARE_NOT_EQUAL:		bot.mOperator = lemon::Operator::COMPARE_EQUAL;  return true;
					case lemon::Operator::COMPARE_LESS:				bot.mOperator = lemon::Operator::COMPARE_GREATER_OR_EQUAL;  return true;
					case lemon::Operator::COMPARE_LESS_OR_EQUAL:	bot.mOperator = lemon::Operator::COMPARE_GREATER;  return true;
					case lemon::Operator::COMPARE_GREATER:			bot.mOperator = lemon::Operator::COMPARE_LESS_OR_EQUAL;  return true;
					case lemon::Operator::COMPARE_GREATER_OR_EQUAL: bot.mOperator = lemon::Operator::COMPARE_LESS;  return true;

					// TODO: Support logical && and || as well
				}

				break;
			}

			case lemon::ParenthesisToken::TYPE:
			{
				lemon::ParenthesisToken& pt = token.as<lemon::ParenthesisToken>();
				if (pt.mContent.size() == 1 && pt.mContent[0].isStatement())
				{
					return negateCondition(static_cast<lemon::StatementToken&>(pt.mContent[0]));
				}
			}
		}

		// Can't negate this token tree
		return false;
	}

}
