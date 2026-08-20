/*
*	Lemonizer -- turns 68K code into lemon script
*	Copyright (C) 2021 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#pragma once

#include <rmxbase.h>
#include "assembly/AssemblyDefinitions.h"
#include <lemon/compiler/TokenTypes.h>


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
			BREAK_OR_CONTINUE
		};

	public:
		template<typename T> T& as()  { return static_cast<T&>(*this); }
		template<typename T> const T& as() const  { return static_cast<const T&>(*this); }

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
		bool mOutputAsSingleLine = false;
	};



	class CodeAssembly : public Code
	{
	public:
		static const uint32 TYPE = ASSEMBLY;

	public:
		inline CodeAssembly() : Code(TYPE) {}

	public:
		const assembly::AssemblyCode* mAssemblyCode = nullptr;
	};


	class CodeLemonTokenTree : public Code
	{
	public:
		static const uint32 TYPE = LEMONTOKENTREE;

	public:
		inline CodeLemonTokenTree() : Code(TYPE) {}

		// Note: This returns in no case an actual valid pointer, instead it's a casted uint8
		static lemon::Variable* getRegisterVariable(assembly::Register reg, const assembly::DataType& dataType);
		static void splitRegisterVariable(const lemon::Variable* variable, assembly::Register& outReg, assembly::DataType& outDataType);

	public:
		lemon::TokenPtr<lemon::StatementToken> mRoot;
	};


	class CodeIfElse : public Code
	{
	public:
		static const uint32 TYPE = IFELSE;

	public:
		inline CodeIfElse() : Code(TYPE) {}

	public:
		assembly::Condition mCondition;
		assembly::ExtRegister mLoopRegister = assembly::ExtRegister::NONE;
		bool mNegateWholeCondition = false;
		const assembly::AssemblyCode* mAssemblyCode = nullptr;	// Optional condition assembly code
		Block mIfBlock;
		Block mElseBlock;
	};


	class CodeWhile : public Code
	{
	public:
		static const uint32 TYPE = WHILE;

	public:
		inline CodeWhile() : Code(TYPE) {}

	public:
		Block mInnerBlock;
	};


	class CodeJumpOrCall : public Code
	{
	public:
		static const uint32 TYPE = JUMP_OR_CALL;

	public:
		inline CodeJumpOrCall() : Code(TYPE) {}

	public:
		bool mIsCall = false;
		uint32 mDestinationAddress = 0;
	};


	class CodeBreakOrContinue : public Code
	{
	public:
		static const uint32 TYPE = BREAK_OR_CONTINUE;

	public:
		inline CodeBreakOrContinue() : Code(TYPE) {}

	public:
		bool mIsContinue = false;
	};

}
