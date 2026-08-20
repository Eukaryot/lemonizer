/*
*	Lemonizer -- turns 68K code into lemon script
*	Copyright (C) 2021 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#pragma once

#include "builder/FunctionBuilder.h"
#include "lemonizer/LemonizerCode.h"


class HtmlWriter
{
public:
	HtmlWriter(FunctionBuilder& functionBuilder);

	void writeOutput();

private:
	struct LineData : public lemonizer::LineData
	{
		const RomContent::Instruction* mInstruction = nullptr;
	};

	struct FunctionInfo;
	typedef std::set<FunctionInfo*> FunctionSet;

	struct FunctionInfo
	{
		const FunctionBuilder::FunctionInfo* mBase = nullptr;

		uint32 mStartAddress = 0xffffffff;
		std::map<uint32, FunctionSet> mEntryPointCalls;
		FunctionSet mCallsTo;
		std::string mDisplayName;

		// Only for output
		std::string mAddressString;
		FunctionInfo* mParent = nullptr;
		std::vector<FunctionInfo*> mChildren;
		bool mAddedToHierarchy = false;
		uint32 mHierarchyTag = 0;
		std::string mSourceScript;

		std::vector<LineData> mLines;
	};
	std::map<uint32, FunctionInfo> mFunctions;

	struct HierarchyNode
	{
		FunctionInfo* mFunction = nullptr;
		std::vector<HierarchyNode*> mChildren;
		uint32 mDepth = 0;
	};
	HierarchyNode mFunctionHierarchy;
	std::vector<HierarchyNode*> mLinearFunctionHierarchy;

	struct OutputStreams
	{
		std::stringstream mHtmlStream;
		std::stringstream mTextStream;
	};

private:
	void processTracerData();
	void addToFunctionHierarchy(HierarchyNode& parentNode, FunctionInfo& func);

	void addHtmlBoxForFunction(OutputStreams& str, const HierarchyNode& node);
	void writeSortedFunctionList(std::stringstream& str, const FunctionSet& map);
	const char* getContentClassByInstruction(const RomContent::Instruction& instruction);
	void writeLemonizerCode(OutputStreams& str, const lemonizer::Block& block);
	void writeLemonizerCode(OutputStreams& str, const lemonizer::Block& block, uint32 indentation);

	void translateFunctionSet(FunctionSet& output, const FunctionBuilder::FunctionSet& input);
	std::vector<FunctionInfo*> getSortedFunctionList(const FunctionSet& input);

private:
	FunctionBuilder& mFunctionBuilder;
};
