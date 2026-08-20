#pragma once

#include <rmxbase.h>

namespace assembly
{
	struct AssemblyCode;
}


namespace lemonizer
{
	class Block;
	class Code;


	class Helper
	{
	public:
		static bool isSimpleUnconditionalJump(const assembly::AssemblyCode& ac, uint32& destinationAddress);
		static bool isSimpleUnconditionalJump(const Code& code, uint32& destinationAddress);
		static bool isSimpleUnconditionalJump(uint32 address, uint32& destinationAddress);

		static void foreachBlockInside(Block& block, const std::function<void(Block&)>& func, bool parentsBeforeChildren = true);
		static void collectInnerBlocks(std::vector<Block*>& outBlocks, Block& block);
		static void findJumpTargets(std::vector<uint32>& outAddresses, Block& block);
	};
}
