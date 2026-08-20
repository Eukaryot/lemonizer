/*
*	Lemonizer -- turns 68K code into lemon script
*	Copyright (C) 2021 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#include "pch.h"
#include "lemonizer/LemonizerOptimization.h"
#include "lemonizer/LemonizerCode.h"
#include "assembly/CodeOutputHelper.h"

#include <lemon/compiler/TokenProcessing.h>
#include <lemon/program/Variable.h>


namespace lemonizer
{
	namespace detail
	{

		// ----- General helper functions ----- //

		const assembly::AssemblyCode* getAssemblyCode(const Code& code)
		{
			if (code.getType() == Code::ASSEMBLY)
			{
				return code.as<CodeAssembly>().mAssemblyCode;
			}
			return nullptr;
		}

		const assembly::AssemblyCode* getAssemblyCode(const Code& code, assembly::CodeType filterType)
		{
			const assembly::AssemblyCode* ac = getAssemblyCode(code);
			return (nullptr != ac && ac->mType == filterType) ? ac : nullptr;
		}

		bool isSimpleUnconditionalJump(const Code& code, uint32& destinationAddress)
		{
			// Must be a jump
			const assembly::AssemblyCode* ac = getAssemblyCode(code, assembly::CodeType::CODE_JUMP);
			if (nullptr != ac)
			{
				// Unconditional jump
				if (ac->mParamSource.mType != assembly::Parameter::Type::CONDITION)
				{
					// Fixed destination address
					if (ac->mParamDest.mType == assembly::Parameter::Type::CONSTANT)
					{
						destinationAddress = ac->mParamDest.mConstant.mValue;
						return true;
					}
				}
			}
			return false;
		}

		bool isSimpleConditionalJump(const Code& code, uint32& destinationAddress, assembly::Condition& condition, assembly::ExtRegister& loopRegister)
		{
			// Must be a jump
			const assembly::AssemblyCode* ac = getAssemblyCode(code, assembly::CodeType::CODE_JUMP);
			if (nullptr != ac)
			{
				// Conditional jump
				if (ac->mParamSource.mType == assembly::Parameter::Type::CONDITION)
				{
					// Fixed destination address
					if (ac->mParamDest.mType == assembly::Parameter::Type::CONSTANT)
					{
						destinationAddress = ac->mParamDest.mConstant.mValue;
						condition = ac->mParamSource.mCondition.mCondition;
						loopRegister = ac->mParamSource.mCondition.mLoopRegister;
						return true;
					}
				}
			}
			return false;
		}

		bool findAddressInBlock(Block& block, uint32 address, bool ignoreEndAddress, size_t& pos)
		{
			if (address < block.mEndAddress)
			{
				for (size_t i = 0; i < block.size(); ++i)
				{
					for (const LineData* line : block[i].mLines)
					{
						if (line->mAddress == address)
						{
							pos = i;
							return true;
						}
					}
				}
			}
			else if (address == block.mEndAddress && !ignoreEndAddress)
			{
				pos = block.size();
				return true;
			}
			return false;
		}

		bool containsReturn(Block& block, size_t startPos, size_t endPos)
		{
			for (size_t i = startPos; i < endPos; ++i)
			{
				if (nullptr != getAssemblyCode(block[i], assembly::CodeType::CODE_RETURN))
				{
					return true;
				}
			}
			return false;
		}

		void moveToInnerBlock(Block& block, size_t startPos, size_t endPos, Block& innerBlock)
		{
			// Move over codes to the inner block
			for (size_t i = startPos; i < endPos; ++i)
			{
				innerBlock.add(block[i]);
			}

			// Erase from original block
			block.erase(startPos, endPos - startPos);
		}

		void mergeLinesInto(std::vector<const LineData*>& primary, std::vector<const LineData*>& secondary, bool atStart = false)
		{
			primary.insert(atStart ? primary.begin() : primary.end(), secondary.begin(), secondary.end());
			secondary.clear();
		}

		void mergeLinesInto(std::vector<const LineData*>& output, std::vector<const LineData*>& input1, std::vector<const LineData*>& input2)
		{
			output.swap(input1);
			mergeLinesInto(output, input2);
		}

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

		lemon::BinaryOperationToken* castAssignmentToken(lemon::TokenPtr<lemon::StatementToken>& token)
		{
			if (token->getType() != lemon::Token::Type::BINARY_OPERATION)
				return nullptr;
			lemon::BinaryOperationToken& assignmentBot = token->as<lemon::BinaryOperationToken>();
			if (assignmentBot.mOperator != lemon::Operator::ASSIGN)
				return nullptr;
			return &assignmentBot;
		}



		// ----- Structure creation ----- //

		// Forward declaration
		void createStructuredBlocks(Block& block);

		CodeIfElse& replaceWithIfElse(Block& block, uint32 endAddress, size_t startPos, size_t endPos, assembly::Condition condition, assembly::ExtRegister loopRegister = assembly::ExtRegister::NONE)
		{
			// Insert new code, replacing the conditional jump
			std::vector<const LineData*> lineData = std::move(block[startPos].mLines);
			CodeIfElse& ci = block.createReplaceAt<CodeIfElse>(startPos);
			ci.mCondition = condition;
			ci.mLoopRegister = loopRegister;
			ci.mIfBlock.mEndAddress = endAddress;
			ci.mLines.swap(lineData);

			// Move conditional code into if-block
			if (startPos + 1 < endPos)
			{
				moveToInnerBlock(block, startPos + 1, endPos, ci.mIfBlock);

				// Recursively handle content
				createStructuredBlocks(ci.mIfBlock);
			}
			return ci;
		}

		CodeWhile& replaceWithWhile(Block& block, uint32 endAddress, size_t startPos, size_t endPos, assembly::Condition condition, assembly::ExtRegister loopRegister = assembly::ExtRegister::NONE)
		{
			// Insert new code just after the jump back
			CodeWhile& cw = block.createAt<CodeWhile>(endPos);
			cw.mInnerBlock.mEndAddress = endAddress;
			cw.mLines = block[endPos - 1].mLines;

			// Move conditional code into inner block
			if (startPos < endPos)
			{
				moveToInnerBlock(block, startPos, endPos, cw.mInnerBlock);

				// Add an if-block at the end
				std::vector<const LineData*> lineData = std::move(cw.mInnerBlock.back().mLines);
				CodeIfElse& ci = cw.mInnerBlock.createReplaceAt<CodeIfElse>(cw.mInnerBlock.size() - 1);
				ci.mIfBlock.mEndAddress = endAddress;
				ci.mIfBlock.mOutputAsSingleLine = true;
				ci.mCondition = condition;
				ci.mLoopRegister = loopRegister;
				ci.mNegateWholeCondition = true;
				ci.mLines.swap(lineData);

				// Add inner block for if that consists only of "break"
				CodeBreakOrContinue& cboc = ci.mIfBlock.createBack<CodeBreakOrContinue>();
				cboc.mIsContinue = false;
				cboc.mLines = ci.mLines;

				// Recursively handle while-loop's inner block content
				createStructuredBlocks(cw.mInnerBlock);
			}
			return cw;
		}

		void createStructuredBlocks(Block& block)
		{
			// Create while-blocks where possible
			for (size_t pos = 0; pos < block.size(); ++pos)
			{
				const Code& code = block[pos];

				uint32 destinationAddress;
				assembly::Condition condition;
				assembly::ExtRegister loopRegister;
				if (isSimpleConditionalJump(code, destinationAddress, condition, loopRegister))
				{
					// Try to make a while-block, if it's a backward jump
					if (destinationAddress < code.mLines[0]->mAddress)
					{
						const size_t endPos = pos + 1;
						size_t startPos;
						if (findAddressInBlock(block, destinationAddress, false, startPos))
						{
							if (startPos < endPos && !containsReturn(block, startPos, endPos-1))	// TODO: As an improvement, this could check only for returns that can't be avoided (e.g. by a conditional jump over it)
							{
								const uint32 endAddress = (endPos < block.size()) ? block[endPos].mLines.back()->mAddress : block.mEndAddress;
								replaceWithWhile(block, endAddress, startPos, endPos, condition, loopRegister);
								continue;
							}
						}
					}
				}
			}

			// Create if-blocks where possible
			for (size_t pos = 0; pos < block.size(); ++pos)
			{
				const Code& code = block[pos];

				uint32 destinationAddress;
				assembly::Condition condition;
				assembly::ExtRegister loopRegister;
				if (isSimpleConditionalJump(code, destinationAddress, condition, loopRegister))
				{
					const bool hasLoopRegister = (loopRegister != assembly::ExtRegister::NONE);

					// Try to make an if-block, if it's a forward jump
					if (!hasLoopRegister && destinationAddress > code.mLines[0]->mAddress)
					{
						const size_t startPos = pos;
						size_t endPos;
						if (findAddressInBlock(block, destinationAddress, false, endPos))
						{
							if (endPos > startPos + 1)
							{
								const uint32 endAddress = (endPos < block.size()) ? block[endPos].mLines.back()->mAddress : block.mEndAddress;
								replaceWithIfElse(block, endAddress, startPos, endPos, assembly::negateCondition(condition));
								continue;
							}
						}
					}

					// Couldn't replace it with a block, so just split into if + jump
					{
						if (hasLoopRegister)
						{
							condition = assembly::negateCondition(condition);
						}
						CodeIfElse& ci = replaceWithIfElse(block, code.mLines.back()->mAddress, pos, pos, condition, loopRegister);
						CodeJumpOrCall& cj = ci.mIfBlock.create<CodeJumpOrCall>();
						cj.mIsCall = false;
						cj.mDestinationAddress = destinationAddress;
						cj.mLines = ci.mLines;
					}
				}
			}

			// After all if-blocks were inserted, check which could be assigned an else-block as well
			for (size_t pos = 0; pos < block.size(); ++pos)
			{
				Code& code = block[pos];
				if (code.getType() == Code::IFELSE)
				{
					CodeIfElse& ci = code.as<CodeIfElse>();

					// Check for else-block
					uint32 destinationAddress;
					if (!ci.mIfBlock.empty() && isSimpleUnconditionalJump(ci.mIfBlock.back(), destinationAddress))
					{
						size_t elseEndPos;
						if (findAddressInBlock(block, destinationAddress, false, elseEndPos))
						{
							if (elseEndPos > pos + 1)
							{
								ci.mElseBlock.mEndAddress = destinationAddress;

								// Erase unconditional jump just before else
								ci.mIfBlock.erase(ci.mIfBlock.size() - 1, 1);

								// Move code into else-block
								moveToInnerBlock(block, pos + 1, elseEndPos, ci.mElseBlock);

								// Recursive structure creation
								createStructuredBlocks(ci.mElseBlock);
							}
						}
					}
				}
			}
		}

		void collectInnerBlocks(Block& block, std::vector<Block*>& outBlocks)
		{
			outBlocks.push_back(&block);

			for (size_t pos = 0; pos < block.size(); ++pos)
			{
				if (block[pos].getType() == Code::IFELSE)
				{
					CodeIfElse& ci = block[pos].as<CodeIfElse>();
					collectInnerBlocks(ci.mIfBlock, outBlocks);
					collectInnerBlocks(ci.mElseBlock, outBlocks);
				}
				else if (block[pos].getType() == Code::WHILE)
				{
					CodeWhile& cw = block[pos].as<CodeWhile>();
					collectInnerBlocks(cw.mInnerBlock, outBlocks);
				}
			}
		}



		// ----- Lemon token tree usage ----- //

		const lemon::DataTypeDefinition* getLemonDataType(const assembly::DataType& dataType)
		{
			switch (dataType.mSize)
			{
				case assembly::DataType::Size::SIZE_8:   return dataType.isSigned() ? &lemon::PredefinedDataTypes::INT_8  : &lemon::PredefinedDataTypes::UINT_8;
				case assembly::DataType::Size::SIZE_16:  return dataType.isSigned() ? &lemon::PredefinedDataTypes::INT_16 : &lemon::PredefinedDataTypes::UINT_16;
				case assembly::DataType::Size::SIZE_32:  return dataType.isSigned() ? &lemon::PredefinedDataTypes::INT_32 : &lemon::PredefinedDataTypes::UINT_32;
			}
			return &lemon::PredefinedDataTypes::UINT_32;
		}

		void createLemonTokenTreeForParameter(lemon::TokenPtr<lemon::StatementToken>& tokenPtr, const assembly::Parameter& param, const assembly::DataType& dataType)
		{
			switch (param.mType)
			{
				case assembly::Parameter::Type::REGISTER:
				{
					if (param.mIsMemory)
					{
						lemon::MemoryAccessToken& memoryAccessToken = tokenPtr.create<lemon::MemoryAccessToken>();
						memoryAccessToken.mDataType = getLemonDataType(dataType);

						lemon::VariableToken& variableToken = memoryAccessToken.mAddress.create<lemon::VariableToken>();
						variableToken.mVariable = CodeLemonTokenTree::getRegisterVariable(param.mRegister.mRegister, assembly::DataType::u32);
						variableToken.mDataType = &lemon::PredefinedDataTypes::UINT_32;
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

						lemon::MemoryAccessToken& memoryAccessToken = tokenPtr.create<lemon::MemoryAccessToken>();
						memoryAccessToken.mDataType = getLemonDataType(dataType);

						lemon::ConstantToken& constantToken = memoryAccessToken.mAddress.create<lemon::ConstantToken>();
						constantToken.mValue = param.mConstant.mValue;
						constantToken.mDataType = &lemon::PredefinedDataTypes::UINT_32;
					}
					else
					{
						lemon::ConstantToken& constantToken = tokenPtr.create<lemon::ConstantToken>();
						constantToken.mValue = param.mConstant.mValue;
					}
					break;
				}

				case assembly::Parameter::Type::COMBINED:
				{
					if (param.mIsMemory)
					{
						lemon::MemoryAccessToken& memoryAccessToken = tokenPtr.create<lemon::MemoryAccessToken>();
						memoryAccessToken.mDataType = getLemonDataType(dataType);
						lemon::TokenPtr<lemon::StatementToken>* currentParent = &memoryAccessToken.mAddress;

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

							lemon::ConstantToken& constantToken = binaryOperationToken.mRight.create<lemon::ConstantToken>();
							constantToken.mValue = param.mCombined.mDisplacement;
							if (param.mCombined.mDisplacement < 0)
							{
								// Change something in the form "u8[A0 + 0xfffffe80]" to "u8[A0 - 0x180]"
								binaryOperationToken.mOperator = lemon::Operator::BINARY_MINUS;
								constantToken.mValue = -param.mCombined.mDisplacement;
							}
							constantToken.mDataType = getLemonDataType(getDataTypeForConstant(param.mCombined.mDisplacement));
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

							lemon::ConstantToken& constantToken = binaryOperationToken.mRight.create<lemon::ConstantToken>();
							constantToken.mValue = param.mCombined.mDisplacement;
							if (param.mCombined.mDisplacement < 0)
							{
								// Change something in the form "u8[A0 + D0.s16 + 0xfffffe80]" to "u8[A0 + D0.s16 - 0x180]"
								binaryOperationToken.mOperator = lemon::Operator::BINARY_MINUS;
								constantToken.mValue = -param.mCombined.mDisplacement;
							}
							constantToken.mDataType = getLemonDataType(getDataTypeForConstant(param.mCombined.mDisplacement));
						}
						else
						{
							// Something like "u8[0xffff8000 + D0.s16]"
							lemon::BinaryOperationToken& binaryOperationToken = currentParent->create<lemon::BinaryOperationToken>();
							binaryOperationToken.mOperator = lemon::Operator::BINARY_PLUS;
							binaryOperationToken.mDataType = &lemon::PredefinedDataTypes::UINT_32;

							lemon::ConstantToken& constantToken = binaryOperationToken.mLeft.create<lemon::ConstantToken>();
							constantToken.mValue = param.mCombined.mDisplacement;
							constantToken.mDataType = &lemon::PredefinedDataTypes::UINT_32;

							const assembly::DataType dataType(param.mCombined.mSizeOfRegister1, assembly::DataType::Sign::SIGNED);
							lemon::VariableToken& variableToken = binaryOperationToken.mRight.create<lemon::VariableToken>();
							variableToken.mVariable = CodeLemonTokenTree::getRegisterVariable(param.mCombined.mRegister1, dataType);
							variableToken.mDataType = getLemonDataType(dataType);
						}
					}
					else
					{
						// TODO: Does this ever happen?
					}
					break;
				}
			}
		}

		void tryConvertToTokenTree(Block& block, size_t pos)
		{
			const assembly::AssemblyCode* ac = getAssemblyCode(block[pos]);
			if (nullptr == ac)
				return;

			const assembly::AssemblyCode& code = *ac;
			switch (code.mType)
			{
				case assembly::CodeType::CODE_CLEAR:
				{
					// TODO: Get rid of these restrictions
					const bool canHandleDest = (code.mParamDest.isPureRegister() && code.mParamDest.mRegister.mPreDecrement == 0 && code.mParamDest.mRegister.mPostIncrement == 0);

					if (canHandleDest && !code.mDataType.isSigned())
					{
						CodeLemonTokenTree& tokenTree = genericmanager::Manager<Code>::create<CodeLemonTokenTree>();
						lemon::BinaryOperationToken& bot = tokenTree.mRoot.create<lemon::BinaryOperationToken>();
						bot.mOperator = lemon::Operator::ASSIGN;

						if (code.mParamDest.isPureRegister())
						{
							lemon::VariableToken& variableToken = bot.mLeft.create<lemon::VariableToken>();
							variableToken.mVariable = CodeLemonTokenTree::getRegisterVariable(code.mParamDest.mRegister.mRegister, code.mDataType);
							variableToken.mDataType = getLemonDataType(code.mDataType);
						}

						createLemonTokenTreeForParameter(bot.mLeft, code.mParamDest, code.mDataType);
						lemon::ConstantToken& constantToken = bot.mRight.create<lemon::ConstantToken>();
						constantToken.mValue = 0;

						tokenTree.mLines.swap(block[pos].mLines);
						block.replace(tokenTree, pos);
					}
					break;
				}

				case assembly::CodeType::CODE_MOVE:
				{
					// TODO: Get rid of these restrictions
					const bool canHandleDest = (code.mParamDest.isPureRegister() && code.mParamDest.mRegister.mPreDecrement == 0 && code.mParamDest.mRegister.mPostIncrement == 0);
					const bool canHandleSource = (code.mParamSource.isRegister() && code.mParamSource.mRegister.mPreDecrement == 0 && code.mParamSource.mRegister.mPostIncrement == 0) || code.mParamSource.isConstant() || code.mParamSource.isCombinedMemory();

					if (canHandleDest && canHandleSource && !code.mDataType.isSigned())
					{
						CodeLemonTokenTree& tokenTree = genericmanager::Manager<Code>::create<CodeLemonTokenTree>();
						lemon::BinaryOperationToken& bot = tokenTree.mRoot.create<lemon::BinaryOperationToken>();
						bot.mOperator = lemon::Operator::ASSIGN;

						if (code.mParamDest.isPureRegister())
						{
							lemon::VariableToken& variableToken = bot.mLeft.create<lemon::VariableToken>();
							variableToken.mVariable = CodeLemonTokenTree::getRegisterVariable(code.mParamDest.mRegister.mRegister, code.mDataType);
							variableToken.mDataType = getLemonDataType(code.mDataType);
						}

						createLemonTokenTreeForParameter(bot.mLeft, code.mParamDest, code.mDataType);
						createLemonTokenTreeForParameter(bot.mRight, code.mParamSource, code.mDataType);

						tokenTree.mLines.swap(block[pos].mLines);
						block.replace(tokenTree, pos);
					}
					break;
				}

				case assembly::CodeType::CODE_ADD:
				case assembly::CodeType::CODE_SUB:
				case assembly::CodeType::CODE_MUL:
					// Do not handle CODE_DIV, as it performs two things at once (divide and modulo)
				case assembly::CodeType::CODE_AND:
				case assembly::CodeType::CODE_OR:
				case assembly::CodeType::CODE_XOR:
				{
					// TODO: Get rid of these restrictions
					const bool canHandleDest = (code.mParamDest.isPureRegister() && code.mParamDest.mRegister.mPreDecrement == 0 && code.mParamDest.mRegister.mPostIncrement == 0);
					const bool canHandleSource = (code.mParamSource.isRegister() && code.mParamSource.mRegister.mPreDecrement == 0 && code.mParamSource.mRegister.mPostIncrement == 0) || code.mParamSource.isConstant() || code.mParamSource.isCombinedMemory();

					if (canHandleDest && canHandleSource && !code.mDataType.isSigned())
					{
						CodeLemonTokenTree& tokenTree = genericmanager::Manager<Code>::create<CodeLemonTokenTree>();

						if ((code.mType == assembly::CodeType::CODE_ADD || code.mType == assembly::CodeType::CODE_SUB) &&
							code.mParamSource.isConstantValue() && code.mParamSource.mConstant.mValue == 1)
						{
							// Special handling for +1 and -1
							lemon::UnaryOperationToken& uot = tokenTree.mRoot.create<lemon::UnaryOperationToken>();
							uot.mOperator = (code.mType == assembly::CodeType::CODE_SUB) ? lemon::Operator::UNARY_DECREMENT : lemon::Operator::UNARY_INCREMENT;
							createLemonTokenTreeForParameter(uot.mArgument, code.mParamDest, code.mDataType);
						}
						else
						{
							lemon::BinaryOperationToken& bot = tokenTree.mRoot.create<lemon::BinaryOperationToken>();
							switch (code.mType)
							{
								case assembly::CodeType::CODE_ADD:	bot.mOperator = lemon::Operator::ASSIGN_PLUS;		break;
								case assembly::CodeType::CODE_SUB:	bot.mOperator = lemon::Operator::ASSIGN_MINUS;		break;
								case assembly::CodeType::CODE_MUL:	bot.mOperator = lemon::Operator::ASSIGN_MULTIPLY;	break;
								case assembly::CodeType::CODE_DIV:	bot.mOperator = lemon::Operator::ASSIGN_DIVIDE;		break;
								case assembly::CodeType::CODE_AND:	bot.mOperator = lemon::Operator::ASSIGN_AND;		break;
								case assembly::CodeType::CODE_OR:	bot.mOperator = lemon::Operator::ASSIGN_OR;			break;
								case assembly::CodeType::CODE_XOR:	bot.mOperator = lemon::Operator::ASSIGN_XOR;		break;
							}
							createLemonTokenTreeForParameter(bot.mLeft, code.mParamDest, code.mDataType);
							createLemonTokenTreeForParameter(bot.mRight, code.mParamSource, code.mDataType);
						}

						tokenTree.mLines.swap(block[pos].mLines);
						block.replace(tokenTree, pos);
					}
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

						if (code.mParamDest.isPureRegister())
						{
							lemon::VariableToken& variableToken = bot.mLeft.create<lemon::VariableToken>();
							variableToken.mVariable = CodeLemonTokenTree::getRegisterVariable(code.mParamDest.mRegister.mRegister, code.mDataType);
							variableToken.mDataType = getLemonDataType(code.mDataType);
						}

						createLemonTokenTreeForParameter(bot.mLeft, code.mParamDest, code.mDataType);
						createLemonTokenTreeForParameter(bot.mRight, code.mParamDest, assembly::DataType((assembly::DataType::Size)((int)code.mDataType.mSize / 2), assembly::DataType::Sign::SIGNED));

						tokenTree.mLines.swap(block[pos].mLines);
						block.replace(tokenTree, pos);
					}
					break;
				}
			}
		}

		void optimizeBlock(Block& block)
		{
			// Merge assignment with sign extension
			// TODO: Can this be done with lemon token trees instead?
			for (size_t pos = 0; pos + 1 < block.size(); ++pos)
			{
				const assembly::AssemblyCode* ac1 = getAssemblyCode(block[pos], assembly::CodeType::CODE_MOVE);
				const assembly::AssemblyCode* ac2 = getAssemblyCode(block[pos + 1], assembly::CodeType::CODE_EXTEND_SIGNED);
				if (nullptr != ac1 && nullptr != ac2)
				{
					if (ac1->mDataType.mSize <= assembly::DataType::Size::SIZE_16 && ac1->mParamDest.isPureRegister() && ac1->mParamSource.isConstantMemory() &&
						ac2->mDataType.getSizeInBytes() == (ac1->mDataType.getSizeInBytes() * 2) && ac2->mParamDest.isPureRegister(ac1->mParamDest.mRegister.mRegister))
					{
						const uint8 bytes = ac1->mDataType.getSizeInBytes();

						CodeLemonTokenTree& tokenTree = genericmanager::Manager<Code>::create<CodeLemonTokenTree>();
						lemon::BinaryOperationToken& bot = tokenTree.mRoot.create<lemon::BinaryOperationToken>();
						bot.mOperator = lemon::Operator::ASSIGN;

						lemon::VariableToken& left = bot.mLeft.create<lemon::VariableToken>();
						left.mVariable = CodeLemonTokenTree::getRegisterVariable(ac1->mParamDest.mRegister.mRegister, ac2->mDataType);

						lemon::MemoryAccessToken& memoryAccessToken = bot.mRight.create<lemon::MemoryAccessToken>();
						memoryAccessToken.mDataType = getLemonDataType(assembly::DataType(ac1->mDataType.mSize, assembly::DataType::Sign::SIGNED));

						lemon::ConstantToken& constantToken = memoryAccessToken.mAddress.create<lemon::ConstantToken>();
						constantToken.mValue = ac1->mParamSource.mConstant.mValue;
						constantToken.mDataType = &lemon::PredefinedDataTypes::UINT_32;

						mergeLinesInto(tokenTree.mLines, block[pos].mLines, block[pos + 1].mLines);
						block.replace(tokenTree, pos);
						block.erase(pos + 1);
					}
				}
			}

			// Convert codes to token trees
			for (size_t pos = 0; pos < block.size(); ++pos)
			{
				tryConvertToTokenTree(block, pos);
			}

			// Merge zero assignment to register with additional assignment right after it
			//  D0 = 0
			//  D0.u16 = ...
			for (size_t pos = 0; pos + 1 < block.size(); ++pos)
			{
				if (block[pos].getType() != Code::LEMONTOKENTREE || block[pos + 1].getType() != Code::LEMONTOKENTREE)
					continue;

				lemon::TokenPtr<lemon::StatementToken>& root1 = block[pos].as<CodeLemonTokenTree>().mRoot;
				lemon::TokenPtr<lemon::StatementToken>& root2 = block[pos + 1].as<CodeLemonTokenTree>().mRoot;

				lemon::BinaryOperationToken* bot1 = castAssignmentToken(root1);
				if (nullptr == bot1)
					continue;

				if (bot1->mLeft->getType() != lemon::Token::Type::VARIABLE)
					continue;
				assembly::Register reg1;
				assembly::DataType dataType1;
				CodeLemonTokenTree::splitRegisterVariable(bot1->mLeft->as<lemon::VariableToken>().mVariable, reg1, dataType1);
				if (dataType1.mSize == assembly::DataType::Size::SIZE_8 || dataType1.isSigned())
					continue;

				if (bot1->mRight->getType() != lemon::Token::Type::CONSTANT)
					continue;
				if (bot1->mRight->as<lemon::ConstantToken>().mValue != 0)
					continue;

				const lemon::BinaryOperationToken* bot2 = castAssignmentToken(root2);
				if (nullptr == bot2)
					continue;

				if (bot2->mLeft->getType() != lemon::Token::Type::VARIABLE)
					continue;
				assembly::Register reg2;
				assembly::DataType dataType2;
				CodeLemonTokenTree::splitRegisterVariable(bot2->mLeft->as<lemon::VariableToken>().mVariable, reg2, dataType2);
				if (reg1 != reg2 || dataType1.mSize < dataType2.mSize || dataType2.isSigned())
					continue;

				bot1->mRight = bot2->mRight;
				
				mergeLinesInto(block[pos].mLines, block[pos + 1].mLines);
				block.erase(pos + 1);
			}

			// Form multiplication by two where appropriate
			for (size_t pos = 0; pos < block.size(); ++pos)
			{
				if (block[pos].getType() != Code::LEMONTOKENTREE)
					continue;

				lemon::TokenPtr<lemon::StatementToken>& root = block[pos].as<CodeLemonTokenTree>().mRoot;
				if (root->getType() != lemon::Token::Type::BINARY_OPERATION)
					continue;
				lemon::BinaryOperationToken& bot = root->as<lemon::BinaryOperationToken>();
				if (bot.mOperator != lemon::Operator::ASSIGN_PLUS)
					continue;

				if (bot.mLeft->getType() != lemon::Token::Type::VARIABLE || bot.mRight->getType() != lemon::Token::Type::VARIABLE)
					continue;
				if (bot.mLeft->as<lemon::VariableToken>().mVariable != bot.mRight->as<lemon::VariableToken>().mVariable)
					continue;

				bot.mOperator = lemon::Operator::ASSIGN_MULTIPLY;
				lemon::ConstantToken& constantToken = bot.mRight.create<lemon::ConstantToken>();
				constantToken.mValue = 2;
				constantToken.mDataType = &lemon::PredefinedDataTypes::UINT_8;
			}

			// Merge assignment and combined assign + operation
			//  D0.u16 = ...
			//  D0.u16 += ...
			for (size_t pos = 0; pos + 1 < block.size(); ++pos)
			{
				if (block[pos].getType() != Code::LEMONTOKENTREE || block[pos + 1].getType() != Code::LEMONTOKENTREE)
					continue;

				lemon::TokenPtr<lemon::StatementToken>& root1 = block[pos].as<CodeLemonTokenTree>().mRoot;
				lemon::TokenPtr<lemon::StatementToken>& root2 = block[pos + 1].as<CodeLemonTokenTree>().mRoot;

				// The operation must be an assignment
				lemon::BinaryOperationToken* assignmentBot = castAssignmentToken(root1);
				if (nullptr == assignmentBot)
					continue;

				// Accept only variables as target for the assignment
				if (assignmentBot->mLeft->getType() != lemon::Token::Type::VARIABLE)
					continue;

				// The second line must be a binary assignment operation
				if (root2->getType() != lemon::Token::Type::BINARY_OPERATION)
					continue;

				// Get the second line's binary operation and make sure it uses the same variable as target
				lemon::BinaryOperationToken& secondBot = root2->as<lemon::BinaryOperationToken>();
				if (secondBot.mLeft->getType() != lemon::Token::Type::VARIABLE)
					continue;
				if (assignmentBot->mLeft->as<lemon::VariableToken>().mVariable != secondBot.mLeft->as<lemon::VariableToken>().mVariable)
					continue;

				lemon::Operator binaryOperator;
				switch (secondBot.mOperator)
				{
					case lemon::Operator::ASSIGN_PLUS:			binaryOperator = lemon::Operator::BINARY_PLUS;			break;
					case lemon::Operator::ASSIGN_MINUS:			binaryOperator = lemon::Operator::BINARY_MINUS;			break;
					case lemon::Operator::ASSIGN_MULTIPLY:		binaryOperator = lemon::Operator::BINARY_MULTIPLY;		break;
					case lemon::Operator::ASSIGN_DIVIDE:		binaryOperator = lemon::Operator::BINARY_DIVIDE;		break;
					case lemon::Operator::ASSIGN_MODULO:		binaryOperator = lemon::Operator::BINARY_MODULO;		break;
					case lemon::Operator::ASSIGN_SHIFT_LEFT:	binaryOperator = lemon::Operator::BINARY_SHIFT_LEFT;	break;
					case lemon::Operator::ASSIGN_SHIFT_RIGHT:	binaryOperator = lemon::Operator::BINARY_SHIFT_RIGHT;	break;
					case lemon::Operator::ASSIGN_AND:			binaryOperator = lemon::Operator::BINARY_AND;			break;
					case lemon::Operator::ASSIGN_OR:			binaryOperator = lemon::Operator::BINARY_OR;			break;
					case lemon::Operator::ASSIGN_XOR:			binaryOperator = lemon::Operator::BINARY_XOR;			break;
					default:
						continue;
				}

				bool needsParentheses = false;
				if (assignmentBot->mRight->getType() == lemon::Token::Type::BINARY_OPERATION)
				{
					const uint8 prio1 = lemon::TokenProcessing::getOperatorPriority(assignmentBot->mRight->as<lemon::BinaryOperationToken>().mOperator);
					const uint8 prio2 = lemon::TokenProcessing::getOperatorPriority(secondBot.mOperator);

					// Lower values mean higher priority here, so this actually means that we need parentheses if the right side has a higher priority
					needsParentheses = (prio1 > prio2);
				}

				// Replace the second line's operator, removing the assigment part (and potentially adding parentheses)
				secondBot.mOperator = binaryOperator;
				if (needsParentheses)
				{
					lemon::ParenthesisToken& pt = secondBot.mLeft.create<lemon::ParenthesisToken>();
					pt.mContent.add(*assignmentBot->mRight);
					pt.mParenthesisType = lemon::ParenthesisType::PARENTHESIS;
					pt.mDataType = assignmentBot->mDataType;
				}
				else
				{
					secondBot.mLeft = assignmentBot->mRight;
				}
				assignmentBot->mRight = secondBot;

				mergeLinesInto(block[pos].mLines, block[pos + 1].mLines);
				block.erase(pos + 1);
				--pos;		// Check again at the same position
			}

			// TODO: Add more
		}

		void postprocessIfBlocks(Block& block, const assembly::AssemblyCode* formerAssemblyCode)
		{
			// Refine if-block conditions where possible
			for (size_t pos = 0; pos < block.size(); ++pos)
			{
				if (block[pos].getType() == Code::IFELSE)
				{
					CodeIfElse& ci = block[pos].as<CodeIfElse>();

					const assembly::AssemblyCode* lastAssemblyCode = nullptr;
					bool mergeAllowed = false;
					if (pos == 0)
					{
						// If we're lucky, this block is inside an outer if-block, and we can just take over the same condition from there
						lastAssemblyCode = formerAssemblyCode;
					}
					else
					{
						const Code& formerCode = block[pos-1];

						// Have a look at assembly code just before this if-block
						if (formerCode.getType() == Code::ASSEMBLY)
						{
							lastAssemblyCode = formerCode.as<CodeAssembly>().mAssemblyCode;
							mergeAllowed = true;	// If it is the right type of assembly code, we may remove it afterwards
						}
						// Is former code just another if-code?
						else if (formerCode.getType() == Code::IFELSE)
						{
							lastAssemblyCode = formerCode.as<CodeIfElse>().mAssemblyCode;
						}
					}

					if (lastAssemblyCode != nullptr)
					{
						switch (lastAssemblyCode->mType)
						{
							case assembly::CodeType::CODE_CMP:
							case assembly::CodeType::CODE_TEST:
							case assembly::CodeType::CODE_TEST_BIT:
							{
								ci.mAssemblyCode = lastAssemblyCode;

								// Merge with last code -- only allowed if it is actually inside the same block
								if (mergeAllowed)
								{
									mergeLinesInto(block[pos].mLines, block[pos - 1].mLines, true);
									block.erase(pos-1, 1);
									--pos;
								}
								break;
							}

							case assembly::CodeType::CODE_MOVE:
							case assembly::CodeType::CODE_ADD:
							case assembly::CodeType::CODE_SUB:
							case assembly::CodeType::CODE_AND:
							case assembly::CodeType::CODE_OR:
							case assembly::CodeType::CODE_SHIFT_LEFT:
							case assembly::CodeType::CODE_SHIFT_RIGHT:		// TODO: Add more?
							case assembly::CodeType::CODE_EXTEND_SIGNED:
							{
								ci.mAssemblyCode = lastAssemblyCode;
								break;
							}
						}
					}

					// Go deeper
					//  -> It's important to do this after we fully processed the current if-code
					postprocessIfBlocks(ci.mIfBlock, ci.mAssemblyCode);
					postprocessIfBlocks(ci.mElseBlock, ci.mAssemblyCode);
				}
				else if (block[pos].getType() == Code::WHILE)
				{
					CodeWhile& cw = block[pos].as<CodeWhile>();

					// Go deeper here as well
					postprocessIfBlocks(cw.mInnerBlock, nullptr);
				}
			}
		}

	}


	void Optimization::optimize(Block& block)
	{
		detail::createStructuredBlocks(block);
		detail::postprocessIfBlocks(block, nullptr);

		// Build list of all blocks, including inner blocks of if/else/while
		std::vector<Block*> allBlocks;
		detail::collectInnerBlocks(block, allBlocks);

		for (Block* childBlock : allBlocks)
		{
			detail::optimizeBlock(*childBlock);
		}
	}

}
