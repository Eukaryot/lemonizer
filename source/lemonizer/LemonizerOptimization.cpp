#include "pch.h"
#include "lemonizer/LemonizerOptimization.h"
#include "lemonizer/LemonizerCode.h"
#include "lemonizer/LemonizerHelper.h"
#include "lemonizer/LemonizerStructuring.h"
#include "lemonizer/TokenTreeConverter.h"
#include "assembly/CodeOutputHelper.h"
#include "builder/RomContent.h"

#include <lemon/compiler/frontend/TokenProcessing.h>
#include <lemon/program/Variable.h>


namespace lemonizer
{
	namespace detail
	{
		// Wrapper that points to either a TokenPtr or TokenList entry
		struct StatementTokenReference
		{
			StatementTokenReference(lemon::TokenPtr<lemon::StatementToken>& ptr) : mPtr(&ptr) {}
			StatementTokenReference(lemon::TokenList& list, size_t index) : mList(&list), mIndex(index) {}

			lemon::StatementToken* operator->() const  { return (nullptr != mPtr) ? mPtr->get() : &static_cast<lemon::StatementToken&>((*mList)[mIndex]); }

			void operator=(lemon::StatementToken& token) const
			{
				if (nullptr != mPtr)
					(*mPtr) = token;
				else
					mList->replace(token, mIndex);
			}

		private:
			lemon::TokenPtr<lemon::StatementToken>* mPtr = nullptr;
			lemon::TokenList* mList = nullptr;
			size_t mIndex = 0;
		};


		bool isControlFlowEnd(const Code& code)
		{
			// Must be an unconditional jump or a return
			const CodeJumpOrCall* jumpOrCall = code.cast<CodeJumpOrCall>();
			if (nullptr != jumpOrCall)
			{
				return !jumpOrCall->mIsCall;
			}

			if (code.isA<CodeReturn>())
				return true;
			if (nullptr != CodeAssembly::getAssemblyCode(code, assembly::CodeType::CODE_RETURN))
				return true;

			return false;
		}

		void mergeLinesInto(std::vector<const LineData*>& destination, std::vector<const LineData*>& source, bool atStart = false)
		{
			destination.insert(atStart ? destination.begin() : destination.end(), source.begin(), source.end());
			source.clear();
		}

		void mergeLinesInto(std::vector<const LineData*>& output, std::vector<const LineData*>& input1, std::vector<const LineData*>& input2)
		{
			output.swap(input1);
			mergeLinesInto(output, input2);
		}

		void mergeCodes(Block& block, int pos, int count)
		{
			RMX_ASSERT(count >= 2, "Count is expected to be at least 2");
			for (int k = 1; k < count; ++k)
			{
				mergeLinesInto(block[pos].mLines, block[pos + k].mLines);
			}
			block.erase(pos + 1, count - 1);
		}

		void mergeCodes(CodeLemonTokenTree& tokenTree, Block& block, int pos, int count)
		{
			RMX_ASSERT(count >= 2, "Count is expected to be at least 2");
			tokenTree.mLines.swap(block[pos].mLines);
			for (int k = 1; k < count; ++k)
			{
				mergeLinesInto(tokenTree.mLines, block[pos + k].mLines);
			}
			block.replace(tokenTree, pos);
			block.erase(pos + 1, count - 1);
		}

		lemon::UnaryOperationToken* castUnaryOperationToken(lemon::TokenPtr<lemon::StatementToken>& token, lemon::Operator op)
		{
			lemon::UnaryOperationToken* assignmentUot = token->cast<lemon::UnaryOperationToken>();
			if (nullptr == assignmentUot || assignmentUot->mOperator != op)
				return nullptr;
			return assignmentUot;
		}

		lemon::BinaryOperationToken* castBinaryOperationToken(lemon::TokenPtr<lemon::StatementToken>& token, lemon::Operator op)
		{
			lemon::BinaryOperationToken* assignmentBot = token->cast<lemon::BinaryOperationToken>();
			if (nullptr == assignmentBot || assignmentBot->mOperator != op)
				return nullptr;
			return assignmentBot;
		}

		lemon::BinaryOperationToken* castAssignmentToken(lemon::TokenPtr<lemon::StatementToken>& token)
		{
			return castBinaryOperationToken(token, lemon::Operator::ASSIGN);
		}

		lemon::BinaryOperationToken* getAssignmentToken(Code& code)
		{
			CodeLemonTokenTree* tokenTree = code.cast<CodeLemonTokenTree>();
			if (nullptr == tokenTree)
				return nullptr;
			return castAssignmentToken(tokenTree->mRoot);
		}

		const lemon::DataTypeDefinition* getLemonDataType(const assembly::DataType& dataType, bool isSigned)
		{
			switch (dataType.mSize)
			{
				case assembly::DataType::Size::SIZE_8:   return isSigned ? &lemon::PredefinedDataTypes::INT_8  : &lemon::PredefinedDataTypes::UINT_8;
				case assembly::DataType::Size::SIZE_16:  return isSigned ? &lemon::PredefinedDataTypes::INT_16 : &lemon::PredefinedDataTypes::UINT_16;
				case assembly::DataType::Size::SIZE_32:  return isSigned ? &lemon::PredefinedDataTypes::INT_32 : &lemon::PredefinedDataTypes::UINT_32;
			}
			return &lemon::PredefinedDataTypes::UINT_32;
		}

		const lemon::DataTypeDefinition* getLemonDataType(const assembly::DataType& dataType)
		{
			return getLemonDataType(dataType, dataType.isSigned());
		}

		const lemon::DataTypeDefinition* makeSignedDataType(const lemon::DataTypeDefinition* dataType)
		{
			if (dataType == &lemon::PredefinedDataTypes::UINT_8)
				return &lemon::PredefinedDataTypes::INT_8;
			else if (dataType == &lemon::PredefinedDataTypes::UINT_16)
				return &lemon::PredefinedDataTypes::INT_16;
			else if (dataType == &lemon::PredefinedDataTypes::UINT_32)
				return &lemon::PredefinedDataTypes::INT_32;
			else if (dataType == &lemon::PredefinedDataTypes::UINT_64)
				return &lemon::PredefinedDataTypes::INT_64;
			else
				return dataType;
		}

