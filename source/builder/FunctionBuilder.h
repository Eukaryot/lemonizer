/*
*	Lemonizer -- Turns 68K code into lemonscript
*	Copyright (C) 2017-2026 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#pragma once

#include "builder/RomContent.h"


class FunctionBuilder
{
public:
	typedef uint32 FunctionId;
	typedef RomContent::Instruction Instruction;

	struct FunctionInfo;
	typedef std::set<FunctionInfo*> FunctionSet;

	struct FunctionInfo
	{
		uint32 mStartAddress = 0xffffffff;
		std::map<uint32, FunctionSet> mEntryPointCalls;
		FunctionSet mCallsTo;
		uint32 mCallCounter = 0;
		std::vector<const Instruction*> mInstructions;
	};

public:
	FunctionBuilder(RomContent& romContent);
	~FunctionBuilder();

	inline const std::map<FunctionId, FunctionInfo>& getFunctions() const  { return mFunctions; }

	void processTracerData();

private:
	void buildFunctions();
	Instruction& resolveFunctionIdIndirection(Instruction& instruction);

	void fillCallCounters();

private:
	RomContent& mRomContent;

	// Functions
	std::map<FunctionId, FunctionInfo> mFunctions;
};
