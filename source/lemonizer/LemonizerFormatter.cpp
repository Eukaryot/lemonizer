/*
*	Lemonizer -- turns 68K code into lemon script
*	Copyright (C) 2021 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#include "pch.h"
#include "lemonizer/LemonizerFormatter.h"
#include "lemonizer/LemonizerCode.h"
#include "assembly/CodeOutputHelper.h"
#include <iomanip>


namespace lemonizer
{
	namespace
	{

		struct FormattingOptions
		{
			assembly::DataType mDataType;
			bool mDecimal = false;

			inline FormattingOptions(const assembly::DataType& dataType = assembly::DataType(), bool decimal = false) : mDataType(dataType), mDecimal(decimal) {}
		};


		std::string hexString(uint32 value, const char* prefix = "0x")
		{
			// This variant always uses an even number of digits
			std::stringstream str;
			str << prefix;
			if (value < 0x100)
				str << std::setw(2);
			else if (value < 0x10000)
				str << std::setw(4);
			else if (value < 0x1000000)
				str << std::setw(6);
			else
				str << std::setw(8);
			str << std::setfill('0') << std::hex << value;
			return str.str();
		}

		std::string hexString(uint32 value, size_t digits, const char* prefix = "0x")
		{
			std::stringstream str;
			str << prefix << std::hex << std::setfill('0') << std::setw(digits) << value;
			return str.str();
		}

		std::string hexString(uint32 value, const assembly::DataType& dataType, const char* prefix = "0x")
		{
			return hexString(value, dataType.getSizeInBytes() * 2, prefix);
		}

		bool isNegativeConstant(uint32 value, const assembly::DataType& dataType, int32& output)
		{
			if (dataType.isSigned())
			{
				switch (dataType.mSize)
				{
					case assembly::DataType::Size::SIZE_8:   output = (int8)value;   break;
					case assembly::DataType::Size::SIZE_16:  output = (int16)value;  break;
					case assembly::DataType::Size::SIZE_32:  output = (int32)value;  break;
				}
				return (output < 0);
			}
			return false;
		}

		bool isParameterConstantValue(const assembly::Parameter& param, int32 value)
		{
			return (param.isConstantValue() && param.mConstant.mValue == value);
		}

		std::string getRegisterName(assembly::ExtRegister reg, const assembly::DataType& dataType)
		{
			std::string result = assembly::CodeOutputHelper::getRegisterName(reg);
			if (dataType.isUnspecified())
				return result;
			if (dataType.mSize == assembly::DataType::Size::SIZE_32 && !dataType.isSigned())
				return result;
			result += "." + dataType.toString();
			return result;
		}

		std::string getRegisterName(assembly::Register reg, const assembly::DataType& dataType)
		{
			return getRegisterName((assembly::ExtRegister)reg, dataType);
		}

		std::string getParameterAsStringIgnoreMemory(const assembly::Parameter& param, const FormattingOptions& formattingOptions)
		{
			std::stringstream str;
			switch (param.mType)
			{
				case assembly::Parameter::Type::CONSTANT:
				{
					const uint32 value = param.mConstant.mValue;
					if (formattingOptions.mDecimal)
					{
						str << std::dec << value;
					}
					else
					{
						int32 signedValue;
						if (isNegativeConstant(value, formattingOptions.mDataType, signedValue))
							str << "-" << hexString(-signedValue);
						else
							str << hexString(value);
					}
					break;
				}

				case assembly::Parameter::Type::REGISTER:
				{
					std::string registerName = getRegisterName(param.mRegister.mRegister, formattingOptions.mDataType);
					if (param.mRegister.mPreDecrement)
						str << registerName << "-=" << param.mRegister.mPreDecrement;
					else if (param.mRegister.mPostIncrement)
						str << "(" << registerName << "+=" << param.mRegister.mPostIncrement << ")-" << param.mRegister.mPostIncrement;
					else
						str << registerName;
					break;
				}

				case assembly::Parameter::Type::COMBINED:
				{
					const assembly::DataType register1Type(param.mCombined.mSizeOfRegister1, (param.mCombined.mSizeOfRegister1 < assembly::DataType::Size::SIZE_32) ? assembly::DataType::Sign::SIGNED : assembly::DataType::Sign::UNSIGNED);
					const std::string register1Name = getRegisterName(param.mCombined.mRegister1, register1Type);

					if (param.mCombined.mRegister2 == assembly::ExtRegister::NONE)
					{
						if (register1Type.mSize == assembly::DataType::Size::SIZE_16 && param.mCombined.mDisplacement >= 0x1000)
						{
							// Special handling for cases like "D0.s16 + 0x012345" --> reverse order
							str << hexString(param.mCombined.mDisplacement, 6) + " + " + register1Name;
							break;
						}
						str << register1Name;
					}
					else
						str << getRegisterName(param.mCombined.mRegister2, formattingOptions.mDataType) << " + " << register1Name;

					if (param.mCombined.mDisplacement > 0)
						str << " + " + hexString(param.mCombined.mDisplacement);
					else if (param.mCombined.mDisplacement < 0)
						str << " - " + hexString(-param.mCombined.mDisplacement);
					break;
				}

				default:
				{
					str << "...";
					break;
				}
			}
			return str.str();
		}

		std::string getParameterAsString(const assembly::Parameter& param, FormattingOptions& formattingOptions)
		{
			if (param.mIsMemory)
			{
				return formattingOptions.mDataType.toString() + '[' + getParameterAsStringIgnoreMemory(param, FormattingOptions()) + ']';
			}
			else
			{
				return getParameterAsStringIgnoreMemory(param, formattingOptions);
			}
		}

		std::string getParameterAsString(const assembly::Parameter& param, const assembly::DataType& dataType)
		{
			FormattingOptions formattingOptions(dataType);
			return getParameterAsString(param, formattingOptions);
		}

		std::string getParameterAsBitValue(const assembly::Parameter& param, FormattingOptions& formattingOptions)
		{
			if (!param.mIsMemory && param.mType == assembly::Parameter::Type::CONSTANT)
			{
				return hexString(1 << param.mConstant.mValue, formattingOptions.mDataType);
			}
			else
			{
				return "(" + formattingOptions.mDataType.toString() + "(1) << " + getParameterAsString(param, formattingOptions) + ")";
			}
		}

		std::string getParameterAsBitValue(const assembly::Parameter& param, const assembly::DataType& dataType)
		{
			FormattingOptions formattingOptions(dataType);
			return getParameterAsBitValue(param, formattingOptions);
		}

		const std::string* getConditionStringAndTypeSign(assembly::Condition condition, assembly::DataType::Sign& typeSign, bool comparingToZero)
		{
			enum OperatorSign
			{
				INVALID,
				UNSIGNED,
				SIGNED,
				BOTH,
				NEGATIVE
			};
			static const OperatorSign operatorSigns[] =
			{
				UNSIGNED,	// CS == "below.u"
				UNSIGNED,	// CC == "aboveEqual.u"
				INVALID,	// VS == "overflow"
				INVALID,	// VC == "!overflow"
				BOTH,		// NE == "!equal"
				BOTH,		// EQ == "equal"
				NEGATIVE,	// MI == "negative"
				NEGATIVE,	// PL == "!negative"
				SIGNED,		// LT == "less.s"
				SIGNED,		// GE == "greaterEqual.s"
				UNSIGNED,	// HI == "above.u"
				UNSIGNED,	// LS == "belowEqual.u"
				SIGNED,		// GT == "greater.s"
				SIGNED,		// LE == "lessEqual.s"
				INVALID		// NONE
			};

			static const std::string operatorString[] = { " < ", " >= ", "", "", " != ", " == ", " < ", " >= ", " < ", " >= ", " > ", " <= ", " > ", " <= ", "" };
			switch (operatorSigns[(size_t)condition])
			{
				case UNSIGNED:
					typeSign = assembly::DataType::Sign::UNSIGNED;
					return &operatorString[(size_t)condition];

				case SIGNED:
					typeSign = assembly::DataType::Sign::SIGNED;
					return &operatorString[(size_t)condition];

				case BOTH:
					typeSign = assembly::DataType::Sign::UNSPECIFIED;
					return &operatorString[(size_t)condition];

				case NEGATIVE:
					// When comparing to zero, the value has to be interpreted as signed
					typeSign = comparingToZero ? assembly::DataType::Sign::SIGNED : assembly::DataType::Sign::UNSIGNED;
					return &operatorString[(size_t)condition];
			}
			return nullptr;
		}

		std::string getConditionString(const assembly::AssemblyCode& ac, assembly::Condition condition)
		{
			switch (ac.mType)
			{
				case assembly::CodeType::CODE_CMP:
				{
					assembly::DataType dataType = ac.mDataType;
					const std::string* operatorString = getConditionStringAndTypeSign(condition, dataType.mSign, false);
					if (nullptr != operatorString)
					{
						std::string text;
						text += getParameterAsString(ac.mParamDest, dataType);
						text += *operatorString;
						text += getParameterAsString(ac.mParamSource, dataType);
						return text;
					}
					break;
				}

				case assembly::CodeType::CODE_TEST:
				case assembly::CodeType::CODE_MOVE:
				case assembly::CodeType::CODE_ADD:
				case assembly::CodeType::CODE_SUB:
				case assembly::CodeType::CODE_AND:
				case assembly::CodeType::CODE_OR:
				case assembly::CodeType::CODE_XOR:
				case assembly::CodeType::CODE_SHIFT_LEFT:
				case assembly::CodeType::CODE_SHIFT_RIGHT:
				case assembly::CodeType::CODE_EXTEND_SIGNED:
				{
					// Currently only supported are checks that can be done with the result only
					//  -> Another use-case is carry flag checks (which get wrong output as "aboveEqual.u", "below.u"), but these can't easily be handled
					switch (condition)
					{
						case assembly::Condition::EQ:
						case assembly::Condition::NE:
						case assembly::Condition::MI:
						case assembly::Condition::PL:
						case assembly::Condition::LT:
						case assembly::Condition::GE:
						case assembly::Condition::HI:
						case assembly::Condition::LS:
						case assembly::Condition::GT:
						case assembly::Condition::LE:
						{
							assembly::DataType dataType = ac.mDataType;
							const std::string* operatorString = getConditionStringAndTypeSign(condition, dataType.mSign, true);
							if (nullptr != operatorString)
							{
								std::string text;
								text += getParameterAsString(ac.mParamDest, dataType);
								text += *operatorString + "0";
								return text;
							}
						}

						case assembly::Condition::CC:  return "!carryFlag()";
						case assembly::Condition::CS:  return "carryFlag()";
					}
					break;
				}

				case assembly::CodeType::CODE_TEST_BIT:
				{
					assembly::DataType dataType = ac.mDataType;
					const std::string dst = getParameterAsString(ac.mParamDest, dataType);
					const std::string src = getParameterAsBitValue(ac.mParamSource, dataType);

					if (condition == assembly::Condition::EQ)
					{
						return "(" + dst + " & " + src + ") == 0";
					}
					else if (condition == assembly::Condition::NE)
					{
						return dst + " & " + src;
					}
					break;
				}
			}

			// Fallback
			return assembly::CodeOutputHelper::getConditionLongname(condition) + "()";
		}

		const std::string& getLemonDataTypeString(const lemon::DataTypeDefinition* dataType)
		{
			static const std::string typeString_u8  = "u8";
			static const std::string typeString_u16 = "u16";
			static const std::string typeString_u32 = "u32";
			static const std::string typeString_s8  = "s8";
			static const std::string typeString_s16 = "s16";
			static const std::string typeString_s32 = "s32";
			static const std::string typeString_invalid = "<invalid>";

			if (dataType->mClass == lemon::DataTypeDefinition::Class::INTEGER)
			{
				const lemon::IntegerDataType& integerType = dataType->as<lemon::IntegerDataType>();
				switch (integerType.mBytes)
				{
					case 1:  return integerType.mIsSigned ? typeString_s8 : typeString_u8;
					case 2:  return integerType.mIsSigned ? typeString_s16 : typeString_u16;
					case 4:  return integerType.mIsSigned ? typeString_s32 : typeString_u32;
				}
			}
			return typeString_invalid;
		}

	}


	void Formatter::formatCode(const Code& code, uint32 address, Output& output)
	{
		switch (code.getType())
		{
			case Code::IFELSE:
			{
				const CodeIfElse& ci = code.as<CodeIfElse>();
				std::string text = "if (";

				// Add main condition
				if (ci.mCondition != assembly::Condition::NONE)
				{
					const assembly::Condition condition = ci.mNegateWholeCondition ? assembly::negateCondition(ci.mCondition) : ci.mCondition;
					if (nullptr == ci.mAssemblyCode)
					{
						text += assembly::CodeOutputHelper::getConditionLongname(condition) + "()";
					}
					else
					{
						text += getConditionString(*ci.mAssemblyCode, condition);
					}
				}

				// Add loop condition
				if (ci.mLoopRegister != assembly::ExtRegister::NONE)
				{
					if (ci.mCondition != assembly::Condition::NONE)
					{
						text += ci.mNegateWholeCondition ? " || " : " && ";
					}

					const std::string registerName = getRegisterName(ci.mLoopRegister, assembly::DataType(assembly::DataType::Size::SIZE_16, assembly::DataType::Sign::SIGNED));
					output.addToken("--" + registerName);
					output.newLine();
					text += registerName + (ci.mNegateWholeCondition ? " < 0" : " >= 0");
				}

				text += ")";
				output.addToken(text);
				break;
			}

			case Code::WHILE:
			{
				const CodeWhile& cw = code.as<CodeWhile>();
				output.addToken("while (true)");
				break;
			}

			case Code::JUMP_OR_CALL:
			{
				const CodeJumpOrCall& cj = code.as<CodeJumpOrCall>();
				const JumpCallFormatting jcf = (cj.mIsCall) ? JumpCallFormatting::CALL :
											   (cj.mLines.front()->mLeadsToLabel) ? JumpCallFormatting::JUMP : JumpCallFormatting::CALL_RETURN;
				formatJumpCall(jcf, cj.mDestinationAddress, output);
				break;
			}

			case Code::BREAK_OR_CONTINUE:
			{
				const CodeBreakOrContinue& cboc = code.as<CodeBreakOrContinue>();
				output.addToken(cboc.mIsContinue ? "continue" : "break");
				break;
			}

			case Code::ASSEMBLY:
			{
				const CodeAssembly& ca = code.as<CodeAssembly>();
				if (!formatAssemblyCode(*ca.mAssemblyCode, output))
				{
					// Fallback to disassembly
					assembly::Formatter::formatCode(*ca.mAssemblyCode, address, output);
				}
				break;
			}

			case Code::LEMONTOKENTREE:
			{
				const CodeLemonTokenTree& cltt = code.as<CodeLemonTokenTree>();
				RMX_CHECK(cltt.mRoot.valid(), "Lemon token tree root is empty", RMX_REACT_THROW);
				formatLemonTokenTreeNode(*cltt.mRoot.get(), output);
				break;
			}
		}
	}

	bool Formatter::formatAssemblyCode(const assembly::AssemblyCode& code, Output& output)
	{
		assembly::DataType dataType(code.mDataType.mSize);

		switch (code.mType)
		{
			case assembly::CodeType::CODE_MOVE:
			{
				assembly::DataType dstType = dataType;
				if (code.mDataType.isSigned())
				{
					// Signed is always s32(...) = s16(...)
					dstType = assembly::DataType(assembly::DataType::Size::SIZE_32, assembly::DataType::Sign::SIGNED);
					if (code.mParamDest.mType == assembly::Parameter::Type::REGISTER)
					{
						// Special handling for assignments to address registers
						//  -> Use the more obvious output as e.g. "A0 = 0xffff0000 + D0.u16"
						if (code.mParamDest.mRegister.mRegister >= assembly::Register::A0 && code.mParamDest.mRegister.mRegister <= assembly::Register::A7)
						{
							dstType.mSign = assembly::DataType::Sign::UNSPECIFIED;
							const std::string dst = getParameterAsString(code.mParamDest, dstType);
							const std::string src = getParameterAsString(code.mParamSource, assembly::DataType::u16);
							output.addToken(dst + " = 0xffff0000 + " + src);
							return true;
						}
					}
				}

				const std::string dst = getParameterAsString(code.mParamDest, dstType);
				if (isParameterConstantValue(code.mParamSource, 0))
				{
					output.addToken(dst + " = 0");
				}
				else
				{
					assembly::DataType srcType = code.mDataType.isSigned() ? assembly::DataType::s16 : dataType;
					const std::string src = getParameterAsString(code.mParamSource, srcType);
					output.addToken(dst + " = " + src);
				}
				return true;
			}

			case assembly::CodeType::CODE_MOVE_P:
			{
				const std::string dst = getParameterAsString(code.mParamDest, dataType);
				const std::string src = getParameterAsString(code.mParamSource, dataType);
				output.addToken("movep(" + dst + " = " + src + ")");
				return true;
			}

			case assembly::CodeType::CODE_LOAD_REGISTERS:
			{
				if (code.mParamDest.mType != assembly::Parameter::Type::CONSTANT)
					return false;

				const std::vector<assembly::Register> registers = assembly::CodeOutputHelper::getRegisterListFromBitmask(code.mParamDest.mConstant.mValue);
				if (code.mParamSource.mType == assembly::Parameter::Type::REGISTER)
				{
					for (size_t i = 0; i < registers.size(); ++i)
					{
						if (i > 0)
							output.newLine();

						if (code.mParamSource.mRegister.mRegister == assembly::Register::A7)
						{
							const std::string dst = getRegisterName(registers[i], assembly::DataType::Size::SIZE_32);
							if (code.mDataType.mSize == assembly::DataType::Size::SIZE_16)
								output.addToken(dst + " = s16(pop())");
							else
								output.addToken(dst + " = pop()");
						}
						else
						{
							const std::string dst = getRegisterName(registers[i], dataType);
							if (code.mParamSource.mRegister.mPostIncrement == 0)
							{
								const std::string srcRegName = getRegisterName(code.mParamSource.mRegister.mRegister, assembly::DataType::u32);
								const std::string addition = (i == 0) ? "" : (" + " + hexString((int)i * dataType.getSizeInBytes(), 2));
								output.addToken(dst + " = " + code.mDataType.toString() + "[" + srcRegName + addition + "]");
							}
							else
							{
								output.addToken(dst + " = " + getParameterAsString(code.mParamSource, dataType));
							}
						}
					}
				}
				else if (code.mParamSource.mType == assembly::Parameter::Type::COMBINED)
				{
					const std::string srcRegName = getRegisterName(code.mParamSource.mCombined.mRegister1, assembly::DataType::u32);
					for (size_t i = 0; i < registers.size(); ++i)
					{
						if (i > 0)
							output.newLine();

						const int offset = code.mParamSource.mCombined.mDisplacement + (int)i * dataType.getSizeInBytes();
						const std::string dst = getRegisterName(registers[i], dataType);
						const std::string addition = (offset == 0) ? "" : (offset < 0) ? (" - " + hexString(-offset, 2)) : (" + " + hexString(offset, 2));
						output.addToken(dst + " = " + code.mDataType.toString() + "[" + srcRegName + addition + "]");
					}
				}
				else
				{
					return false;
				}
				return true;
			}

			case assembly::CodeType::CODE_SAVE_REGISTERS:
			{
				if (code.mParamSource.mType != assembly::Parameter::Type::CONSTANT)
					return false;

				const std::vector<assembly::Register> registers = assembly::CodeOutputHelper::getRegisterListFromBitmask(code.mParamSource.mConstant.mValue);
				if (code.mParamDest.mType == assembly::Parameter::Type::REGISTER)
				{
					for (size_t i = 0; i < registers.size(); ++i)
					{
						if (i > 0)
							output.newLine();

						const std::string src = getRegisterName(registers[i], dataType);
						if (code.mParamDest.mRegister.mRegister == assembly::Register::A7)
						{
							output.addToken("push(" + src + ")");
						}
						else
						{
							output.addToken(getParameterAsString(code.mParamDest, dataType) + " = " + src);
						}
					}
				}
				else if (code.mParamDest.mType == assembly::Parameter::Type::COMBINED)
				{
					const std::string srcRegName = getRegisterName(code.mParamDest.mCombined.mRegister1, assembly::DataType::u32);
					for (size_t i = 0; i < registers.size(); ++i)
					{
						if (i > 0)
							output.newLine();

						const int offset = code.mParamDest.mCombined.mDisplacement + (int)i * dataType.getSizeInBytes();
						const std::string src = getRegisterName(registers[i], dataType);
						const std::string addition = (offset == 0) ? "" : (offset < 0) ? (" - " + hexString(-offset, 2)) : (" + " + hexString(offset, 2));
						output.addToken(code.mDataType.toString() + "[" + srcRegName + addition + "] = " + src);
					}
				}
				else
				{
					return false;
				}
				return true;
			}

			case assembly::CodeType::CODE_GET_STATUS:
			{
				const std::string dst = getParameterAsString(code.mParamDest, dataType);
				output.addToken(dst + " = get_status_register()");
				return true;
			}

			case assembly::CodeType::CODE_SET_STATUS:
			{
				const std::string src = getParameterAsString(code.mParamSource, dataType);
				output.addToken("set_status_register(" + src + ")");
				return true;
			}

			case assembly::CodeType::CODE_SET_STATUS_AND:
			{
				const std::string src = getParameterAsString(code.mParamSource, dataType);
				output.addToken("set_status_register(get_status_register() & " + src + ")");
				return true;
			}

			case assembly::CodeType::CODE_SET_STATUS_OR:
			{
				const std::string src = getParameterAsString(code.mParamSource, dataType);
				output.addToken("set_status_register(get_status_register() | " + src + ")");
				return true;
			}

			case assembly::CodeType::CODE_LOAD_EA:
			{
				const std::string dst = getRegisterName(code.mParamDest.mRegister.mRegister, assembly::DataType());
				const std::string src = getParameterAsStringIgnoreMemory(code.mParamSource, dataType);
				output.addToken(dst + " = " + src);
				return true;
			}

			case assembly::CodeType::CODE_PUSH_EA:
			{
				const std::string src = getParameterAsStringIgnoreMemory(code.mParamSource, dataType);
				output.addToken("push(" + src + ")");
				return true;
			}

			case assembly::CodeType::CODE_TEST:
			{
				const std::string dst = getParameterAsString(code.mParamDest, dataType);
				output.addToken("compare" + assembly::CodeOutputHelper::getSizePostfix(code.mDataType.mSize) + "(" + dst + ", 0)");
				return true;
			}

			case assembly::CodeType::CODE_CLEAR:
			{
				const std::string dst = getParameterAsString(code.mParamDest, dataType);
				output.addToken(dst + " = 0");
				return true;
			}

			case assembly::CodeType::CODE_ADD:
			{
				assembly::DataType dstType = dataType;
				if (code.mDataType.isSigned())
				{
					// Signed is always u32(...) += s16(...)
					//  -> Actually it's s32(...), but that does not make any difference here
					dstType = assembly::DataType(assembly::DataType::Size::SIZE_32, assembly::DataType::Sign::UNSPECIFIED);
				}

				const std::string dst = getParameterAsString(code.mParamDest, dstType);
				if (isParameterConstantValue(code.mParamSource, 1))
				{
					output.addToken("++" + dst);
				}
				else
				{
					const std::string src = getParameterAsString(code.mParamSource, dataType);
					output.addToken(dst + " += " + src);
				}
				return true;
			}

			case assembly::CodeType::CODE_ADDX:
			{
				const std::string dst = getParameterAsString(code.mParamDest, dataType);
				const std::string src = getParameterAsString(code.mParamSource, dataType);
				output.addToken(dst + " += " + src + " + FLAG_X");
				return true;
			}

			case assembly::CodeType::CODE_SUB:
			{
				assembly::DataType dstType = dataType;
				if (code.mDataType.isSigned())
				{
					// Signed is always u32(...) -= s16(...)
					//  -> Actually it's s32(...), but that does not make any difference here
					dstType = assembly::DataType(assembly::DataType::Size::SIZE_32, assembly::DataType::Sign::UNSPECIFIED);
				}

				const std::string dst = getParameterAsString(code.mParamDest, dstType);
				if (isParameterConstantValue(code.mParamSource, 1))
				{
					output.addToken("--" + dst);
				}
				else
				{
					const std::string src = getParameterAsString(code.mParamSource, dataType);
					output.addToken(dst + " -= " + src);
				}
				return true;
			}

			case assembly::CodeType::CODE_SUBX:
			{
				const std::string dst = getParameterAsString(code.mParamDest, dataType);
				const std::string src = getParameterAsString(code.mParamSource, dataType);
				output.addToken(dst + " -= " + src + " + FLAG_X");
				return true;
			}

			case assembly::CodeType::CODE_CMP:
			{
				const std::string dst = getParameterAsString(code.mParamDest, dataType);
				const std::string src = getParameterAsString(code.mParamSource, dataType);
				output.addToken("compare" + assembly::CodeOutputHelper::getSizePostfix(code.mDataType.mSize) + "(" + dst + ", " + src + ")");
				return true;
			}

			case assembly::CodeType::CODE_MUL:
			{
				dataType.mSign = code.mDataType.mSign;
				const assembly::DataType::Size resultSize = (dataType.mSize == assembly::DataType::Size::SIZE_8) ? assembly::DataType::Size::SIZE_16 : assembly::DataType::Size::SIZE_32;
				assembly::DataType resultType(resultSize, dataType.mSign);
				const std::string res = getParameterAsString(code.mParamDest, resultType);
				const std::string dst = getParameterAsString(code.mParamDest, dataType);
				const std::string src = getParameterAsString(code.mParamSource, dataType);
				output.addToken(res + " = " + resultType.toString() + "(" + dst + ") * " + src);
				return true;
			}

			case assembly::CodeType::CODE_DIV:
			{
				dataType.mSign = code.mDataType.mSign;
				const assembly::DataType::Size dstSize = (dataType.mSize == assembly::DataType::Size::SIZE_16) ? assembly::DataType::Size::SIZE_32 : assembly::DataType::Size::SIZE_16;
				assembly::DataType dstType(dstSize, dataType.mSign);
				const std::string res = getParameterAsString(code.mParamDest, dataType);
				const std::string dst = getParameterAsString(code.mParamDest, dstType);
				const std::string src = getParameterAsString(code.mParamSource, dataType);
				output.addToken(res + " = " + dst + " / " + src + "   // Result should also contain modulo in upper byte/word");
				return true;
			}

			case assembly::CodeType::CODE_NEG:
			{
				dataType.mSign = assembly::DataType::Sign::SIGNED;
				const std::string dst = getParameterAsString(code.mParamDest, dataType);
				output.addToken(dst + " = -" + dst);
				return true;
			}

			case assembly::CodeType::CODE_NOT:
			{
				const std::string dst = getParameterAsString(code.mParamDest, dataType);
				const std::string bitmask = (dataType.mSize == assembly::DataType::Size::SIZE_32) ? "0xffffffff" :
											(dataType.mSize == assembly::DataType::Size::SIZE_16) ? "0xffff" : "0xff";
				output.addToken(dst + " ^= " + bitmask);
				return true;
			}

			case assembly::CodeType::CODE_AND:
			{
				dataType.mSign = assembly::DataType::Sign::UNSIGNED;
				const std::string dst = getParameterAsString(code.mParamDest, dataType);
				const std::string src = getParameterAsString(code.mParamSource, dataType);
				output.addToken(dst + " &= " + src);
				return true;
			}

			case assembly::CodeType::CODE_OR:
			{
				dataType.mSign = assembly::DataType::Sign::UNSIGNED;
				const std::string dst = getParameterAsString(code.mParamDest, dataType);
				const std::string src = getParameterAsString(code.mParamSource, dataType);
				output.addToken(dst + " |= " + src);
				return true;
			}

			case assembly::CodeType::CODE_XOR:
			{
				dataType.mSign = assembly::DataType::Sign::UNSIGNED;
				const std::string dst = getParameterAsString(code.mParamDest, dataType);
				const std::string src = getParameterAsString(code.mParamSource, dataType);
				output.addToken(dst + " ^= " + src);
				return true;
			}

			case assembly::CodeType::CODE_SET_BY_CONDITION:
			{
				RMX_ASSERT(code.mParamSource.mType == assembly::Parameter::Type::CONDITION, "Condition parameter expected");

				const std::string dst = getParameterAsString(code.mParamDest, assembly::DataType::u8);	// Always u8
				const std::string& cond = assembly::CodeOutputHelper::getConditionLongname(code.mParamSource.mCondition.mCondition);
				output.addToken(dst + " = (" + cond + "()) ? 0xff : 0x00");
				return true;
			}

			case assembly::CodeType::CODE_SHIFT_LEFT:
			case assembly::CodeType::CODE_SHIFT_RIGHT:
			{
				const bool shiftRight = (code.mType == assembly::CodeType::CODE_SHIFT_RIGHT);
				switch (code.mShiftType)
				{
					case assembly::ShiftType::LOGICAL:
					case assembly::ShiftType::ARITHMETIC:
					{
						dataType.mSign = (code.mShiftType == assembly::ShiftType::ARITHMETIC) ? assembly::DataType::Sign::SIGNED : assembly::DataType::Sign::UNSIGNED;
						FormattingOptions srcFormat(assembly::DataType(), true);
						const std::string dst = getParameterAsString(code.mParamDest, dataType);
						const std::string src = getParameterAsString(code.mParamSource, srcFormat);
						const std::string op = shiftRight ? " >>= " : " <<= ";
						output.addToken(dst + op + src);
						return true;
					}

					case assembly::ShiftType::ROTATE:
					case assembly::ShiftType::ROTATE_X:
					{
						if (code.mParamSource.mIsMemory || code.mParamSource.mType != assembly::Parameter::Type::CONSTANT)
							return false;

						const uint32 shiftValue = code.mParamSource.mConstant.mValue;
						const uint32 invShiftValue = code.mDataType.getSizeInBits() - shiftValue;
						const std::string dst = getParameterAsString(code.mParamDest, dataType);
						const std::string fwdShift = dst + (shiftRight ? " >> " : " << ") + std::to_string(shiftValue);
						const std::string invShift = dst + (shiftRight ? " << " : " >> ") + std::to_string(invShiftValue);
						
						std::string outString = dst + " = (" + fwdShift + ") + (" + invShift + ")";
						if (code.mShiftType == assembly::ShiftType::ROTATE_X)
						{
							outString += " + FLAG_X";
						}
						output.addToken(outString);
						return true;
					}
				}
				return false;
			}

			case assembly::CodeType::CODE_SET_BIT:
			{
				dataType.mSign = assembly::DataType::Sign::UNSIGNED;
				const std::string dst = getParameterAsString(code.mParamDest, dataType);
				const std::string src = getParameterAsBitValue(code.mParamSource, dataType);
				output.addToken(dst + " |= " + src);
				return true;
			}

			case assembly::CodeType::CODE_CLEAR_BIT:
			{
				dataType.mSign = assembly::DataType::Sign::UNSIGNED;
				const std::string dst = getParameterAsString(code.mParamDest, dataType);
				const std::string src = getParameterAsBitValue(code.mParamSource, dataType);
				output.addToken(dst + " &= ~" + src);
				return true;
			}

			case assembly::CodeType::CODE_CHANGE_BIT:
			{
				dataType.mSign = assembly::DataType::Sign::UNSIGNED;
				const std::string dst = getParameterAsString(code.mParamDest, dataType);
				const std::string src = getParameterAsBitValue(code.mParamSource, dataType);
				output.addToken(dst + " ^= " + src);
				return true;
			}

			case assembly::CodeType::CODE_TEST_BIT:
			{
				dataType.mSign = assembly::DataType::Sign::UNSIGNED;
				const std::string dst = getParameterAsString(code.mParamDest, dataType);
				const std::string src = getParameterAsBitValue(code.mParamSource, dataType);
				output.addToken("testbit(" + dst + " & " + src + ")");
				return true;
			}

			case assembly::CodeType::CODE_EXTEND_SIGNED:
			{
				dataType.mSign = assembly::DataType::Sign::SIGNED;
				const std::string dst = getParameterAsString(code.mParamDest, dataType);
				dataType.mSize = (assembly::DataType::Size)((size_t)dataType.mSize >> 1);
				const std::string src = getParameterAsString(code.mParamDest, dataType);
				output.addToken(dst + " = " + src);
				return true;
			}

			case assembly::CodeType::CODE_SWAP_WORDS:
			{
				const std::string dst = getParameterAsString(code.mParamDest, assembly::DataType::u32);
				output.addToken(dst + " = (" + dst + " << 16) + (" + dst + " >> 16)");
				return true;
			}

			case assembly::CodeType::CODE_LINK:
			{
				const std::string dst = getParameterAsString(code.mParamDest, assembly::DataType::u32);
				output.addToken(dst + " = link(" + dst + ", " + getParameterAsString(code.mParamSource, assembly::DataType::s16) + ")");
				return true;
			}

			case assembly::CodeType::CODE_UNLINK:
			{
				const std::string dst = getParameterAsString(code.mParamDest, assembly::DataType::u32);
				output.addToken(dst + " = unlink(" + dst + ")");
				return true;
			}

			case assembly::CodeType::CODE_JUMP:
			{
				// Output only unconditional jumps
				if (code.mParamSource.mType != assembly::Parameter::Type::CONDITION)
				{
					formatJumpCall(JumpCallFormatting::CALL_RETURN, code.mParamDest, output);
					return true;
				}
				break;
			}

			case assembly::CodeType::CODE_CALL:
			{
				formatJumpCall(JumpCallFormatting::CALL, code.mParamDest, output);
				return true;
			}

			case assembly::CodeType::CODE_RETURN:
			{
				output.addToken("return");
				return true;
			}
		}
		
		return false;
	}

	void Formatter::formatLemonTokenTreeNode(const lemon::StatementToken& token, Output& output)
	{
		switch (token.getType())
		{
			case lemon::Token::Type::CONSTANT:
			{
				const lemon::ConstantToken& constantToken = token.as<lemon::ConstantToken>();
				if (constantToken.mValue == 0)
				{
					output.addToken("0");
				}
				else if (constantToken.mDataType == &lemon::PredefinedDataTypes::UINT_32)
				{
					output.addToken(hexString((uint32)constantToken.mValue, 6));
				}
				else if (constantToken.mDataType == &lemon::PredefinedDataTypes::UINT_8 && constantToken.mValue <= 2)
				{
					output.addToken(std::to_string(constantToken.mValue));
				}
				else
				{
					output.addToken(hexString((uint32)constantToken.mValue));
				}
				break;
			}

			case lemon::Token::Type::IDENTIFIER:
			{
				const lemon::IdentifierToken& identifierToken = token.as<lemon::IdentifierToken>();
				output.addToken(identifierToken.mIdentifier);
				break;
			}

			case lemon::Token::Type::PARENTHESIS:
			{
				const lemon::ParenthesisToken& parenthesisToken = token.as<lemon::ParenthesisToken>();
				static const char* charactersLookup[2][2] = { { "(", ")" }, { "[", "]" } };
				const char** characters = charactersLookup[(size_t)parenthesisToken.mParenthesisType];
				output.addToken(characters[0]);
				formatLemonTokenTreeNode(parenthesisToken.mContent[0].as<lemon::StatementToken>(), output);
				output.addToken(characters[1]);
				break;
			}

			case lemon::Token::Type::UNARY_OPERATION:
			{
				const lemon::UnaryOperationToken& uot = token.as<lemon::UnaryOperationToken>();
				switch (uot.mOperator)
				{
					case lemon::Operator::UNARY_INCREMENT:	output.addToken("++");   break;
					case lemon::Operator::UNARY_DECREMENT:	output.addToken("--");   break;
					case lemon::Operator::UNARY_BITNOT:		output.addToken("~");   break;
					case lemon::Operator::UNARY_NOT:		output.addToken("!");   break;
				}
				formatLemonTokenTreeNode(*uot.mArgument.get(), output);
				break;
			}

			case lemon::Token::Type::BINARY_OPERATION:
			{
				const lemon::BinaryOperationToken& bot = token.as<lemon::BinaryOperationToken>();
				formatLemonTokenTreeNode(*bot.mLeft.get(), output);
				switch (bot.mOperator)
				{
					case lemon::Operator::ASSIGN:				output.addToken(" = ");   break;
					case lemon::Operator::ASSIGN_PLUS:			output.addToken(" += ");  break;
					case lemon::Operator::ASSIGN_MINUS:			output.addToken(" -= ");  break;
					case lemon::Operator::ASSIGN_MULTIPLY:		output.addToken(" *= ");  break;
					case lemon::Operator::ASSIGN_DIVIDE:		output.addToken(" /= ");  break;
					case lemon::Operator::ASSIGN_MODULO:		output.addToken(" %= ");  break;
					case lemon::Operator::ASSIGN_SHIFT_LEFT:	output.addToken(" <<= "); break;
					case lemon::Operator::ASSIGN_SHIFT_RIGHT:	output.addToken(" >>= "); break;
					case lemon::Operator::ASSIGN_AND:			output.addToken(" &= ");  break;
					case lemon::Operator::ASSIGN_OR:			output.addToken(" |= ");  break;
					case lemon::Operator::ASSIGN_XOR:			output.addToken(" ^= ");  break;
					case lemon::Operator::BINARY_PLUS:			output.addToken(" + ");   break;
					case lemon::Operator::BINARY_MINUS:			output.addToken(" - ");   break;
					case lemon::Operator::BINARY_MULTIPLY:		output.addToken(" * ");   break;
					case lemon::Operator::BINARY_DIVIDE:		output.addToken(" / ");   break;
					case lemon::Operator::BINARY_MODULO:		output.addToken(" % ");   break;
					case lemon::Operator::BINARY_SHIFT_LEFT:	output.addToken(" << ");  break;
					case lemon::Operator::BINARY_SHIFT_RIGHT:	output.addToken(" >> ");  break;
					case lemon::Operator::BINARY_AND:			output.addToken(" & ");   break;
					case lemon::Operator::BINARY_OR:			output.addToken(" | ");   break;
					case lemon::Operator::BINARY_XOR:			output.addToken(" ^ ");   break;
					case lemon::Operator::LOGICAL_AND:			output.addToken(" && ");  break;
					case lemon::Operator::LOGICAL_OR:			output.addToken(" || ");  break;
					// TODO: Support more
				}
				formatLemonTokenTreeNode(*bot.mRight.get(), output);
				break;
			}

			case lemon::Token::Type::VARIABLE:
			{
				// Only the registers are using "lemon::VariableToken", all defines are using "lemon::IdentifierToken" instead
				const lemon::VariableToken& variableToken = token.as<lemon::VariableToken>();
				assembly::Register reg;
				assembly::DataType dataType;
				CodeLemonTokenTree::splitRegisterVariable(variableToken.mVariable, reg, dataType);
				output.addToken(getRegisterName(reg, dataType));
				break;
			}

			case lemon::Token::Type::MEMORY_ACCESS:
			{
				const lemon::MemoryAccessToken& memoryAccessToken = token.as<lemon::MemoryAccessToken>();
				output.addToken(getLemonDataTypeString(memoryAccessToken.mDataType));
				output.addToken("[");
				formatLemonTokenTreeNode(*memoryAccessToken.mAddress.get(), output);
				output.addToken("]");
				break;
			}

			case lemon::Token::Type::VALUE_CAST:
			{
				const lemon::ValueCastToken& valueCastToken = token.as<lemon::ValueCastToken>();
				output.addToken(getLemonDataTypeString(valueCastToken.mDataType));
				output.addToken("(");
				formatLemonTokenTreeNode(*valueCastToken.mArgument.get(), output);
				output.addToken(")");
				break;
			}
		}
	}

	void Formatter::formatJumpCall(JumpCallFormatting jumpCallFormatting, const assembly::Parameter& paramDest, Output& output)
	{
		// Fixed destination?
		if (paramDest.mType == assembly::Parameter::Type::CONSTANT)
		{
			formatJumpCall(jumpCallFormatting, paramDest.mConstant.mValue, output);
		}
		else
		{
			const std::string dst = getParameterAsStringIgnoreMemory(paramDest, assembly::DataType());
			output.addToken(((jumpCallFormatting != JumpCallFormatting::JUMP) ? "call " : "jump ") + dst);

			if (jumpCallFormatting == JumpCallFormatting::CALL_RETURN)
			{
				output.newLine();
				output.addToken("return");
			}
		}
	}

	void Formatter::formatJumpCall(JumpCallFormatting jumpCallFormatting, uint32 destAddress, Output& output)
	{
		output.addToken((jumpCallFormatting != JumpCallFormatting::JUMP) ? "call " : "jump ");
		output.addToken(Token::JUMP_TARGET, hexString(destAddress, 6), destAddress);

		if (jumpCallFormatting == JumpCallFormatting::CALL_RETURN)
		{
			output.newLine();
			output.addToken("return");
		}
	}

}