		void replaceRegisterInTokenTree(const StatementTokenReference& token, assembly::Register reg, assembly::DataType dataType, lemon::ConstantTokenExt& constantToken, bool& outFoundSizeMismatch)
		{
			if (lemon::VariableToken* vt = token->cast<lemon::VariableToken>())
			{
				assembly::Register reg1;
				assembly::DataType dataType1;
				CodeLemonTokenTree::splitRegisterVariable(vt->mVariable, reg1, dataType1);
				if (reg == reg1)
				{
					if (dataType1.getSizeInBytes() <= dataType.getSizeInBytes())
					{
						token = constantToken;
					}
					else
					{
						outFoundSizeMismatch = true;
					}
				}
			}
			else if (lemon::BinaryOperationToken* bot = token->cast<lemon::BinaryOperationToken>())
			{
				replaceRegisterInTokenTree(StatementTokenReference(bot->mLeft), reg, dataType, constantToken, outFoundSizeMismatch);
				replaceRegisterInTokenTree(StatementTokenReference(bot->mRight), reg, dataType, constantToken, outFoundSizeMismatch);
			}
			else if (lemon::UnaryOperationToken* uot = token->cast<lemon::UnaryOperationToken>())
			{
				replaceRegisterInTokenTree(StatementTokenReference(uot->mArgument), reg, dataType, constantToken, outFoundSizeMismatch);
			}
			else if (lemon::MemoryAccessToken* mat = token->cast<lemon::MemoryAccessToken>())
			{
				replaceRegisterInTokenTree(StatementTokenReference(mat->mAddress), reg, dataType, constantToken, outFoundSizeMismatch);
			}
			else if (lemon::ParenthesisToken* pt = token->cast<lemon::ParenthesisToken>())
			{
				for (size_t k = 0; k < pt->mContent.size(); ++k)
				{
					replaceRegisterInTokenTree(StatementTokenReference(pt->mContent, k), reg, dataType, constantToken, outFoundSizeMismatch);
				}
			}
			else if (lemon::ValueCastToken* vct = token->cast<lemon::ValueCastToken>())
			{
				replaceRegisterInTokenTree(StatementTokenReference(vct->mArgument), reg, dataType, constantToken, outFoundSizeMismatch);
			}
		}


