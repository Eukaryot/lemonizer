/*
*	Lemonizer -- turns 68K code into lemon script
*	Copyright (C) 2021 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#pragma once

#include "assembly/AssemblyDefinitions.h"


class RomContent : public SingleInstance<RomContent>
{
public:
	struct Instruction;

public:
	bool loadRom(const std::wstring& filename);

	uint8  read8(uint32 address) const;
	uint16 read16(uint32 address) const;
	uint32 read32(uint32 address) const;

	const assembly::AssemblyCode* getCodeByAddress(uint32 address);

	Instruction* getInstructionByAddress(uint32 address);
	void getPresentInstructions(std::vector<Instruction*>& instructions);

public:
	enum class EntryState
	{
		NONE,		// Address not yet tried to parse
		FAILED,		// Parsing failed
		PARSED		// Parsing succeeded
	};

	struct CodeEntry
	{
		EntryState mState = EntryState::NONE;
		assembly::AssemblyCode mCode;
	};

	struct InstructionFlag
	{
		enum
		{
			// Presence & source of entry
			PRESENT		= 1<<0,		// There is an opcode at the given address; this is a requirement for any other flag to be set
			EXECUTED	= 1<<1,		// Entry was found or is confirmed by run-time execution (i.e. not only static analysis)

			// Jump type
			JUMP		= 1<<8,		// Opcode is an unconditional jump
			COND_JUMP	= 1<<9,		// Opcode is a conditional jump
			CALL		= 1<<10,	// Opcode is a "call", i.e. a jump with return afterwards
			RETURN		= 1<<11,	// Opcode is a "return", i.e. a jump to the location after the last call on stack
			ANY_JUMP	= JUMP + COND_JUMP + CALL + RETURN,
		};
	};

	enum class JumpType
	{
		COND_JUMP,
		JUMP,
		CALL,
		RETURN
	};
	struct JumpTargetEntry
	{
		uint32 mDestAddress;
		inline JumpTargetEntry(uint32 destAddress) : mDestAddress(destAddress) {}
	};
	struct JumpTargetList
	{
		uint32 mSourceAddress;
		JumpType mJumpType;
		std::map<uint32, JumpTargetEntry> mTargets;
	};

	typedef uint32 FunctionId;

	struct Instruction
	{
		uint32 mAddress;
		uint8  mLength = 0;
		uint32 mFlags = 0;
		JumpTargetList* mJumpTargetList = nullptr;
		FunctionId mFunctionId = 0xffffffff;
		uint32 mScriptFunctionId = 0xffffffff;
	};

private:
	static bool isTargetedJump(const Instruction& instruction);
	static bool isJumpAway(const Instruction& instruction);

private:
	bool fillUpInstructionByCode(Instruction& instruction, const assembly::AssemblyCode& assemblyCode);
	void runCartographer(uint32 startAddress);
	void updateJumpTargets(uint32 sourceAddress, uint32 destAddress);

private:
	std::vector<uint8> mRom;
	CArray<CodeEntry> mCodes;			// Not "std::vector" here as it produces too much overhead in Debug builds
	CArray<Instruction> mInstructions;	// Not "std::vector" here as it produces too much overhead in Debug builds

	// Jump target tracking
	ObjectPool<JumpTargetList> mJumpTargetLists;
};
