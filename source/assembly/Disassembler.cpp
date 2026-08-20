/*
*	Lemonizer -- Turns 68K code into lemonscript
*	Copyright (C) 2017-2026 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#include "pch.h"
#include "assembly/Disassembler.h"
#include "assembly/CodeOutputHelper.h"
#include <iomanip>


namespace assembly
{

	namespace
	{

		// TODO: This is a duplicate
		std::string addressString(uint32 address)
		{
			std::stringstream buffer;
			buffer << std::hex << std::setfill('0') << std::setw(6) << address;
			return buffer.str();
		}

		void addressString(uint32 address, std::stringstream& buffer)
		{
			buffer << std::hex << std::setfill('0') << std::setw(6) << address;
		}

		std::string outputInstructionWithSize(const std::string& base, DataType::Size size)
		{
			return base + CodeOutputHelper::getSizePostfix(size);
		}

		std::string outputInstructionWithCondition(const std::string& base, const Parameter& param)
		{
			if (param.mType == Parameter::Type::CONDITION)
			{
				std::string addition;
				if (param.mCondition.mLoopRegister != ExtRegister::NONE)
				{
					addition = "<loop:" + CodeOutputHelper::getRegisterName(param.mCondition.mLoopRegister) + ">";
				}

				if (param.mCondition.mCondition == Condition::NONE)
				{
					return base + addition;
				}
				else
				{
					return base + '<' + CodeOutputHelper::getConditionShortname(param.mCondition.mCondition) + '>' + addition;
				}
			}
			return base;
		}

		std::string getParameterAsString(const Parameter& param)
		{
			std::stringstream str;
			if (param.mIsMemory)
				str << '[';

			switch (param.mType)
			{
				case Parameter::Type::CONSTANT:
				{
					str << std::hex << "0x" << param.mConstant.mValue;
					break;
				}

				case Parameter::Type::REGISTER:
				{
					if (param.mRegister.mPreDecrement)
						str << "--";
					str << CodeOutputHelper::getRegisterName(param.mRegister.mRegister);
					if (param.mRegister.mPostIncrement)
						str << "++";
					break;
				}

				case Parameter::Type::COMBINED:
				{
					// TODO: What about "mSizeOfRegister1"?
					str << CodeOutputHelper::getRegisterName(param.mCombined.mRegister1);

					if (param.mCombined.mRegister2 != assembly::ExtRegister::NONE)
						str << " + " << CodeOutputHelper::getRegisterName(param.mCombined.mRegister2);

					if (param.mCombined.mDisplacement > 0)
						str << " + " << std::hex << "0x" << param.mCombined.mDisplacement;
					else if (param.mCombined.mDisplacement < 0)
						str << " - " << std::hex << "0x" << (-param.mCombined.mDisplacement);
					break;
				}

				default:
				{
					str << "...";
					break;
				}
			}

			if (param.mIsMemory)
				str << ']';
			return str.str();
		}

		std::string outputParameter(const Parameter& param)
		{
			return getParameterAsString(param);
		}

		std::string outputRegisterListParameter(const Parameter& param)
		{
			if (param.mType == Parameter::Type::CONSTANT)
			{
				const uint32 registerList = param.mConstant.mValue;
				const bool reverseTraversal = (registerList & 0x80000000) != 0;
				std::string output = "{";
				for (uint32 i = 0; i < 16; ++i)
				{
					if ((registerList >> i) & 0x01)
					{
						if (output.length() > 1)
							output += ",";
						output += CodeOutputHelper::getRegisterName((Register)(reverseTraversal ? (15-i) : i));
					}
				}
				output += "}";
				return output;
			}
			return "";
		}
	}


	bool Disassembler::getStringRepresentation(const AssemblyCode& code, std::string& outInstruction, std::string& outParam1, std::string& outParam2)
	{
		switch (code.mType)
		{
			case assembly::CodeType::CODE_NOP:
			{
				outInstruction = "nop";
				break;
			}

			case assembly::CodeType::CODE_MOVE:
			{
				outInstruction = outputInstructionWithSize("move", code.mDataType.mSize);
				outParam1 = outputParameter(code.mParamDest);
				outParam2 = outputParameter(code.mParamSource);
				break;
			}

			case assembly::CodeType::CODE_LOAD_REGISTERS:
			{
				outInstruction = outputInstructionWithSize("movem", code.mDataType.mSize);
				outParam1 = outputRegisterListParameter(code.mParamDest);
				outParam2 = outputParameter(code.mParamSource);
				break;
			}

			case assembly::CodeType::CODE_SAVE_REGISTERS:
			{
				outInstruction = outputInstructionWithSize("movem", code.mDataType.mSize);
				outParam1 = outputParameter(code.mParamDest);
				outParam2 = outputRegisterListParameter(code.mParamSource);
				break;
			}

			case assembly::CodeType::CODE_SET_FLAGS:
			{
				outInstruction = "move_toc";
				outParam1 = outputParameter(code.mParamSource);
				break;
			}

			case assembly::CodeType::CODE_GET_STATUS:
			{
				outInstruction = "move_frs";
				outParam1 = outputParameter(code.mParamDest);
				break;
			}

			case assembly::CodeType::CODE_SET_STATUS:
			{
				outInstruction = "move_tos";
				outParam1 = outputParameter(code.mParamSource);
				break;
			}

			case assembly::CodeType::CODE_SET_STATUS_AND:
			{
				outInstruction = "andi_tos";
				outParam1 = outputParameter(code.mParamSource);
				break;
			}

			case assembly::CodeType::CODE_SET_STATUS_OR:
			{
				outInstruction = "ori_tos";
				outParam1 = outputParameter(code.mParamSource);
				break;
			}

			case assembly::CodeType::CODE_GET_USP:
			{
				outInstruction = "move_fru";
				outParam1 = outputParameter(code.mParamDest);
				break;
			}

			case assembly::CodeType::CODE_SET_USP:
			{
				outInstruction = "move_tou";
				outParam1 = outputParameter(code.mParamSource);
				break;
			}

			case assembly::CodeType::CODE_LOAD_EA:
			{
				outInstruction = "lea";   // It's always 32-bit
				outParam1 = outputParameter(code.mParamDest);
				outParam2 = outputParameter(code.mParamSource);
				break;
			}

			case assembly::CodeType::CODE_PUSH_EA:
			{
				outInstruction = "pea";   // It's always 32-bit
				outParam1 = outputParameter(code.mParamDest);
				outParam2 = outputParameter(code.mParamSource);
				break;
			}

			case assembly::CodeType::CODE_TEST:
			{
				outInstruction = outputInstructionWithSize("test", code.mDataType.mSize);
				outParam1 = outputParameter(code.mParamDest);
				break;
			}

			case assembly::CodeType::CODE_CLEAR:
			{
				outInstruction = outputInstructionWithSize("clr", code.mDataType.mSize);
				outParam1 = outputParameter(code.mParamDest);
				break;
			}

			case assembly::CodeType::CODE_ADD:
			{
				outInstruction = outputInstructionWithSize("add", code.mDataType.mSize);
				outParam1 = outputParameter(code.mParamDest);
				outParam2 = outputParameter(code.mParamSource);
				break;
			}

			case assembly::CodeType::CODE_SUB:
			{
				outInstruction = outputInstructionWithSize("sub", code.mDataType.mSize);
				outParam1 = outputParameter(code.mParamDest);
				outParam2 = outputParameter(code.mParamSource);
				break;
			}

			case assembly::CodeType::CODE_CMP:
			{
				outInstruction = outputInstructionWithSize("cmp", code.mDataType.mSize);
				outParam1 = outputParameter(code.mParamDest);
				outParam2 = outputParameter(code.mParamSource);
				break;
			}

			case assembly::CodeType::CODE_NEG:
			{
				outInstruction = outputInstructionWithSize("neg", code.mDataType.mSize);
				outParam1 = outputParameter(code.mParamDest);
				break;
			}

			case assembly::CodeType::CODE_MUL:
			{
				outInstruction = outputInstructionWithSize(code.mDataType.isSigned() ? "muls" : "mulu", code.mDataType.mSize);
				outParam1 = outputParameter(code.mParamDest);
				outParam2 = outputParameter(code.mParamSource);
				break;
			}

			case assembly::CodeType::CODE_DIV:
			{
				outInstruction = outputInstructionWithSize(code.mDataType.isSigned() ? "divs" : "divu", code.mDataType.mSize);
				outParam1 = outputParameter(code.mParamDest);
				outParam2 = outputParameter(code.mParamSource);
				break;
			}

			case assembly::CodeType::CODE_ADD_BCD:
			{
				outInstruction = outputInstructionWithSize("abcd", code.mDataType.mSize);
				outParam1 = outputParameter(code.mParamDest);
				outParam2 = outputParameter(code.mParamSource);
				break;
			}

			case assembly::CodeType::CODE_SHIFT_LEFT:
			{
				switch (code.mShiftType)
				{
					case ShiftType::ARITHMETIC: outInstruction = outputInstructionWithSize("asl", code.mDataType.mSize);  break;
					case ShiftType::LOGICAL:	outInstruction = outputInstructionWithSize("lsl", code.mDataType.mSize);  break;
					case ShiftType::ROTATE:		outInstruction = outputInstructionWithSize("rol", code.mDataType.mSize);  break;
					case ShiftType::ROTATE_X:	outInstruction = outputInstructionWithSize("roxl", code.mDataType.mSize);  break;
				}
				outParam1 = outputParameter(code.mParamDest);
				outParam2 = outputParameter(code.mParamSource);
				break;
			}

			case assembly::CodeType::CODE_SHIFT_RIGHT:
			{
				switch (code.mShiftType)
				{
					case ShiftType::ARITHMETIC: outInstruction = outputInstructionWithSize("asr", code.mDataType.mSize);  break;
					case ShiftType::LOGICAL:	outInstruction = outputInstructionWithSize("lsr", code.mDataType.mSize);  break;
					case ShiftType::ROTATE:		outInstruction = outputInstructionWithSize("ror", code.mDataType.mSize);  break;
					case ShiftType::ROTATE_X:	outInstruction = outputInstructionWithSize("roxr", code.mDataType.mSize);  break;
				}
				outParam1 = outputParameter(code.mParamDest);
				outParam2 = outputParameter(code.mParamSource);
				break;
			}

			case assembly::CodeType::CODE_NOT:
			{
				outInstruction = outputInstructionWithSize("not", code.mDataType.mSize);
				outParam1 = outputParameter(code.mParamDest);
				break;
			}

			case assembly::CodeType::CODE_AND:
			{
				outInstruction = outputInstructionWithSize("and", code.mDataType.mSize);
				outParam1 = outputParameter(code.mParamDest);
				outParam2 = outputParameter(code.mParamSource);
				break;
			}

			case assembly::CodeType::CODE_OR:
			{
				outInstruction = outputInstructionWithSize("or", code.mDataType.mSize);
				outParam1 = outputParameter(code.mParamDest);
				outParam2 = outputParameter(code.mParamSource);
				break;
			}

			case assembly::CodeType::CODE_XOR:
			{
				outInstruction = outputInstructionWithSize("xor", code.mDataType.mSize);
				outParam1 = outputParameter(code.mParamDest);
				outParam2 = outputParameter(code.mParamSource);
				break;
			}

			case assembly::CodeType::CODE_SET_BY_CONDITION:
			{
				outInstruction = outputInstructionWithCondition("set.8", code.mParamSource);
				outParam1 = outputParameter(code.mParamDest);
				break;
			}

			case assembly::CodeType::CODE_SET_BIT:
			{
				outInstruction = outputInstructionWithSize("bset", code.mDataType.mSize);
				outParam1 = outputParameter(code.mParamDest);
				outParam2 = outputParameter(code.mParamSource);
				break;
			}

			case assembly::CodeType::CODE_CLEAR_BIT:
			{
				outInstruction = outputInstructionWithSize("bclr", code.mDataType.mSize);
				outParam1 = outputParameter(code.mParamDest);
				outParam2 = outputParameter(code.mParamSource);
				break;
			}

			case assembly::CodeType::CODE_CHANGE_BIT:
			{
				outInstruction = outputInstructionWithSize("bchg", code.mDataType.mSize);
				outParam1 = outputParameter(code.mParamDest);
				outParam2 = outputParameter(code.mParamSource);
				break;
			}

			case assembly::CodeType::CODE_TEST_BIT:
			{
				outInstruction = outputInstructionWithSize("btst", code.mDataType.mSize);
				outParam1 = outputParameter(code.mParamDest);
				outParam2 = outputParameter(code.mParamSource);
				break;
			}

			case assembly::CodeType::CODE_EXTEND_SIGNED:
			{
				outInstruction = outputInstructionWithSize("ext", code.mDataType.mSize);
				outParam1 = outputParameter(code.mParamDest);
				break;
			}

			case assembly::CodeType::CODE_SWAP_WORDS:
			{
				outInstruction = "swap.32";
				outParam1 = outputParameter(code.mParamDest);
				break;
			}

			case assembly::CodeType::CODE_EXCHANGE:
			{
				outInstruction = "exg.32";
				outParam1 = outputParameter(code.mParamDest);
				outParam2 = outputParameter(code.mParamSource);
				break;
			}

			case assembly::CodeType::CODE_LINK:
			{
				outInstruction = "link";
				outParam1 = outputParameter(code.mParamDest);
				outParam2 = outputParameter(code.mParamSource);
				break;
			}

			case assembly::CodeType::CODE_UNLINK:
			{
				outInstruction = "unlk";
				outParam1 = outputParameter(code.mParamDest);
				break;
			}

			case assembly::CodeType::CODE_CALL:
			{
				outInstruction = outputInstructionWithCondition("call", code.mParamSource);
				if (code.mParamDest.mType == Parameter::Type::CONSTANT)
				{
					outParam1 = "@" + addressString(code.mParamDest.mConstant.mValue);
				}
				else
				{
					outParam1 = outputParameter(code.mParamDest);
				}
				break;
			}

			case assembly::CodeType::CODE_JUMP:
			{
				outInstruction = outputInstructionWithCondition("jmp", code.mParamSource);
				if (code.mParamDest.mType == Parameter::Type::CONSTANT)
				{
					outParam1 = "@" + addressString(code.mParamDest.mConstant.mValue);
				}
				else
				{
					outParam1 = outputParameter(code.mParamDest);
				}
				break;
			}

			case assembly::CodeType::CODE_RETURN:
			{
				outInstruction = "ret";
				break;
			}

			case assembly::CodeType::CODE_RETURN_EXCEPTION:
			{
				outInstruction = "rte";
				break;
			}

			default:
				return false;
		}

		return true;
	}

}