		void optimizeBlock(Block& block)
		{
			// Merge assignment with sign extension
			//  D0.s8 = ...
			//  D0.s16 = D0.s8
			for (size_t pos = 0; pos + 1 < block.size(); ++pos)
			{
				const assembly::AssemblyCode* ac1 = CodeAssembly::getAssemblyCode(block[pos], assembly::CodeType::CODE_MOVE);
				const assembly::AssemblyCode* ac2 = CodeAssembly::getAssemblyCode(block[pos + 1], assembly::CodeType::CODE_EXTEND_SIGNED);
				if (nullptr != ac1 && nullptr != ac2)
				{
					if (ac1->mDataType.mSize <= assembly::DataType::Size::SIZE_16 && ac1->mParamDest.isPureRegister() &&
						ac2->mDataType.getSizeInBytes() == (ac1->mDataType.getSizeInBytes() * 2) && ac2->mParamDest.isPureRegister(ac1->mParamDest.mRegister.mRegister))
					{
						// Check if there's another sign extension afterwards that we can merge as well
						const assembly::DataType* mergeTwoSignExtensions = nullptr;
						if (pos + 2 < block.size())
						{
							const assembly::AssemblyCode* ac3 = CodeAssembly::getAssemblyCode(block[pos + 2], assembly::CodeType::CODE_EXTEND_SIGNED);
							if (nullptr != ac3)
							{
								if (ac3->mDataType.getSizeInBytes() == (ac1->mDataType.getSizeInBytes() * 4) && ac3->mParamDest.isPureRegister(ac1->mParamDest.mRegister.mRegister))
								{
									mergeTwoSignExtensions = &ac3->mDataType;
								}
							}
						}

						CodeLemonTokenTree& tokenTree = genericmanager::Manager<Code>::create<CodeLemonTokenTree>();
						lemon::BinaryOperationToken& bot = tokenTree.mRoot.create<lemon::BinaryOperationToken>();
						bot.mOperator = lemon::Operator::ASSIGN;

						lemon::VariableToken& left = bot.mLeft.create<lemon::VariableToken>();
						const assembly::DataType outputType = (nullptr != mergeTwoSignExtensions) ? *mergeTwoSignExtensions : ac2->mDataType;
						left.mVariable = CodeLemonTokenTree::getRegisterVariable(ac1->mParamDest.mRegister.mRegister, outputType);

						const assembly::DataType signedDataType(ac1->mDataType.mSize, assembly::DataType::Sign::SIGNED);
						TokenTreeConverter::createLemonTokenTreeForParameter(bot.mRight, ac1->mParamSource, signedDataType);

						mergeCodes(tokenTree, block, (int)pos, (nullptr != mergeTwoSignExtensions) ? 3 : 2);
					}
				}
			}

			// Sign two extensions (in case there's no assignment before)
			//  D0.s16 = D0.s8
			//  D0.s32 = D0.s16
			for (size_t pos = 0; pos + 1 < block.size(); ++pos)
			{
				const assembly::AssemblyCode* ac1 = CodeAssembly::getAssemblyCode(block[pos], assembly::CodeType::CODE_EXTEND_SIGNED);
				const assembly::AssemblyCode* ac2 = CodeAssembly::getAssemblyCode(block[pos + 1], assembly::CodeType::CODE_EXTEND_SIGNED);
				if (nullptr != ac1 && nullptr != ac2)
				{
					if (ac1->mDataType.mSize == assembly::DataType::Size::SIZE_16 && ac1->mParamDest.isPureRegister() &&
						ac2->mDataType.mSize == assembly::DataType::Size::SIZE_32 && ac2->mParamDest.isPureRegister(ac1->mParamDest.mRegister.mRegister))
					{
						CodeLemonTokenTree& tokenTree = genericmanager::Manager<Code>::create<CodeLemonTokenTree>();
						lemon::BinaryOperationToken& bot = tokenTree.mRoot.create<lemon::BinaryOperationToken>();
						bot.mOperator = lemon::Operator::ASSIGN;

						lemon::VariableToken& left = bot.mLeft.create<lemon::VariableToken>();
						const assembly::DataType outputType = ac2->mDataType;
						left.mVariable = CodeLemonTokenTree::getRegisterVariable(ac1->mParamDest.mRegister.mRegister, outputType);

						const assembly::DataType signedDataType(assembly::DataType::Size::SIZE_8, assembly::DataType::Sign::SIGNED);
						TokenTreeConverter::createLemonTokenTreeForParameter(bot.mRight, ac1->mParamDest, signedDataType);

						mergeCodes(tokenTree, block, (int)pos, 2);
					}
				}
			}

			// Merge word swaps with clear afterwards
			//  D0 = (D0 << 16) + (D0 >> 16)
			//  D0.u16 = 0
			for (size_t pos = 0; pos + 1 < block.size(); ++pos)
			{
				const assembly::AssemblyCode* ac1 = CodeAssembly::getAssemblyCode(block[pos], assembly::CodeType::CODE_SWAP_WORDS);
				const assembly::AssemblyCode* ac2 = CodeAssembly::getAssemblyCode(block[pos + 1], assembly::CodeType::CODE_CLEAR);
				if (nullptr != ac1 && nullptr != ac2)
				{
					if (ac1->mDataType.getSizeInBytes() == 4 && ac1->mParamDest.isPureRegister() &&
						ac2->mDataType.getSizeInBytes() == 2 && ac2->mParamDest.isPureRegister(ac1->mParamDest.mRegister.mRegister))
					{
						// Check if there's an additional word swap afterwards taht can be merged as well
						if (pos + 2 < block.size())
						{
							const assembly::AssemblyCode* ac3 = CodeAssembly::getAssemblyCode(block[pos + 2], assembly::CodeType::CODE_SWAP_WORDS);
							if (nullptr != ac3 && ac3->mDataType.getSizeInBytes() == 4 && ac3->mParamDest.isPureRegister(ac1->mParamDest.mRegister.mRegister))
							{
								CodeLemonTokenTree& tokenTree = genericmanager::Manager<Code>::create<CodeLemonTokenTree>();
								lemon::BinaryOperationToken& bot = tokenTree.mRoot.create<lemon::BinaryOperationToken>();
								bot.mOperator = lemon::Operator::ASSIGN_AND;

								TokenTreeConverter::createLemonTokenTreeForParameter(bot.mLeft, ac1->mParamDest, ac1->mDataType);
								TokenTreeConverter::createConstantToken(bot.mRight, 0xffff, &lemon::PredefinedDataTypes::UINT_32);

								mergeCodes(tokenTree, block, (int)pos, 3);
								continue;
							}
						}

						CodeLemonTokenTree& tokenTree = genericmanager::Manager<Code>::create<CodeLemonTokenTree>();
						lemon::BinaryOperationToken& bot = tokenTree.mRoot.create<lemon::BinaryOperationToken>();
						bot.mOperator = lemon::Operator::ASSIGN_SHIFT_LEFT;

						TokenTreeConverter::createLemonTokenTreeForParameter(bot.mLeft, ac1->mParamDest, ac1->mDataType);
						TokenTreeConverter::createConstantToken(bot.mRight, 16, &lemon::PredefinedDataTypes::UINT_32);

						mergeCodes(tokenTree, block, (int)pos, 2);
					}
				}
			}

			// Merge word swaps with clear before
			//  D0.u16 = 0
			//  D0 = (D0 << 16) + (D0 >> 16)
			for (size_t pos = 0; pos + 1 < block.size(); ++pos)
			{
				const assembly::AssemblyCode* ac1 = CodeAssembly::getAssemblyCode(block[pos], assembly::CodeType::CODE_CLEAR);
				const assembly::AssemblyCode* ac2 = CodeAssembly::getAssemblyCode(block[pos + 1], assembly::CodeType::CODE_SWAP_WORDS);
				if (nullptr != ac1 && nullptr != ac2)
				{
					if (ac1->mDataType.getSizeInBytes() == 2 && ac1->mParamDest.isPureRegister() &&
						ac2->mDataType.getSizeInBytes() == 4 && ac2->mParamDest.isPureRegister(ac1->mParamDest.mRegister.mRegister))
					{
						CodeLemonTokenTree& tokenTree = genericmanager::Manager<Code>::create<CodeLemonTokenTree>();
						lemon::BinaryOperationToken& bot = tokenTree.mRoot.create<lemon::BinaryOperationToken>();
						bot.mOperator = lemon::Operator::ASSIGN_SHIFT_RIGHT;

						TokenTreeConverter::createLemonTokenTreeForParameter(bot.mLeft, ac1->mParamDest, ac2->mDataType);
						TokenTreeConverter::createConstantToken(bot.mRight, 16, &lemon::PredefinedDataTypes::UINT_32);

						mergeCodes(tokenTree, block, (int)pos, 2);
					}
				}
			}

			// Convert codes to token trees
			for (size_t pos = 0; pos < block.size(); ++pos)
			{
				TokenTreeConverter::tryConvertToTokenTree(block, pos);
			}

			// Clean up simple register additions / subtractions
			//  D0 = D0 + 8
			for (size_t pos = 0; pos < block.size(); ++pos)
			{
				lemon::BinaryOperationToken* bot1 = getAssignmentToken(block[pos]);
				if (nullptr == bot1)
					continue;

				lemon::BinaryOperationToken* bot2 = bot1->mRight->cast<lemon::BinaryOperationToken>();
				if (nullptr == bot2)
					continue;
				if (bot2->mOperator != lemon::Operator::BINARY_PLUS && bot2->mOperator != lemon::Operator::BINARY_MINUS)
					continue;

				lemon::VariableToken* var1 = bot1->mLeft->cast<lemon::VariableToken>();
				lemon::VariableToken* var2 = bot2->mLeft->cast<lemon::VariableToken>();
				if (nullptr == var1 || nullptr == var2)
					continue;
				if (var1->mVariable != var2->mVariable)
					continue;

				bot1->mOperator = (bot2->mOperator == lemon::Operator::BINARY_PLUS) ? lemon::Operator::ASSIGN_PLUS : lemon::Operator::ASSIGN_MINUS;
				lemon::TokenPtr<lemon::StatementToken> tmp = bot2->mRight;
				bot1->mRight = tmp;
			}

			// Create a zero assignment where subtraction is used to set a register to zero
			//  D0 -= D0
			for (size_t pos = 0; pos < block.size(); ++pos)
			{
				if (!block[pos].isA<CodeLemonTokenTree>())
					continue;

				lemon::BinaryOperationToken* bot = castBinaryOperationToken(block[pos].as<CodeLemonTokenTree>().mRoot, lemon::Operator::ASSIGN_MINUS);
				if (nullptr == bot)
					continue;

				lemon::VariableToken* var1 = bot->mLeft->cast<lemon::VariableToken>();
				lemon::VariableToken* var2 = bot->mRight->cast<lemon::VariableToken>();
				if (nullptr == var1 || nullptr == var2)
					continue;
				if (var1->mVariable != var2->mVariable)
					continue;

				bot->mOperator = lemon::Operator::ASSIGN;
				TokenTreeConverter::createConstantToken(bot->mRight, 0, &lemon::PredefinedDataTypes::UINT_32);
			}

			// Merge zero assignment to register with additional assignment right after it
			//  D0 = 0
			//  D0.u16 = ...
			for (size_t pos = 0; pos + 1 < block.size(); ++pos)
			{
				lemon::BinaryOperationToken* bot1 = getAssignmentToken(block[pos]);
				const lemon::BinaryOperationToken* bot2 = getAssignmentToken(block[pos + 1]);
				if (nullptr == bot1 || nullptr == bot2)
					continue;

				if (!bot1->mLeft->isA<lemon::VariableToken>())
					continue;
				assembly::Register reg1;
				assembly::DataType dataType1;
				CodeLemonTokenTree::splitRegisterVariable(bot1->mLeft->as<lemon::VariableToken>().mVariable, reg1, dataType1);
				if (dataType1.mSize == assembly::DataType::Size::SIZE_8 || dataType1.isSigned())
					continue;

				lemon::ConstantTokenExt* ct = bot1->mRight->cast<lemon::ConstantTokenExt>();
				if (nullptr == ct || ct->mValue.get<uint64>() != 0)
					continue;

				if (!bot2->mLeft->isA<lemon::VariableToken>())
					continue;
				assembly::Register reg2;
				assembly::DataType dataType2;
				CodeLemonTokenTree::splitRegisterVariable(bot2->mLeft->as<lemon::VariableToken>().mVariable, reg2, dataType2);
				if (reg1 != reg2 || dataType1.mSize < dataType2.mSize || dataType2.isSigned())
					continue;

				bot1->mRight = bot2->mRight;

				mergeCodes(block, (int)pos, 2);
			}

			// Form multiplication by two where appropriate
			for (size_t pos = 0; pos < block.size(); ++pos)
			{
				if (!block[pos].isA<CodeLemonTokenTree>())
					continue;

				lemon::TokenPtr<lemon::StatementToken>& root = block[pos].as<CodeLemonTokenTree>().mRoot;
				if (!root->isA<lemon::BinaryOperationToken>())
					continue;
				lemon::BinaryOperationToken& bot = root->as<lemon::BinaryOperationToken>();
				if (bot.mOperator != lemon::Operator::ASSIGN_PLUS)
					continue;

				if (!bot.mLeft->isA<lemon::VariableToken>() || !bot.mRight->isA<lemon::VariableToken>())
					continue;
				if (bot.mLeft->as<lemon::VariableToken>().mVariable != bot.mRight->as<lemon::VariableToken>().mVariable)
					continue;

				bot.mOperator = lemon::Operator::ASSIGN_MULTIPLY;
				lemon::ConstantTokenExt& ct = TokenTreeConverter::createConstantToken(bot.mRight, 2, &lemon::PredefinedDataTypes::UINT_8);
				ct.mOutputAsDecimal = true;
			}

			// Merge assignment and negation
			//  D0.u16 = ...
			//  D0.s16 = -D0.s16
			for (size_t pos = 0; pos + 1 < block.size(); ++pos)
			{
				// Both operations must be assignments
				lemon::BinaryOperationToken* assignmentBot = getAssignmentToken(block[pos]);
				lemon::BinaryOperationToken* secondBot = getAssignmentToken(block[pos + 1]);
				if (nullptr == assignmentBot || nullptr == secondBot)
					continue;

				// Check for negation in the second line
				lemon::UnaryOperationToken* uot = castUnaryOperationToken(secondBot->mRight, lemon::Operator::BINARY_MINUS);
				if (nullptr == uot)
					continue;

				// Target of the first assignment and negated value must both be variables
				lemon::VariableToken* destVar = assignmentBot->mLeft->cast<lemon::VariableToken>();
				lemon::VariableToken* sourceVar = uot->mArgument->cast<lemon::VariableToken>();
				if (nullptr == destVar || nullptr == sourceVar)
					continue;

				// Both variables must refer to the same register and size, but signs may be different
				assembly::Register reg1;
				assembly::DataType dataType1;
				CodeLemonTokenTree::splitRegisterVariable(destVar->mVariable, reg1, dataType1);
				assembly::Register reg2;
				assembly::DataType dataType2;
				CodeLemonTokenTree::splitRegisterVariable(sourceVar->mVariable, reg2, dataType2);
				if (reg1 != reg2 || dataType1.mSize != dataType2.mSize)
					continue;

				// Use the signed version as destination for the output assignment
				destVar->mVariable = sourceVar->mVariable;

				// Temporarily using the second line's right side to build what is meant to become the first line's right side
				bool castNeeded = true;
				if (assignmentBot->mRight->isA<lemon::VariableToken>())
				{
					assembly::Register reg3;
					assembly::DataType dataType3;
					CodeLemonTokenTree::splitRegisterVariable(assignmentBot->mRight->as<lemon::VariableToken>().mVariable, reg3, dataType3);
					dataType3.mSign = assembly::DataType::Sign::SIGNED;
					assignmentBot->mRight->as<lemon::VariableToken>().mVariable = CodeLemonTokenTree::getRegisterVariable(reg3, dataType3);
					castNeeded = (dataType3.mSize != dataType1.mSize);
				}
				else if (assignmentBot->mRight->isA<lemon::MemoryAccessToken>())
				{
					const lemon::DataTypeDefinition*& memoryAccessDataType = assignmentBot->mRight->as<lemon::MemoryAccessToken>().mDataType;
					memoryAccessDataType = makeSignedDataType(memoryAccessDataType);
					castNeeded = (memoryAccessDataType->getBytes() != dataType1.getSizeInBytes());
				}
				else
				{
					// This includes the case that it's an identifier (like a define)
					if (nullptr != assignmentBot->mRight->mDataType && assignmentBot->mRight->mDataType->getClass() == lemon::DataTypeDefinition::Class::INTEGER)
					{
						castNeeded = !static_cast<const lemon::IntegerDataType*>(assignmentBot->mRight->mDataType)->mIsSigned;
					}
				}

				if (castNeeded)
				{
					// Cast needed
					lemon::ValueCastToken& valueCastToken = uot->mArgument.create<lemon::ValueCastToken>();
					valueCastToken.mArgument = assignmentBot->mRight;
					valueCastToken.mDataType = getLemonDataType(dataType2);
				}
				else
				{
					uot->mArgument = assignmentBot->mRight;
				}
				assignmentBot->mRight = uot;

				mergeCodes(block, (int)pos, 2);
			}

			// Merge assignment and combined assign + operation
			//  D0.u16 = ...
			//  D0.u16 += ...
			for (size_t pos = 0; pos + 1 < block.size(); ++pos)
			{
				// The operation must be an assignment
				lemon::BinaryOperationToken* assignmentBot = getAssignmentToken(block[pos]);
				if (nullptr == assignmentBot)
					continue;

				// Accept only variables as target for the assignment
				lemon::VariableToken* vt1 = assignmentBot->mLeft->cast<lemon::VariableToken>();
				if (nullptr == vt1)
					continue;

				if (!block[pos + 1].isA<CodeLemonTokenTree>())
					continue;

				lemon::TokenPtr<lemon::StatementToken>& root2 = block[pos + 1].as<CodeLemonTokenTree>().mRoot;

				// The second line must be a binary are unary assignment operation
				if (root2->isA<lemon::BinaryOperationToken>())
				{
					// Get the second line's binary operation and make sure it's a variable as well
					lemon::BinaryOperationToken& secondBot = root2->as<lemon::BinaryOperationToken>();
					lemon::VariableToken* vt2 = secondBot.mLeft->cast<lemon::VariableToken>();
					if (nullptr == vt2)
						continue;

					// If both variables are identical, merging is no problem
					//  -> Otherwise we allow merging only in certain cases
					if (vt1->mVariable != vt2->mVariable)
					{
						assembly::Register reg1;
						assembly::DataType dataType1;
						CodeLemonTokenTree::splitRegisterVariable(vt1->mVariable, reg1, dataType1);
						assembly::Register reg2;
						assembly::DataType dataType2;
						CodeLemonTokenTree::splitRegisterVariable(vt2->mVariable, reg2, dataType2);

						// It needs to be the same register, though different signs might be okay
						if (reg1 != reg2 || dataType1.mSize != dataType2.mSize)
							continue;

						if (secondBot.mOperator == lemon::Operator::ASSIGN_PLUS || secondBot.mOperator == lemon::Operator::ASSIGN_MINUS)
						{
							// Allow additions and subtractions even if signedness differs, but ensure the right side is casted properly
							if (nullptr == assignmentBot->mRight->mDataType || assignmentBot->mRight->mDataType->getBytes() != dataType1.getSizeInBytes())
							{
								lemon::TokenPtr<lemon::StatementToken> tmp = assignmentBot->mRight;
								lemon::ValueCastToken& vct = assignmentBot->mRight.create<lemon::ValueCastToken>();
								vct.mArgument = tmp;
								vct.mDataType = getLemonDataType(dataType1);
							}
						}
						else if (secondBot.mOperator == lemon::Operator::ASSIGN_SHIFT_LEFT || secondBot.mOperator == lemon::Operator::ASSIGN_SHIFT_RIGHT)
						{
							// The right side of the first assignment must match the size of the register
							if (nullptr == assignmentBot->mRight->mDataType || assignmentBot->mRight->mDataType->getBytes() != dataType1.getSizeInBytes())
								continue;

							// The right side of the first assignment must also be a register
							lemon::VariableToken* vt3 = assignmentBot->mRight->cast<lemon::VariableToken>();
							if (nullptr == vt3)
								continue;

							// Change the signedness of both registers to fit
							dataType1 = dataType2;
							vt1->mVariable = CodeLemonTokenTree::getRegisterVariable(reg1, dataType1);

							assembly::Register reg3;
							assembly::DataType dataType3;
							CodeLemonTokenTree::splitRegisterVariable(vt3->mVariable, reg3, dataType3);
							dataType3 = dataType2;
							vt3->mVariable = CodeLemonTokenTree::getRegisterVariable(reg3, dataType3);
						}
						else
						{
							// TODO: We could possibly allow subtractions and others as well
							continue;
						}
					}

					const lemon::Operator binaryOperator = lemon::OperatorHelper::getBinaryForAssign(secondBot.mOperator);
					if (binaryOperator == lemon::Operator::_INVALID)
						continue;

					const bool needsParentheses = TokenTreeConverter::needsParentheses(assignmentBot->mRight, binaryOperator, true);

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

					mergeCodes(block, (int)pos, 2);
					--pos;		// Check again at the same position
				}
				else if (root2->isA<lemon::UnaryOperationToken>())
				{
					// Get the second line's binary operation and make sure it's a variable as well
					lemon::UnaryOperationToken& secondUot = root2->as<lemon::UnaryOperationToken>();
					lemon::VariableToken* vt2 = secondUot.mArgument->cast<lemon::VariableToken>();
					if (nullptr == vt2)
						continue;

					lemon::Operator binaryOperator = lemon::Operator::_INVALID;
					switch (secondUot.mOperator)
					{
						// Currently supporting only increment and decrement
						case lemon::Operator::UNARY_DECREMENT:  binaryOperator = lemon::Operator::BINARY_MINUS;  break;
						case lemon::Operator::UNARY_INCREMENT:  binaryOperator = lemon::Operator::BINARY_PLUS;  break;
						default:
							break;
					}
					if (binaryOperator == lemon::Operator::_INVALID)
						continue;

					// If both variables are identical, merging is no problem
					//  -> Otherwise we allow merging only in certain cases
					if (vt1->mVariable != vt2->mVariable)
					{
						assembly::Register reg1;
						assembly::DataType dataType1;
						CodeLemonTokenTree::splitRegisterVariable(vt1->mVariable, reg1, dataType1);
						assembly::Register reg2;
						assembly::DataType dataType2;
						CodeLemonTokenTree::splitRegisterVariable(vt2->mVariable, reg2, dataType2);

						// It needs to be the same register, though different signs might be okay
						if (reg1 != reg2 || dataType1.mSize != dataType2.mSize)
							continue;
					}

					const bool needsParentheses = TokenTreeConverter::needsParentheses(assignmentBot->mRight, binaryOperator, true);

					lemon::TokenPtr<lemon::StatementToken> newValuePtr;
					lemon::BinaryOperationToken& newBot = newValuePtr.create<lemon::BinaryOperationToken>();
					newBot.mDataType = secondUot.mDataType;
					newBot.mOperator = binaryOperator;

					if (needsParentheses)
					{
						lemon::ParenthesisToken& pt = newBot.mLeft.create<lemon::ParenthesisToken>();
						pt.mParenthesisType = lemon::ParenthesisType::PARENTHESIS;
						pt.mDataType = assignmentBot->mDataType;
						pt.mContent.add(*assignmentBot->mRight);
					}
					else
					{
						newBot.mLeft = assignmentBot->mRight;
					}

					lemon::ConstantTokenExt& ct = TokenTreeConverter::createConstantToken(newBot.mRight, 1, newBot.mDataType);
					ct.mOutputAsDecimal = true;

					assignmentBot->mRight = newValuePtr;

					mergeCodes(block, (int)pos, 2);
					--pos;		// Check again at the same position
				}
			}

			// For "A7 += x", update data type for x if it is below 10 to enforce decimal output
			for (size_t pos = 0; pos < block.size(); ++pos)
			{
				if (!block[pos].isA<CodeLemonTokenTree>())
					continue;

				lemon::TokenPtr<lemon::StatementToken>& root = block[pos].as<CodeLemonTokenTree>().mRoot;
				lemon::BinaryOperationToken* bot = castBinaryOperationToken(root, lemon::Operator::ASSIGN_PLUS);
				if (nullptr == bot)
				{
					bot = castBinaryOperationToken(root, lemon::Operator::ASSIGN_MINUS);
					if (nullptr == bot)
						continue;
				}

				lemon::VariableToken* vt = bot->mLeft->cast<lemon::VariableToken>();
				if (nullptr == vt)
					continue;

				assembly::Register reg;
				assembly::DataType dataType;
				CodeLemonTokenTree::splitRegisterVariable(vt->mVariable, reg, dataType);
				if (reg != assembly::Register::A7)
					continue;

				if (!bot->mRight->isA<lemon::ConstantTokenExt>())
					continue;

				lemon::ConstantTokenExt& ct = bot->mRight->as<lemon::ConstantTokenExt>();
				if (nullptr != ct.mDataType || ct.mValue.get<uint64>() >= 10)
					continue;

				ct.mDataType = &lemon::PredefinedDataTypes::UINT_8;
			}

			// When a register is assigned a constant, try to replace its following appearances directly with the constant
			for (size_t pos = 0; pos + 1 < block.size(); ++pos)
			{
				// The operation must be an assignment
				lemon::BinaryOperationToken* assignmentBot = getAssignmentToken(block[pos]);
				if (nullptr == assignmentBot)
					continue;

				// Accept only variables as target for the assignment
				lemon::VariableToken* vt1 = assignmentBot->mLeft->cast<lemon::VariableToken>();
				if (nullptr == vt1)
					continue;

				// Right side must be a constant
				lemon::ConstantTokenExt* ct = assignmentBot->mRight->cast<lemon::ConstantTokenExt>();
				if (nullptr == ct)
					continue;

				assembly::Register reg1;
				assembly::DataType dataType1;
				CodeLemonTokenTree::splitRegisterVariable(vt1->mVariable, reg1, dataType1);

				// Check the next codes
				bool isEventuallyOverwritten = false;
				bool foundAnySizeMismatch = false;
				for (size_t k = pos + 1; k < block.size(); ++k)
				{
					CodeLemonTokenTree* lemonTree2 = block[k].cast<CodeLemonTokenTree>();
					if (nullptr == lemonTree2)
						break;

					// There must not be a label in between
					bool anyLabel = false;
					for (const LineData* lineData : lemonTree2->mLines)
					{
						anyLabel |= lineData->mIsLabel;
					}
					if (anyLabel)
						break;

					// The operation must be a binary operation
					lemon::BinaryOperationToken* bot2 = lemonTree2->mRoot->cast<lemon::BinaryOperationToken>();
					if (nullptr == bot2)
						break;

					// Check if the register appears as target of the assignment
					bool isTarget = false;
					bool isCorrectSize = false;
					{
						lemon::VariableToken* vt2 = bot2->mLeft->cast<lemon::VariableToken>();
						if (nullptr != vt2)
						{
							assembly::Register reg2;
							assembly::DataType dataType2;
							CodeLemonTokenTree::splitRegisterVariable(vt2->mVariable, reg2, dataType2);

							if (reg1 == reg2)
							{
								isTarget = true;
								isEventuallyOverwritten = (dataType2.getSizeInBytes() >= dataType1.getSizeInBytes());
								isCorrectSize = (dataType2.getSizeInBytes() == dataType1.getSizeInBytes());
								foundAnySizeMismatch |= !isCorrectSize;
							}
						}
					}

					// Check if left side is the searched register and it's a combined assignment and binary operation
					if (isTarget && isCorrectSize && bot2->mOperator != lemon::Operator::ASSIGN &&
						(lemon::OperatorHelper::getOperatorType(bot2->mOperator) == lemon::OperatorHelper::OperatorType::ASSIGNMENT || lemon::OperatorHelper::getOperatorType(bot2->mOperator) == lemon::OperatorHelper::OperatorType::ASSIGNMENT_INT))
					{
						// Resolve assignment with binary operation, i.e. resolve something like "A0 += 4" to "A0 = A0 + 4"
						TokenTreeConverter::resolveBinaryToPureAssignment(*bot2);
					}

					// Search occurences of the register that can be replaced by the constant, and perform the actual replacement
					replaceRegisterInTokenTree(StatementTokenReference(bot2->mRight), reg1, dataType1, *ct, foundAnySizeMismatch);

					// Abort if the register is the target of the assignment
					if (isTarget)
						break;

					// Try to replace on the left side as well, in case it's a memory access like "u8[A2] = ..."
					replaceRegisterInTokenTree(StatementTokenReference(bot2->mLeft), reg1, dataType1, *ct, foundAnySizeMismatch);
				}

				if (isEventuallyOverwritten && !foundAnySizeMismatch)
				{
					// Merge into the next line
					mergeLinesInto(block[pos + 1].mLines, block[pos].mLines, true);
					block.erase(pos);
					--pos;
				}
			}

			// Build trinary operations from if-else where possible
			for (size_t pos = 0; pos < block.size(); ++pos)
			{
				CodeIfElse* ci = block[pos].cast<CodeIfElse>();
				if (nullptr == ci || !ci->mConditionRoot.valid() || ci->mIfBlock.size() != 1 || ci->mElseBlock.size() != 1)
					continue;

				// Both if and else block content must be a single assignment
				lemon::BinaryOperationToken* assignmentBot1 = getAssignmentToken(ci->mIfBlock[0]);
				lemon::BinaryOperationToken* assignmentBot2 = getAssignmentToken(ci->mElseBlock[0]);
				if (nullptr == assignmentBot1 || nullptr == assignmentBot2)
					continue;

				// Accept only variables as target for the assignments
				lemon::VariableToken* vt1 = assignmentBot1->mLeft->cast<lemon::VariableToken>();
				lemon::VariableToken* vt2 = assignmentBot2->mLeft->cast<lemon::VariableToken>();
				if (nullptr == vt1 || nullptr == vt2)
					continue;

				// Both variables must refer to the same register and size, but signs may be different
				assembly::Register reg1;
				assembly::DataType dataType1;
				CodeLemonTokenTree::splitRegisterVariable(vt1->mVariable, reg1, dataType1);
				assembly::Register reg2;
				assembly::DataType dataType2;
				CodeLemonTokenTree::splitRegisterVariable(vt2->mVariable, reg2, dataType2);
				if (reg1 != reg2 || dataType1.mSize != dataType2.mSize)
					continue;

				// Right sides must both be constants
				lemon::ConstantTokenExt* ct1 = assignmentBot1->mRight->cast<lemon::ConstantTokenExt>();
				lemon::ConstantTokenExt* ct2 = assignmentBot2->mRight->cast<lemon::ConstantTokenExt>();
				if (nullptr == ct1 || nullptr == ct2)
					continue;

				CodeLemonTokenTree& tokenTree = genericmanager::Manager<Code>::create<CodeLemonTokenTree>();
				lemon::BinaryOperationToken& newAssignmentBot = tokenTree.mRoot.create<lemon::BinaryOperationToken>();
				newAssignmentBot.mDataType = assignmentBot1->mDataType;
				newAssignmentBot.mOperator = lemon::Operator::ASSIGN;
				newAssignmentBot.mLeft = assignmentBot1->mLeft;

				lemon::BinaryOperationToken& trinaryA = newAssignmentBot.mRight.create<lemon::BinaryOperationToken>();
				trinaryA.mDataType = assignmentBot1->mDataType;
				trinaryA.mOperator = lemon::Operator::QUESTIONMARK;

				lemon::ParenthesisToken& pt = trinaryA.mLeft.create<lemon::ParenthesisToken>();
				pt.mParenthesisType = lemon::ParenthesisType::PARENTHESIS;
				pt.mDataType = assignmentBot1->mDataType;
				pt.mContent.add(*ci->mConditionRoot);

				lemon::BinaryOperationToken& trinaryB = trinaryA.mRight.create<lemon::BinaryOperationToken>();
				trinaryB.mDataType = assignmentBot1->mDataType;
				trinaryB.mOperator = lemon::Operator::COLON;
				trinaryB.mLeft = ct1;
				trinaryB.mRight = ct2;

				std::swap(tokenTree.mLines, ci->mLines);
				mergeLinesInto(tokenTree.mLines, ci->mIfBlock[0].mLines);
				mergeLinesInto(tokenTree.mLines, ci->mElseBlock[0].mLines);

				block.replace(tokenTree, pos);
			}

			// Replace unconditional jumps that just lead to a return with the return itself
			for (size_t pos = 0; pos < block.size(); ++pos)
			{
				CodeJumpOrCall* code = block[pos].cast<CodeJumpOrCall>();
				if (nullptr == code || code->mIsCall)
					continue;

				const RomContent::Instruction* instruction = RomContent::instance().getInstructionByAddress(code->mDestinationAddress);
				if (nullptr == instruction || (instruction->mFlags & RomContent::InstructionFlag::RETURN) == 0)
					continue;

				CodePtr<CodeReturn> newCodePtr;
				newCodePtr.create<CodeReturn>();
				std::swap(newCodePtr->mLines, code->mLines);
				block.replace(*newCodePtr, pos);
			}
		}

