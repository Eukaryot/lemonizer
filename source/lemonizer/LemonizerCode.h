/*
*	Lemonizer -- Turns 68K code into lemonscript
*	Copyright (C) 2017-2026 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#pragma once

#include "assembly/AssemblyDefinitions.h"
#include "lemonizer/CustomLemonTokens.h"


namespace lemon
{
	class GlobalVariable;
}

namespace lemonizer
{

	struct LineData
	{
		uint32 mAddress = 0xffffffff;
		const assembly::AssemblyCode* mAssemblyCode = nullptr;
		bool mIsLabel = false;
		bool mLeadsToLabel = false;
		bool mShowWithLabel = false;
		bool mIsOutOfOrder = false;
	};


	class Code : public genericmanager::Element<Code>
	{
	public:
		enum CodeType
		{
			ASSEMBLY,
			LEMONTOKENTREE,
			IFELSE,
			WHILE,
			JUMP_OR_CALL,
			RETURN,
			BREAK,
			CONTINUE
		};

	protected:
		inline Code(uint32 type) : genericmanager::Element<Code>(type) {}
		inline virtual ~Code() {}

	public:
		std::vector<const LineData*> mLines;
	};


	class Block : public genericmanager::ElementList<Code, 32>
	{
	public:
		uint32 mEndAddress = 0xffffffff;
		uint32 mBreakTargetAddress = 0xffffffff;
		bool mOutputAsSingleLine = false;
		bool mAddJumpToEndAddress = false;
		Block* mParentBlock = nullptr;
	};


	template<typename T>
	class CodePtr : public genericmanager::ElementPtr<T, Code>
	{
	public:
		using genericmanager::ElementPtr<T, Code>::operator=;
	};



	#define DEFINE_CODE_TYPE(_class_, _type_) \
	public: \
		DEFINE_GENERIC_MANAGER_ELEMENT_TYPE(Code, Code, _class_, (uint32)_type_)


	class CodeAssembly : public Code
	{
		DEFINE_CODE_TYPE(CodeAssembly, ASSEMBLY)

	public:
		static const assembly::AssemblyCode* getAssemblyCode(const Code& code);
		static const assembly::AssemblyCode* getAssemblyCode(const Code& code, assembly::CodeType filterType);

	public:
		const assembly::AssemblyCode* mAssemblyCode = nullptr;
	};


	class CodeLemonTokenTree : public Code
	{
		DEFINE_CODE_TYPE(CodeLemonTokenTree, LEMONTOKENTREE)

	public:
		// Note: This returns in no case an actual valid pointer, instead it's a casted uint8
		static lemon::Variable* getRegisterVariable(assembly::Register reg, const assembly::DataType& dataType);
		static void splitRegisterVariable(const lemon::Variable* variable, assembly::Register& outReg, assembly::DataType& outDataType);

	public:
		lemon::TokenPtr<lemon::StatementToken> mRoot;
	};


	class CodeIfElse : public Code
	{
		DEFINE_CODE_TYPE(CodeIfElse, IFELSE)

	public:
		assembly::Condition mCondition;
		assembly::ExtRegister mLoopRegister = assembly::ExtRegister::NONE;
		bool mNegateWholeCondition = false;
		const assembly::AssemblyCode* mConditionAssemblyCode = nullptr;	// Optional condition assembly code
		lemon::TokenPtr<lemon::StatementToken> mConditionRoot;
		Block mIfBlock;
		Block mElseBlock;
		std::vector<const LineData*> mElseLines;
	};


	class CodeWhile : public Code
	{
		DEFINE_CODE_TYPE(CodeWhile, WHILE)

	public:
		Block mInnerBlock;
	};


	class CodeJumpOrCall : public Code
	{
		DEFINE_CODE_TYPE(CodeJumpOrCall, JUMP_OR_CALL)

	public:
		bool mIsCall = false;
		uint32 mDestinationAddress = 0;
	};


	class CodeReturn : public Code
	{
		DEFINE_CODE_TYPE(CodeReturn, RETURN)
	};


	class CodeBreak : public Code
	{
		DEFINE_CODE_TYPE(CodeBreak, BREAK)
	};


	class CodeContinue : public Code
	{
		DEFINE_CODE_TYPE(CodeContinue, CONTINUE)
	};


	#undef DEFINE_CODE_TYPE

}
