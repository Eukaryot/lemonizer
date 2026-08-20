/*
*	Lemonizer -- turns 68K code into lemon script
*	Copyright (C) 2021 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#include "pch.h"
#include "builder/RomContent.h"
#include "builder/ProjectData.h"
#include "assembly/AssemblyOpcodeTable.h"
#include "assembly/AssemblyOpcodeParserHelper.h"


bool RomContent::loadRom(const std::wstring& filename)
{
	mRom.clear();
	mInstructions.clear();

	// Load file
	if (!FTX::FileSystem->readFile(filename, mRom))
	{
		RMX_ERROR("Failed to load file at '" << *WString(filename).toString() << "'", );
		return false;
	}

	mCodes.resizeTo(mRom.size() / 2);
	mCodes.count = mRom.size() / 2;

	mInstructions.resizeTo(mRom.size() / 2);
	for (size_t i = 0; i < mInstructions.size; ++i)
	{
		mInstructions[i].mAddress = (uint32)(i * 2);
	}
	mInstructions.count = mInstructions.size;

	// Run cartographer for all seed addresses and all jump targets
	for (uint32 address : ProjectData::instance().mSeedAddresses)
	{
		runCartographer(address);
	}
	for (const auto& pair : ProjectData::instance().mJumpTargets)
	{
		runCartographer(pair.second);
	}

	return true;
}

uint8 RomContent::read8(uint32 address) const
{
	if (address >= (uint32)mRom.size())
		return 0;
	return mRom[address];
}

uint16 RomContent::read16(uint32 address) const
{
	if (address + 1 >= (uint32)mRom.size())
		return 0;
	return swapBytes16(*(uint16*)&mRom[address]);
}

uint32 RomContent::read32(uint32 address) const
{
	if (address + 3 >= (uint32)mRom.size())
		return 0;
	return swapBytes32(*(uint32*)&mRom[address]);
}

const assembly::AssemblyCode* RomContent::getCodeByAddress(uint32 address)
{
	const size_t index = (size_t)(address / 2);
	if (index >= (uint32)mCodes.count)
	{
		return nullptr;
	}

	CodeEntry& entry = mCodes[address / 2];
	if (entry.mState == EntryState::NONE)
	{
		// Read next 16 bytes from ROM
		uint8 memory[16];
		for (uint32 offset = 0; offset < 16; offset += 2)
		{
			*(uint16*)&memory[offset] = read16(address + offset);
		}

		const uint16 opcode = *(uint16*)memory;
		const assembly::OpcodeType opcodeType = assembly::OpcodeTable::mOpcodeTable[opcode];

		assembly::OpcodeParserHelper helper(address, memory, entry.mCode);
		const bool success = helper.parseOpcode(opcodeType);
		if (success)
		{
			entry.mState = EntryState::PARSED;
		}
		else
		{
			// Unable to parse, sorry...
			entry.mCode.mType = assembly::CodeType::INVALID;
			entry.mState = EntryState::FAILED;
		}
	}
	return &entry.mCode;
}

RomContent::Instruction* RomContent::getInstructionByAddress(uint32 address)
{
	const size_t index = (size_t)(address / 2);
	return (index >= mInstructions.count) ? nullptr : &mInstructions[index];
}

void RomContent::getPresentInstructions(std::vector<Instruction*>& instructions)
{
	Instruction* formerInstruction = nullptr;
	for (size_t i = 0; i < mInstructions.count; ++i)
	{
		Instruction& instruction = mInstructions[i];
		if (instruction.mFlags & InstructionFlag::PRESENT)
		{
			instructions.push_back(&instruction);
		}
	}
}

bool RomContent::isTargetedJump(const Instruction& instruction)
{
	static const uint32 targetedJumps = InstructionFlag::JUMP | InstructionFlag::COND_JUMP | InstructionFlag::CALL;
	return ((instruction.mFlags & targetedJumps) != 0);
}

bool RomContent::isJumpAway(const Instruction& instruction)
{
	static const uint32 jumpAway = InstructionFlag::JUMP | InstructionFlag::RETURN;
	return ((instruction.mFlags & jumpAway) != 0);
}

bool RomContent::fillUpInstructionByCode(Instruction& instruction, const assembly::AssemblyCode& assemblyCode)
{
	if (assemblyCode.mType == assembly::CodeType::INVALID)
		return false;

	// Fill up additional data in instruction
	instruction.mLength = assemblyCode.mLength;

	switch (assemblyCode.mType)
	{
		case assembly::CodeType::CODE_JUMP:
		{
			if (assemblyCode.mParamSource.mType == assembly::Parameter::Type::CONDITION)
				instruction.mFlags |= InstructionFlag::COND_JUMP;
			else
				instruction.mFlags |= InstructionFlag::JUMP;
			break;
		}

		case assembly::CodeType::CODE_CALL:
		{
			instruction.mFlags |= InstructionFlag::CALL;
			break;
		}

		case assembly::CodeType::CODE_RETURN:
		case assembly::CodeType::CODE_RETURN_EXCEPTION:
		{
			instruction.mFlags |= InstructionFlag::RETURN;
			break;
		}
	}
	return true;
}

void RomContent::runCartographer(uint32 startAddress)
{
	Instruction* instructionPtr = getInstructionByAddress(startAddress);
	if (nullptr == instructionPtr)
		return;

	Instruction& instruction = *instructionPtr;

	// Did we know this is an instruction at all before?
	if ((instruction.mFlags & InstructionFlag::PRESENT) == 0)
	{
		instruction.mFlags |= InstructionFlag::PRESENT;

		const assembly::AssemblyCode* assemblyCode = getCodeByAddress(startAddress);
		if (nullptr != assemblyCode)
		{
			if (fillUpInstructionByCode(instruction, *assemblyCode))
			{
				bool checkNextOpcode = true;
			
				if ((instruction.mFlags & InstructionFlag::ANY_JUMP) != 0)
				{
					// Follow jump target if possible
					if (isTargetedJump(instruction))
					{
						if (assemblyCode->mParamDest.mType == assembly::Parameter::Type::CONSTANT)
						{
							const uint32 destAddress = assemblyCode->mParamDest.mConstant.mValue;

							// Add as jump target
							updateJumpTargets(instruction.mAddress, destAddress);

							// Run cartographer from there as well
							runCartographer(destAddress);
						}
					}

					// Do not check next opcode if it can't be reached from here
					checkNextOpcode = !isJumpAway(instruction);
				}

				if (checkNextOpcode)
				{
					// Next instruction in sequence
					runCartographer(startAddress + instruction.mLength);
				}
			}
		}
	}
}

void RomContent::updateJumpTargets(uint32 sourceAddress, uint32 destAddress)
{
	// Ignore everything that is not inside the ROM
	if (sourceAddress >= (uint32)mRom.size() || destAddress >= (uint32)mRom.size())
		return;

	Instruction* instruction = getInstructionByAddress(sourceAddress);
	if (nullptr == instruction)
		return;

	// Create jump target list if necessary
	if (nullptr == instruction->mJumpTargetList)
	{
		instruction->mJumpTargetList = &mJumpTargetLists.createObject();
		instruction->mJumpTargetList->mSourceAddress = sourceAddress;

		if (instruction->mFlags & InstructionFlag::CALL)
		{
			instruction->mJumpTargetList->mJumpType = JumpType::CALL;
		}
		else if (instruction->mFlags & InstructionFlag::RETURN)
		{
			instruction->mJumpTargetList->mJumpType = JumpType::RETURN;
		}
		else if (instruction->mFlags & InstructionFlag::COND_JUMP)
		{
			instruction->mJumpTargetList->mJumpType = JumpType::COND_JUMP;
		}
		else if (instruction->mFlags & InstructionFlag::JUMP)
		{
			instruction->mJumpTargetList->mJumpType = JumpType::JUMP;
		}
		else
		{
			RMX_ERROR("Instruction is no jump but has a jump target list", );
		}
	}

	// Add or update entry there
	auto& targets = instruction->mJumpTargetList->mTargets;
	auto it = targets.find(destAddress);
	if (it == targets.end())
	{
		it = targets.emplace(destAddress, destAddress).first;
	}
}