		void postprocessIfBlocks(Block& block, const assembly::AssemblyCode* formerAssemblyCode)
		{
			// Convert remaining jump assembly codes to CodeJumpOrCall
			for (size_t pos = 0; pos < block.size(); ++pos)
			{
				uint32 destinationAddress = 0;
				if (Helper::isSimpleUnconditionalJump(block[pos], destinationAddress))
				{
					std::vector<const LineData*> lineData = std::move(block[pos].mLines);

					CodeJumpOrCall& cj = block.createReplaceAt<CodeJumpOrCall>(pos);
					cj.mIsCall = false;
					cj.mDestinationAddress = destinationAddress;
					cj.mLines = std::move(lineData);
				}
			}

			// Refine if-block conditions where possible
			for (size_t pos = 0; pos < block.size(); ++pos)
			{
				CodeIfElse* ci = block[pos].cast<CodeIfElse>();
				if (nullptr != ci)
				{
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
						if (formerCode.isA<CodeAssembly>())
						{
							lastAssemblyCode = formerCode.as<CodeAssembly>().mAssemblyCode;
							mergeAllowed = true;	// If it is the right type of assembly code, we may remove it afterwards
						}
						// Is former code just another if-code?
						else if (formerCode.isA<CodeIfElse>())
						{
							// Only allow it if both branches are either empty or end with an unconditional jump, otherwise there's a risk any of the block's content actually changes the condition
							if ((formerCode.as<CodeIfElse>().mIfBlock.empty() || isControlFlowEnd(formerCode.as<CodeIfElse>().mIfBlock.back())) &&
								(formerCode.as<CodeIfElse>().mElseBlock.empty() || isControlFlowEnd(formerCode.as<CodeIfElse>().mElseBlock.back())))
							{
								lastAssemblyCode = formerCode.as<CodeIfElse>().mConditionAssemblyCode;
							}
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
								ci->mConditionAssemblyCode = lastAssemblyCode;

								// Merge with last code -- only allowed if it is actually inside the same block
								if (mergeAllowed)
								{
									mergeLinesInto(block[pos].mLines, block[pos - 1].mLines, true);
									block.erase(pos-1, 1);
									--pos;
								}
								break;
							}

							case assembly::CodeType::CODE_SET_BIT:
							case assembly::CodeType::CODE_CLEAR_BIT:
							{
								ci->mConditionAssemblyCode = lastAssemblyCode;

								if (ci->mCondition == assembly::Condition::EQ || ci->mCondition == assembly::Condition::NE)
								{
									if (pos >= 1 && lastAssemblyCode == block[pos-1].as<CodeAssembly>().mAssemblyCode)
									{
										// We can move the bit manipulation into the if- or else-part to simplify the code there
										if ((lastAssemblyCode->mType == assembly::CodeType::CODE_SET_BIT) == (ci->mCondition == assembly::Condition::EQ))
										{
											ci->mIfBlock.insert(block[pos-1], 0);
										}
										else
										{
											// Check for the special case that there's no else-block yet and the if-block is just an unconditional jump or a return
											//  -> In this case, there's no need to create an else-block at all
											if (ci->mElseBlock.empty() && ci->mIfBlock.size() == 1 && ci->mIfBlock[0].isA<CodeJumpOrCall>())
											{
												block.insert(block[pos-1], pos+1);
											}
											else
											{
												ci->mElseBlock.insert(block[pos-1], 0);

												if (nullptr != block[pos-1].mLines[0])
													const_cast<LineData*>(block[pos-1].mLines[0])->mIsOutOfOrder = true;
											}
										}

										block.erase(pos-1);
										--pos;
										break;
									}
								}

								// Let the code generator add something like "bool _condition012345 = D0 & 0x80" in front
								const_cast<assembly::AssemblyCode*>(lastAssemblyCode)->mCodeGenData = ci->mLines[0]->mAddress;
								break;
							}

							case assembly::CodeType::CODE_SUB:
							{
								ci->mConditionAssemblyCode = lastAssemblyCode;

								// Experimental code while trying to resolve some "carryFlag()" conditions
								//  -> However, this leads to very ugly code, as the condition bool setter often needs to be inserted between multiple operations that can otherwise be merged into one line
							#if 0
								if (ci->mCondition == assembly::Condition::CC || ci->mCondition == assembly::Condition::CS)
								{
									if (pos >= 1 && lastAssemblyCode == block[pos-1].as<CodeAssembly>().mConditionAssemblyCode)
									{
										CodeLemonTokenTree& tokenTree = block.createAt<CodeLemonTokenTree>(pos - 1);
										tokenTree.mLines = ci->mLines;

										lemon::BinaryOperationToken& assignment = tokenTree.mRoot.create<lemon::BinaryOperationToken>();
										assignment.mOperator = lemon::Operator::ASSIGN;

										lemon::IdentifierToken& conditionIdentifier = assignment.mLeft.create<lemon::IdentifierToken>();
										conditionIdentifier.mName = "_condition" + rmx::hexString(ci->mLines[0]->mAddress, 6, "");
										conditionIdentifier.mDataType = &lemon::PredefinedDataTypes::BOOL;

										lemon::BinaryOperationToken& comparison = assignment.mRight.create<lemon::BinaryOperationToken>();
										comparison.mOperator = lemon::Operator::COMPARE_LESS;

										TokenTreeConverter::createLemonTokenTreeForParameter(comparison.mLeft, lastAssemblyCode->mParamDest, lastAssemblyCode->mDataType);
										TokenTreeConverter::createLemonTokenTreeForParameter(comparison.mRight, lastAssemblyCode->mParamSource, lastAssemblyCode->mDataType);

										//const_cast<assembly::AssemblyCode*>(lastAssemblyCode)->mCodeGenData = ci->mLines[0]->mAddress;

										++pos;
									}
								}
							#endif
								break;
							}

							case assembly::CodeType::CODE_MOVE:
							case assembly::CodeType::CODE_ADD:
							case assembly::CodeType::CODE_NOT:
							case assembly::CodeType::CODE_AND:
							case assembly::CodeType::CODE_OR:
							case assembly::CodeType::CODE_XOR:
							case assembly::CodeType::CODE_SHIFT_LEFT:
							case assembly::CodeType::CODE_SHIFT_RIGHT:
							case assembly::CodeType::CODE_EXTEND_SIGNED:
							{
								ci->mConditionAssemblyCode = lastAssemblyCode;
								break;
							}
						}

						if (nullptr != ci->mConditionAssemblyCode && ci->mLoopRegister == assembly::ExtRegister::NONE)
						{
							// Convert assembly code to a lemon token tree
							const assembly::Condition condition = ci->mNegateWholeCondition ? assembly::negateCondition(ci->mCondition) : ci->mCondition;
							TokenTreeConverter::createTokenTreeForCondition(ci->mConditionRoot, condition, *ci->mConditionAssemblyCode);
						}
					}

					// Go deeper
					//  -> It's important to do this after we fully processed the current if-code
					postprocessIfBlocks(ci->mIfBlock, ci->mConditionAssemblyCode);
					postprocessIfBlocks(ci->mElseBlock, ci->mConditionAssemblyCode);
				}
				else if (block[pos].getType() == Code::WHILE)
				{
					CodeWhile& cw = block[pos].as<CodeWhile>();

					// Go deeper here as well
					postprocessIfBlocks(cw.mInnerBlock, nullptr);
				}
			}

