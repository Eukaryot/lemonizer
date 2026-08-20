/*
*	Lemonizer -- Turns 68K code into lemonscript
*	Copyright (C) 2017-2026 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#include "pch.h"
#include "lemonizer/LemonizerHelper.h"
#include "lemonizer/LemonizerCode.h"
#include "builder/RomContent.h"


namespace lemonizer
{
	bool Helper::isSimpleUnconditionalJump(const assembly::AssemblyCode& ac, uint32& destinationAddress)
	{
		// Must be an unconditional jump with fixed destination address
		if (ac.mType == assembly::CodeType::CODE_JUMP && ac.mParamSource.isSimpleUnconditional() && ac.mParamDest.isConstantValue())
		{
			destinationAddress = ac.mParamDest.mConstant.mValue;
			return true;
		}
		return false;
	}

	bool Helper::isSimpleUnconditionalJump(const Code& code, uint32& destinationAddress)
	{
		const assembly::AssemblyCode* ac = CodeAssembly::getAssemblyCode(code);
		return (nullptr != ac && isSimpleUnconditionalJump(*ac, destinationAddress));
	}

	bool Helper::isSimpleUnconditionalJump(uint32 address, uint32& destinationAddress)
	{
		const assembly::AssemblyCode* ac = RomContent::instance().getCodeByAddress(address);
		return (nullptr != ac) && isSimpleUnconditionalJump(*ac, destinationAddress);
	}

	void Helper::collectInnerBlocks(std::vector<Block*>& outBlocks, Block& block)
	{
		foreachBlockInside(block, [&outBlocks](Block& block) { outBlocks.push_back(&block); } );
	}

	void Helper::foreachBlockInside(Block& block, const std::function<void(Block&)>& func, bool parentsBeforeChildren)
	{
		if (parentsBeforeChildren)
		{
			func(block);
		}

		for (size_t pos = 0; pos < block.size(); ++pos)
		{
			if (block[pos].getType() == Code::IFELSE)
			{
				CodeIfElse& ci = block[pos].as<CodeIfElse>();
				foreachBlockInside(ci.mIfBlock, func, parentsBeforeChildren);
				foreachBlockInside(ci.mElseBlock, func, parentsBeforeChildren);
			}
			else if (block[pos].getType() == Code::WHILE)
			{
				CodeWhile& cw = block[pos].as<CodeWhile>();
				foreachBlockInside(cw.mInnerBlock, func, parentsBeforeChildren);
			}
		}

		if (!parentsBeforeChildren)
		{
			func(block);
		}
	}

	void Helper::findJumpTargets(std::vector<uint32>& outAddresses, Block& block)
	{
		foreachBlockInside(block, [&outAddresses](Block& childBlock)
		{
			for (size_t pos = 0; pos < childBlock.size(); ++pos)
			{
				const Code& code = childBlock[pos];
				if (code.getType() == Code::JUMP_OR_CALL)
				{
					outAddresses.push_back(code.as<CodeJumpOrCall>().mDestinationAddress);
				}
				else if (code.getType() == Code::ASSEMBLY)
				{
					const assembly::AssemblyCode& ac = *code.as<CodeAssembly>().mAssemblyCode;
					if (ac.mType == assembly::CodeType::CODE_JUMP && ac.mParamDest.isConstant())
					{
						outAddresses.push_back(ac.mParamDest.mConstant.mValue);
					}
				}
			}
		});
	}
}
