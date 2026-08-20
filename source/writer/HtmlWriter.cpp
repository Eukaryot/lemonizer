/*
*	Lemonizer -- turns 68K code into lemon script
*	Copyright (C) 2021 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#include "pch.h"
#include "writer/HtmlWriter.h"
#include "builder/FunctionBuilder.h"
#include "builder/ProjectData.h"
#include "builder/RomContent.h"
#include "lemonizer/LemonizerCode.h"
#include "lemonizer/LemonizerFormatter.h"
#include "lemonizer/LemonizerOptimization.h"


namespace
{
	static std::string addressString(uint32 address)
	{
		return rmx::hexString(address, 6, "");
	}

	std::ostream& operator<<(std::ostream& str, const Color& color)
	{
		uint32 rgb = color.getABGR32() & 0xffffff;
		rgb = ((rgb & 0xff0000) >> 16) | (rgb & 0x00ff00) | ((rgb & 0x0000ff) << 16);
		str << rmx::hexString(rgb, 6, "#");
		return str;
	}


	class HtmlCodeLineHelper
	{
	public:
		void setBackgroundColor(const Color& backgroundColor)
		{
			mBackgroundColor = backgroundColor;
		}

		void setAddress(uint32 address, bool isLabel = false)
		{
			mAddress = address;
			mIsLabel = isLabel;
		}

		void setIndentation(uint32 indentation)
		{
			mIndentation = indentation;
		}

		void setContentClass(const std::string& classname)
		{
			mContentClass = classname;
		}

		void addContent(const std::string& content)
		{
			mHtmlContent << encodeHtml(content);
			mTextContent << content;
			mContentLength += (uint32)content.length();
		}

		void addContentLabel(const std::string& labelName, const std::string& content)
		{
			mHtmlContent << "<a href=\"#" << labelName << "\" "
						 << "onmouseenter=\"highlightLabel('" << labelName << "')\" onmouseleave=\"unhighlightLabel('" << labelName << "')\">";
			addContent(content);
			mHtmlContent << "</a>";
		}

		void addContentSpacesTo(uint32 length, int32 minSpaces)
		{
			while (mContentLength < length || minSpaces > 0)
			{
				mHtmlContent << "&nbsp;";
				mTextContent << " ";
				++mContentLength;
				--minSpaces;
			}
		}

		void addLabel(const std::string& labelName)
		{
			mLabelName = labelName;
		}

		void addComment(const std::string& comment)
		{
			mComments.push_back(comment);
		}

		void newLine()
		{
			mHtmlLines.push_back(mHtmlContent.str());
			mHtmlContent.str("");
			mTextLines.push_back(mTextContent.str());
			mTextContent.str("");
		}

		void setTooltip(const std::string& tooltip)
		{
			mTooltip = encodeHtml(tooltip);
		}

		void outputHtmlLine(std::stringstream& str)
		{
			if (!mHtmlContent.str().empty() || mHtmlLines.empty())
			{
				mHtmlLines.push_back(mHtmlContent.str());
				mHtmlContent.str("");
			}

			// Background
			addSpaces(str, 2);
			if (mBackgroundColor.a > 0.0f)
			{
				str << "<span class=\"codeline\" style=\"background-color:" << mBackgroundColor << ";\"> ";
			}
			else
			{
				str << "<span class=\"codeline\"> ";
			}

			for (size_t lineIndex = 0; lineIndex < mHtmlLines.size(); ++lineIndex)
			{
				// Address
				const bool showAddress = (lineIndex == 0 && mAddress != 0xffffffff);
				if (showAddress)
				{
					const std::string addrString = addressString(mAddress);

					// Tooltip
					if (!mTooltip.empty())
					{
						str << "<span class=\"tooltip\">" << mTooltip << "</span>";
					}

					if (mIsLabel)
					{
						str << "<span class=\"code_label\" id=\"line" << addrString << "\">@" << addrString << "</span>";
					}
					else
					{
						str << "<span class=\"code_address\" id=\"line" << addrString << "\">&nbsp;" << addrString << "</span>";
					}
				}
				else
				{
					str << "<span class=\"code_address\">&nbsp; &nbsp; &nbsp; &nbsp; </span>";
				}
				str << " &nbsp; ";

				// Indentation
				addSpaces(str, mIndentation * 4);

				// Content
				if (mContentClass.empty())
					str << "<span>";
				else
					str << "<span class=\"" << mContentClass << "\">";
				str << mHtmlLines[lineIndex];

				// Comments
				if (!mComments.empty())
				{
					addSpaces(str, std::max(4, 36 - (signed)mContentLength));

					// Only first for now...
					str << "<i style=\"color:#60c060;\">// " << mComments[0] << "</i>";

					// All the others in a separate row
					for (size_t i = 1; i < mComments.size(); ++i)
					{
						str << "<br/>\n\r";
						addSpaces(str, 13 + mIndentation * 2 + 36);

						str << "<i style=\"color:#60c060;\">// " << mComments[i] << "</i>";
					}
				}

				str << "</span>";
				if (lineIndex < mHtmlLines.size()-1)
					str << "<br/>\r\n";
			}
			str << "</span><br/>\r\n";
		}

		void outputTextLine(std::stringstream& str)
		{
			if (!mTextContent.str().empty() || mTextLines.empty())
			{
				mTextLines.push_back(mTextContent.str());
				mTextContent.str("");
			}

			// Label
			if (!mLabelName.empty())
			{
				// Empty line
				str << "\r\n";
				
				// Indentation (one less than usual, i.e. no additional tab here)
				for (uint32 i = 0; i < mIndentation; ++i)
					str << '\t';

				// Content
				str << '@' << mLabelName << ':';
				str << "\r\n";
			}

			for (size_t lineIndex = 0; lineIndex < mTextLines.size(); ++lineIndex)
			{
				// Indentation (with one additional tab)
				for (uint32 i = 0; i < mIndentation + 1; ++i)
					str << '\t';

				// Content
				str << mTextLines[lineIndex];
				str << "\r\n";
			}
		}

	private:
		std::string encodeHtml(const std::string& input)
		{
			std::string output;
			output.reserve(input.size());
			for (char ch : input)
			{
				switch (ch)
				{
					case '<':  output += "&lt;";   break;
					case '>':  output += "&gt;";   break;
					case ' ':  output += "&nbsp;"; break;
					case '\r': break;
					case '\n': output += "<br/>";  break;
					default:   output += ch;	   break;
				}
			}
			return output;
		}

		void addSpaces(std::stringstream& str, uint32 count)
		{
			for (uint32 i = 0; i < count; ++i)
				str << "&nbsp;";
		}

	private:
		Color mBackgroundColor = Color::TRANSPARENT;
		uint32 mAddress = 0xffffffff;
		bool mIsLabel = false;
		uint32 mIndentation = 0;
		std::stringstream mHtmlContent;
		std::stringstream mTextContent;
		uint32 mContentLength = 0;
		std::vector<std::string> mHtmlLines;
		std::vector<std::string> mTextLines;
		std::string mContentClass;
		std::string mLabelName;
		std::vector<std::string> mComments;
		std::string mTooltip;
	};
}



HtmlWriter::HtmlWriter(FunctionBuilder& functionBuilder) :
	mFunctionBuilder(functionBuilder)
{}

void HtmlWriter::writeOutput()
{
	const std::string resourcesPath = "../../data/";

	// Processing of control flow tracer data
	mFunctionBuilder.processTracerData();
	processTracerData();

	// Load HTML template
	String htmlTemplate;
	htmlTemplate.loadFile("data/template.htm");
	htmlTemplate.replace("[$RESOURCES]/", resourcesPath);

	enum TemplateSection
	{
		SECTION_TITLE,
		SECTION_OVERVIEW,
		SECTION_CONTENT,
		SECTION_REST,
		_NUM_SECTIONS
	};

	String templateSection[_NUM_SECTIONS];
	{
		const String sectionMarker[_NUM_SECTIONS-1] = { "[$TITLE]", "[$OVERVIEW]", "[$CONTENT]" };
		int sectionMarkerPosition[_NUM_SECTIONS];

		for (int i = 0; i < _NUM_SECTIONS-1; ++i)
		{
			sectionMarkerPosition[i] = htmlTemplate.findString(sectionMarker[i]);
			RMX_CHECK(sectionMarkerPosition[i] >= 0, "Section marker not found for " << *sectionMarker[i], RMX_REACT_THROW);
		}
		sectionMarkerPosition[_NUM_SECTIONS-1] = htmlTemplate.length();

		for (int i = 0; i < _NUM_SECTIONS; ++i)
		{
			const int templateSectionStart = (i == 0) ? 0 : (sectionMarkerPosition[i-1] + sectionMarker[i-1].length());
			const int templateSectionSize = sectionMarkerPosition[i] - templateSectionStart;
			templateSection[i] = htmlTemplate.getSubString(templateSectionStart, templateSectionSize);
		}
	}

	OutputStreams outputStreams;
	std::stringstream& str = outputStreams.mHtmlStream;

	// Add template before title section
	str << *templateSection[SECTION_TITLE];

	// Insert HTML page title
	const String title = String(ProjectData::instance().mName.c_str()) + " Analysis";
	str << *title;

	// Add template before overview section
	str << *templateSection[SECTION_OVERVIEW];

	// Function overview
	{
		str << "<div class=\"functions\">\r\n";
		for (const HierarchyNode* node : mLinearFunctionHierarchy)
		{
			const FunctionInfo& func = *node->mFunction;

			str << "<div class=\"overview_line\" title=\"Start address: 0x" << func.mAddressString << "\" id=\"ov" << func.mAddressString << "\">\r\n";
			for (uint32 i = 1; i < node->mDepth; ++i)
				str << "&#183;&nbsp;";

			str << "<img src=\"" << resourcesPath << "caret_empty.png\"/>&nbsp;";
			str << "<a href=\"#fn" << func.mAddressString << "\" class=\"overview_link_normal\">";
			str << func.mDisplayName << "</a>";
			if (!func.mSourceScript.empty())
				str << " <span class=\"overview_source\">- " << func.mSourceScript << "</span>";
			str << "</br>\r\n";
			str << "</div>\r\n";
		}
		str << "</div>\r\n";
	}

	// HTML template top
	str << *templateSection[SECTION_CONTENT];

	// Function content
	for (const HierarchyNode* node : mFunctionHierarchy.mChildren)
	{
		addHtmlBoxForFunction(outputStreams, *node);
	}

	// HTML template bottom
	str << *templateSection[SECTION_REST];

	// Save HTML and text file
	FTX::FileSystem->createDirectory(ProjectData::instance().mOutputPath);
	String(outputStreams.mHtmlStream.str().c_str()).saveFile(WString(ProjectData::instance().mOutputPath + L"index.htm").toStdString());
	String(outputStreams.mTextStream.str().c_str()).saveFile(WString(ProjectData::instance().mOutputPath + L"index.lemon").toStdString());
}

void HtmlWriter::processTracerData()
{
	// Build up functions
	for (auto& pair : mFunctionBuilder.getFunctions())
	{
		const FunctionBuilder::FunctionInfo& cfunc = pair.second;
		FunctionInfo& func = mFunctions[pair.first];

		func.mBase = &cfunc;
		func.mStartAddress = cfunc.mStartAddress;

		func.mAddressString = addressString(cfunc.mStartAddress);
		func.mDisplayName = "fn" + func.mAddressString;

		// Add lines
		func.mLines.reserve(cfunc.mInstructions.size());
		for (const RomContent::Instruction* instruction : cfunc.mInstructions)
		{
			// Add a new line
			LineData& line = vectorAdd(func.mLines);
			line.mAddress = instruction->mAddress;
			line.mInstruction = instruction;
			line.mAssemblyCode = RomContent::instance().getCodeByAddress(instruction->mAddress);
		}
	}

	// Translate calls to/from function
	for (auto& pair : mFunctions)
	{
		FunctionInfo& func = pair.second;
		const FunctionBuilder::FunctionInfo& cfunc = *func.mBase;

		for (const auto& pair2 : cfunc.mEntryPointCalls)
		{
			translateFunctionSet(func.mEntryPointCalls[pair2.first], pair2.second);
		}

		translateFunctionSet(func.mCallsTo, cfunc.mCallsTo);
	}

	// Determine hierarchy
	{
		for (auto& pair : mFunctions)
		{
			FunctionInfo& func = pair.second;
			func.mParent = nullptr;

		#if 0
			// Only start a hierarchy for functions with exactly one parent
			if (func.mEntryPointCalls.size() == 1)
			{
				std::set<FunctionInfo*>& entryPointCallers = func.mEntryPointCalls.begin()->second;
				if (entryPointCallers.size() == 1)
				{
					func.mParent = *entryPointCallers.begin();
				}
			}

			if (nullptr != func.mParent)
			{
				func.mParent->mChildren.push_back(&func);
			}
		#endif
		}

		// Build up a tree hierarchy, resolving cycles
		{
			enum HierarchyTags
			{
				TAG_NONE	= 0,
				TAG_ROOT	= 1,
				TAG_CHILD	= 2,
				TAG_PENDING	= 3
			};
			std::vector<FunctionInfo*> pending;

			for (auto& pair : mFunctions)
			{
				FunctionInfo* current = &pair.second;
				while (true)
				{
					if (current->mHierarchyTag != TAG_NONE)
					{
						// Stop here
						if (current->mHierarchyTag == TAG_PENDING)
						{
							// Found a cycle
							current->mHierarchyTag = TAG_ROOT;
							current->mParent = nullptr;
						}
						break;
					}

					if (nullptr == current->mParent)
					{
						// No parent, then this is a root automatically
						current->mHierarchyTag = TAG_ROOT;
						break;
					}

					current->mHierarchyTag = TAG_PENDING;
					pending.push_back(current);

					current = current->mParent;
				}

				for (FunctionInfo* it : pending)
				{
					if (it != current)
					{
						it->mHierarchyTag = TAG_CHILD;
					}
				}
				pending.clear();
			}
		}

		// Add all top level functions into hierarchy root node
		for (auto& pair : mFunctions)
		{
			FunctionInfo& func = pair.second;
			if (func.mParent == nullptr)
			{
				addToFunctionHierarchy(mFunctionHierarchy, func);
			}
		}

		// Check if there were remaining functions, e.g. a loop in structure (can happen because we only consider one parent)
		for (auto& pair : mFunctions)
		{
			addToFunctionHierarchy(mFunctionHierarchy, pair.second);
		}
	}

	// Search for jumps to get labels
	for (auto& pair : mFunctions)
	{
		FunctionInfo& func = pair.second;

		// Search for jumps
		for (LineData& line : func.mLines)
		{
			if (line.mAssemblyCode->mType == assembly::CodeType::CODE_JUMP)
			{
				if (line.mAssemblyCode->mParamDest.mType == assembly::Parameter::Type::CONSTANT)
				{
					// Search for label
					const uint32 labelAddress = line.mAssemblyCode->mParamDest.mConstant.mValue;
					for (LineData& line2 : func.mLines)
					{
						if (line2.mAddress == labelAddress)
						{
							line.mLeadsToLabel = true;
							line2.mIsLabel = true;
							break;
						}
					}
				}
			}
		}
	}
}

void HtmlWriter::addToFunctionHierarchy(HierarchyNode& parentNode, FunctionInfo& func)
{
	// Prevention of endless recursion
	if (func.mAddedToHierarchy)
		return;

	// Add new hierarchy node
	HierarchyNode* node = new HierarchyNode();
	node->mFunction = &func;
	node->mDepth = parentNode.mDepth + 1;
	parentNode.mChildren.push_back(node);

	mLinearFunctionHierarchy.push_back(node);
	func.mAddedToHierarchy = true;

	// Recursively process function's children
	for (FunctionInfo* child : func.mChildren)
	{
		addToFunctionHierarchy(*node, *child);
	}
}

void HtmlWriter::addHtmlBoxForFunction(OutputStreams& outputStreams, const HierarchyNode& node)
{
	std::stringstream& str = outputStreams.mHtmlStream;
	std::stringstream& str2 = outputStreams.mTextStream;
	const FunctionInfo& func = *node.mFunction;

//	// No output at all if function was not called and not calling something
//	if (func.mCallCounter == 0 && func.mCallsTo.empty())
//		return;

	// Outer box div
	str << "<div class=\"box\" id=\"fn" << func.mAddressString << "\">";

	// Title
	str << "<b class=\"title\">" << func.mDisplayName << "</b>";
	str << "<a class=\"sync_link\" href=\"#ov" << func.mAddressString << "\">[sync]</a>";
	str << "<br/>\r\n";

	// Basic info
	str << " &nbsp; Start address: 0x" << func.mAddressString << "<br>\r\n";
	str << "<hr/>\r\n";

	// Call entry points
	if (!func.mEntryPointCalls.empty())
	{
		for (const auto& pair : func.mEntryPointCalls)
		{
			str << " &nbsp; Calling entry point 0x" << addressString(pair.first) << ": &nbsp; ";
			writeSortedFunctionList(str, pair.second);
		}
	}

	// Calls to other functions
	if (!func.mCallsTo.empty())
	{
		str << " &nbsp; Calls following functions: &nbsp; ";
		writeSortedFunctionList(str, func.mCallsTo);
	}

	// Code output
	{
		str << "<hr/>\r\n";
		str << "<span name=\"code\" class=\"code\"/>\r\n";

		if (!func.mLines.empty())
		{
			str2 << "//# address-hook(0x" << func.mAddressString << ") end(0x" << addressString(func.mLines.back().mAddress) << ")" << "\r\n";
			str2 << "function void fn" << func.mAddressString << "()" << "\r\n";
			str2 << "{" << "\r\n";

			std::vector<std::vector<const LineData*>> sequences;

			// Get sequences from lines
			uint32 nextAddress = 0xffffffff;
			for (const LineData& line : func.mLines)
			{
				if (nextAddress == 0xffffffff || line.mAddress > nextAddress)
				{
					// Start a new sequence here
					sequences.emplace_back();
				}
				sequences.back().push_back(&line);
				nextAddress = line.mAddress + line.mAssemblyCode->mLength;
			}

			// Go over all sequences
			bool firstSequence = true;
			for (const std::vector<const LineData*>& sequence : sequences)
			{
				if (!firstSequence)
				{
					// Mark parts between sequences
					str << " &nbsp; &nbsp; ...<br/>";
				}
				firstSequence = false;

				// Build lemonizer code
				lemonizer::Block lemonizerBlock;

				for (const LineData* line : sequence)
				{
					lemonizer::CodeAssembly& code = lemonizerBlock.create<lemonizer::CodeAssembly>();
					code.mLines.push_back(line);
					code.mAssemblyCode = line->mAssemblyCode;
				}

				// TODO: This won't work if last code is unrecognized (i.e. INVALID type)
				lemonizerBlock.mEndAddress = sequence.back()->mAddress + sequence.back()->mAssemblyCode->mLength;

				// Do optimization here
				//  -> This could be made optional, so we can output good old disassembly as well
				lemonizer::Optimization::optimize(lemonizerBlock);

				writeLemonizerCode(outputStreams, lemonizerBlock);
			}

			str2 << "}" << "\r\n" << "\r\n" << "\r\n";
		}

		str << "</span>\r\n";
	}

	// End outer box div
	str << "</div>\r\n";

	// And now for the children
	if (!node.mChildren.empty())
	{
		str << "<div class=\"indent\">";
		for (const HierarchyNode* childNode : node.mChildren)
		{
			addHtmlBoxForFunction(outputStreams, *childNode);
		}
		str << "</div>\r\n";
	}
}

void HtmlWriter::writeSortedFunctionList(std::stringstream& str, const FunctionSet& functionSet)
{
	bool isFirst = true;
	for (const FunctionInfo* func : getSortedFunctionList(functionSet))
	{
		if (isFirst)
			isFirst = false;
		else
			str << " --&nbsp;";

		str << "<a href=\"#fn" << func->mAddressString << "\">" << func->mDisplayName << "</a>";
	}
	str << "<br/>\r\n";
}

const char* HtmlWriter::getContentClassByInstruction(const RomContent::Instruction& instruction)
{
	// Is it a control flow instruction?
	return ((instruction.mFlags & (RomContent::InstructionFlag::ANY_JUMP)) != 0) ? "line_ctrlflow" : "line_normal";
}

void HtmlWriter::writeLemonizerCode(OutputStreams& outputStreams, const lemonizer::Block& lemonizerBlock)
{
	writeLemonizerCode(outputStreams, lemonizerBlock, 0);
}

void HtmlWriter::writeLemonizerCode(OutputStreams& outputStreams, const lemonizer::Block& lemonizerBlock, uint32 indentation)
{
	const bool createBlockBraces = (indentation > 0 && !lemonizerBlock.mOutputAsSingleLine);
	if (createBlockBraces)
	{
		HtmlCodeLineHelper codeLineHelper;
		codeLineHelper.setIndentation(indentation - 1);
		codeLineHelper.setContentClass("line_ctrlflow");
		codeLineHelper.addContent("{");
		codeLineHelper.outputHtmlLine(outputStreams.mHtmlStream);
		codeLineHelper.outputTextLine(outputStreams.mTextStream);
	}

	for (size_t i = 0; i < lemonizerBlock.size(); ++i)
	{
		const lemonizer::Code& code = lemonizerBlock[i];
		RMX_CHECK(!code.mLines.empty(), "No lines in lemonizer code", continue);

		const LineData& lineData = *static_cast<const LineData*>(code.mLines[0]);	// Evaluating only the first should be okay
		const RomContent::Instruction& instruction = *lineData.mInstruction;
		const uint32 address = lineData.mAddress;

		// Actual line content
		{
			HtmlCodeLineHelper codeLineHelper;
			if (code.getType() != lemonizer::Code::CodeType::WHILE)
			{
				codeLineHelper.setAddress(address, lineData.mIsLabel);
			}
			codeLineHelper.setIndentation(indentation);

			// Lemonizer code type dependent style
			switch (code.getType())
			{
				case lemonizer::Code::IFELSE:
				case lemonizer::Code::WHILE:
				{
					codeLineHelper.setContentClass("line_ctrlflow");
					break;
				}
			
				default:
				{
					codeLineHelper.setContentClass(getContentClassByInstruction(instruction));
					break;
				}
			}

			// TODO: Add this in when ready...
		#if 0
			if (lineData.mIsLabel)
			{
				codeLineHelper.addLabel(rmx::hexString(address, 6, ""));
			}
		#endif

			// Line content is produced by the lemonizer code formatter
			{
				lemonizer::Formatter::Output formatted;
				lemonizer::Formatter::formatCode(code, address, formatted);

				for (const assembly::Formatter::Output::Line& line : formatted.mLines)
				{
					for (const assembly::Formatter::Token& token : line.mTokens)
					{
						switch (token.mType)
						{
							case assembly::Formatter::Token::TEXT:
								codeLineHelper.addContent(token.mText);
								break;

							case assembly::Formatter::Token::JUMP_TARGET:
								if (token.mValue == 0)
									codeLineHelper.addContentLabel("line" + token.mText, "0x" + token.mText);
								else
									codeLineHelper.addContentLabel("line" + addressString(token.mValue), token.mText);
								break;

							case assembly::Formatter::Token::TABULATOR:
								codeLineHelper.addContentSpacesTo(token.mValue, 1);
								break;
						}
					}
					codeLineHelper.newLine();
				}
			}

			// Assembly output as tooltip
			if (!code.mLines.empty())
			{
				// TODO: Formatting is very reduced this way, maybe we could use a different CodeLineHelper instance for the tooltip content?

				std::string tooltip;
				
				for (const lemonizer::LineData* lineData : code.mLines)
				{
					if (nullptr != lineData->mAssemblyCode)
					{
						if (!tooltip.empty())
							tooltip += "\n";

						tooltip += addressString(lineData->mAddress) + "   ";

						assembly::Formatter::Output formatted;
						assembly::Formatter::formatCode(*lineData->mAssemblyCode, address, formatted);

						for (const assembly::Formatter::Token& token : formatted.mLines.front().mTokens)
						{
							switch (token.mType)
							{
								case assembly::Formatter::Token::TEXT:
									tooltip += token.mText;
									break;

								case assembly::Formatter::Token::JUMP_TARGET:
									tooltip += "@" + token.mText;
									break;

								case assembly::Formatter::Token::TABULATOR:
									tooltip += " ";
									break;
							}
						}
					}
				}

				codeLineHelper.setTooltip(tooltip);
			}

			// Write line
			codeLineHelper.outputHtmlLine(outputStreams.mHtmlStream);
			codeLineHelper.outputTextLine(outputStreams.mTextStream);
		}

		// Additional stuff for this code (e.g. child blocks)
		if (code.getType() == lemonizer::Code::IFELSE)
		{
			const lemonizer::CodeIfElse& ci = code.as<lemonizer::CodeIfElse>();

			// Output the 'if' block
			writeLemonizerCode(outputStreams, ci.mIfBlock, indentation + 1);

			// Output the 'else' block, if there is one
			if (!ci.mElseBlock.empty())
			{
				HtmlCodeLineHelper codeLineHelper;
				codeLineHelper.setIndentation(indentation);
				codeLineHelper.setContentClass("line_ctrlflow");
				codeLineHelper.addContent("else");
				codeLineHelper.outputHtmlLine(outputStreams.mHtmlStream);
				codeLineHelper.outputTextLine(outputStreams.mTextStream);

				writeLemonizerCode(outputStreams, ci.mElseBlock, indentation + 1);
			}
		}
		else if (code.getType() == lemonizer::Code::WHILE)
		{
			// Output the inner block
			const lemonizer::CodeWhile& cw = code.as<lemonizer::CodeWhile>();
			writeLemonizerCode(outputStreams, cw.mInnerBlock, indentation + 1);
		}
	}

	if (createBlockBraces)
	{
		HtmlCodeLineHelper codeLineHelper;
		codeLineHelper.setIndentation(indentation - 1);
		codeLineHelper.setContentClass("line_ctrlflow");
		codeLineHelper.addContent("}");
		codeLineHelper.outputHtmlLine(outputStreams.mHtmlStream);
		codeLineHelper.outputTextLine(outputStreams.mTextStream);
	}
}

void HtmlWriter::translateFunctionSet(FunctionSet& output, const FunctionBuilder::FunctionSet& input)
{
	for (const auto& pair : input)
	{
		const auto it = mFunctions.find(pair->mStartAddress);
		if (it != mFunctions.end())
		{
			output.insert(&it->second);
		}
		else
		{
			RMX_ERROR("Function not found", );
		}
	}
}

std::vector<HtmlWriter::FunctionInfo*> HtmlWriter::getSortedFunctionList(const FunctionSet& input)
{
	std::vector<FunctionInfo*> funcs;
	for (auto& func : input)
	{
		funcs.emplace_back(func);
	}

	std::sort(funcs.begin(), funcs.end(),
			  [](const FunctionInfo* a, const FunctionInfo* b) { return a->mStartAddress < b->mStartAddress; }
	);

	return funcs;
}