			// Check for nested if-blocks that can be combined with &&
			for (size_t pos = 0; pos < block.size(); ++pos)
			{
				CodeIfElse* ci1 = block[pos].cast<CodeIfElse>();
				if (nullptr == ci1 || !ci1->mConditionRoot.valid() || ci1->mIfBlock.size() != 1 || !ci1->mElseBlock.empty())
					continue;

				CodeIfElse* ci2 = ci1->mIfBlock[0].cast<CodeIfElse>();
				if (nullptr == ci2 || !ci2->mConditionRoot.valid() || !ci2->mElseBlock.empty())
					continue;

				// Merge the conditions
				lemon::TokenPtr<lemon::StatementToken> newCondition;
				lemon::BinaryOperationToken& bot = newCondition.create<lemon::BinaryOperationToken>();
				bot.mOperator = lemon::Operator::LOGICAL_AND;
				bot.mDataType = &lemon::PredefinedDataTypes::BOOL;

				const bool needsParentheses1 = TokenTreeConverter::needsParentheses(ci1->mConditionRoot, bot.mOperator, false);
				const bool needsParentheses2 = TokenTreeConverter::needsParentheses(ci2->mConditionRoot, bot.mOperator, true);

				TokenTreeConverter::putWithOptionalParenthesis(bot.mLeft, *ci1->mConditionRoot, needsParentheses1);
				TokenTreeConverter::putWithOptionalParenthesis(bot.mRight, *ci2->mConditionRoot, needsParentheses2);

				ci1->mConditionRoot = newCondition;

				// Move the content of the inner if-block into the other if-block
				Block* parentBlock = ci1->mIfBlock.mParentBlock;
				const uint32 endAddress = ci1->mIfBlock.mEndAddress;
				const uint32 breakTargetAddress = ci1->mIfBlock.mBreakTargetAddress;

				mergeLinesInto(ci1->mLines, ci2->mLines);

				Block tmp = std::move(ci2->mIfBlock);
				ci1->mIfBlock = std::move(tmp);
				ci1->mIfBlock.mParentBlock = parentBlock;
				ci1->mIfBlock.mEndAddress = endAddress;
				ci1->mIfBlock.mBreakTargetAddress = breakTargetAddress;
			}

