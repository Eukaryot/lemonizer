#include "pch.h"
#include "lemonizer/LemonizerStructuring.h"
#include "lemonizer/LemonizerCode.h"
#include "lemonizer/LemonizerHelper.h"


namespace lemonizer
{
	namespace detail
	{
		bool isSimpleConditionalJump(const Code& code, uint32& destinationAddress, assembly::Condition& condition, assembly::ExtRegister& loopRegister)
		{
			// Must be a jump
			const assembly::AssemblyCode* ac = CodeAssembly::getAssemblyCode(code, assembly::CodeType::CODE_JUMP);
			if (nullptr != ac)
			{
				// Conditional jump
				if (ac->mParamSource.mType == assembly::Parameter::Type::CONDITION)
				{
					// Fixed destination address
					if (ac->mParamDest.isConstantValue())
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

		bool isSequenceBreak(const Code& code)
		{
			// Must be an unconditional jump or return
			const assembly::AssemblyCode* ac = CodeAssembly::getAssemblyCode(code);
			if (nullptr != ac)
			{
				if (ac->mType == assembly::CodeType::CODE_JUMP)
				{
					return (ac->mParamSource.mType != assembly::Parameter::Type::CONDITION);
				}
				else if (ac->mType == assembly::CodeType::CODE_RETURN)
				{
					return true;
				}
			}
			return false;
		}

		bool findJumpTarget(uint32& address, uint32 searchedAddress)
		{
			uint32 currentAddress = address;
			while (currentAddress != searchedAddress)
			{
				uint32 jumpAddress;
				if (!Helper::isSimpleUnconditionalJump(currentAddress, jumpAddress))
					return false;

				if (jumpAddress <= currentAddress)		// Safety check to avoid a theoretical endless loop
					return false;

				currentAddress = jumpAddress;
			}
			address = currentAddress;
			return true;
		}

		bool findAddressInBlock(Block& block, uint32 address, bool ignoreEndAddress, size_t startPos, size_t& outPos, bool followEndJumps, bool considerIndirections)
		{
			if (followEndJumps && !ignoreEndAddress)
			{
				// Special handling for following jumps at given address
				if (findJumpTarget(address, block.mEndAddress))
				{
					outPos = block.size();
					return true;
				}
			}

			// Find address in the block itself, or optionally at its end address (which is usually outside the block)
			if (address < block.mEndAddress)
			{
				for (size_t i = startPos; i < block.size(); ++i)
				{
					// Search for the address itself
					for (const LineData* line : block[i].mLines)
					{
						if (line->mAddress == address)
						{
							outPos = i;
							return true;
						}
					}
				}
			}
			else if (address == block.mEndAddress && !ignoreEndAddress)
			{
				outPos = block.size();
				return true;
			}

			// TODO: This code is disabled for now, as it leads to improvements in some cases, but can complicate things elsewhere
		#if 0
			// Alternatively, also allow unconditional jumps to the given address
			if (considerIndirections)
			{
				for (size_t i = startPos; i < block.size(); ++i)
				{
					uint32 destinationAddress;
					if (Helper::isSimpleUnconditionalJump(block[i], destinationAddress))
					{
						if (destinationAddress == address)
						{
							outPos = i;
							return true;
						}
					}
				}
			}
		#endif

			return false;
		}

		bool containsReturn(Block& block, size_t startPos, size_t endPos)
		{
			for (size_t i = startPos; i < endPos; ++i)
			{
				if (nullptr != CodeAssembly::getAssemblyCode(block[i], assembly::CodeType::CODE_RETURN))
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

		Block& getOutermostParentBlock(Block& block)
		{
			// Note that this can return the block itself
			Block* parent = &block;
			while (nullptr != parent->mParentBlock)
				parent = parent->mParentBlock;
			return *parent;
		}



		CodeIfElse& replaceWithIfElse(Block& block, uint32 endAddress, size_t startPos, size_t endPos, assembly::Condition condition, assembly::ExtRegister loopRegister = assembly::ExtRegister::NONE)
		{
			// Insert new code, replacing the conditional jump
			std::vector<const LineData*> lineData = std::move(block[startPos].mLines);
			CodeIfElse& ci = block.createReplaceAt<CodeIfElse>(startPos);
			ci.mCondition = condition;
			ci.mLoopRegister = loopRegister;
			ci.mIfBlock.mEndAddress = endAddress;
			ci.mIfBlock.mBreakTargetAddress = block.mBreakTargetAddress;
			ci.mIfBlock.mParentBlock = &block;
			ci.mLines.swap(lineData);

			// Move conditional code into if-block
			if (startPos + 1 < endPos)
			{
				moveToInnerBlock(block, startPos + 1, endPos, ci.mIfBlock);
			}
			return ci;
		}

		void addElseBlock(CodeIfElse& ci, Block& block, uint32 endAddress, size_t startPos, size_t endPos, const Block& outermostParent)
		{
			ci.mElseBlock.mEndAddress = endAddress;
			ci.mElseBlock.mBreakTargetAddress = block.mBreakTargetAddress;
			ci.mElseBlock.mParentBlock = &block;

			// Also update the if-block's end address, as leaving it now effectively is a jump after the else-block's content
			ci.mIfBlock.mEndAddress = endAddress;

			// Erase or replace unconditional jump just before else
			//  -> Unless it's a jump right to the start of the following function
			if (endAddress < outermostParent.mEndAddress && !ci.mIfBlock.empty())
			{
				if (endAddress == block.mBreakTargetAddress)
				{
					// Replace the jump with a break
					std::vector<const LineData*> lineData = std::move(ci.mIfBlock.back().mLines);
					CodeBreak& cb = ci.mIfBlock.createReplaceAt<CodeBreak>(ci.mIfBlock.size() - 1);
					cb.mLines.swap(lineData);
					return;
				}
				else
				{
					// Remove the jump
					ci.mElseLines = std::move(ci.mIfBlock.back().mLines);
					ci.mIfBlock.erase(ci.mIfBlock.size() - 1, 1);
				}
			}

			// Move code into else-block
			detail::moveToInnerBlock(block, startPos, endPos, ci.mElseBlock);

			// Recursive structure creation
			Structuring::createStructuredBlocks(ci.mElseBlock);
		}

		CodeWhile& replaceWithWhile(Block& block, uint32 endAddress, size_t startPos, size_t endPos, assembly::Condition condition, assembly::ExtRegister loopRegister = assembly::ExtRegister::NONE)
		{
			// Insert new code just after the jump back
			CodeWhile& cw = block.createAt<CodeWhile>(endPos);
			cw.mInnerBlock.mEndAddress = endAddress;
			cw.mInnerBlock.mBreakTargetAddress = endAddress;
			cw.mInnerBlock.mParentBlock = &block;
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
				ci.mIfBlock.mParentBlock = &cw.mInnerBlock;
				ci.mCondition = condition;
				ci.mLoopRegister = loopRegister;
				ci.mNegateWholeCondition = true;
				ci.mLines.swap(lineData);

				// Add inner block for if that consists only of "break"
				CodeBreak& cb = ci.mIfBlock.createBack<CodeBreak>();
				cb.mLines = ci.mLines;

				// Go through the inner block and identify what else should be turned into breaks or continues
				const uint32 startAddress = cw.mInnerBlock[0].mLines[0]->mAddress;
				for (size_t pos = 0; pos < cw.mInnerBlock.size() - 1; ++pos)
				{
					const Code& code = cw.mInnerBlock[pos];

					uint32 destinationAddress;
					assembly::Condition innerCondition;
					assembly::ExtRegister innerLoopRegister;
					if (isSimpleConditionalJump(code, destinationAddress, innerCondition, innerLoopRegister))
					{
						if (loopRegister == assembly::ExtRegister::NONE && destinationAddress == block[startPos].mLines[0]->mAddress)
						{
							// Replacing the conditional jump with an "if (...) continue"
							//  -> Note that this is more theoretical... at least in TJE2, this never happens
							std::vector<const LineData*> lineData = std::move(cw.mInnerBlock[pos].mLines);
							CodeIfElse& ci = cw.mInnerBlock.createReplaceAt<CodeIfElse>(pos);
							ci.mIfBlock.mEndAddress = endAddress;
							ci.mIfBlock.mOutputAsSingleLine = true;
							ci.mIfBlock.mParentBlock = &cw.mInnerBlock;
							ci.mCondition = innerCondition;
							ci.mLoopRegister = innerLoopRegister;
							ci.mLines = lineData;

							CodeContinue& cc = ci.mIfBlock.createBack<CodeContinue>();
							cc.mLines = lineData;
						}
						else if (destinationAddress == endAddress)
						{
							// Replacing the conditional jump with an "if (...) break"
							std::vector<const LineData*> lineData = std::move(cw.mInnerBlock[pos].mLines);
							CodeIfElse& ci = cw.mInnerBlock.createReplaceAt<CodeIfElse>(pos);
							ci.mIfBlock.mEndAddress = endAddress;
							ci.mIfBlock.mOutputAsSingleLine = true;
							ci.mIfBlock.mParentBlock = &cw.mInnerBlock;
							ci.mCondition = innerCondition;
							ci.mLoopRegister = innerLoopRegister;
							ci.mLines = lineData;

							CodeBreak& cb2 = ci.mIfBlock.createBack<CodeBreak>();
							cb2.mLines = lineData;
						}
					}
				}

				if (!cw.mInnerBlock.empty())
				{
					// Add a label at the start of while-blocks, if it's right at the start of the function or there's a hard sequence break before
					if (!cw.mInnerBlock[0].mLines.empty())
					{
						if (startPos == 0 || isSequenceBreak(block[startPos-1]))
						{
							const_cast<LineData*>(cw.mInnerBlock[0].mLines[0])->mShowWithLabel = true;
						}
					}

					// Recursively handle while-loop's inner block content
					Structuring::createStructuredBlocks(cw.mInnerBlock);
				}
			}
			return cw;
		}

	}	// namespace detail



	void Structuring::createStructuredBlocks(Block& block)
	{
		Block& outermostParent = detail::getOutermostParentBlock(block);

		// Create while-blocks where possible
		for (size_t pos = 0; pos < block.size(); ++pos)
		{
			const Code& code = block[pos];

			uint32 destinationAddress;
			assembly::Condition condition;
			assembly::ExtRegister loopRegister;
			if (detail::isSimpleConditionalJump(code, destinationAddress, condition, loopRegister))
			{
				// Try to make a while-block, if it's a backward jump
				if (destinationAddress < code.mLines[0]->mAddress)
				{
					const size_t endPos = pos + 1;
					size_t startPos;
					if (detail::findAddressInBlock(block, destinationAddress, false, 0, startPos, false, false))
					{
						if (startPos < endPos)
						{
							const uint32 endAddress = (endPos < block.size()) ? block[endPos].mLines.back()->mAddress : block.mEndAddress;
							detail::replaceWithWhile(block, endAddress, startPos, endPos, condition, loopRegister);
							pos = startPos;
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
			if (detail::isSimpleConditionalJump(code, destinationAddress, condition, loopRegister))
			{
				const bool hasLoopRegister = (loopRegister != assembly::ExtRegister::NONE);

				// Try to make an if-block, if it's a forward jump
				if (!hasLoopRegister && destinationAddress > code.mLines[0]->mAddress)
				{
					const size_t startPos = pos;
					size_t endPos;
					if (detail::findAddressInBlock(block, destinationAddress, false, startPos, endPos, true, true))
					{
						if (endPos > startPos + 1)
						{
							// Add an if-block here
							const uint32 endAddress = (endPos < block.size()) ? block[endPos].mLines.back()->mAddress : block.mEndAddress;
							CodeIfElse& ci = detail::replaceWithIfElse(block, endAddress, startPos, endPos, assembly::negateCondition(condition));

							// Check if we could add an else-block as well
							//  -> This is the case whenever the if-block ends in an unconditional forward jump, and the code in between has no outside jumps
							uint32 destinationAddress;
							if (Helper::isSimpleUnconditionalJump(ci.mIfBlock.back(), destinationAddress))
							{
								size_t elseStartPos = pos + 1;
								size_t elseEndPos;
								if (detail::findAddressInBlock(block, destinationAddress, false, elseStartPos, elseEndPos, true, true))
								{
									if (elseEndPos > elseStartPos)
									{
										detail::addElseBlock(ci, block, destinationAddress, elseStartPos, elseEndPos, outermostParent);
									}
								}
							}

							// Recursively handle content of the if-block
							//  -> This is done here instead of inside "replaceWithIfElse", so we can take advantage of the updated end address of the if-block in case an else-block was added
							Structuring::createStructuredBlocks(ci.mIfBlock);

							// If the if-block is empty, remove it
							if (ci.mIfBlock.empty())
							{
								ci.mNegateWholeCondition = !ci.mNegateWholeCondition;
								ci.mIfBlock.swapWith(ci.mElseBlock);
							}

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
					CodeIfElse& ci = detail::replaceWithIfElse(block, code.mLines.back()->mAddress, pos, pos, condition, loopRegister);
					CodeJumpOrCall& cj = ci.mIfBlock.create<CodeJumpOrCall>();
					cj.mIsCall = false;
					cj.mDestinationAddress = destinationAddress;
					cj.mLines = ci.mLines;
				}
			}
		}
	}

}
