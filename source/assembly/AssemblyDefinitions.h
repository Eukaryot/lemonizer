/*
*	Lemonizer -- turns 68K code into lemon script
*	Copyright (C) 2021 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#pragma once

#include <rmxbase.h>


namespace assembly
{

	// --- Enum type definitions

	enum class CodeType
	{
		INVALID = 0,
		CODE_NOP,
		CODE_MOVE,
		CODE_MOVE_P,
		CODE_LOAD_REGISTERS,
		CODE_SAVE_REGISTERS,
		CODE_SET_FLAGS,
		CODE_GET_STATUS,
		CODE_SET_STATUS,
		CODE_SET_STATUS_AND,
		CODE_SET_STATUS_OR,
		CODE_GET_USP,
		CODE_SET_USP,
		CODE_LOAD_EA,
		CODE_PUSH_EA,
		CODE_TEST,
		CODE_CLEAR,
		CODE_ADD,
		CODE_ADDX,
		CODE_SUB,
		CODE_SUBX,
		CODE_CMP,
		CODE_MUL,
		CODE_DIV,
		CODE_ADD_BCD,
		CODE_NEG,
		CODE_NOT,
		CODE_AND,
		CODE_OR,
		CODE_XOR,
		CODE_SHIFT_LEFT,
		CODE_SHIFT_RIGHT,
		CODE_SET_BY_CONDITION,
		CODE_SET_BIT,
		CODE_CLEAR_BIT,
		CODE_CHANGE_BIT,
		CODE_TEST_BIT,
		CODE_EXTEND_SIGNED,
		CODE_SWAP_WORDS,
		CODE_EXCHANGE,
		CODE_LINK,
		CODE_UNLINK,
		CODE_JUMP,
		CODE_CALL,
		CODE_RETURN,
		CODE_RETURN_EXCEPTION
	};

	struct DataType
	{
		enum class Size
		{
			UNSPECIFIED	= 0,
			SIZE_8		= 1,
			SIZE_16		= 2,
			SIZE_32		= 4
		};

		enum class Sign
		{
			UNSPECIFIED	= 0,
			UNSIGNED	= 1,
			SIGNED		= 2,
		};

		Size mSize = Size::UNSPECIFIED;
		Sign mSign = Sign::UNSPECIFIED;

		static const DataType u8;
		static const DataType u16;
		static const DataType u32;
		static const DataType s8;
		static const DataType s16;
		static const DataType s32;

		inline DataType() {}
		inline DataType(Size size, Sign sign = Sign::UNSPECIFIED) : mSize(size), mSign(sign) {}

		inline bool isUnspecified() const	{ return mSize == Size::UNSPECIFIED; }

		inline bool isSigned() const		{ return mSign == Sign::SIGNED; }
		inline bool isUnsigned() const		{ return mSign == Sign::UNSIGNED; }

		inline uint32 getSizeInBits() const		{ return (uint32)mSize * 8; }
		inline uint32 getSizeInBytes() const	{ return (uint32)mSize; }
		inline static uint32 getSizeInBits(Size size)	{ return (uint32)size * 8; }
		inline static uint32 getSizeInBytes(Size size)	{ return (uint32)size; }

		const std::string& toString() const
		{
			static const std::string unspecified;
			static const std::string postfixUnsigned[] = { "", "u8", "u16", "", "u32" };
			static const std::string postfixSigned[]   = { "", "s8", "s16", "", "s32" };
			return isSigned() ? postfixSigned[(size_t)mSize] : postfixUnsigned[(size_t)mSize];
		}
	};

	enum class Register : uint32
	{
		D0, D1, D2, D3, D4, D5, D6, D7,
		A0, A1, A2, A3, A4, A5, A6, A7
	};

	enum class ExtRegister : uint32
	{
		D0, D1, D2, D3, D4, D5, D6, D7,
		A0, A1, A2, A3, A4, A5, A6, A7,
		NONE
	};

	enum class Condition : uint8
	{
		CS, CC, VS, VC, NE, EQ, MI, PL,
		LT, GE, HI, LS, GT, LE,
		NONE
	};

	enum class ShiftType : uint8
	{
		LOGICAL,		// Unsigned bit shift
		ARITHMETIC,		// Signed bit shift
		ROTATE,			// Bit rotation
		ROTATE_X		// Bit rotation with FLAG_X as additional bit
	};



	// --- Parameter definitions

	struct Parameter
	{
	public:
		enum class Type
		{
			UNDEFINED = 0,
			CONSTANT,	// Constant value
			REGISTER,	// One of the registers Ax or Dx
			COMBINED,	// Combination <Register1> + <Register2> + displacement
			CONDITION	// Condition value
		};

		Type mType = Type::UNDEFINED;
		bool mIsMemory = false;		// If true, parameter content only given the address of where to look

		struct RegisterParameter
		{
			Register mRegister;
			uint32 mPreDecrement;
			uint32 mPostIncrement;

			explicit RegisterParameter(Register reg) : mRegister(reg), mPreDecrement(0), mPostIncrement(0) {}
		};

		struct ConstantParameter
		{
			uint32 mValue;
			uint32 mImmSize;
		
			explicit ConstantParameter(uint32 value, uint32 immSize) : mValue(value), mImmSize(immSize) {}
		};

		struct CombinedParameter
		{
			Register		mRegister1;
			DataType::Size	mSizeOfRegister1;
			ExtRegister		mRegister2;
			int32			mDisplacement;
			uint32			mImmSize;
		};

		struct ConditionParameter
		{
			Condition	mCondition;
			ExtRegister mLoopRegister;

			explicit ConditionParameter(Condition cond) : mCondition(cond), mLoopRegister(ExtRegister::NONE) {}
		};

		union
		{
			RegisterParameter  mRegister;
			ConstantParameter  mConstant;
			CombinedParameter  mCombined;
			ConditionParameter mCondition;
		};

	public:
		inline Parameter()
		{
			memset(&mRegister.mRegister, 0, sizeof(CombinedParameter));
		}

		inline Parameter(const RegisterParameter& param)  : mType(Type::REGISTER),  mRegister(param)  {}
		inline Parameter(const ConstantParameter& param)  : mType(Type::CONSTANT),  mConstant(param)  {}
		inline Parameter(const CombinedParameter& param)  : mType(Type::COMBINED),  mCombined(param)  {}
		inline Parameter(const ConditionParameter& param) : mType(Type::CONDITION), mCondition(param) {}

		inline bool isRegister() const					{ return mType == Type::REGISTER; }
		inline bool isPureRegister() const				{ return mType == Type::REGISTER && !mIsMemory; }
		inline bool isPureRegister(Register reg) const  { return mType == Type::REGISTER && mRegister.mRegister == reg && !mIsMemory; }

		inline bool isConstant() const			{ return mType == Type::CONSTANT; }
		inline bool isConstantValue() const		{ return mType == Type::CONSTANT && !mIsMemory; }
		inline bool isConstantMemory() const	{ return mType == Type::CONSTANT && mIsMemory; }

		inline bool isCombined() const			{ return mType == Type::COMBINED; }
		inline bool isCombinedMemory() const	{ return mType == Type::COMBINED && mIsMemory; }
	};



	// --- Single assembly code

	struct AssemblyCode
	{
		CodeType mType = CodeType::INVALID;
		uint32 mLength = 0;

		DataType mDataType;			// Signed can have different meanings (used for MUL and DIV, also "movea", "adda", etc.)
		Parameter mParamDest;
		Parameter mParamSource;

		bool mWriteFlags = false;	// If true, write flags register
		uint8 mUseCycles = 0xff;	// Used for shifts; 0xff = use default
		ShiftType mShiftType;		// Only for shifts
	};



	// TODO: Move to a helper header/class

	inline Condition negateCondition(Condition condition)
	{
		static const Condition negatedCondition[] =
		{
			Condition::CC, Condition::CS, Condition::VC, Condition::VS, Condition::EQ, Condition::NE, Condition::PL, Condition::MI,
			Condition::GE, Condition::LT, Condition::LS, Condition::HI, Condition::LE, Condition::GT,
			Condition::NONE
		};
		return negatedCondition[(int)condition];
	}

}
