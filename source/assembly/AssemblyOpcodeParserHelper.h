/*
*	Lemonizer -- Turns 68K code into lemonscript
*	Copyright (C) 2017-2026 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#pragma once

#include "assembly/AssemblyDefinitions.h"
#include "assembly/AssemblyOpcodeTable.h"


namespace assembly
{

	class OpcodeParserHelper
	{
	public:
		OpcodeParserHelper(uint32 address, const uint8* memory, AssemblyCode& outputCode);

		bool parseOpcode(OpcodeType opcodeType);

	private:
		enum ParamVariant
		{
			PARAM_i,		// Parameter is a constant value
			PARAM_d,		// Parameter is content of Dx register
			PARAM_a,		// Parameter is content of Ax register
			PARAM_ai,		// Memory access at address [Ax]
			PARAM_di,		// Memory access at address [Ax + displacement]
			PARAM_pi,		// Memory access at address [Ax] with post-increment
			PARAM_pi7,		// Memory access at address [A7] with post-increment of fixed 2 bytes
			PARAM_pd,		// Memory access at address [Ax] with pre-decrement
			PARAM_pd7,		// Memory access at address [A7] with pre-decrement of fixed 2 bytes
			PARAM_ix,		// Memory access at address [Ax + <register> + displacement]
			PARAM_aw,		// Memory access at given absolute word address (i.e. uint16 address)
			PARAM_al,		// Memory access at given absolute long address (i.e. uint32 address)
			PARAM_pcdi,		// Memory access at address [PC + displacement]
			PARAM_pcix		// Memory access at address [PC + <register> + displacement]
		};

		enum class XorY { X, Y };

	private:
		void parseOpcodeInternal(OpcodeType opcodeType);

		uint8  getImmediate8(uint32 offset) const  { return mMemory[offset]; }
		uint16 getImmediate16(uint32 offset) const { return *(uint16*)&mMemory[offset]; }
		uint32 getImmediate32(uint32 offset) const { return ((uint32)getImmediate16(offset) << 16) + (uint32)getImmediate16(offset + 2); }

		uint8  readImmediate8()  { mImmediatePosition += 2; return getImmediate8(mImmediatePosition - 2); }
		uint16 readImmediate16() { mImmediatePosition += 2; return getImmediate16(mImmediatePosition - 2); }
		uint32 readImmediate32() { mImmediatePosition += 4; return getImmediate32(mImmediatePosition - 4); }

		inline Register getImmediateAX() { return (Register)((uint32)Register::A0 + ((getImmediate16(0) >> 9) & 7)); }
		inline Register getImmediateAY() { return (Register)((uint32)Register::A0 + (getImmediate16(0) & 7)); }
		inline Register getImmediateDX() { return (Register)((uint32)Register::D0 + ((getImmediate16(0) >> 9) & 7)); }
		inline Register getImmediateDY() { return (Register)((uint32)Register::D0 + (getImmediate16(0) & 7)); }

		template<XorY> Register getImmediateA() { return Register::A0; }
		template<XorY> Register getImmediateD() { return Register::D0; }
		template<> inline Register getImmediateA<XorY::X>() { return getImmediateAX(); }
		template<> inline Register getImmediateA<XorY::Y>() { return getImmediateAY(); }
		template<> inline Register getImmediateD<XorY::X>() { return getImmediateDX(); }
		template<> inline Register getImmediateD<XorY::Y>() { return getImmediateDY(); }


		template<XorY xy>
		Parameter getMemoryAddressParameterByVariant(DataType::Size size, ParamVariant paramVariant)
		{
			Parameter result;
			switch (paramVariant)
			{
				case PARAM_ai:
					result = Parameter::RegisterParameter(getImmediateA<xy>());
					break;

				case PARAM_di:
					result.mType = Parameter::Type::COMBINED;
					result.mCombined.mRegister1 = getImmediateA<xy>();
					result.mCombined.mSizeOfRegister1 = DataType::Size::SIZE_32;
					result.mCombined.mRegister2 = ExtRegister::NONE;
					result.mCombined.mDisplacement = (int16)readImmediate16();
					result.mCombined.mImmSize = 2;
					break;

				case PARAM_pi:
					result = Parameter::RegisterParameter(getImmediateA<xy>());
					result.mRegister.mPostIncrement = DataType::getSizeInBytes(size);	// Post-increment AX/AY
					break;

				case PARAM_pi7:
					result = Parameter::RegisterParameter(Register::A7);
					result.mRegister.mPostIncrement = 2;	// Post-increment A7
					break;

				case PARAM_pd:
					result = Parameter::RegisterParameter(getImmediateA<xy>());
					result.mRegister.mPreDecrement = DataType::getSizeInBytes(size);	// Pre-decrement AX/AY
					break;

				case PARAM_pd7:
					result = Parameter::RegisterParameter(Register::A7);
					result.mRegister.mPreDecrement = 2;		// Pre-decrement A7
					break;

				case PARAM_ix:
				{
					const uint16 extension = readImmediate16();
					result.mType = Parameter::Type::COMBINED;
					result.mCombined.mRegister1 = (Register)(extension >> 12);
					result.mCombined.mSizeOfRegister1 = ((extension & 0x0800) != 0) ? DataType::Size::SIZE_32 : DataType::Size::SIZE_16;
					result.mCombined.mRegister2 = (ExtRegister)(getImmediateA<xy>());
					result.mCombined.mDisplacement = (int8)(extension);
					result.mCombined.mImmSize = 2;
					break;
				}

				case PARAM_aw:
					result = Parameter::ConstantParameter((int32)(int16)readImmediate16(), 2);
					break;

				case PARAM_al:
					result = Parameter::ConstantParameter(readImmediate32(), 4);
					break;

				case PARAM_pcdi:
				{
					const uint32 baseAddress = mAddress + mImmediatePosition;
					result = Parameter::ConstantParameter(baseAddress + (int16)readImmediate16(), 2);
					break;
				}

				case PARAM_pcix:
				{
					const uint32 baseAddress = mAddress + mImmediatePosition;
					const uint16 extension = readImmediate16();
					result.mType = Parameter::Type::COMBINED;
					result.mCombined.mRegister1 = (Register)(extension >> 12);
					result.mCombined.mSizeOfRegister1 = ((extension & 0x0800) != 0) ? DataType::Size::SIZE_32 : DataType::Size::SIZE_16;
					result.mCombined.mRegister2 = ExtRegister::NONE;
					result.mCombined.mDisplacement = baseAddress + (int8)(extension);
					result.mCombined.mImmSize = 2;
					break;
				}
			}

			result.mIsMemory = true;
			return result;
		}

		inline Parameter getMemoryAddressParameterByVariantY(DataType::Size size, ParamVariant paramVariant)
		{
			return getMemoryAddressParameterByVariant<XorY::Y>(size, paramVariant);
		}


		template<XorY xy>
		Parameter getParameterByVariant(DataType::Size size, ParamVariant paramVariant)
		{
			switch (paramVariant)
			{
				case PARAM_i:
				{
					switch (size)
					{
						case DataType::Size::SIZE_8:	return Parameter::ConstantParameter(readImmediate8(), 2);	// Can't just go on a single byte
						case DataType::Size::SIZE_16:	return Parameter::ConstantParameter(readImmediate16(), 2);
						case DataType::Size::SIZE_32:	return Parameter::ConstantParameter(readImmediate32(), 4);
					}
				}

				case PARAM_d:
				{
					return Parameter::RegisterParameter(getImmediateD<xy>());
				}

				case PARAM_a:
				{
					return Parameter::RegisterParameter(getImmediateA<xy>());
				}

				default:	// All the other cases
				{
					return getMemoryAddressParameterByVariant<xy>(size, paramVariant);
				}
			}
		}

		inline Parameter getParameterByVariantX(DataType::Size size, ParamVariant paramVariant)
		{
			return getParameterByVariant<XorY::X>(size, paramVariant);
		}

		inline Parameter getParameterByVariantY(DataType::Size size, ParamVariant paramVariant)
		{
			return getParameterByVariant<XorY::Y>(size, paramVariant);
		}



		// --- Move

		void parseOpcode_move(DataType::Size size, ParamVariant paramVariantDest, ParamVariant paramVariantSource)
		{
			mOutputCode.mType = CodeType::CODE_MOVE;
			mOutputCode.mDataType.mSize = size;
			mOutputCode.mParamSource = getParameterByVariantY(size, paramVariantSource);
			mOutputCode.mParamDest = getParameterByVariantX(size, paramVariantDest);
			mOutputCode.mWriteFlags = true;
		}

		void parseOpcode_movea(DataType::Size size, ParamVariant paramVariantSource)
		{
			mOutputCode.mType = CodeType::CODE_MOVE;
			mOutputCode.mDataType.mSize = size;
			mOutputCode.mDataType.mSign = (size == DataType::Size::SIZE_16) ? DataType::Sign::SIGNED : DataType::Sign::UNSIGNED;	// Signed extension to 32-bit for SIZE_16
			mOutputCode.mParamSource = getParameterByVariantY(size, paramVariantSource);
			mOutputCode.mParamDest = Parameter::RegisterParameter(getImmediateAX());
			mOutputCode.mWriteFlags = false;
		}

		void parseOpcode_movem_er(DataType::Size size, ParamVariant paramVariant)
		{
			const uint16 registerList = readImmediate16();
			mOutputCode.mType = CodeType::CODE_LOAD_REGISTERS;
			mOutputCode.mDataType.mSize = size;
			mOutputCode.mParamSource = getMemoryAddressParameterByVariantY(size, paramVariant);
			mOutputCode.mParamDest = Parameter::ConstantParameter(registerList, 2);
			mOutputCode.mWriteFlags = false;
		}

		void parseOpcode_movem_re(DataType::Size size, ParamVariant paramVariant)
		{
			uint32 registerList = readImmediate16();
			if (paramVariant == ParamVariant::PARAM_pd)
				registerList |= 0x80000000;				// This is the marker for reverse register traversal
			mOutputCode.mType = CodeType::CODE_SAVE_REGISTERS;
			mOutputCode.mDataType.mSize = size;
			mOutputCode.mParamSource = Parameter::ConstantParameter(registerList, 2);
			mOutputCode.mParamDest = getMemoryAddressParameterByVariantY(size, paramVariant);
			mOutputCode.mWriteFlags = false;
		}

		void parseOpcode_movep_er(DataType::Size size)
		{
			mOutputCode.mType = CodeType::CODE_MOVE_P;
			mOutputCode.mDataType.mSize = size;
			mOutputCode.mParamSource = getMemoryAddressParameterByVariantY(size, ParamVariant::PARAM_di);
			mOutputCode.mParamDest = Parameter::RegisterParameter(getImmediateDX());
			mOutputCode.mWriteFlags = false;
		}

		void parseOpcode_movep_re(DataType::Size size)
		{
			mOutputCode.mType = CodeType::CODE_MOVE_P;
			mOutputCode.mDataType.mSize = size;
			mOutputCode.mParamSource = Parameter::RegisterParameter(getImmediateDX());
			mOutputCode.mParamDest = getMemoryAddressParameterByVariantY(size, ParamVariant::PARAM_di);
			mOutputCode.mWriteFlags = false;
		}

		void parseOpcode_moveq()
		{
			mOutputCode.mType = CodeType::CODE_MOVE;
			mOutputCode.mDataType.mSize = DataType::Size::SIZE_32;
			mOutputCode.mParamSource = Parameter::ConstantParameter((int32)(int8)getImmediate8(0), 0);
			mOutputCode.mParamDest = Parameter::RegisterParameter(getImmediateDX());
			mOutputCode.mWriteFlags = true;
		}

		void parseOpcode_move_toc(ParamVariant paramVariant)
		{
			mOutputCode.mType = CodeType::CODE_SET_FLAGS;
			mOutputCode.mDataType.mSize = DataType::Size::SIZE_16;
			mOutputCode.mParamSource = getParameterByVariantY(DataType::Size::SIZE_16, paramVariant);
			mOutputCode.mWriteFlags = true;
		}

		void parseOpcode_move_frs(ParamVariant paramVariant)
		{
			mOutputCode.mType = CodeType::CODE_GET_STATUS;
			mOutputCode.mDataType.mSize = DataType::Size::SIZE_16;
			mOutputCode.mParamDest = getParameterByVariantY(DataType::Size::SIZE_16, paramVariant);
			mOutputCode.mWriteFlags = false;
		}

		void parseOpcode_move_tos(ParamVariant paramVariant)
		{
			mOutputCode.mType = CodeType::CODE_SET_STATUS;
			mOutputCode.mDataType.mSize = DataType::Size::SIZE_16;
			mOutputCode.mParamSource = getParameterByVariantY(DataType::DataType::Size::SIZE_16, paramVariant);
			mOutputCode.mWriteFlags = false;
		}

		void parseOpcode_move_fru(ParamVariant paramVariant)
		{
			mOutputCode.mType = CodeType::CODE_GET_USP;
			mOutputCode.mDataType.mSize = DataType::Size::SIZE_32;
			mOutputCode.mParamDest = getParameterByVariantY(DataType::Size::SIZE_32, paramVariant);
			mOutputCode.mWriteFlags = false;
		}

		void parseOpcode_move_tou(ParamVariant paramVariant)
		{
			mOutputCode.mType = CodeType::CODE_SET_USP;
			mOutputCode.mDataType.mSize = DataType::Size::SIZE_32;
			mOutputCode.mParamSource = getParameterByVariantY(DataType::Size::SIZE_32, paramVariant);
			mOutputCode.mWriteFlags = false;
		}


		// --- Load effective address

		void parseOpcode_lea(ParamVariant paramVariant)
		{
			mOutputCode.mType = CodeType::CODE_LOAD_EA;
			mOutputCode.mDataType.mSize = DataType::Size::SIZE_32;
			mOutputCode.mParamSource = getMemoryAddressParameterByVariantY(DataType::Size::SIZE_32, paramVariant);
			mOutputCode.mParamDest = Parameter::RegisterParameter(getImmediateAX());
			mOutputCode.mWriteFlags = false;
		}


		// --- Push effective address

		void parseOpcode_pea(ParamVariant paramVariant)
		{
			mOutputCode.mType = CodeType::CODE_PUSH_EA;
			mOutputCode.mDataType.mSize = DataType::Size::SIZE_32;
			mOutputCode.mParamSource = getMemoryAddressParameterByVariantY(DataType::Size::SIZE_32, paramVariant);
			mOutputCode.mWriteFlags = false;
		}


		// --- Test

		void parseOpcode_tst(DataType::Size size, ParamVariant paramVariant)
		{
			mOutputCode.mType = CodeType::CODE_TEST;
			mOutputCode.mDataType.mSize = size;
			mOutputCode.mParamDest = getParameterByVariantY(size, paramVariant);
			mOutputCode.mWriteFlags = true;
		}


		// --- Clear

		void parseOpcode_clr(DataType::Size size, ParamVariant paramVariant)
		{
			mOutputCode.mType = CodeType::CODE_CLEAR;
			mOutputCode.mDataType.mSize = size;
			mOutputCode.mParamDest = getParameterByVariantY(size, paramVariant);
			mOutputCode.mWriteFlags = true;
		}


		// --- Shared for several binary operations

		void parseOpcode_binaryoperation_er(CodeType type, DataType::Size size, ParamVariant paramVariant, bool writeFlags = true)
		{
			mOutputCode.mType = type;
			mOutputCode.mDataType.mSize = size;
			mOutputCode.mParamSource = getParameterByVariantY(size, paramVariant);
			mOutputCode.mParamDest = Parameter::RegisterParameter(getImmediateDX());
			mOutputCode.mWriteFlags = writeFlags;
		}

		void parseOpcode_binaryoperation_re(CodeType type, DataType::Size size, ParamVariant paramVariant, bool writeFlags = true)
		{
			mOutputCode.mType = type;
			mOutputCode.mDataType.mSize = size;
			mOutputCode.mParamSource = Parameter::RegisterParameter(getImmediateDX());
			mOutputCode.mParamDest = getParameterByVariantY(size, paramVariant);
			mOutputCode.mWriteFlags = writeFlags;
		}

		void parseOpcode_binaryoperation_a(CodeType type, DataType::Size size, ParamVariant paramVariant, bool writeFlags)
		{
			mOutputCode.mType = type;
			mOutputCode.mDataType.mSize = size;
			mOutputCode.mParamSource = getParameterByVariantY(size, paramVariant);
			mOutputCode.mParamDest = Parameter::RegisterParameter(getImmediateAX());
			mOutputCode.mWriteFlags = writeFlags;
			mOutputCode.mDataType.mSign = (size == DataType::Size::SIZE_16) ? DataType::Sign::SIGNED : DataType::Sign::UNSIGNED;	// Signed extension to 32-bit for SIZE_16
		}

		void parseOpcode_binaryoperation_i(CodeType type, DataType::Size size, ParamVariant paramVariant, bool writeFlags = true)
		{
			mOutputCode.mType = type;
			mOutputCode.mDataType.mSize = size;
			mOutputCode.mParamSource = getParameterByVariantX(size, PARAM_i);
			mOutputCode.mParamDest = getParameterByVariantY(size, paramVariant);
			mOutputCode.mWriteFlags = writeFlags;
		}

		void parseOpcode_binaryoperation_q(CodeType type, DataType::Size size, ParamVariant paramVariant, bool writeFlags = true)
		{
			mOutputCode.mType = type;
			mOutputCode.mDataType.mSize = size;
			mOutputCode.mParamSource = Parameter::ConstantParameter((((getImmediate16(0) >> 9) - 1) & 7) + 1, 0);
			mOutputCode.mParamDest = getParameterByVariantY(size, paramVariant);
			mOutputCode.mWriteFlags = writeFlags;
		}

		void parseOpcode_binaryoperation_rr(CodeType type, DataType::Size size)
		{
			mOutputCode.mType = type;
			mOutputCode.mDataType.mSize = size;
			mOutputCode.mParamSource = Parameter::RegisterParameter(getImmediateDY());
			mOutputCode.mParamDest = Parameter::RegisterParameter(getImmediateDX());
			mOutputCode.mWriteFlags = true;
		}


		// --- Addition

		void parseOpcode_add_er(DataType::Size size, ParamVariant paramVariant)
		{
			parseOpcode_binaryoperation_er(CodeType::CODE_ADD, size, paramVariant, true);
		}

		void parseOpcode_add_re(DataType::Size size, ParamVariant paramVariant)
		{
			parseOpcode_binaryoperation_re(CodeType::CODE_ADD, size, paramVariant, true);
		}

		void parseOpcode_adda(DataType::Size size, ParamVariant paramVariant)
		{
			parseOpcode_binaryoperation_a(CodeType::CODE_ADD, size, paramVariant, false);
		}

		void parseOpcode_addi(DataType::Size size, ParamVariant paramVariant)
		{
			parseOpcode_binaryoperation_i(CodeType::CODE_ADD, size, paramVariant, true);
		}

		void parseOpcode_addq(DataType::Size size, ParamVariant paramVariant, bool writeFlags = true)
		{
			parseOpcode_binaryoperation_q(CodeType::CODE_ADD, size, paramVariant, writeFlags);
		}

		void parseOpcode_addx_rr(DataType::Size size)
		{
			parseOpcode_binaryoperation_rr(CodeType::CODE_ADDX, size);
		}


		// --- Subtraction

		void parseOpcode_sub_er(DataType::Size size, ParamVariant paramVariant)
		{
			parseOpcode_binaryoperation_er(CodeType::CODE_SUB, size, paramVariant, true);
		}

		void parseOpcode_sub_re(DataType::Size size, ParamVariant paramVariant)
		{
			parseOpcode_binaryoperation_re(CodeType::CODE_SUB, size, paramVariant, true);
		}

		void parseOpcode_suba(DataType::Size size, ParamVariant paramVariant)
		{
			parseOpcode_binaryoperation_a(CodeType::CODE_SUB, size, paramVariant, false);
		}

		void parseOpcode_subi(DataType::Size size, ParamVariant paramVariant)
		{
			parseOpcode_binaryoperation_i(CodeType::CODE_SUB, size, paramVariant, true);
		}

		void parseOpcode_subq(DataType::Size size, ParamVariant paramVariant, bool writeFlags = true)
		{
			parseOpcode_binaryoperation_q(CodeType::CODE_SUB, size, paramVariant, writeFlags);
		}

		void parseOpcode_subx_rr(DataType::Size size)
		{
			parseOpcode_binaryoperation_rr(CodeType::CODE_SUBX, size);
		}


		// --- Comparison

		void parseOpcode_cmp(DataType::Size size, ParamVariant paramVariant)
		{
			parseOpcode_binaryoperation_er(CodeType::CODE_CMP, size, paramVariant, true);
		}

		void parseOpcode_cmpa(DataType::Size size, ParamVariant paramVariant)
		{
			parseOpcode_binaryoperation_a(CodeType::CODE_CMP, size, paramVariant, true);
		}

		void parseOpcode_cmpi(DataType::Size size, ParamVariant paramVariant)
		{
			parseOpcode_binaryoperation_i(CodeType::CODE_CMP, size, paramVariant, true);
		}


		// --- Negation

		void parseOpcode_neg(DataType::Size size, ParamVariant paramVariant)
		{
			mOutputCode.mType = CodeType::CODE_NEG;
			mOutputCode.mDataType.mSize = size;
			mOutputCode.mParamDest = getParameterByVariantY(size, paramVariant);
			mOutputCode.mWriteFlags = true;
		}


		// --- Multiplication

		void parseOpcode_mul(DataType::Size size, ParamVariant paramVariant, bool isSigned)
		{
			mOutputCode.mType = CodeType::CODE_MUL;
			mOutputCode.mDataType.mSize = size;
			mOutputCode.mDataType.mSign = isSigned ? DataType::Sign::SIGNED : DataType::Sign::UNSIGNED;
			mOutputCode.mParamSource = getParameterByVariantY(size, paramVariant);
			mOutputCode.mParamDest = Parameter::RegisterParameter(getImmediateDX());
			mOutputCode.mWriteFlags = true;
		}

		void parseOpcode_muls(DataType::Size size, ParamVariant paramVariant)
		{
			parseOpcode_mul(size, paramVariant, true);
		}

		void parseOpcode_mulu(DataType::Size size, ParamVariant paramVariant)
		{
			parseOpcode_mul(size, paramVariant, false);
		}


		// --- Division

		void parseOpcode_div(DataType::Size size, ParamVariant paramVariant, bool isSigned)
		{
			mOutputCode.mType = CodeType::CODE_DIV;
			mOutputCode.mDataType.mSize = size;
			mOutputCode.mDataType.mSign = isSigned ? DataType::Sign::SIGNED : DataType::Sign::UNSIGNED;
			mOutputCode.mParamSource = getParameterByVariantY(size, paramVariant);
			mOutputCode.mParamDest = Parameter::RegisterParameter(getImmediateDX());
			mOutputCode.mWriteFlags = true;
		}

		void parseOpcode_divs(DataType::Size size, ParamVariant paramVariant)
		{
			parseOpcode_div(size, paramVariant, true);
		}

		void parseOpcode_divu(DataType::Size size, ParamVariant paramVariant)
		{
			parseOpcode_div(size, paramVariant, false);
		}


		// --- Add BCD

		void parseOpcode_abcd_rr(DataType::Size size)
		{
			mOutputCode.mType = CodeType::CODE_ADD_BCD;
			mOutputCode.mDataType.mSize = size;
			mOutputCode.mParamSource = Parameter::RegisterParameter(getImmediateDY());
			mOutputCode.mParamDest = Parameter::RegisterParameter(getImmediateDX());
			mOutputCode.mWriteFlags = true;
		}

		void parseOpcode_abcd_mm(DataType::Size size, ParamVariant paramVariantSource, ParamVariant paramVariantDest)
		{
			mOutputCode.mType = CodeType::CODE_ADD_BCD;
			mOutputCode.mDataType.mSize = size;
			mOutputCode.mParamSource = getParameterByVariantY(size, paramVariantSource);
			mOutputCode.mParamDest = getParameterByVariantX(size, paramVariantDest);
			mOutputCode.mWriteFlags = true;
		}


		// --- Shift

		void parseOpcode_shift_s(DataType::Size size, CodeType codeType, ShiftType shiftType)
		{
			const uint32 shift = (((getImmediate16(0) >> 9) - 1) & 7) + 1;
			mOutputCode.mType = codeType;
			mOutputCode.mShiftType = shiftType;
			mOutputCode.mDataType.mSize = size;
			mOutputCode.mParamSource = Parameter::ConstantParameter(shift, 0);
			mOutputCode.mParamDest = Parameter::RegisterParameter(getImmediateDY());
			mOutputCode.mWriteFlags = true;
		}

		void parseOpcode_shiftLeft_s(DataType::Size size, ShiftType shiftType)
		{
			parseOpcode_shift_s(size, CodeType::CODE_SHIFT_LEFT, shiftType);
		}

		void parseOpcode_shiftRight_s(DataType::Size size, ShiftType shiftType)
		{
			parseOpcode_shift_s(size, CodeType::CODE_SHIFT_RIGHT, shiftType);
		}

		void parseOpcode_shift_r(DataType::Size size, CodeType codeType, ShiftType shiftType)
		{
			mOutputCode.mType = codeType;
			mOutputCode.mShiftType = shiftType;
			mOutputCode.mDataType.mSize = size;
			mOutputCode.mParamSource = Parameter::RegisterParameter(getImmediateDX());
			mOutputCode.mParamDest = Parameter::RegisterParameter(getImmediateDY());
			mOutputCode.mWriteFlags = true;
		}

		void parseOpcode_shiftLeft_r(DataType::Size size, ShiftType shiftType)
		{
			parseOpcode_shift_r(size, CodeType::CODE_SHIFT_LEFT, shiftType);
		}

		void parseOpcode_shiftRight_r(DataType::Size size, ShiftType shiftType)
		{
			parseOpcode_shift_r(size, CodeType::CODE_SHIFT_RIGHT, shiftType);
		}

		void parseOpcode_shift_param(DataType::Size size, CodeType codeType, ShiftType shiftType, ParamVariant paramVariant)
		{
			mOutputCode.mType = codeType;
			mOutputCode.mShiftType = shiftType;
			mOutputCode.mDataType.mSize = size;
			mOutputCode.mParamSource = Parameter::ConstantParameter(1, 0);
			mOutputCode.mParamDest = getParameterByVariantY(size, paramVariant);
			mOutputCode.mWriteFlags = true;
			mOutputCode.mUseCycles = 0;
		}

		void parseOpcode_shiftLeft_param(DataType::Size size, ShiftType shiftType, ParamVariant paramVariant)
		{
			parseOpcode_shift_param(size, CodeType::CODE_SHIFT_LEFT, shiftType, paramVariant);
		}

		void parseOpcode_shiftRight_param(DataType::Size size, ShiftType shiftType, ParamVariant paramVariant)
		{
			parseOpcode_shift_param(size, CodeType::CODE_SHIFT_RIGHT, shiftType, paramVariant);
		}


		// --- Bitwise NOT

		void parseOpcode_not(DataType::Size size, ParamVariant paramVariant)
		{
			mOutputCode.mType = CodeType::CODE_NOT;
			mOutputCode.mDataType.mSize = size;
			mOutputCode.mParamDest = getParameterByVariantY(size, paramVariant);
			mOutputCode.mWriteFlags = true;
		}


		// --- Bitwise AND

		void parseOpcode_and_er(DataType::Size size, ParamVariant paramVariant)
		{
			parseOpcode_binaryoperation_er(CodeType::CODE_AND, size, paramVariant, true);
		}

		void parseOpcode_and_re(DataType::Size size, ParamVariant paramVariant)
		{
			parseOpcode_binaryoperation_re(CodeType::CODE_AND, size, paramVariant, true);
		}

		void parseOpcode_andi(DataType::Size size, ParamVariant paramVariant)
		{
			parseOpcode_binaryoperation_i(CodeType::CODE_AND, size, paramVariant, true);
		}

		void parseOpcode_andi_tos()
		{
			mOutputCode.mType = CodeType::CODE_SET_STATUS_AND;
			mOutputCode.mDataType.mSize = DataType::Size::SIZE_16;
			mOutputCode.mParamSource = getParameterByVariantY(DataType::DataType::Size::SIZE_16, ParamVariant::PARAM_i);
			mOutputCode.mWriteFlags = false;
		}


		// --- Bitwise OR

		void parseOpcode_or_er(DataType::Size size, ParamVariant paramVariant)
		{
			parseOpcode_binaryoperation_er(CodeType::CODE_OR, size, paramVariant, true);
		}

		void parseOpcode_or_re(DataType::Size size, ParamVariant paramVariant)
		{
			parseOpcode_binaryoperation_re(CodeType::CODE_OR, size, paramVariant, true);
		}

		void parseOpcode_ori(DataType::Size size, ParamVariant paramVariant)
		{
			parseOpcode_binaryoperation_i(CodeType::CODE_OR, size, paramVariant, true);
		}

		void parseOpcode_ori_tos()
		{
			mOutputCode.mType = CodeType::CODE_SET_STATUS_OR;
			mOutputCode.mDataType.mSize = DataType::Size::SIZE_16;
			mOutputCode.mParamSource = getParameterByVariantY(DataType::DataType::Size::SIZE_16, ParamVariant::PARAM_i);
			mOutputCode.mWriteFlags = false;
		}


		// --- Bitwise XOR

		void parseOpcode_eor(DataType::Size size, ParamVariant paramVariant)
		{
			parseOpcode_binaryoperation_re(CodeType::CODE_XOR, size, paramVariant, true);
		}

		void parseOpcode_eori(DataType::Size size, ParamVariant paramVariant)
		{
			parseOpcode_binaryoperation_i(CodeType::CODE_XOR, size, paramVariant, true);
		}


		// --- Set/clear all bits

		void parseOpcode_st(DataType::Size size, ParamVariant paramVariant)
		{
			// We just make this something like "x = 0xff"
			mOutputCode.mType = CodeType::CODE_MOVE;
			mOutputCode.mDataType.mSize = size;
			mOutputCode.mParamSource = Parameter::ConstantParameter(0xff, 0);
			mOutputCode.mParamDest = getParameterByVariantY(size, paramVariant);
			mOutputCode.mWriteFlags = false;
		}

		void parseOpcode_sf(DataType::Size size, ParamVariant paramVariant)
		{
			// We just make this something like "x = 0"
			mOutputCode.mType = CodeType::CODE_CLEAR;
			mOutputCode.mDataType.mSize = size;
			mOutputCode.mParamDest = getParameterByVariantY(size, paramVariant);
			mOutputCode.mWriteFlags = false;
		}

		void parseOpcode_sxx(DataType::Size size, ParamVariant paramVariant, Condition condition)
		{
			mOutputCode.mType = CodeType::CODE_SET_BY_CONDITION;
			mOutputCode.mDataType.mSize = size;
			mOutputCode.mParamDest = getParameterByVariantY(size, paramVariant);
			mOutputCode.mParamSource = Parameter::ConditionParameter(condition);
			mOutputCode.mWriteFlags = false;
			mOutputCode.mUseCycles = (paramVariant == ParamVariant::PARAM_d) ? 14 : 0;
		}


		// --- Shared for bit operations

		void parseOpcode_bitoperation_r(CodeType codeType, DataType::Size size, ParamVariant paramVariant)
		{
			mOutputCode.mType = codeType;
			mOutputCode.mDataType.mSize = size;
			mOutputCode.mParamSource = Parameter::RegisterParameter(getImmediateDX());
			mOutputCode.mParamDest = getParameterByVariantY(size, paramVariant);
			mOutputCode.mWriteFlags = true;
		}

		void parseOpcode_bitoperation_s(CodeType codeType, DataType::Size size, ParamVariant paramVariant)
		{
			mOutputCode.mType = codeType;
			mOutputCode.mDataType.mSize = size;
			mOutputCode.mParamSource = getParameterByVariantX(DataType::Size::SIZE_8, PARAM_i);
			mOutputCode.mParamDest = getParameterByVariantY(size, paramVariant);
			mOutputCode.mWriteFlags = true;
		}


		// --- Bit set

		void parseOpcode_bset_r(DataType::Size size, ParamVariant paramVariant)
		{
			parseOpcode_bitoperation_r(CodeType::CODE_SET_BIT, size, paramVariant);
		}

		void parseOpcode_bset_s(DataType::Size size, ParamVariant paramVariant)
		{
			parseOpcode_bitoperation_s(CodeType::CODE_SET_BIT, size, paramVariant);
		}


		// --- Bit clear

		void parseOpcode_bclr_r(DataType::Size size, ParamVariant paramVariant)
		{
			parseOpcode_bitoperation_r(CodeType::CODE_CLEAR_BIT, size, paramVariant);
		}

		void parseOpcode_bclr_s(DataType::Size size, ParamVariant paramVariant)
		{
			parseOpcode_bitoperation_s(CodeType::CODE_CLEAR_BIT, size, paramVariant);
		}


		// --- Bit change

		void parseOpcode_bchg_r(DataType::Size size, ParamVariant paramVariant)
		{
			parseOpcode_bitoperation_r(CodeType::CODE_CHANGE_BIT, size, paramVariant);
		}

		void parseOpcode_bchg_s(DataType::Size size, ParamVariant paramVariant)
		{
			parseOpcode_bitoperation_s(CodeType::CODE_CHANGE_BIT, size, paramVariant);
		}


		// --- Bit test

		void parseOpcode_btst_r(DataType::Size size, ParamVariant paramVariant)
		{
			parseOpcode_bitoperation_r(CodeType::CODE_TEST_BIT, size, paramVariant);
		}

		void parseOpcode_btst_s(DataType::Size size, ParamVariant paramVariant)
		{
			parseOpcode_bitoperation_s(CodeType::CODE_TEST_BIT, size, paramVariant);
		}


		// --- Signed extension

		void parseOpcode_ext(DataType::Size size)
		{
			mOutputCode.mType = CodeType::CODE_EXTEND_SIGNED;
			mOutputCode.mDataType.mSize = size;
			mOutputCode.mDataType.mSign = DataType::Sign::SIGNED;
			mOutputCode.mParamDest = Parameter::RegisterParameter(getImmediateDY());
			mOutputCode.mWriteFlags = true;
		}


		// --- Swap words

		void parseOpcode_swap(DataType::Size size)
		{
			mOutputCode.mType = CodeType::CODE_SWAP_WORDS;
			mOutputCode.mDataType.mSize = size;
			mOutputCode.mParamDest = Parameter::RegisterParameter(getImmediateDY());
			mOutputCode.mWriteFlags = true;
		}


		// --- Exchange

		void parseOpcode_exg(DataType::Size size, bool xIsData, bool yIsData)
		{
			mOutputCode.mType = CodeType::CODE_EXCHANGE;
			mOutputCode.mDataType.mSize = size;
			mOutputCode.mParamSource = Parameter::RegisterParameter(xIsData ? getImmediateDX() : getImmediateAX());
			mOutputCode.mParamDest = Parameter::RegisterParameter(yIsData ? getImmediateDY() : getImmediateAY());
			mOutputCode.mWriteFlags = false;
		}


		// --- Link / unlink

		void parseOpcode_link()
		{
			mOutputCode.mType = CodeType::CODE_LINK;
			mOutputCode.mDataType.mSize = DataType::Size::SIZE_32;
			mOutputCode.mParamSource = Parameter::ConstantParameter((int16)readImmediate16(), 2);
			mOutputCode.mParamDest = Parameter::RegisterParameter(getImmediateAY());
		}

		void parseOpcode_unlink()
		{
			mOutputCode.mType = CodeType::CODE_UNLINK;
			mOutputCode.mDataType.mSize = DataType::Size::SIZE_32;
			mOutputCode.mParamDest = Parameter::RegisterParameter(getImmediateAY());
		}


		// --- Jumps and calls
		
		void parseOpcode_jump_call(DataType::Size size, bool isCall)
		{
			mOutputCode.mType = isCall ? CodeType::CODE_CALL : CodeType::CODE_JUMP;
			mOutputCode.mDataType.mSize = size;
			if (size == DataType::Size::SIZE_16)
			{
				mOutputCode.mParamDest = Parameter::ConstantParameter(mAddress + 2 + (int16)readImmediate16(), 2);
			}
			else  // Both SIZE_8 and SIZE_32
			{
				mOutputCode.mParamDest = Parameter::ConstantParameter(mAddress + 2 + (int8)getImmediate8(0), 0);
			}
		}

		void parseOpcode_jump_call_32(ParamVariant paramVariant, bool isCall)
		{
			mOutputCode.mType = isCall ? CodeType::CODE_CALL : CodeType::CODE_JUMP;
			mOutputCode.mDataType.mSize = DataType::Size::SIZE_32;
			mOutputCode.mParamDest = getMemoryAddressParameterByVariantY(DataType::Size::SIZE_32, paramVariant);
		}

		void parseOpcode_conditional_jump(DataType::Size size, Condition condition, bool useLoopRegisterDy = false)
		{
			parseOpcode_jump_call(size, false);

			mOutputCode.mParamSource.mType = Parameter::Type::CONDITION;
			mOutputCode.mParamSource.mCondition.mCondition = condition;
			mOutputCode.mParamSource.mCondition.mLoopRegister = useLoopRegisterDy ? (ExtRegister)getImmediateDY() : ExtRegister::NONE;
		}


		// --- Return
		
		void parseOpcode_rte()
		{
			mOutputCode.mType = CodeType::CODE_RETURN_EXCEPTION;
		}

		void parseOpcode_rts()
		{
			mOutputCode.mType = CodeType::CODE_RETURN;
		}


		// --- Misc

		void parseOpcode_nop(uint32 incPC)
		{
			mOutputCode.mType = CodeType::CODE_NOP;
			mImmediatePosition += incPC;		// TODO: This won't get executed correctly
		}


	private:
		const uint32 mAddress;
		const uint8* mMemory;
		uint32 mImmediatePosition;
		AssemblyCode& mOutputCode;
	};

}