			// Check for an immediate inner if that's just a jump right to the else-block, to merge into one if-block combining both conditions using &&
			for (size_t pos = 0; pos < block.size(); ++pos)
			{
				CodeIfElse* ci1 = block[pos].cast<CodeIfElse>();
				if (nullptr == ci1 || !ci1->mConditionRoot.valid() || ci1->mIfBlock.empty() || ci1->mElseBlock.empty())
					continue;

				CodeIfElse* ci2 = ci1->mIfBlock[0].cast<CodeIfElse>();
				if (nullptr == ci2 || !ci2->mConditionRoot.valid() || ci2->mIfBlock.size() != 1 || !ci2->mElseBlock.empty())
					continue;

				const CodeJumpOrCall* jumpCode = ci2->mIfBlock[0].cast<CodeJumpOrCall>();
				if (nullptr == jumpCode || jumpCode->mIsCall)
					continue;
				if (jumpCode->mDestinationAddress != ci1->mElseBlock[0].mLines[0]->mAddress)
					continue;

				// Inner condition needs to be negated
				if (!TokenTreeConverter::negateCondition(*ci2->mConditionRoot))
					continue;

				// Merge the conditions
				lemon::TokenPtr<lemon::StatementToken> newCondition;
				lemon::BinaryOperationToken& bot = newCondition.create<lemon::BinaryOperationToken>();
				bot.mOperator = lemon::Operator::LOGICAL_AND;
				bot.mDataType = &lemon::PredefinedDataTypes::BOOL;

				const bool needsParentheses1 = TokenTreeConverter::needsParentheses(ci1->mConditionRoot, bot.mOperator, false);
				const bool needsParentheses2 = TokenTreeConverter::needsParentheses(ci2->mConditionRoot, bot.mOperator, true);

				TokenTreeConverter::putWithOptionalParenthesis(bot.mLeft, *ci1->mConditionRoot, needsParentheses1);
				TokenTreeConverter::putWithOptionalParenthesis(bot.mRight, *ci2->mConditionRoot, needsParentheses2);

				ci1->mConditionRoot = newCondition;

				// Remove the inner if-block
				mergeLinesInto(ci1->mLines, ci2->mLines);
				ci1->mIfBlock.erase(0, 1);

				--pos;		// Check again at the same position
			}

