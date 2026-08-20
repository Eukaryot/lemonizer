/*
*	Lemonizer -- Turns 68K code into lemonscript
*	Copyright (C) 2017-2026 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#include "pch.h"
#include "lemonizer/LemonizerFormatter.h"
#include "lemonizer/LemonizerCode.h"
#include "assembly/CodeOutputHelper.h"
#include "builder/KnowledgeBase.h"
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

		const KnowledgeBase::DefineInfo* getDefineInfo(uint32 address, const assembly::DataType& dataType)
		{
			const KnowledgeBase::DefineInfo* result = nullptr;
			const uint8 bytes = dataType.getSizeInBytes();
			if (!dataType.isSigned())
			{
				result = KnowledgeBase::instance().getDefineInfo(KnowledgeBase::DefineKey(address, bytes, false));
			}
			if (result == nullptr && !dataType.isUnsigned())
			{
				result = KnowledgeBase::instance().getDefineInfo(KnowledgeBase::DefineKey(address, bytes, true));
			}
			return result;
		}

		const KnowledgeBase::DefineInfo* getDefineInfo(assembly::Register baseRegister, int32 addressOffset, const assembly::DataType& dataType)
		{
			const KnowledgeBase::DefineInfo* result = nullptr;
			const uint8 bytes = dataType.getSizeInBytes();
			if (!dataType.isSigned())
			{
				result = KnowledgeBase::instance().getDefineInfo(KnowledgeBase::DefineKey(baseRegister, addressOffset, bytes, false));
			}
			if (result == nullptr && !dataType.isUnsigned())
			{
				result = KnowledgeBase::instance().getDefineInfo(KnowledgeBase::DefineKey(baseRegister, addressOffset, bytes, true));
			}
			return result;
		}

		const KnowledgeBase::DefineInfo* getDefineInfoForParameter(const assembly::Parameter& param, const assembly::DataType& dataType)
		{
			if (param.mIsMemory)
			{
				switch (param.mType)
				{
					case assembly::Parameter::Type::CONSTANT:
					{
						const uint32 address = param.mConstant.mValue;
						if (address >= 0xffff0000)
						{
							return getDefineInfo(address, dataType);
						}
						break;
					}

					case assembly::Parameter::Type::COMBINED:
					{
						if (param.mCombined.mRegister2 == assembly::ExtRegister::NONE)
						{
							return getDefineInfo(param.mCombined.mRegister1, param.mCombined.mDisplacement, dataType);
						}
						break;
					}
				}
			}
			return nullptr;
		}

		std::string getParameterAsString(const assembly::Parameter& param, FormattingOptions& formattingOptions)
		{
			if (param.mIsMemory)
			{
				// Check knowledge base for fitting define information
				const KnowledgeBase::DefineInfo* defineInfo = getDefineInfoForParameter(param, formattingOptions.mDataType);
				if (nullptr != defineInfo)
				{
					formattingOptions.mDataType.mSign = defineInfo->mKey.mSigned ? assembly::DataType::Sign::SIGNED : assembly::DataType::Sign::UNSIGNED;
					return defineInfo->mName;
				}
				else
				{
					return formattingOptions.mDataType.toString() + '[' + getParameterAsStringIgnoreMemory(param, FormattingOptions()) + ']';
				}
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
				// Prefer using just 4 hex digits where possible - e.g. in "if (D4 & 0x0040)"
				if (param.mConstant.mValue < 16)
				{
					return hexString(1 << param.mConstant.mValue, std::min<size_t>(formattingOptions.mDataType.getSizeInBytes() * 2, 4));
				}
				else
				{
					return hexString(1 << param.mConstant.mValue, formattingOptions.mDataType);
				}
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

		const std::string* getConditionStringAndTypeSign(assembly::Condition condition, assembly::DataType::Sign& outTypeSign, bool comparingToZero)
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
					outTypeSign = assembly::DataType::Sign::UNSIGNED;
					return &operatorString[(size_t)condition];

				case SIGNED:
					outTypeSign = assembly::DataType::Sign::SIGNED;
					return &operatorString[(size_t)condition];

				case BOTH:
					outTypeSign = assembly::DataType::Sign::UNSPECIFIED;
					return &operatorString[(size_t)condition];

				case NEGATIVE:
					// When comparing to zero, the value has to be interpreted as signed
					outTypeSign = comparingToZero ? assembly::DataType::Sign::SIGNED : assembly::DataType::Sign::UNSIGNED;
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
					assembly::DataType dataType = ac.mDataType;

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
							const std::string* operatorString = getConditionStringAndTypeSign(condition, dataType.mSign, true);
							if (nullptr != operatorString)
							{
								std::string text;
								text += getParameterAsString(ac.mParamDest, dataType);
								text += *operatorString + "0";
								return text;
							}
							break;
						}

						case assembly::Condition::CC:
						case assembly::Condition::CS:
						{
							// Only handle subtraction for now
							if (ac.mType == assembly::CodeType::CODE_SUB)
							{
								// And also, ensure that only looking at the sign of the result actually gives us the right condition
								//  -> We can only really ensure this if the subtracted part is a constant not too large
								if (ac.mParamSource.isConstant() && ac.mParamSource.mConstant.mValue < ((uint32)1 << (dataType.getSizeInBits() - 1)))
								{
									dataType.mSign = assembly::DataType::Sign::SIGNED;
									const std::string dst = getParameterAsString(ac.mParamDest, dataType);
									if (condition == assembly::Condition::CC)
									{
										return dst + " >= 0";
									}
									else
									{
										return dst + " < 0";
									}
								}
							}

							return (condition == assembly::Condition::CC) ? "!carryFlag()" : "carryFlag()";
						}
					}
					break;
				}

				case assembly::CodeType::CODE_NOT:
				{
					const std::string dst = getParameterAsString(ac.mParamDest, ac.mDataType);
					if (condition == assembly::Condition::EQ)
					{
						return dst + " == 0";
					}
					else if (condition == assembly::Condition::NE)
					{
						return dst + " != 0";
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
						return "(" + dst + " & " + src + ") != 0";
					}
					break;
				}

				case assembly::CodeType::CODE_SET_BIT:
				case assembly::CodeType::CODE_CLEAR_BIT:
				{
					if (ac.mCodeGenData != 0)
					{
						// Use previously defined "_condition" variable
						if (condition == assembly::Condition::EQ)
						{
							return "!_condition" + hexString(ac.mCodeGenData, 6, "");
						}
						else if (condition == assembly::Condition::NE)
						{
							return "_condition" + hexString(ac.mCodeGenData, 6, "");
						}
					}
					else
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

			if (dataType->getClass() == lemon::DataTypeDefinition::Class::INTEGER)
			{
				const lemon::IntegerDataType& integerType = dataType->as<lemon::IntegerDataType>();
				switch (integerType.getBytes())
				{
					case 1:  return integerType.mIsSigned ? typeString_s8 : typeString_u8;
					case 2:  return integerType.mIsSigned ? typeString_s16 : typeString_u16;
					case 4:  return integerType.mIsSigned ? typeString_s32 : typeString_u32;
				}
			}
			return typeString_invalid;
		}

		template<typename T>
		void outputConstantTokenSigned(Formatter::Output& output, lemon::AnyBaseValue anyBaseValue, bool preferDecimalConstants)
		{
			const T value = anyBaseValue.get<T>();
			if (preferDecimalConstants)
			{
				output.addToken(std::to_string(value));
			}
			else if (value < 0)
			{
				output.addToken("-" + hexString((T)-value));
			}
			else
			{
				output.addToken(hexString((T)value));
			}
		}

	}


	void Formatter::formatCode(const Code& code, uint32 address, Output& output)
	{
		switch (code.getType())
		{
			case Code::IFELSE:
			{
				const CodeIfElse& ci = code.as<CodeIfElse>();
				if (ci.mConditionRoot.valid())
				{
					// Translate lemonscript
					output.addToken("if (");
					formatLemonTokenTreeNode(*ci.mConditionRoot.get(), output);
					output.addToken(")");
				}
				else
				{
					std::string text = "if (";
					const bool hasLoopRegister = (ci.mLoopRegister != assembly::ExtRegister::NONE);

					// Add main condition
					if (ci.mCondition != assembly::Condition::NONE)
					{
						const assembly::Condition condition = (ci.mNegateWholeCondition != hasLoopRegister) ? assembly::negateCondition(ci.mCondition) : ci.mCondition;
						if (ci.mConditionAssemblyCode == nullptr)
						{
							text += assembly::CodeOutputHelper::getConditionLongname(condition) + "()";
						}
						else
						{
							text += getConditionString(*ci.mConditionAssemblyCode, condition);
						}
					}

					// Add loop condition
					if (hasLoopRegister)
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
				}
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
				JumpCallFormatting jcf;
				if (cj.mIsCall)
				{
					jcf = JumpCallFormatting::CALL;
				}
				else
				{
					const bool outputAsCallReturn = (mGlobalSettings.mOutputFarJumpsAsCallReturn && !cj.mLines.front()->mLeadsToLabel);
					jcf = outputAsCallReturn ? JumpCallFormatting::CALL_RETURN : JumpCallFormatting::JUMP;
				}
				formatJumpCall(jcf, cj.mDestinationAddress, code.mLines.back()->mAddress + 2, output);	// Return address could be off in this case
				break;
			}

			case Code::RETURN:
			{
				if (mGlobalSettings.mPushPopAddressOnCall)
				{
					output.addToken("asm_return()");
					output.newLine();
				}
				output.addToken("return");
				break;
			}

			case Code::BREAK:
			{
				output.addToken("break");
				break;
			}

			case Code::CONTINUE:
			{
				output.addToken("continue");
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

					// However, if we already that know the right side is positive, make it an unsigned assignment
					if (code.mParamSource.isConstantValue() && code.mParamSource.mConstant.mValue <= 0x7fff)
					{
						dstType = assembly::DataType(assembly::DataType::Size::SIZE_32, assembly::DataType::Sign::UNSIGNED);
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

					// Check for nasty cases like "u16[A7-=2] = u16[A7 + 0x04]", where order of execution would matter, and actually conflict with lemonscript's order of execution (namely left side memory address gets evaluated first)
					if (code.mParamDest.isRegister() && code.mParamDest.mIsMemory && (code.mParamDest.mRegister.mPreDecrement != 0 || code.mParamDest.mRegister.mPostIncrement != 0) &&
						code.mParamSource.isCombinedMemory() && code.mParamSource.mCombined.mRegister1 == code.mParamDest.mRegister.mRegister)
					{
						// Split into two lines
						output.addToken(dstType.toString() + " tmp = " + src);
						output.newLine();
						output.addToken(dst + " = tmp");
					}
					else
					{
						output.addToken(dst + " = " + src);
					}
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
				if (code.mParamSource.mType == assembly::Parameter::Type::CONSTANT)
				{
					for (size_t i = 0; i < registers.size(); ++i)
					{
						if (i > 0)
							output.newLine();

						const int address = code.mParamSource.mConstant.mValue + (int)i * dataType.getSizeInBytes();
						const std::string dst = getRegisterName(registers[i], dataType);
						output.addToken(dst + " = " + code.mDataType.toString() + "[" + hexString(address) + "]");
					}
				}
				else if (code.mParamSource.mType == assembly::Parameter::Type::REGISTER)
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
				if (code.mParamDest.mType == assembly::Parameter::Type::CONSTANT)
				{
					for (size_t i = 0; i < registers.size(); ++i)
					{
						if (i > 0)
							output.newLine();

						const int address = code.mParamDest.mConstant.mValue + (int)i * dataType.getSizeInBytes();
						const std::string src = getRegisterName(registers[i], dataType);
						output.addToken(code.mDataType.toString() + "[" + hexString(address) + "] = " + src);
					}
				}
				else if (code.mParamDest.mType == assembly::Parameter::Type::REGISTER)
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

				if (code.mCodeGenData != 0)
				{
					output.addToken("bool _condition" + hexString(code.mCodeGenData, 6, "") + " = " + dst + " & " + src);
					output.newLine();
				}

				output.addToken(dst + " |= " + src);
				return true;
			}

			case assembly::CodeType::CODE_CLEAR_BIT:
			{
				dataType.mSign = assembly::DataType::Sign::UNSIGNED;
				const std::string dst = getParameterAsString(code.mParamDest, dataType);
				const std::string src = getParameterAsBitValue(code.mParamSource, dataType);

				if (code.mCodeGenData != 0)
				{
					output.addToken("bool _condition" + hexString(code.mCodeGenData, 6, "") + " = " + dst + " & " + src);
					output.newLine();
				}

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
				if (code.mParamDest.isPureRegister() && code.mParamDest.mRegister.mRegister == assembly::Register::A6)
				{
					output.addToken("createStackFrame_A6(" + getParameterAsString(code.mParamSource, assembly::DataType::s16) + ")");

				}
				else
				{
					const std::string dst = getParameterAsString(code.mParamDest, assembly::DataType::u32);
					output.addToken(dst + " = createStackFrame(" + dst + ", " + getParameterAsString(code.mParamSource, assembly::DataType::s16) + ")");
				}
				return true;
			}

			case assembly::CodeType::CODE_UNLINK:
			{
				if (code.mParamDest.isPureRegister() && code.mParamDest.mRegister.mRegister == assembly::Register::A6)
				{
					output.addToken("resolveStackFrame_A6()");
				}
				else
				{
					const std::string dst = getParameterAsString(code.mParamDest, assembly::DataType::u32);
					output.addToken(dst + " = resolveStackFrame(" + dst + ")");
				}
				return true;
			}

			case assembly::CodeType::CODE_JUMP:
			{
				// Output only unconditional jumps
				if (code.mParamSource.mType != assembly::Parameter::Type::CONDITION)
				{
					const JumpCallFormatting jcf = mGlobalSettings.mOutputFarJumpsAsCallReturn ? JumpCallFormatting::CALL_RETURN : JumpCallFormatting::JUMP;
					formatJumpCall(jcf, code.mParamDest, code.mAddress + code.mLength, output);
					return true;
				}
				break;
			}

			case assembly::CodeType::CODE_CALL:
			{
				formatJumpCall(JumpCallFormatting::CALL, code.mParamDest, code.mAddress + code.mLength, output);
				return true;
			}

			case assembly::CodeType::CODE_RETURN:
			{
				if (mGlobalSettings.mPushPopAddressOnCall)
				{
					output.addToken("asm_return()");
					output.newLine();
				}
				output.addToken("return");
				return true;
			}
		}

		return false;
	}

	void Formatter::formatLemonTokenTreeNode(const lemon::StatementToken& token, Output& output, bool useCompactNotation, bool preferDecimalConstants)
	{
		switch (token.getType())
		{
			case lemon::ConstantTokenExt::TYPE:
			{
				const lemon::ConstantTokenExt& constantToken = token.as<lemon::ConstantTokenExt>();
				if (constantToken.mValue.get<uint64>() == 0)
				{
					output.addToken("0");
					break;
				}

				if (constantToken.mOutputAsDecimal)
				{
					preferDecimalConstants = true;
				}

				// Handle signed values
				if (nullptr != constantToken.mDataType && constantToken.mDataType->as<lemon::IntegerDataType>().mIsSigned)
				{
					if (constantToken.mDataType == &lemon::PredefinedDataTypes::INT_8)
					{
						outputConstantTokenSigned<int8>(output, constantToken.mValue, preferDecimalConstants);
						break;
					}
					else if (constantToken.mDataType == &lemon::PredefinedDataTypes::INT_16)
					{
						outputConstantTokenSigned<int16>(output, constantToken.mValue, preferDecimalConstants);
						break;
					}
					else if (constantToken.mDataType == &lemon::PredefinedDataTypes::INT_32)
					{
						outputConstantTokenSigned<int32>(output, constantToken.mValue, preferDecimalConstants);
						break;
					}
				}

				const uint64 value = constantToken.mValue.get<uint64>();
				if (preferDecimalConstants)
				{
					output.addToken(std::to_string(value));
				}
				else if (constantToken.mOutputWithDataTypeSize && nullptr != constantToken.mDataType)
				{
					if (constantToken.mDataType->getBytes() == 1)
					{
						output.addToken(hexString((uint32)value, 2));
					}
					else if (constantToken.mDataType->getBytes() == 2)
					{
						output.addToken(hexString((uint32)value, 4));
					}
					else if (constantToken.mDataType->getBytes() == 4)
					{
						output.addToken(hexString((uint32)value, 6));
					}
					else
					{
						output.addToken(hexString((uint32)value));
					}
				}
				else if (constantToken.mOutputAsAddress)
				{
					output.addToken(hexString((uint32)value, 6));
				}
				else
				{
					output.addToken(hexString((uint32)value));
				}
				break;
			}

			case lemon::IdentifierToken::TYPE:
			{
				const lemon::IdentifierToken& identifierToken = token.as<lemon::IdentifierToken>();
				output.addToken(identifierToken.mName.getString());
				break;
			}

			case lemon::ParenthesisToken::TYPE:
			{
				const lemon::ParenthesisToken& parenthesisToken = token.as<lemon::ParenthesisToken>();
				static const char* charactersLookup[2][2] = { { "(", ")" }, { "[", "]" } };
				const char** characters = charactersLookup[(size_t)parenthesisToken.mParenthesisType];
				output.addToken(characters[0]);
				formatLemonTokenTreeNode(parenthesisToken.mContent[0].as<lemon::StatementToken>(), output, useCompactNotation);
				output.addToken(characters[1]);
				break;
			}

			case lemon::UnaryOperationToken::TYPE:
			{
				const lemon::UnaryOperationToken& uot = token.as<lemon::UnaryOperationToken>();
				switch (uot.mOperator)
				{
					case lemon::Operator::UNARY_INCREMENT:	output.addToken("++");  break;
					case lemon::Operator::UNARY_DECREMENT:	output.addToken("--");  break;
					case lemon::Operator::UNARY_BITNOT:		output.addToken("~");   break;
					case lemon::Operator::UNARY_NOT:		output.addToken("!");   break;
					case lemon::Operator::BINARY_MINUS:		output.addToken("-");   break;
				}
				formatLemonTokenTreeNode(*uot.mArgument.get(), output, useCompactNotation);
				break;
			}

			case lemon::BinaryOperationToken::TYPE:
			{
				const lemon::BinaryOperationToken& bot = token.as<lemon::BinaryOperationToken>();
				formatLemonTokenTreeNode(*bot.mLeft.get(), output, useCompactNotation);

				bool preferDecimalConstants = false;
				switch (bot.mOperator)
				{
					case lemon::Operator::ASSIGN:					output.addToken(useCompactNotation ? "="   : " = ");   break;
					case lemon::Operator::ASSIGN_PLUS:				output.addToken(useCompactNotation ? "+="  : " += ");  break;
					case lemon::Operator::ASSIGN_MINUS:				output.addToken(useCompactNotation ? "-="  : " -= ");  break;
					case lemon::Operator::ASSIGN_MULTIPLY:			output.addToken(useCompactNotation ? "*="  : " *= ");  break;
					case lemon::Operator::ASSIGN_DIVIDE:			output.addToken(useCompactNotation ? "/="  : " /= ");  break;
					case lemon::Operator::ASSIGN_MODULO:			output.addToken(useCompactNotation ? "%="  : " %= ");  break;
					case lemon::Operator::ASSIGN_SHIFT_LEFT:		output.addToken(useCompactNotation ? "<<=" : " <<= "); preferDecimalConstants = true;  break;
					case lemon::Operator::ASSIGN_SHIFT_RIGHT:		output.addToken(useCompactNotation ? ">>=" : " >>= "); preferDecimalConstants = true;  break;
					case lemon::Operator::ASSIGN_AND:				output.addToken(useCompactNotation ? "&="  : " &= ");  break;
					case lemon::Operator::ASSIGN_OR:				output.addToken(useCompactNotation ? "|="  : " |= ");  break;
					case lemon::Operator::ASSIGN_XOR:				output.addToken(useCompactNotation ? "^="  : " ^= ");  break;
					case lemon::Operator::BINARY_PLUS:				output.addToken(useCompactNotation ? "+"   : " + ");   break;
					case lemon::Operator::BINARY_MINUS:				output.addToken(useCompactNotation ? "-"   : " - ");   break;
					case lemon::Operator::BINARY_MULTIPLY:			output.addToken(useCompactNotation ? "*"   : " * ");   break;
					case lemon::Operator::BINARY_DIVIDE:			output.addToken(useCompactNotation ? "/"   : " / ");   break;
					case lemon::Operator::BINARY_MODULO:			output.addToken(useCompactNotation ? "%"   : " % ");   break;
					case lemon::Operator::BINARY_SHIFT_LEFT:		output.addToken(useCompactNotation ? "<<"  : " << ");  preferDecimalConstants = true;  break;
					case lemon::Operator::BINARY_SHIFT_RIGHT:		output.addToken(useCompactNotation ? ">>"  : " >> ");  preferDecimalConstants = true;  break;
					case lemon::Operator::BINARY_AND:				output.addToken(useCompactNotation ? "&"   : " & ");   break;
					case lemon::Operator::BINARY_OR:				output.addToken(useCompactNotation ? "|"   : " | ");   break;
					case lemon::Operator::BINARY_XOR:				output.addToken(useCompactNotation ? "^"   : " ^ ");   break;
					case lemon::Operator::LOGICAL_AND:				output.addToken(useCompactNotation ? "&&"  : " && ");  break;
					case lemon::Operator::LOGICAL_OR:				output.addToken(useCompactNotation ? "||"  : " || ");  break;
					case lemon::Operator::COMPARE_EQUAL:			output.addToken(useCompactNotation ? "=="  : " == ");  break;
					case lemon::Operator::COMPARE_NOT_EQUAL:		output.addToken(useCompactNotation ? "!="  : " != ");  break;
					case lemon::Operator::COMPARE_LESS:				output.addToken(useCompactNotation ? "<"   : " < ");   break;
					case lemon::Operator::COMPARE_LESS_OR_EQUAL:	output.addToken(useCompactNotation ? "<="  : " <= ");  break;
					case lemon::Operator::COMPARE_GREATER:			output.addToken(useCompactNotation ? ">"   : " > ");   break;
					case lemon::Operator::COMPARE_GREATER_OR_EQUAL:	output.addToken(useCompactNotation ? ">="  : " >= ");  break;
					case lemon::Operator::QUESTIONMARK:				output.addToken(useCompactNotation ? "?"   : " ? ");   break;
					case lemon::Operator::COLON:					output.addToken(useCompactNotation ? ":"   : " : ");   break;
				}
				formatLemonTokenTreeNode(*bot.mRight.get(), output, useCompactNotation, preferDecimalConstants);
				break;
			}

			case lemon::VariableToken::TYPE:
			{
				// Only the registers are using "lemon::VariableToken", all defines are using "lemon::IdentifierToken" instead
				const lemon::VariableToken& variableToken = token.as<lemon::VariableToken>();
				assembly::Register reg;
				assembly::DataType dataType;
				CodeLemonTokenTree::splitRegisterVariable(variableToken.mVariable, reg, dataType);
				output.addToken(getRegisterName(reg, dataType));
				break;
			}

			case lemon::MemoryAccessToken::TYPE:
			{
				const lemon::MemoryAccessToken& memoryAccessToken = token.as<lemon::MemoryAccessToken>();
				output.addToken(getLemonDataTypeString(memoryAccessToken.mDataType));
				output.addToken("[");

				// Special case: If the memory address is a register with post increment (like "(A0+=1)-1"), then use compact notation without spaces
				//  -> Note that the detection of this case is rather basic, but it works, so it's probably okay
				const bool isPostIncrementRegister = (memoryAccessToken.mAddress->isA<lemon::BinaryOperationToken>() && memoryAccessToken.mAddress->as<lemon::BinaryOperationToken>().mLeft->isA<lemon::ParenthesisToken>());
				formatLemonTokenTreeNode(*memoryAccessToken.mAddress.get(), output, useCompactNotation || isPostIncrementRegister);
				output.addToken("]");
				break;
			}

			case lemon::ValueCastToken::TYPE:
			{
				const lemon::ValueCastToken& valueCastToken = token.as<lemon::ValueCastToken>();
				output.addToken(getLemonDataTypeString(valueCastToken.mDataType));
				output.addToken("(");
				formatLemonTokenTreeNode(*valueCastToken.mArgument.get(), output, useCompactNotation);
				output.addToken(")");
				break;
			}
		}
	}

	void Formatter::formatJumpCall(JumpCallFormatting jumpCallFormatting, const assembly::Parameter& paramDest, uint32 returnAddress, Output& output)
	{
		// Fixed destination?
		if (paramDest.mType == assembly::Parameter::Type::CONSTANT)
		{
			formatJumpCall(jumpCallFormatting, paramDest.mConstant.mValue, returnAddress, output);
		}
		else
		{
			const bool addPush = (jumpCallFormatting == JumpCallFormatting::CALL) && mGlobalSettings.mPushPopAddressOnCall;
			if (addPush)
			{
				if (mGlobalSettings.mPreCallWithReturnAddress)
					output.addToken("pre_call(" + rmx::hexString(returnAddress, 6) + ")");
				else
					output.addToken("pre_call()");
				output.newLine();
			}

			const std::string dst = getParameterAsStringIgnoreMemory(paramDest, assembly::DataType());
			output.addToken(((jumpCallFormatting != JumpCallFormatting::JUMP) ? "call " : "jump ") + dst);

			if (jumpCallFormatting == JumpCallFormatting::CALL_RETURN)
			{
				output.newLine();
				output.addToken("return");
			}
		}
	}

	void Formatter::formatJumpCall(JumpCallFormatting jumpCallFormatting, uint32 destAddress, uint32 returnAddress, Output& output)
	{
		const bool addPush = (jumpCallFormatting == JumpCallFormatting::CALL) && mGlobalSettings.mPushPopAddressOnCall;
		if (addPush)
		{
			if (mGlobalSettings.mPreCallWithReturnAddress)
				output.addToken("pre_call(" + rmx::hexString(returnAddress, 6) + ")");
			else
				output.addToken("pre_call()");
			output.newLine();
		}

		const KnowledgeBase::FunctionInfo* functionInfo = KnowledgeBase::instance().getFunctionInfo(destAddress);
		if (nullptr != functionInfo && functionInfo->mAutomaticCallReplacement && jumpCallFormatting != JumpCallFormatting::JUMP)
		{
			output.addToken(Token::JUMP_TARGET, functionInfo->mName + "()", destAddress);
		}
		else
		{
			output.addToken((jumpCallFormatting != JumpCallFormatting::JUMP) ? "call " : "jump ");
			output.addToken(Token::JUMP_TARGET, hexString(destAddress, 6), destAddress);
		}

		if (jumpCallFormatting == JumpCallFormatting::CALL_RETURN)
		{
			output.newLine();
			output.addToken("return");
		}
	}

}
