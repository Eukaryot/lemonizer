/*
*	Lemonizer -- Turns 68K code into lemonscript
*	Copyright (C) 2017-2026 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#include "pch.h"
#include "builder/FunctionBuilder.h"
#include "builder/RomContent.h"


namespace
{
	struct Connection
	{
		enum Type
		{
			SEQUENCE,	// Next opcode can be reached by its direct predecessor in memory
			LOCALITY,	// Next opcode can not be reached by its direct predecessor in memory
			COND_JUMP,	// Conditional jump connection
			JUMP,		// Unconditional jump connection
			CALL,		// Call connection
			RETURN,		// Connected by return (can be derived from dynamic analysis only)
			_NUM_TYPES
		};

		RomContent::Instruction& mFrom;
		RomContent::Instruction& mTo;
		uint32 mCounter = 0;

		inline Connection(RomContent::Instruction& from, RomContent::Instruction& to) : mFrom(from), mTo(to) {}
	};


	bool isJumpAway(const RomContent::Instruction& instruction)
	{
		static const uint32 jumpAway = RomContent::InstructionFlag::JUMP | RomContent::InstructionFlag::RETURN;
		return ((instruction.mFlags & jumpAway) != 0);
	}
}



FunctionBuilder::FunctionBuilder(RomContent& romContent) :
	mRomContent(romContent)
{
}

FunctionBuilder::~FunctionBuilder()
{
}

void FunctionBuilder::processTracerData()
{
	buildFunctions();
	fillCallCounters();
}

void FunctionBuilder::buildFunctions()
{
	// Get a listing of all relevant instructions
	std::vector<Instruction*> instructions;
	mRomContent.getPresentInstructions(instructions);

	// Build function IDs
	{
		// Find control flow connections between instructions
		std::vector<Connection> connections[Connection::_NUM_TYPES];
		connections[Connection::SEQUENCE].reserve(10000);
		connections[Connection::LOCALITY].reserve(100);
		connections[Connection::COND_JUMP].reserve(100);
		connections[Connection::JUMP].reserve(100);
		connections[Connection::CALL].reserve(100);
		connections[Connection::RETURN].reserve(100);

		// Build up connections that define what is a "function"
		{
			Instruction* formerInstruction = nullptr;
			for (Instruction* instruction_ : instructions)
			{
				Instruction& instruction = *instruction_;

				// At first, every instruction is its own representant
				instruction.mFunctionId = instruction.mAddress;

				if (nullptr != formerInstruction)
				{
					const bool isFollowing = (formerInstruction->mLength == 0) ?	// Length may be zero here for opcodes we could not parse
						(instruction.mAddress <= formerInstruction->mAddress + 8) :
						(instruction.mAddress == formerInstruction->mAddress + formerInstruction->mLength);

					if (isFollowing)
					{
						const Connection::Type type = isJumpAway(*formerInstruction) ? Connection::LOCALITY : Connection::SEQUENCE;
						connections[type].emplace_back(*formerInstruction, instruction);
					}
				}

				formerInstruction = &instruction;

				// Jumps and calls
				if (instruction.mFlags & (RomContent::InstructionFlag::CALL | RomContent::InstructionFlag::JUMP | RomContent::InstructionFlag::COND_JUMP))
				{
					const assembly::AssemblyCode* assemblyCode = mRomContent.getCodeByAddress(instruction.mAddress);
					if (nullptr != assemblyCode && assemblyCode->mParamDest.mType == assembly::Parameter::Type::CONSTANT)
					{
						Instruction* destInstruction = mRomContent.getInstructionByAddress(assemblyCode->mParamDest.mConstant.mValue);
						const Connection::Type type = (instruction.mFlags & RomContent::InstructionFlag::CALL) ? Connection::CALL :
													  (instruction.mFlags & RomContent::InstructionFlag::JUMP) ? Connection::JUMP : Connection::COND_JUMP;
						connections[type].emplace_back(instruction, *destInstruction);
					}
				}
			}
		}

		// Perform connection to build functions
		{
			// Sequence is always connected
			for (const Connection& connection : connections[Connection::SEQUENCE])
			{
				Instruction& representant1 = resolveFunctionIdIndirection(connection.mFrom);
				Instruction& representant2 = resolveFunctionIdIndirection(connection.mTo);

				// Representant is always the first one, with lower address
				if (representant1.mFunctionId < representant2.mFunctionId)
				{
					representant2.mFunctionId = representant1.mFunctionId;
				}
				else
				{
					representant1.mFunctionId = representant2.mFunctionId;
				}
			}

			// Also connect by conditional jumps right to the start of the next, directly neighboring sequence
			for (const Connection& connection : connections[Connection::LOCALITY])
			{
				Instruction& representant11 = resolveFunctionIdIndirection(connection.mFrom);
				Instruction& representant12 = resolveFunctionIdIndirection(connection.mTo);

				if (representant11.mFunctionId != representant12.mFunctionId)
				{
					// Is there a condition jump connecting the same functions (and also in the same order)?
					for (const Connection& connection2 : connections[Connection::COND_JUMP])
					{
						Instruction& representant21 = resolveFunctionIdIndirection(connection2.mFrom);

						if (representant11.mFunctionId == representant21.mFunctionId &&
							representant12.mFunctionId == connection2.mTo.mAddress)
						{
							// Representant is always the first one, with lower address
							if (representant11.mFunctionId < representant12.mFunctionId)
							{
								representant12.mFunctionId = representant11.mFunctionId;
							}
							else
							{
								representant11.mFunctionId = representant12.mFunctionId;
							}
							break;
						}
					}
				}
			}
		}

		// Resolve remaining representant indirections
		for (Instruction* instruction_ : instructions)
		{
			Instruction& instruction = *instruction_;
			if (instruction.mFunctionId != 0xffffffff)
			{
				resolveFunctionIdIndirection(instruction);
			}
		}
	}

	// Create functions from instructions
	{
		FunctionId lastFunctionId = 0xffffffff;
		FunctionInfo* lastFunction = nullptr;

		for (Instruction* instruction_ : instructions)
		{
			Instruction& instruction = *instruction_;
			const FunctionId functionId = instruction.mFunctionId;
			if (functionId != 0xffffffff)
			{
				// Find or create function info (using a simple caching mechanism here)
				FunctionInfo* func = nullptr;
				if (functionId == lastFunctionId && nullptr != lastFunction)
				{
					func = lastFunction;
				}
				else
				{
					auto it = mFunctions.find(functionId);
					if (it == mFunctions.end())
					{
						func = &mFunctions[functionId];
						func->mStartAddress = functionId;
					}
					else
					{
						func = &it->second;
					}

					lastFunctionId = functionId;
					lastFunction = func;
				}

				// Add instruction to function
				func->mInstructions.emplace_back(&instruction);
			}
		}
	}
}

RomContent::Instruction& FunctionBuilder::resolveFunctionIdIndirection(Instruction& instruction)
{
	if (instruction.mFunctionId == instruction.mAddress)
	{
		return instruction;
	}
	RMX_CHECK(instruction.mFunctionId < instruction.mAddress, "Error in representants indirections", RMX_REACT_THROW);

	Instruction& representant = resolveFunctionIdIndirection(*mRomContent.getInstructionByAddress(instruction.mFunctionId));
	instruction.mFunctionId = representant.mFunctionId;
	return representant;
}

void FunctionBuilder::fillCallCounters()
{
	// Evaluate call map
	for (auto& it1 : mFunctions)
	{
		FunctionInfo& func1 = it1.second;
		for (const Instruction* instruction1 : func1.mInstructions)
		{
			if (nullptr == instruction1->mJumpTargetList)
				continue;

			// Consider calls and all kinds of jumps, but no returns
			if (instruction1->mJumpTargetList->mJumpType == RomContent::JumpType::RETURN)
				continue;

			const auto& targets = instruction1->mJumpTargetList->mTargets;
			for (const auto& target : targets)
			{
				const uint32 destAddress = target.second.mDestAddress;

				Instruction* instruction2 = mRomContent.getInstructionByAddress(destAddress);
				RMX_ASSERT(nullptr != instruction2, "Should not happen");

				const auto it2 = mFunctions.find(instruction2->mFunctionId);
				if (it2 != mFunctions.end())
				{
					FunctionInfo& func2 = it2->second;

					// Ignore jumps of the function to itself
					if (&func1 == &func2)
						continue;

					func1.mCallsTo.insert(&func2);

					auto it = func2.mEntryPointCalls.find(destAddress);
					if (it == func2.mEntryPointCalls.end())
					{
						it = func2.mEntryPointCalls.emplace(destAddress, FunctionSet()).first;
					}
					it->second.insert(&func1);
				}
			}
		}
	}
}