			// Check for multiple successive if-blocks that are just conditional jumps to the same address, to merge into one if-block combining both conditions using ||
			for (size_t pos = 0; pos + 1 < block.size(); ++pos)
			{
				CodeIfElse* ci1 = block[pos].cast<CodeIfElse>();
				CodeIfElse* ci2 = block[pos + 1].cast<CodeIfElse>();
				if (nullptr == ci1 || !ci1->mConditionRoot.valid() || ci1->mIfBlock.size() != 1 || !ci1->mElseBlock.empty())
					continue;
				if (nullptr == ci2 || !ci2->mConditionRoot.valid() || ci2->mIfBlock.size() != 1 || !ci2->mElseBlock.empty())
					continue;

				const CodeJumpOrCall* jumpCode1 = ci1->mIfBlock[0].cast<CodeJumpOrCall>();
				if (nullptr == jumpCode1 || jumpCode1->mIsCall)
					continue;

				const CodeJumpOrCall* jumpCode2 = ci2->mIfBlock[0].cast<CodeJumpOrCall>();
				if (nullptr == jumpCode2 || jumpCode2->mIsCall)
					continue;

				if (jumpCode1->mDestinationAddress != jumpCode2->mDestinationAddress)
					continue;

				// Merge the conditions
				lemon::TokenPtr<lemon::StatementToken> newCondition;
				lemon::BinaryOperationToken& bot = newCondition.create<lemon::BinaryOperationToken>();
				bot.mOperator = lemon::Operator::LOGICAL_OR;
				bot.mDataType = &lemon::PredefinedDataTypes::BOOL;

				const bool needsParentheses1 = TokenTreeConverter::needsParentheses(ci1->mConditionRoot, bot.mOperator, false);
				const bool needsParentheses2 = TokenTreeConverter::needsParentheses(ci2->mConditionRoot, bot.mOperator, true);

				TokenTreeConverter::putWithOptionalParenthesis(bot.mLeft, *ci1->mConditionRoot, needsParentheses1);
				TokenTreeConverter::putWithOptionalParenthesis(bot.mRight, *ci2->mConditionRoot, needsParentheses2);

				ci1->mConditionRoot = newCondition;

				// Remove the second if-block
				mergeCodes(block, (int)pos, 2);
				--pos;		// Check again at the same position
			}
		}

	}	// namespace detail



	void Optimization::optimize(Block& block)
	{
		Structuring::createStructuredBlocks(block);
		detail::postprocessIfBlocks(block, nullptr);

		// Go through all blocks, including inner blocks of if/else/while
		Helper::foreachBlockInside(block, [](Block& childBlock)
		{
			detail::optimizeBlock(childBlock);
		}, false);
	}

}
