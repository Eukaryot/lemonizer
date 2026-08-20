/*
*	Lemonizer -- Turns 68K code into lemonscript
*	Copyright (C) 2017-2026 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#include "pch.h"
#include "assembly/AssemblyOpcodeParserHelper.h"


namespace assembly
{

	OpcodeParserHelper::OpcodeParserHelper(uint32 address, const uint8* memory, AssemblyCode& outputCode) :
		mAddress(address),
		mMemory(memory),
		mImmediatePosition(2),
		mOutputCode(outputCode)
	{
		// Nothing here
	}

	bool OpcodeParserHelper::parseOpcode(OpcodeType opcodeType)
	{
		parseOpcodeInternal(opcodeType);

		if (mOutputCode.mType != CodeType::INVALID)
		{
			// Parsing opcode succeeded
			mOutputCode.mLength = mImmediatePosition;
			return true;
		}

		// Unrecognized opcode
		return false;
	}

	void OpcodeParserHelper::parseOpcodeInternal(OpcodeType opcodeType)
	{
		switch (opcodeType)
		{
			case OpcodeType::OPCODE_nop:	return parseOpcode_nop(0);

			// Move
			case OpcodeType::OPCODE_move_8_d_d:		 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_d, PARAM_d);
			case OpcodeType::OPCODE_move_8_d_ai:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_d, PARAM_ai);
			case OpcodeType::OPCODE_move_8_d_pi:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_d, PARAM_pi);
			case OpcodeType::OPCODE_move_8_d_pi7:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_d, PARAM_pi7);
			case OpcodeType::OPCODE_move_8_d_pd:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_d, PARAM_pd);
			case OpcodeType::OPCODE_move_8_d_pd7:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_d, PARAM_pd7);
			case OpcodeType::OPCODE_move_8_d_di:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_d, PARAM_di);
			case OpcodeType::OPCODE_move_8_d_ix:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_d, PARAM_ix);
			case OpcodeType::OPCODE_move_8_d_aw:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_d, PARAM_aw);
			case OpcodeType::OPCODE_move_8_d_al:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_d, PARAM_al);
			case OpcodeType::OPCODE_move_8_d_pcdi:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_d, PARAM_pcdi);
			case OpcodeType::OPCODE_move_8_d_pcix:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_d, PARAM_pcix);
			case OpcodeType::OPCODE_move_8_d_i:		 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_d, PARAM_i);
			case OpcodeType::OPCODE_move_8_ai_d:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_ai, PARAM_d);
			case OpcodeType::OPCODE_move_8_ai_ai:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_ai, PARAM_ai);
			case OpcodeType::OPCODE_move_8_ai_pi:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_ai, PARAM_pi);
			case OpcodeType::OPCODE_move_8_ai_pi7:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_ai, PARAM_pi7);
			case OpcodeType::OPCODE_move_8_ai_pd:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_ai, PARAM_pd);
			case OpcodeType::OPCODE_move_8_ai_pd7:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_ai, PARAM_pd7);
			case OpcodeType::OPCODE_move_8_ai_di:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_ai, PARAM_di);
			case OpcodeType::OPCODE_move_8_ai_ix:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_ai, PARAM_ix);
			case OpcodeType::OPCODE_move_8_ai_aw:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_ai, PARAM_aw);
			case OpcodeType::OPCODE_move_8_ai_al:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_ai, PARAM_al);
			case OpcodeType::OPCODE_move_8_ai_pcdi:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_ai, PARAM_pcdi);
			case OpcodeType::OPCODE_move_8_ai_pcix:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_ai, PARAM_pcix);
			case OpcodeType::OPCODE_move_8_ai_i:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_ai, PARAM_i);
			case OpcodeType::OPCODE_move_8_pi_d:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pi, PARAM_d);
			case OpcodeType::OPCODE_move_8_pi_ai:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pi, PARAM_ai);
			case OpcodeType::OPCODE_move_8_pi_pi:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pi, PARAM_pi);
			case OpcodeType::OPCODE_move_8_pi_pi7:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pi, PARAM_pi7);
			case OpcodeType::OPCODE_move_8_pi_pd:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pi, PARAM_pd);
			case OpcodeType::OPCODE_move_8_pi_pd7:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pi, PARAM_pd7);
			case OpcodeType::OPCODE_move_8_pi_di:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pi, PARAM_di);
			case OpcodeType::OPCODE_move_8_pi_ix:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pi, PARAM_ix);
			case OpcodeType::OPCODE_move_8_pi_aw:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pi, PARAM_aw);
			case OpcodeType::OPCODE_move_8_pi_al:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pi, PARAM_al);
			case OpcodeType::OPCODE_move_8_pi_pcdi:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pi, PARAM_pcdi);
			case OpcodeType::OPCODE_move_8_pi_pcix:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pi, PARAM_pcix);
			case OpcodeType::OPCODE_move_8_pi_i:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pi, PARAM_i);
			case OpcodeType::OPCODE_move_8_pi7_d:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pi7, PARAM_d);
			case OpcodeType::OPCODE_move_8_pi7_ai:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pi7, PARAM_ai);
			case OpcodeType::OPCODE_move_8_pi7_pi:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pi7, PARAM_pi);
			case OpcodeType::OPCODE_move_8_pi7_pi7:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pi7, PARAM_pi7);
			case OpcodeType::OPCODE_move_8_pi7_pd:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pi7, PARAM_pd);
			case OpcodeType::OPCODE_move_8_pi7_pd7:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pi7, PARAM_pd7);
			case OpcodeType::OPCODE_move_8_pi7_di:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pi7, PARAM_di);
			case OpcodeType::OPCODE_move_8_pi7_ix:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pi7, PARAM_ix);
			case OpcodeType::OPCODE_move_8_pi7_aw:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pi7, PARAM_aw);
			case OpcodeType::OPCODE_move_8_pi7_al:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pi7, PARAM_al);
			case OpcodeType::OPCODE_move_8_pi7_pcdi: return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pi7, PARAM_pcdi);
			case OpcodeType::OPCODE_move_8_pi7_pcix: return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pi7, PARAM_pcix);
			case OpcodeType::OPCODE_move_8_pi7_i:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pi7, PARAM_i);
			case OpcodeType::OPCODE_move_8_pd_d:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pd, PARAM_d);
			case OpcodeType::OPCODE_move_8_pd_ai:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pd, PARAM_ai);
			case OpcodeType::OPCODE_move_8_pd_pi:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pd, PARAM_pi);
			case OpcodeType::OPCODE_move_8_pd_pi7:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pd, PARAM_pi7);
			case OpcodeType::OPCODE_move_8_pd_pd:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pd, PARAM_pd);
			case OpcodeType::OPCODE_move_8_pd_pd7:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pd, PARAM_pd7);
			case OpcodeType::OPCODE_move_8_pd_di:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pd, PARAM_di);
			case OpcodeType::OPCODE_move_8_pd_ix:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pd, PARAM_ix);
			case OpcodeType::OPCODE_move_8_pd_aw:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pd, PARAM_aw);
			case OpcodeType::OPCODE_move_8_pd_al:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pd, PARAM_al);
			case OpcodeType::OPCODE_move_8_pd_pcdi:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pd, PARAM_pcdi);
			case OpcodeType::OPCODE_move_8_pd_pcix:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pd, PARAM_pcix);
			case OpcodeType::OPCODE_move_8_pd_i:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pd, PARAM_i);
			case OpcodeType::OPCODE_move_8_pd7_d:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pd7, PARAM_d);
			case OpcodeType::OPCODE_move_8_pd7_ai:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pd7, PARAM_ai);
			case OpcodeType::OPCODE_move_8_pd7_pi:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pd7, PARAM_pi);
			case OpcodeType::OPCODE_move_8_pd7_pi7:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pd7, PARAM_pi7);
			case OpcodeType::OPCODE_move_8_pd7_pd:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pd7, PARAM_pd);
			case OpcodeType::OPCODE_move_8_pd7_pd7:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pd7, PARAM_pd7);
			case OpcodeType::OPCODE_move_8_pd7_di:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pd7, PARAM_di);
			case OpcodeType::OPCODE_move_8_pd7_ix:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pd7, PARAM_ix);
			case OpcodeType::OPCODE_move_8_pd7_aw:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pd7, PARAM_aw);
			case OpcodeType::OPCODE_move_8_pd7_al:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pd7, PARAM_al);
			case OpcodeType::OPCODE_move_8_pd7_pcdi: return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pd7, PARAM_pcdi);
			case OpcodeType::OPCODE_move_8_pd7_pcix: return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pd7, PARAM_pcix);
			case OpcodeType::OPCODE_move_8_pd7_i:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_pd7, PARAM_i);
			case OpcodeType::OPCODE_move_8_di_d:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_di, PARAM_d);
			case OpcodeType::OPCODE_move_8_di_ai:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_di, PARAM_ai);
			case OpcodeType::OPCODE_move_8_di_pi:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_di, PARAM_pi);
			case OpcodeType::OPCODE_move_8_di_pi7:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_di, PARAM_pi7);
			case OpcodeType::OPCODE_move_8_di_pd:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_di, PARAM_pd);
			case OpcodeType::OPCODE_move_8_di_pd7:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_di, PARAM_pd7);
			case OpcodeType::OPCODE_move_8_di_di:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_di, PARAM_di);
			case OpcodeType::OPCODE_move_8_di_ix:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_di, PARAM_ix);
			case OpcodeType::OPCODE_move_8_di_aw:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_di, PARAM_aw);
			case OpcodeType::OPCODE_move_8_di_al:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_di, PARAM_al);
			case OpcodeType::OPCODE_move_8_di_pcdi:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_di, PARAM_pcdi);
			case OpcodeType::OPCODE_move_8_di_pcix:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_di, PARAM_pcix);
			case OpcodeType::OPCODE_move_8_di_i:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_di, PARAM_i);
			case OpcodeType::OPCODE_move_8_ix_d:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_ix, PARAM_d);
			case OpcodeType::OPCODE_move_8_ix_ai:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_ix, PARAM_ai);
			case OpcodeType::OPCODE_move_8_ix_pi:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_ix, PARAM_pi);
			case OpcodeType::OPCODE_move_8_ix_pi7:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_ix, PARAM_pi7);
			case OpcodeType::OPCODE_move_8_ix_pd:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_ix, PARAM_pd);
			case OpcodeType::OPCODE_move_8_ix_pd7:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_ix, PARAM_pd7);
			case OpcodeType::OPCODE_move_8_ix_di:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_ix, PARAM_di);
			case OpcodeType::OPCODE_move_8_ix_ix:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_ix, PARAM_ix);
			case OpcodeType::OPCODE_move_8_ix_aw:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_ix, PARAM_aw);
			case OpcodeType::OPCODE_move_8_ix_al:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_ix, PARAM_al);
			case OpcodeType::OPCODE_move_8_ix_pcdi:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_ix, PARAM_pcdi);
			case OpcodeType::OPCODE_move_8_ix_pcix:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_ix, PARAM_pcix);
			case OpcodeType::OPCODE_move_8_ix_i:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_ix, PARAM_i);
			case OpcodeType::OPCODE_move_8_aw_d:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_aw, PARAM_d);
			case OpcodeType::OPCODE_move_8_aw_ai:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_aw, PARAM_ai);
			case OpcodeType::OPCODE_move_8_aw_pi:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_aw, PARAM_pi);
			case OpcodeType::OPCODE_move_8_aw_pi7:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_aw, PARAM_pi7);
			case OpcodeType::OPCODE_move_8_aw_pd:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_aw, PARAM_pd);
			case OpcodeType::OPCODE_move_8_aw_pd7:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_aw, PARAM_pd7);
			case OpcodeType::OPCODE_move_8_aw_di:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_aw, PARAM_di);
			case OpcodeType::OPCODE_move_8_aw_ix:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_aw, PARAM_ix);
			case OpcodeType::OPCODE_move_8_aw_aw:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_aw, PARAM_aw);
			case OpcodeType::OPCODE_move_8_aw_al:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_aw, PARAM_al);
			case OpcodeType::OPCODE_move_8_aw_pcdi:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_aw, PARAM_pcdi);
			case OpcodeType::OPCODE_move_8_aw_pcix:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_aw, PARAM_pcix);
			case OpcodeType::OPCODE_move_8_aw_i:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_aw, PARAM_i);
			case OpcodeType::OPCODE_move_8_al_d:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_al, PARAM_d);
			case OpcodeType::OPCODE_move_8_al_ai:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_al, PARAM_ai);
			case OpcodeType::OPCODE_move_8_al_pi:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_al, PARAM_pi);
			case OpcodeType::OPCODE_move_8_al_pi7:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_al, PARAM_pi7);
			case OpcodeType::OPCODE_move_8_al_pd:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_al, PARAM_pd);
			case OpcodeType::OPCODE_move_8_al_pd7:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_al, PARAM_pd7);
			case OpcodeType::OPCODE_move_8_al_di:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_al, PARAM_di);
			case OpcodeType::OPCODE_move_8_al_ix:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_al, PARAM_ix);
			case OpcodeType::OPCODE_move_8_al_aw:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_al, PARAM_aw);
			case OpcodeType::OPCODE_move_8_al_al:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_al, PARAM_al);
			case OpcodeType::OPCODE_move_8_al_pcdi:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_al, PARAM_pcdi);
			case OpcodeType::OPCODE_move_8_al_pcix:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_al, PARAM_pcix);
			case OpcodeType::OPCODE_move_8_al_i:	 return parseOpcode_move(DataType::Size::SIZE_8, ParamVariant::PARAM_al, PARAM_i);

			case OpcodeType::OPCODE_move_16_d_d:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_d, PARAM_d);
			case OpcodeType::OPCODE_move_16_d_a:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_d, PARAM_a);
			case OpcodeType::OPCODE_move_16_d_ai:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_d, PARAM_ai);
			case OpcodeType::OPCODE_move_16_d_pi:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_d, PARAM_pi);
			case OpcodeType::OPCODE_move_16_d_pd:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_d, PARAM_pd);
			case OpcodeType::OPCODE_move_16_d_di:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_d, PARAM_di);
			case OpcodeType::OPCODE_move_16_d_ix:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_d, PARAM_ix);
			case OpcodeType::OPCODE_move_16_d_aw:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_d, PARAM_aw);
			case OpcodeType::OPCODE_move_16_d_al:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_d, PARAM_al);
			case OpcodeType::OPCODE_move_16_d_pcdi:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_d, PARAM_pcdi);
			case OpcodeType::OPCODE_move_16_d_pcix:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_d, PARAM_pcix);
			case OpcodeType::OPCODE_move_16_d_i:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_d, PARAM_i);
			case OpcodeType::OPCODE_move_16_ai_d:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_ai, PARAM_d);
			case OpcodeType::OPCODE_move_16_ai_a:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_ai, PARAM_a);
			case OpcodeType::OPCODE_move_16_ai_ai:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_ai, PARAM_ai);
			case OpcodeType::OPCODE_move_16_ai_pi:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_ai, PARAM_pi);
			case OpcodeType::OPCODE_move_16_ai_pd:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_ai, PARAM_pd);
			case OpcodeType::OPCODE_move_16_ai_di:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_ai, PARAM_di);
			case OpcodeType::OPCODE_move_16_ai_ix:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_ai, PARAM_ix);
			case OpcodeType::OPCODE_move_16_ai_aw:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_ai, PARAM_aw);
			case OpcodeType::OPCODE_move_16_ai_al:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_ai, PARAM_al);
			case OpcodeType::OPCODE_move_16_ai_pcdi: return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_ai, PARAM_pcdi);
			case OpcodeType::OPCODE_move_16_ai_pcix: return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_ai, PARAM_pcix);
			case OpcodeType::OPCODE_move_16_ai_i:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_ai, PARAM_i);
			case OpcodeType::OPCODE_move_16_pi_d:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_pi, PARAM_d);
			case OpcodeType::OPCODE_move_16_pi_a:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_pi, PARAM_a);
			case OpcodeType::OPCODE_move_16_pi_ai:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_pi, PARAM_ai);
			case OpcodeType::OPCODE_move_16_pi_pi:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_pi, PARAM_pi);
			case OpcodeType::OPCODE_move_16_pi_pd:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_pi, PARAM_pd);
			case OpcodeType::OPCODE_move_16_pi_di:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_pi, PARAM_di);
			case OpcodeType::OPCODE_move_16_pi_ix:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_pi, PARAM_ix);
			case OpcodeType::OPCODE_move_16_pi_aw:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_pi, PARAM_aw);
			case OpcodeType::OPCODE_move_16_pi_al:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_pi, PARAM_al);
			case OpcodeType::OPCODE_move_16_pi_pcdi: return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_pi, PARAM_pcdi);
			case OpcodeType::OPCODE_move_16_pi_pcix: return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_pi, PARAM_pcix);
			case OpcodeType::OPCODE_move_16_pi_i:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_pi, PARAM_i);
			case OpcodeType::OPCODE_move_16_pd_d:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_pd, PARAM_d);
			case OpcodeType::OPCODE_move_16_pd_a:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_pd, PARAM_a);
			case OpcodeType::OPCODE_move_16_pd_ai:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_pd, PARAM_ai);
			case OpcodeType::OPCODE_move_16_pd_pi:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_pd, PARAM_pi);
			case OpcodeType::OPCODE_move_16_pd_pd:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_pd, PARAM_pd);
			case OpcodeType::OPCODE_move_16_pd_di:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_pd, PARAM_di);
			case OpcodeType::OPCODE_move_16_pd_ix:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_pd, PARAM_ix);
			case OpcodeType::OPCODE_move_16_pd_aw:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_pd, PARAM_aw);
			case OpcodeType::OPCODE_move_16_pd_al:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_pd, PARAM_al);
			case OpcodeType::OPCODE_move_16_pd_pcdi: return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_pd, PARAM_pcdi);
			case OpcodeType::OPCODE_move_16_pd_pcix: return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_pd, PARAM_pcix);
			case OpcodeType::OPCODE_move_16_pd_i:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_pd, PARAM_i);
			case OpcodeType::OPCODE_move_16_di_d:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_di, PARAM_d);
			case OpcodeType::OPCODE_move_16_di_a:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_di, PARAM_a);
			case OpcodeType::OPCODE_move_16_di_ai:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_di, PARAM_ai);
			case OpcodeType::OPCODE_move_16_di_pi:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_di, PARAM_pi);
			case OpcodeType::OPCODE_move_16_di_pd:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_di, PARAM_pd);
			case OpcodeType::OPCODE_move_16_di_di:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_di, PARAM_di);
			case OpcodeType::OPCODE_move_16_di_ix:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_di, PARAM_ix);
			case OpcodeType::OPCODE_move_16_di_aw:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_di, PARAM_aw);
			case OpcodeType::OPCODE_move_16_di_al:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_di, PARAM_al);
			case OpcodeType::OPCODE_move_16_di_pcdi: return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_di, PARAM_pcdi);
			case OpcodeType::OPCODE_move_16_di_pcix: return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_di, PARAM_pcix);
			case OpcodeType::OPCODE_move_16_di_i:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_di, PARAM_i);
			case OpcodeType::OPCODE_move_16_ix_d:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_ix, PARAM_d);
			case OpcodeType::OPCODE_move_16_ix_a:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_ix, PARAM_a);
			case OpcodeType::OPCODE_move_16_ix_ai:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_ix, PARAM_ai);
			case OpcodeType::OPCODE_move_16_ix_pi:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_ix, PARAM_pi);
			case OpcodeType::OPCODE_move_16_ix_pd:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_ix, PARAM_pd);
			case OpcodeType::OPCODE_move_16_ix_di:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_ix, PARAM_di);
			case OpcodeType::OPCODE_move_16_ix_ix:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_ix, PARAM_ix);
			case OpcodeType::OPCODE_move_16_ix_aw:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_ix, PARAM_aw);
			case OpcodeType::OPCODE_move_16_ix_al:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_ix, PARAM_al);
			case OpcodeType::OPCODE_move_16_ix_pcdi: return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_ix, PARAM_pcdi);
			case OpcodeType::OPCODE_move_16_ix_pcix: return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_ix, PARAM_pcix);
			case OpcodeType::OPCODE_move_16_ix_i:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_ix, PARAM_i);
			case OpcodeType::OPCODE_move_16_aw_d:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_aw, PARAM_d);
			case OpcodeType::OPCODE_move_16_aw_a:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_aw, PARAM_a);
			case OpcodeType::OPCODE_move_16_aw_ai:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_aw, PARAM_ai);
			case OpcodeType::OPCODE_move_16_aw_pi:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_aw, PARAM_pi);
			case OpcodeType::OPCODE_move_16_aw_pd:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_aw, PARAM_pd);
			case OpcodeType::OPCODE_move_16_aw_di:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_aw, PARAM_di);
			case OpcodeType::OPCODE_move_16_aw_ix:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_aw, PARAM_ix);
			case OpcodeType::OPCODE_move_16_aw_aw:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_aw, PARAM_aw);
			case OpcodeType::OPCODE_move_16_aw_al:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_aw, PARAM_al);
			case OpcodeType::OPCODE_move_16_aw_pcdi: return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_aw, PARAM_pcdi);
			case OpcodeType::OPCODE_move_16_aw_pcix: return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_aw, PARAM_pcix);
			case OpcodeType::OPCODE_move_16_aw_i:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_aw, PARAM_i);
			case OpcodeType::OPCODE_move_16_al_d:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_al, PARAM_d);
			case OpcodeType::OPCODE_move_16_al_a:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_al, PARAM_a);
			case OpcodeType::OPCODE_move_16_al_ai:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_al, PARAM_ai);
			case OpcodeType::OPCODE_move_16_al_pi:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_al, PARAM_pi);
			case OpcodeType::OPCODE_move_16_al_pd:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_al, PARAM_pd);
			case OpcodeType::OPCODE_move_16_al_di:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_al, PARAM_di);
			case OpcodeType::OPCODE_move_16_al_ix:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_al, PARAM_ix);
			case OpcodeType::OPCODE_move_16_al_aw:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_al, PARAM_aw);
			case OpcodeType::OPCODE_move_16_al_al:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_al, PARAM_al);
			case OpcodeType::OPCODE_move_16_al_pcdi: return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_al, PARAM_pcdi);
			case OpcodeType::OPCODE_move_16_al_pcix: return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_al, PARAM_pcix);
			case OpcodeType::OPCODE_move_16_al_i:	 return parseOpcode_move(DataType::Size::SIZE_16, ParamVariant::PARAM_al, PARAM_i);

			case OpcodeType::OPCODE_move_32_d_d:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_d, PARAM_d);
			case OpcodeType::OPCODE_move_32_d_a:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_d, PARAM_a);
			case OpcodeType::OPCODE_move_32_d_ai:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_d, PARAM_ai);
			case OpcodeType::OPCODE_move_32_d_pi:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_d, PARAM_pi);
			case OpcodeType::OPCODE_move_32_d_pd:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_d, PARAM_pd);
			case OpcodeType::OPCODE_move_32_d_di:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_d, PARAM_di);
			case OpcodeType::OPCODE_move_32_d_ix:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_d, PARAM_ix);
			case OpcodeType::OPCODE_move_32_d_aw:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_d, PARAM_aw);
			case OpcodeType::OPCODE_move_32_d_al:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_d, PARAM_al);
			case OpcodeType::OPCODE_move_32_d_pcdi:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_d, PARAM_pcdi);
			case OpcodeType::OPCODE_move_32_d_pcix:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_d, PARAM_pcix);
			case OpcodeType::OPCODE_move_32_d_i:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_d, PARAM_i);
			case OpcodeType::OPCODE_move_32_ai_d:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_ai, PARAM_d);
			case OpcodeType::OPCODE_move_32_ai_a:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_ai, PARAM_a);
			case OpcodeType::OPCODE_move_32_ai_ai:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_ai, PARAM_ai);
			case OpcodeType::OPCODE_move_32_ai_pi:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_ai, PARAM_pi);
			case OpcodeType::OPCODE_move_32_ai_pd:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_ai, PARAM_pd);
			case OpcodeType::OPCODE_move_32_ai_di:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_ai, PARAM_di);
			case OpcodeType::OPCODE_move_32_ai_ix:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_ai, PARAM_ix);
			case OpcodeType::OPCODE_move_32_ai_aw:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_ai, PARAM_aw);
			case OpcodeType::OPCODE_move_32_ai_al:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_ai, PARAM_al);
			case OpcodeType::OPCODE_move_32_ai_pcdi: return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_ai, PARAM_pcdi);
			case OpcodeType::OPCODE_move_32_ai_pcix: return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_ai, PARAM_pcix);
			case OpcodeType::OPCODE_move_32_ai_i:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_ai, PARAM_i);
			case OpcodeType::OPCODE_move_32_pi_d:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_pi, PARAM_d);
			case OpcodeType::OPCODE_move_32_pi_a:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_pi, PARAM_a);
			case OpcodeType::OPCODE_move_32_pi_ai:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_pi, PARAM_ai);
			case OpcodeType::OPCODE_move_32_pi_pi:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_pi, PARAM_pi);
			case OpcodeType::OPCODE_move_32_pi_pd:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_pi, PARAM_pd);
			case OpcodeType::OPCODE_move_32_pi_di:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_pi, PARAM_di);
			case OpcodeType::OPCODE_move_32_pi_ix:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_pi, PARAM_ix);
			case OpcodeType::OPCODE_move_32_pi_aw:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_pi, PARAM_aw);
			case OpcodeType::OPCODE_move_32_pi_al:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_pi, PARAM_al);
			case OpcodeType::OPCODE_move_32_pi_pcdi: return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_pi, PARAM_pcdi);
			case OpcodeType::OPCODE_move_32_pi_pcix: return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_pi, PARAM_pcix);
			case OpcodeType::OPCODE_move_32_pi_i:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_pi, PARAM_i);
			case OpcodeType::OPCODE_move_32_pd_d:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_pd, PARAM_d);
			case OpcodeType::OPCODE_move_32_pd_a:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_pd, PARAM_a);
			case OpcodeType::OPCODE_move_32_pd_ai:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_pd, PARAM_ai);
			case OpcodeType::OPCODE_move_32_pd_pi:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_pd, PARAM_pi);
			case OpcodeType::OPCODE_move_32_pd_pd:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_pd, PARAM_pd);
			case OpcodeType::OPCODE_move_32_pd_di:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_pd, PARAM_di);
			case OpcodeType::OPCODE_move_32_pd_ix:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_pd, PARAM_ix);
			case OpcodeType::OPCODE_move_32_pd_aw:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_pd, PARAM_aw);
			case OpcodeType::OPCODE_move_32_pd_al:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_pd, PARAM_al);
			case OpcodeType::OPCODE_move_32_pd_pcdi: return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_pd, PARAM_pcdi);
			case OpcodeType::OPCODE_move_32_pd_pcix: return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_pd, PARAM_pcix);
			case OpcodeType::OPCODE_move_32_pd_i:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_pd, PARAM_i);
			case OpcodeType::OPCODE_move_32_di_d:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_di, PARAM_d);
			case OpcodeType::OPCODE_move_32_di_a:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_di, PARAM_a);
			case OpcodeType::OPCODE_move_32_di_ai:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_di, PARAM_ai);
			case OpcodeType::OPCODE_move_32_di_pi:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_di, PARAM_pi);
			case OpcodeType::OPCODE_move_32_di_pd:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_di, PARAM_pd);
			case OpcodeType::OPCODE_move_32_di_di:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_di, PARAM_di);
			case OpcodeType::OPCODE_move_32_di_ix:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_di, PARAM_ix);
			case OpcodeType::OPCODE_move_32_di_aw:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_di, PARAM_aw);
			case OpcodeType::OPCODE_move_32_di_al:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_di, PARAM_al);
			case OpcodeType::OPCODE_move_32_di_pcdi: return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_di, PARAM_pcdi);
			case OpcodeType::OPCODE_move_32_di_pcix: return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_di, PARAM_pcix);
			case OpcodeType::OPCODE_move_32_di_i:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_di, PARAM_i);
			case OpcodeType::OPCODE_move_32_ix_d:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_ix, PARAM_d);
			case OpcodeType::OPCODE_move_32_ix_a:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_ix, PARAM_a);
			case OpcodeType::OPCODE_move_32_ix_ai:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_ix, PARAM_ai);
			case OpcodeType::OPCODE_move_32_ix_pi:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_ix, PARAM_pi);
			case OpcodeType::OPCODE_move_32_ix_pd:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_ix, PARAM_pd);
			case OpcodeType::OPCODE_move_32_ix_di:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_ix, PARAM_di);
			case OpcodeType::OPCODE_move_32_ix_ix:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_ix, PARAM_ix);
			case OpcodeType::OPCODE_move_32_ix_aw:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_ix, PARAM_aw);
			case OpcodeType::OPCODE_move_32_ix_al:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_ix, PARAM_al);
			case OpcodeType::OPCODE_move_32_ix_pcdi: return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_ix, PARAM_pcdi);
			case OpcodeType::OPCODE_move_32_ix_pcix: return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_ix, PARAM_pcix);
			case OpcodeType::OPCODE_move_32_ix_i:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_ix, PARAM_i);
			case OpcodeType::OPCODE_move_32_aw_d:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_aw, PARAM_d);
			case OpcodeType::OPCODE_move_32_aw_a:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_aw, PARAM_a);
			case OpcodeType::OPCODE_move_32_aw_ai:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_aw, PARAM_ai);
			case OpcodeType::OPCODE_move_32_aw_pi:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_aw, PARAM_pi);
			case OpcodeType::OPCODE_move_32_aw_pd:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_aw, PARAM_pd);
			case OpcodeType::OPCODE_move_32_aw_di:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_aw, PARAM_di);
			case OpcodeType::OPCODE_move_32_aw_ix:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_aw, PARAM_ix);
			case OpcodeType::OPCODE_move_32_aw_aw:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_aw, PARAM_aw);
			case OpcodeType::OPCODE_move_32_aw_al:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_aw, PARAM_al);
			case OpcodeType::OPCODE_move_32_aw_pcdi: return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_aw, PARAM_pcdi);
			case OpcodeType::OPCODE_move_32_aw_pcix: return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_aw, PARAM_pcix);
			case OpcodeType::OPCODE_move_32_aw_i:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_aw, PARAM_i);
			case OpcodeType::OPCODE_move_32_al_d:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_al, PARAM_d);
			case OpcodeType::OPCODE_move_32_al_a:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_al, PARAM_a);
			case OpcodeType::OPCODE_move_32_al_ai:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_al, PARAM_ai);
			case OpcodeType::OPCODE_move_32_al_pi:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_al, PARAM_pi);
			case OpcodeType::OPCODE_move_32_al_pd:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_al, PARAM_pd);
			case OpcodeType::OPCODE_move_32_al_di:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_al, PARAM_di);
			case OpcodeType::OPCODE_move_32_al_ix:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_al, PARAM_ix);
			case OpcodeType::OPCODE_move_32_al_aw:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_al, PARAM_aw);
			case OpcodeType::OPCODE_move_32_al_al:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_al, PARAM_al);
			case OpcodeType::OPCODE_move_32_al_pcdi: return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_al, PARAM_pcdi);
			case OpcodeType::OPCODE_move_32_al_pcix: return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_al, PARAM_pcix);
			case OpcodeType::OPCODE_move_32_al_i:	 return parseOpcode_move(DataType::Size::SIZE_32, ParamVariant::PARAM_al, PARAM_i);

			case OpcodeType::OPCODE_movea_16_d:		 return parseOpcode_movea(DataType::Size::SIZE_16, PARAM_d);
			case OpcodeType::OPCODE_movea_16_a:		 return parseOpcode_movea(DataType::Size::SIZE_16, PARAM_a);
			case OpcodeType::OPCODE_movea_16_ai:	 return parseOpcode_movea(DataType::Size::SIZE_16, PARAM_ai);
			case OpcodeType::OPCODE_movea_16_pi:	 return parseOpcode_movea(DataType::Size::SIZE_16, PARAM_pi);
			case OpcodeType::OPCODE_movea_16_pd:	 return parseOpcode_movea(DataType::Size::SIZE_16, PARAM_pd);
			case OpcodeType::OPCODE_movea_16_di:	 return parseOpcode_movea(DataType::Size::SIZE_16, PARAM_di);
			case OpcodeType::OPCODE_movea_16_ix:	 return parseOpcode_movea(DataType::Size::SIZE_16, PARAM_ix);
			case OpcodeType::OPCODE_movea_16_aw:	 return parseOpcode_movea(DataType::Size::SIZE_16, PARAM_aw);
			case OpcodeType::OPCODE_movea_16_al:	 return parseOpcode_movea(DataType::Size::SIZE_16, PARAM_al);
			case OpcodeType::OPCODE_movea_16_pcdi:	 return parseOpcode_movea(DataType::Size::SIZE_16, PARAM_pcdi);
			case OpcodeType::OPCODE_movea_16_pcix:	 return parseOpcode_movea(DataType::Size::SIZE_16, PARAM_pcix);
			case OpcodeType::OPCODE_movea_16_i:		 return parseOpcode_movea(DataType::Size::SIZE_16, PARAM_i);
			case OpcodeType::OPCODE_movea_32_d:		 return parseOpcode_movea(DataType::Size::SIZE_32, PARAM_d);
			case OpcodeType::OPCODE_movea_32_a:		 return parseOpcode_movea(DataType::Size::SIZE_32, PARAM_a);
			case OpcodeType::OPCODE_movea_32_ai:	 return parseOpcode_movea(DataType::Size::SIZE_32, PARAM_ai);
			case OpcodeType::OPCODE_movea_32_pi:	 return parseOpcode_movea(DataType::Size::SIZE_32, PARAM_pi);
			case OpcodeType::OPCODE_movea_32_pd:	 return parseOpcode_movea(DataType::Size::SIZE_32, PARAM_pd);
			case OpcodeType::OPCODE_movea_32_di:	 return parseOpcode_movea(DataType::Size::SIZE_32, PARAM_di);
			case OpcodeType::OPCODE_movea_32_ix:	 return parseOpcode_movea(DataType::Size::SIZE_32, PARAM_ix);
			case OpcodeType::OPCODE_movea_32_aw:	 return parseOpcode_movea(DataType::Size::SIZE_32, PARAM_aw);
			case OpcodeType::OPCODE_movea_32_al:	 return parseOpcode_movea(DataType::Size::SIZE_32, PARAM_al);
			case OpcodeType::OPCODE_movea_32_pcdi:	 return parseOpcode_movea(DataType::Size::SIZE_32, PARAM_pcdi);
			case OpcodeType::OPCODE_movea_32_pcix:	 return parseOpcode_movea(DataType::Size::SIZE_32, PARAM_pcix);
			case OpcodeType::OPCODE_movea_32_i:		 return parseOpcode_movea(DataType::Size::SIZE_32, PARAM_i);

			case OpcodeType::OPCODE_movem_16_er_ai:		return parseOpcode_movem_er(DataType::Size::SIZE_16, PARAM_ai);
			case OpcodeType::OPCODE_movem_16_er_pi:		return parseOpcode_movem_er(DataType::Size::SIZE_16, PARAM_pi);
			case OpcodeType::OPCODE_movem_16_er_di:		return parseOpcode_movem_er(DataType::Size::SIZE_16, PARAM_di);
			case OpcodeType::OPCODE_movem_16_er_ix:		return parseOpcode_movem_er(DataType::Size::SIZE_16, PARAM_ix);
			case OpcodeType::OPCODE_movem_16_er_aw:		return parseOpcode_movem_er(DataType::Size::SIZE_16, PARAM_aw);
			case OpcodeType::OPCODE_movem_16_er_al:		return parseOpcode_movem_er(DataType::Size::SIZE_16, PARAM_al);
			case OpcodeType::OPCODE_movem_16_er_pcdi:	return parseOpcode_movem_er(DataType::Size::SIZE_16, PARAM_pcdi);
			case OpcodeType::OPCODE_movem_16_er_pcix:	return parseOpcode_movem_er(DataType::Size::SIZE_16, PARAM_pcix);
			case OpcodeType::OPCODE_movem_32_er_ai:		return parseOpcode_movem_er(DataType::Size::SIZE_32, PARAM_ai);
			case OpcodeType::OPCODE_movem_32_er_pi:		return parseOpcode_movem_er(DataType::Size::SIZE_32, PARAM_pi);
			case OpcodeType::OPCODE_movem_32_er_di:		return parseOpcode_movem_er(DataType::Size::SIZE_32, PARAM_di);
			case OpcodeType::OPCODE_movem_32_er_ix:		return parseOpcode_movem_er(DataType::Size::SIZE_32, PARAM_ix);
			case OpcodeType::OPCODE_movem_32_er_aw:		return parseOpcode_movem_er(DataType::Size::SIZE_32, PARAM_aw);
			case OpcodeType::OPCODE_movem_32_er_al:		return parseOpcode_movem_er(DataType::Size::SIZE_32, PARAM_al);
			case OpcodeType::OPCODE_movem_32_er_pcdi:	return parseOpcode_movem_er(DataType::Size::SIZE_32, PARAM_pcdi);
			case OpcodeType::OPCODE_movem_32_er_pcix:	return parseOpcode_movem_er(DataType::Size::SIZE_32, PARAM_pcix);
			case OpcodeType::OPCODE_movem_16_re_ai:		return parseOpcode_movem_re(DataType::Size::SIZE_16, PARAM_ai);
			case OpcodeType::OPCODE_movem_16_re_pd:		return parseOpcode_movem_re(DataType::Size::SIZE_16, PARAM_pd);
			case OpcodeType::OPCODE_movem_16_re_di:		return parseOpcode_movem_re(DataType::Size::SIZE_16, PARAM_di);
			case OpcodeType::OPCODE_movem_16_re_ix:		return parseOpcode_movem_re(DataType::Size::SIZE_16, PARAM_ix);
			case OpcodeType::OPCODE_movem_16_re_aw:		return parseOpcode_movem_re(DataType::Size::SIZE_16, PARAM_aw);
			case OpcodeType::OPCODE_movem_16_re_al:		return parseOpcode_movem_re(DataType::Size::SIZE_16, PARAM_al);
			case OpcodeType::OPCODE_movem_32_re_ai:		return parseOpcode_movem_re(DataType::Size::SIZE_32, PARAM_ai);
			case OpcodeType::OPCODE_movem_32_re_pd:		return parseOpcode_movem_re(DataType::Size::SIZE_32, PARAM_pd);
			case OpcodeType::OPCODE_movem_32_re_di:		return parseOpcode_movem_re(DataType::Size::SIZE_32, PARAM_di);
			case OpcodeType::OPCODE_movem_32_re_ix:		return parseOpcode_movem_re(DataType::Size::SIZE_32, PARAM_ix);
			case OpcodeType::OPCODE_movem_32_re_aw:		return parseOpcode_movem_re(DataType::Size::SIZE_32, PARAM_aw);
			case OpcodeType::OPCODE_movem_32_re_al:		return parseOpcode_movem_re(DataType::Size::SIZE_32, PARAM_al);

			case OpcodeType::OPCODE_movep_16_re:		return parseOpcode_movep_re(DataType::Size::SIZE_16);
			case OpcodeType::OPCODE_movep_32_re:		return parseOpcode_movep_re(DataType::Size::SIZE_32);
			case OpcodeType::OPCODE_movep_16_er:		return parseOpcode_movep_er(DataType::Size::SIZE_16);
			case OpcodeType::OPCODE_movep_32_er:		return parseOpcode_movep_er(DataType::Size::SIZE_32);

			case OpcodeType::OPCODE_moveq_32:			return parseOpcode_moveq();

			case OpcodeType::OPCODE_move_16_toc_d:		return parseOpcode_move_toc(PARAM_d);
			case OpcodeType::OPCODE_move_16_toc_ai:		return parseOpcode_move_toc(PARAM_ai);
			case OpcodeType::OPCODE_move_16_toc_pi:		return parseOpcode_move_toc(PARAM_pi);
			case OpcodeType::OPCODE_move_16_toc_pd:		return parseOpcode_move_toc(PARAM_pd);
			case OpcodeType::OPCODE_move_16_toc_di:		return parseOpcode_move_toc(PARAM_di);
			case OpcodeType::OPCODE_move_16_toc_ix:		return parseOpcode_move_toc(PARAM_ix);
			case OpcodeType::OPCODE_move_16_toc_aw:		return parseOpcode_move_toc(PARAM_aw);
			case OpcodeType::OPCODE_move_16_toc_al:		return parseOpcode_move_toc(PARAM_al);
			case OpcodeType::OPCODE_move_16_toc_pcdi:	return parseOpcode_move_toc(PARAM_pcdi);
			case OpcodeType::OPCODE_move_16_toc_pcix:	return parseOpcode_move_toc(PARAM_pcix);
			case OpcodeType::OPCODE_move_16_toc_i:		return parseOpcode_move_toc(PARAM_i);

			case OpcodeType::OPCODE_move_16_frs_d:		return parseOpcode_move_frs(PARAM_d);
			case OpcodeType::OPCODE_move_16_frs_ai:		return parseOpcode_move_frs(PARAM_ai);
			case OpcodeType::OPCODE_move_16_frs_pi:		return parseOpcode_move_frs(PARAM_pi);
			case OpcodeType::OPCODE_move_16_frs_pd:		return parseOpcode_move_frs(PARAM_pd);
			case OpcodeType::OPCODE_move_16_frs_di:		return parseOpcode_move_frs(PARAM_di);
			case OpcodeType::OPCODE_move_16_frs_ix:		return parseOpcode_move_frs(PARAM_ix);
			case OpcodeType::OPCODE_move_16_frs_aw:		return parseOpcode_move_frs(PARAM_aw);
			case OpcodeType::OPCODE_move_16_frs_al:		return parseOpcode_move_frs(PARAM_al);

			case OpcodeType::OPCODE_move_16_tos_d:		return parseOpcode_move_tos(PARAM_d);
			case OpcodeType::OPCODE_move_16_tos_ai:		return parseOpcode_move_tos(PARAM_ai);
			case OpcodeType::OPCODE_move_16_tos_pi:		return parseOpcode_move_tos(PARAM_pi);
			case OpcodeType::OPCODE_move_16_tos_pd:		return parseOpcode_move_tos(PARAM_pd);
			case OpcodeType::OPCODE_move_16_tos_di:		return parseOpcode_move_tos(PARAM_di);
			case OpcodeType::OPCODE_move_16_tos_ix:		return parseOpcode_move_tos(PARAM_ix);
			case OpcodeType::OPCODE_move_16_tos_aw:		return parseOpcode_move_tos(PARAM_aw);
			case OpcodeType::OPCODE_move_16_tos_al:		return parseOpcode_move_tos(PARAM_al);
			case OpcodeType::OPCODE_move_16_tos_pcdi:	return parseOpcode_move_tos(PARAM_pcdi);
			case OpcodeType::OPCODE_move_16_tos_pcix:	return parseOpcode_move_tos(PARAM_pcix);
			case OpcodeType::OPCODE_move_16_tos_i:		return parseOpcode_move_tos(PARAM_i);

			case OpcodeType::OPCODE_move_32_fru:		return parseOpcode_move_fru(PARAM_a);
			case OpcodeType::OPCODE_move_32_tou:		return parseOpcode_move_tou(PARAM_a);

			// Load effective address
			case OpcodeType::OPCODE_lea_32_ai:		return parseOpcode_lea(PARAM_ai);
			case OpcodeType::OPCODE_lea_32_di:		return parseOpcode_lea(PARAM_di);
			case OpcodeType::OPCODE_lea_32_ix:		return parseOpcode_lea(PARAM_ix);
			case OpcodeType::OPCODE_lea_32_aw:		return parseOpcode_lea(PARAM_aw);
			case OpcodeType::OPCODE_lea_32_al:		return parseOpcode_lea(PARAM_al);
			case OpcodeType::OPCODE_lea_32_pcdi:	return parseOpcode_lea(PARAM_pcdi);
			case OpcodeType::OPCODE_lea_32_pcix:	return parseOpcode_lea(PARAM_pcix);

			// Push effective address
			case OpcodeType::OPCODE_pea_32_ai:		return parseOpcode_pea(PARAM_ai);
			case OpcodeType::OPCODE_pea_32_di:		return parseOpcode_pea(PARAM_di);
			case OpcodeType::OPCODE_pea_32_ix:		return parseOpcode_pea(PARAM_ix);
			case OpcodeType::OPCODE_pea_32_aw:		return parseOpcode_pea(PARAM_aw);
			case OpcodeType::OPCODE_pea_32_al:		return parseOpcode_pea(PARAM_al);
			case OpcodeType::OPCODE_pea_32_pcdi:	return parseOpcode_pea(PARAM_pcdi);
			case OpcodeType::OPCODE_pea_32_pcix:	return parseOpcode_pea(PARAM_pcix);

			// Test
			case OpcodeType::OPCODE_tst_8_d:		return parseOpcode_tst(DataType::Size::SIZE_8, PARAM_d);
			case OpcodeType::OPCODE_tst_8_ai:		return parseOpcode_tst(DataType::Size::SIZE_8, PARAM_ai);
			case OpcodeType::OPCODE_tst_8_pi:		return parseOpcode_tst(DataType::Size::SIZE_8, PARAM_pi);
			case OpcodeType::OPCODE_tst_8_pi7:		return parseOpcode_tst(DataType::Size::SIZE_8, PARAM_pi7);
			case OpcodeType::OPCODE_tst_8_pd:		return parseOpcode_tst(DataType::Size::SIZE_8, PARAM_pd);
			case OpcodeType::OPCODE_tst_8_pd7:		return parseOpcode_tst(DataType::Size::SIZE_8, PARAM_pd7);
			case OpcodeType::OPCODE_tst_8_di:		return parseOpcode_tst(DataType::Size::SIZE_8, PARAM_di);
			case OpcodeType::OPCODE_tst_8_ix:		return parseOpcode_tst(DataType::Size::SIZE_8, PARAM_ix);
			case OpcodeType::OPCODE_tst_8_aw:		return parseOpcode_tst(DataType::Size::SIZE_8, PARAM_aw);
			case OpcodeType::OPCODE_tst_8_al:		return parseOpcode_tst(DataType::Size::SIZE_8, PARAM_al);
			case OpcodeType::OPCODE_tst_16_d:		return parseOpcode_tst(DataType::Size::SIZE_16, PARAM_d);
			case OpcodeType::OPCODE_tst_16_ai:		return parseOpcode_tst(DataType::Size::SIZE_16, PARAM_ai);
			case OpcodeType::OPCODE_tst_16_pi:		return parseOpcode_tst(DataType::Size::SIZE_16, PARAM_pi);
			case OpcodeType::OPCODE_tst_16_pd:		return parseOpcode_tst(DataType::Size::SIZE_16, PARAM_pd);
			case OpcodeType::OPCODE_tst_16_di:		return parseOpcode_tst(DataType::Size::SIZE_16, PARAM_di);
			case OpcodeType::OPCODE_tst_16_ix:		return parseOpcode_tst(DataType::Size::SIZE_16, PARAM_ix);
			case OpcodeType::OPCODE_tst_16_aw:		return parseOpcode_tst(DataType::Size::SIZE_16, PARAM_aw);
			case OpcodeType::OPCODE_tst_16_al:		return parseOpcode_tst(DataType::Size::SIZE_16, PARAM_al);
			case OpcodeType::OPCODE_tst_32_d:		return parseOpcode_tst(DataType::Size::SIZE_32, PARAM_d);
			case OpcodeType::OPCODE_tst_32_ai:		return parseOpcode_tst(DataType::Size::SIZE_32, PARAM_ai);
			case OpcodeType::OPCODE_tst_32_pi:		return parseOpcode_tst(DataType::Size::SIZE_32, PARAM_pi);
			case OpcodeType::OPCODE_tst_32_pd:		return parseOpcode_tst(DataType::Size::SIZE_32, PARAM_pd);
			case OpcodeType::OPCODE_tst_32_di:		return parseOpcode_tst(DataType::Size::SIZE_32, PARAM_di);
			case OpcodeType::OPCODE_tst_32_ix:		return parseOpcode_tst(DataType::Size::SIZE_32, PARAM_ix);
			case OpcodeType::OPCODE_tst_32_aw:		return parseOpcode_tst(DataType::Size::SIZE_32, PARAM_aw);
			case OpcodeType::OPCODE_tst_32_al:		return parseOpcode_tst(DataType::Size::SIZE_32, PARAM_al);

			// Clear
			case OpcodeType::OPCODE_clr_8_d:		return parseOpcode_clr(DataType::Size::SIZE_8, PARAM_d);
			case OpcodeType::OPCODE_clr_8_ai:		return parseOpcode_clr(DataType::Size::SIZE_8, PARAM_ai);
			case OpcodeType::OPCODE_clr_8_pi:		return parseOpcode_clr(DataType::Size::SIZE_8, PARAM_pi);
			case OpcodeType::OPCODE_clr_8_pi7:		return parseOpcode_clr(DataType::Size::SIZE_8, PARAM_pi7);
			case OpcodeType::OPCODE_clr_8_pd:		return parseOpcode_clr(DataType::Size::SIZE_8, PARAM_pd);
			case OpcodeType::OPCODE_clr_8_pd7:		return parseOpcode_clr(DataType::Size::SIZE_8, PARAM_pd7);
			case OpcodeType::OPCODE_clr_8_di:		return parseOpcode_clr(DataType::Size::SIZE_8, PARAM_di);
			case OpcodeType::OPCODE_clr_8_ix:		return parseOpcode_clr(DataType::Size::SIZE_8, PARAM_ix);
			case OpcodeType::OPCODE_clr_8_aw:		return parseOpcode_clr(DataType::Size::SIZE_8, PARAM_aw);
			case OpcodeType::OPCODE_clr_8_al:		return parseOpcode_clr(DataType::Size::SIZE_8, PARAM_al);
			case OpcodeType::OPCODE_clr_16_d:		return parseOpcode_clr(DataType::Size::SIZE_16, PARAM_d);
			case OpcodeType::OPCODE_clr_16_ai:		return parseOpcode_clr(DataType::Size::SIZE_16, PARAM_ai);
			case OpcodeType::OPCODE_clr_16_pi:		return parseOpcode_clr(DataType::Size::SIZE_16, PARAM_pi);
			case OpcodeType::OPCODE_clr_16_pd:		return parseOpcode_clr(DataType::Size::SIZE_16, PARAM_pd);
			case OpcodeType::OPCODE_clr_16_di:		return parseOpcode_clr(DataType::Size::SIZE_16, PARAM_di);
			case OpcodeType::OPCODE_clr_16_ix:		return parseOpcode_clr(DataType::Size::SIZE_16, PARAM_ix);
			case OpcodeType::OPCODE_clr_16_aw:		return parseOpcode_clr(DataType::Size::SIZE_16, PARAM_aw);
			case OpcodeType::OPCODE_clr_16_al:		return parseOpcode_clr(DataType::Size::SIZE_16, PARAM_al);
			case OpcodeType::OPCODE_clr_32_d:		return parseOpcode_clr(DataType::Size::SIZE_32, PARAM_d);
			case OpcodeType::OPCODE_clr_32_ai:		return parseOpcode_clr(DataType::Size::SIZE_32, PARAM_ai);
			case OpcodeType::OPCODE_clr_32_pi:		return parseOpcode_clr(DataType::Size::SIZE_32, PARAM_pi);
			case OpcodeType::OPCODE_clr_32_pd:		return parseOpcode_clr(DataType::Size::SIZE_32, PARAM_pd);
			case OpcodeType::OPCODE_clr_32_di:		return parseOpcode_clr(DataType::Size::SIZE_32, PARAM_di);
			case OpcodeType::OPCODE_clr_32_ix:		return parseOpcode_clr(DataType::Size::SIZE_32, PARAM_ix);
			case OpcodeType::OPCODE_clr_32_aw:		return parseOpcode_clr(DataType::Size::SIZE_32, PARAM_aw);
			case OpcodeType::OPCODE_clr_32_al:		return parseOpcode_clr(DataType::Size::SIZE_32, PARAM_al);

			// Addition
			case OpcodeType::OPCODE_add_8_er_d:		return parseOpcode_add_er(DataType::Size::SIZE_8, PARAM_d);
			case OpcodeType::OPCODE_add_8_er_ai:	return parseOpcode_add_er(DataType::Size::SIZE_8, PARAM_ai);
			case OpcodeType::OPCODE_add_8_er_pi:	return parseOpcode_add_er(DataType::Size::SIZE_8, PARAM_pi);
			case OpcodeType::OPCODE_add_8_er_pi7:	return parseOpcode_add_er(DataType::Size::SIZE_8, PARAM_pi7);
			case OpcodeType::OPCODE_add_8_er_pd:	return parseOpcode_add_er(DataType::Size::SIZE_8, PARAM_pd);
			case OpcodeType::OPCODE_add_8_er_pd7:	return parseOpcode_add_er(DataType::Size::SIZE_8, PARAM_pd7);
			case OpcodeType::OPCODE_add_8_er_di:	return parseOpcode_add_er(DataType::Size::SIZE_8, PARAM_di);
			case OpcodeType::OPCODE_add_8_er_ix:	return parseOpcode_add_er(DataType::Size::SIZE_8, PARAM_ix);
			case OpcodeType::OPCODE_add_8_er_aw:	return parseOpcode_add_er(DataType::Size::SIZE_8, PARAM_aw);
			case OpcodeType::OPCODE_add_8_er_al:	return parseOpcode_add_er(DataType::Size::SIZE_8, PARAM_al);
			case OpcodeType::OPCODE_add_8_er_pcdi:	return parseOpcode_add_er(DataType::Size::SIZE_8, PARAM_pcdi);
			case OpcodeType::OPCODE_add_8_er_pcix:	return parseOpcode_add_er(DataType::Size::SIZE_8, PARAM_pcix);
			case OpcodeType::OPCODE_add_8_er_i:		return parseOpcode_add_er(DataType::Size::SIZE_8, PARAM_i);
			case OpcodeType::OPCODE_add_16_er_d:	return parseOpcode_add_er(DataType::Size::SIZE_16, PARAM_d);
			case OpcodeType::OPCODE_add_16_er_a:	return parseOpcode_add_er(DataType::Size::SIZE_16, PARAM_a);
			case OpcodeType::OPCODE_add_16_er_ai:	return parseOpcode_add_er(DataType::Size::SIZE_16, PARAM_ai);
			case OpcodeType::OPCODE_add_16_er_pi:	return parseOpcode_add_er(DataType::Size::SIZE_16, PARAM_pi);
			case OpcodeType::OPCODE_add_16_er_pd:	return parseOpcode_add_er(DataType::Size::SIZE_16, PARAM_pd);
			case OpcodeType::OPCODE_add_16_er_di:	return parseOpcode_add_er(DataType::Size::SIZE_16, PARAM_di);
			case OpcodeType::OPCODE_add_16_er_ix:	return parseOpcode_add_er(DataType::Size::SIZE_16, PARAM_ix);
			case OpcodeType::OPCODE_add_16_er_aw:	return parseOpcode_add_er(DataType::Size::SIZE_16, PARAM_aw);
			case OpcodeType::OPCODE_add_16_er_al:	return parseOpcode_add_er(DataType::Size::SIZE_16, PARAM_al);
			case OpcodeType::OPCODE_add_16_er_pcdi:	return parseOpcode_add_er(DataType::Size::SIZE_16, PARAM_pcdi);
			case OpcodeType::OPCODE_add_16_er_pcix:	return parseOpcode_add_er(DataType::Size::SIZE_16, PARAM_pcix);
			case OpcodeType::OPCODE_add_16_er_i:	return parseOpcode_add_er(DataType::Size::SIZE_16, PARAM_i);
			case OpcodeType::OPCODE_add_32_er_d:	return parseOpcode_add_er(DataType::Size::SIZE_32, PARAM_d);
			case OpcodeType::OPCODE_add_32_er_a:	return parseOpcode_add_er(DataType::Size::SIZE_32, PARAM_a);
			case OpcodeType::OPCODE_add_32_er_ai:	return parseOpcode_add_er(DataType::Size::SIZE_32, PARAM_ai);
			case OpcodeType::OPCODE_add_32_er_pi:	return parseOpcode_add_er(DataType::Size::SIZE_32, PARAM_pi);
			case OpcodeType::OPCODE_add_32_er_pd:	return parseOpcode_add_er(DataType::Size::SIZE_32, PARAM_pd);
			case OpcodeType::OPCODE_add_32_er_di:	return parseOpcode_add_er(DataType::Size::SIZE_32, PARAM_di);
			case OpcodeType::OPCODE_add_32_er_ix:	return parseOpcode_add_er(DataType::Size::SIZE_32, PARAM_ix);
			case OpcodeType::OPCODE_add_32_er_aw:	return parseOpcode_add_er(DataType::Size::SIZE_32, PARAM_aw);
			case OpcodeType::OPCODE_add_32_er_al:	return parseOpcode_add_er(DataType::Size::SIZE_32, PARAM_al);
			case OpcodeType::OPCODE_add_32_er_pcdi:	return parseOpcode_add_er(DataType::Size::SIZE_32, PARAM_pcdi);
			case OpcodeType::OPCODE_add_32_er_pcix:	return parseOpcode_add_er(DataType::Size::SIZE_32, PARAM_pcix);
			case OpcodeType::OPCODE_add_32_er_i:	return parseOpcode_add_er(DataType::Size::SIZE_32, PARAM_i);
			case OpcodeType::OPCODE_add_8_re_ai:	return parseOpcode_add_re(DataType::Size::SIZE_8, PARAM_ai);
			case OpcodeType::OPCODE_add_8_re_pi:	return parseOpcode_add_re(DataType::Size::SIZE_8, PARAM_pi);
			case OpcodeType::OPCODE_add_8_re_pi7:	return parseOpcode_add_re(DataType::Size::SIZE_8, PARAM_pi7);
			case OpcodeType::OPCODE_add_8_re_pd:	return parseOpcode_add_re(DataType::Size::SIZE_8, PARAM_pd);
			case OpcodeType::OPCODE_add_8_re_pd7:	return parseOpcode_add_re(DataType::Size::SIZE_8, PARAM_pd7);
			case OpcodeType::OPCODE_add_8_re_di:	return parseOpcode_add_re(DataType::Size::SIZE_8, PARAM_di);
			case OpcodeType::OPCODE_add_8_re_ix:	return parseOpcode_add_re(DataType::Size::SIZE_8, PARAM_ix);
			case OpcodeType::OPCODE_add_8_re_aw:	return parseOpcode_add_re(DataType::Size::SIZE_8, PARAM_aw);
			case OpcodeType::OPCODE_add_8_re_al:	return parseOpcode_add_re(DataType::Size::SIZE_8, PARAM_al);
			case OpcodeType::OPCODE_add_16_re_ai:	return parseOpcode_add_re(DataType::Size::SIZE_16, PARAM_ai);
			case OpcodeType::OPCODE_add_16_re_pi:	return parseOpcode_add_re(DataType::Size::SIZE_16, PARAM_pi);
			case OpcodeType::OPCODE_add_16_re_pd:	return parseOpcode_add_re(DataType::Size::SIZE_16, PARAM_pd);
			case OpcodeType::OPCODE_add_16_re_di:	return parseOpcode_add_re(DataType::Size::SIZE_16, PARAM_di);
			case OpcodeType::OPCODE_add_16_re_ix:	return parseOpcode_add_re(DataType::Size::SIZE_16, PARAM_ix);
			case OpcodeType::OPCODE_add_16_re_aw:	return parseOpcode_add_re(DataType::Size::SIZE_16, PARAM_aw);
			case OpcodeType::OPCODE_add_16_re_al:	return parseOpcode_add_re(DataType::Size::SIZE_16, PARAM_al);
			case OpcodeType::OPCODE_add_32_re_ai:	return parseOpcode_add_re(DataType::Size::SIZE_32, PARAM_ai);
			case OpcodeType::OPCODE_add_32_re_pi:	return parseOpcode_add_re(DataType::Size::SIZE_32, PARAM_pi);
			case OpcodeType::OPCODE_add_32_re_pd:	return parseOpcode_add_re(DataType::Size::SIZE_32, PARAM_pd);
			case OpcodeType::OPCODE_add_32_re_di:	return parseOpcode_add_re(DataType::Size::SIZE_32, PARAM_di);
			case OpcodeType::OPCODE_add_32_re_ix:	return parseOpcode_add_re(DataType::Size::SIZE_32, PARAM_ix);
			case OpcodeType::OPCODE_add_32_re_aw:	return parseOpcode_add_re(DataType::Size::SIZE_32, PARAM_aw);
			case OpcodeType::OPCODE_add_32_re_al:	return parseOpcode_add_re(DataType::Size::SIZE_32, PARAM_al);

			case OpcodeType::OPCODE_adda_16_d:		return parseOpcode_adda(DataType::Size::SIZE_16, PARAM_d);
			case OpcodeType::OPCODE_adda_16_a:		return parseOpcode_adda(DataType::Size::SIZE_16, PARAM_a);
			case OpcodeType::OPCODE_adda_16_ai:		return parseOpcode_adda(DataType::Size::SIZE_16, PARAM_ai);
			case OpcodeType::OPCODE_adda_16_pi:		return parseOpcode_adda(DataType::Size::SIZE_16, PARAM_pi);
			case OpcodeType::OPCODE_adda_16_pd:		return parseOpcode_adda(DataType::Size::SIZE_16, PARAM_pd);
			case OpcodeType::OPCODE_adda_16_di:		return parseOpcode_adda(DataType::Size::SIZE_16, PARAM_di);
			case OpcodeType::OPCODE_adda_16_ix:		return parseOpcode_adda(DataType::Size::SIZE_16, PARAM_ix);
			case OpcodeType::OPCODE_adda_16_aw:		return parseOpcode_adda(DataType::Size::SIZE_16, PARAM_aw);
			case OpcodeType::OPCODE_adda_16_al:		return parseOpcode_adda(DataType::Size::SIZE_16, PARAM_al);
			case OpcodeType::OPCODE_adda_16_pcdi:	return parseOpcode_adda(DataType::Size::SIZE_16, PARAM_pcdi);
			case OpcodeType::OPCODE_adda_16_pcix:	return parseOpcode_adda(DataType::Size::SIZE_16, PARAM_pcix);
			case OpcodeType::OPCODE_adda_16_i:		return parseOpcode_adda(DataType::Size::SIZE_16, PARAM_i);
			case OpcodeType::OPCODE_adda_32_d:		return parseOpcode_adda(DataType::Size::SIZE_32, PARAM_d);
			case OpcodeType::OPCODE_adda_32_a:		return parseOpcode_adda(DataType::Size::SIZE_32, PARAM_a);
			case OpcodeType::OPCODE_adda_32_ai:		return parseOpcode_adda(DataType::Size::SIZE_32, PARAM_ai);
			case OpcodeType::OPCODE_adda_32_pi:		return parseOpcode_adda(DataType::Size::SIZE_32, PARAM_pi);
			case OpcodeType::OPCODE_adda_32_pd:		return parseOpcode_adda(DataType::Size::SIZE_32, PARAM_pd);
			case OpcodeType::OPCODE_adda_32_di:		return parseOpcode_adda(DataType::Size::SIZE_32, PARAM_di);
			case OpcodeType::OPCODE_adda_32_ix:		return parseOpcode_adda(DataType::Size::SIZE_32, PARAM_ix);
			case OpcodeType::OPCODE_adda_32_aw:		return parseOpcode_adda(DataType::Size::SIZE_32, PARAM_aw);
			case OpcodeType::OPCODE_adda_32_al:		return parseOpcode_adda(DataType::Size::SIZE_32, PARAM_al);
			case OpcodeType::OPCODE_adda_32_pcdi:	return parseOpcode_adda(DataType::Size::SIZE_32, PARAM_pcdi);
			case OpcodeType::OPCODE_adda_32_pcix:	return parseOpcode_adda(DataType::Size::SIZE_32, PARAM_pcix);
			case OpcodeType::OPCODE_adda_32_i:		return parseOpcode_adda(DataType::Size::SIZE_32, PARAM_i);

			case OpcodeType::OPCODE_addi_8_d:		return parseOpcode_addi(DataType::Size::SIZE_8, PARAM_d);
			case OpcodeType::OPCODE_addi_8_ai:		return parseOpcode_addi(DataType::Size::SIZE_8, PARAM_ai);
			case OpcodeType::OPCODE_addi_8_pi:		return parseOpcode_addi(DataType::Size::SIZE_8, PARAM_pi);
			case OpcodeType::OPCODE_addi_8_pi7:		return parseOpcode_addi(DataType::Size::SIZE_8, PARAM_pi7);
			case OpcodeType::OPCODE_addi_8_pd:		return parseOpcode_addi(DataType::Size::SIZE_8, PARAM_pd);
			case OpcodeType::OPCODE_addi_8_pd7:		return parseOpcode_addi(DataType::Size::SIZE_8, PARAM_pd7);
			case OpcodeType::OPCODE_addi_8_di:		return parseOpcode_addi(DataType::Size::SIZE_8, PARAM_di);
			case OpcodeType::OPCODE_addi_8_ix:		return parseOpcode_addi(DataType::Size::SIZE_8, PARAM_ix);
			case OpcodeType::OPCODE_addi_8_aw:		return parseOpcode_addi(DataType::Size::SIZE_8, PARAM_aw);
			case OpcodeType::OPCODE_addi_8_al:		return parseOpcode_addi(DataType::Size::SIZE_8, PARAM_al);
			case OpcodeType::OPCODE_addi_16_d:		return parseOpcode_addi(DataType::Size::SIZE_16, PARAM_d);
			case OpcodeType::OPCODE_addi_16_ai:		return parseOpcode_addi(DataType::Size::SIZE_16, PARAM_ai);
			case OpcodeType::OPCODE_addi_16_pi:		return parseOpcode_addi(DataType::Size::SIZE_16, PARAM_pi);
			case OpcodeType::OPCODE_addi_16_pd:		return parseOpcode_addi(DataType::Size::SIZE_16, PARAM_pd);
			case OpcodeType::OPCODE_addi_16_di:		return parseOpcode_addi(DataType::Size::SIZE_16, PARAM_di);
			case OpcodeType::OPCODE_addi_16_ix:		return parseOpcode_addi(DataType::Size::SIZE_16, PARAM_ix);
			case OpcodeType::OPCODE_addi_16_aw:		return parseOpcode_addi(DataType::Size::SIZE_16, PARAM_aw);
			case OpcodeType::OPCODE_addi_16_al:		return parseOpcode_addi(DataType::Size::SIZE_16, PARAM_al);
			case OpcodeType::OPCODE_addi_32_d:		return parseOpcode_addi(DataType::Size::SIZE_32, PARAM_d);
			case OpcodeType::OPCODE_addi_32_ai:		return parseOpcode_addi(DataType::Size::SIZE_32, PARAM_ai);
			case OpcodeType::OPCODE_addi_32_pi:		return parseOpcode_addi(DataType::Size::SIZE_32, PARAM_pi);
			case OpcodeType::OPCODE_addi_32_pd:		return parseOpcode_addi(DataType::Size::SIZE_32, PARAM_pd);
			case OpcodeType::OPCODE_addi_32_di:		return parseOpcode_addi(DataType::Size::SIZE_32, PARAM_di);
			case OpcodeType::OPCODE_addi_32_ix:		return parseOpcode_addi(DataType::Size::SIZE_32, PARAM_ix);
			case OpcodeType::OPCODE_addi_32_aw:		return parseOpcode_addi(DataType::Size::SIZE_32, PARAM_aw);
			case OpcodeType::OPCODE_addi_32_al:		return parseOpcode_addi(DataType::Size::SIZE_32, PARAM_al);

			case OpcodeType::OPCODE_addq_8_d:		return parseOpcode_addq(DataType::Size::SIZE_8, PARAM_d);
			case OpcodeType::OPCODE_addq_8_ai:		return parseOpcode_addq(DataType::Size::SIZE_8, PARAM_ai);
			case OpcodeType::OPCODE_addq_8_pi:		return parseOpcode_addq(DataType::Size::SIZE_8, PARAM_pi);
			case OpcodeType::OPCODE_addq_8_pi7:		return parseOpcode_addq(DataType::Size::SIZE_8, PARAM_pi7);
			case OpcodeType::OPCODE_addq_8_pd:		return parseOpcode_addq(DataType::Size::SIZE_8, PARAM_pd);
			case OpcodeType::OPCODE_addq_8_pd7:		return parseOpcode_addq(DataType::Size::SIZE_8, PARAM_pd7);
			case OpcodeType::OPCODE_addq_8_di:		return parseOpcode_addq(DataType::Size::SIZE_8, PARAM_di);
			case OpcodeType::OPCODE_addq_8_ix:		return parseOpcode_addq(DataType::Size::SIZE_8, PARAM_ix);
			case OpcodeType::OPCODE_addq_8_aw:		return parseOpcode_addq(DataType::Size::SIZE_8, PARAM_aw);
			case OpcodeType::OPCODE_addq_8_al:		return parseOpcode_addq(DataType::Size::SIZE_8, PARAM_al);
			case OpcodeType::OPCODE_addq_16_d:		return parseOpcode_addq(DataType::Size::SIZE_16, PARAM_d);
			case OpcodeType::OPCODE_addq_16_a:		return parseOpcode_addq(DataType::Size::SIZE_32, PARAM_a, false);		// Special case
			case OpcodeType::OPCODE_addq_16_ai:		return parseOpcode_addq(DataType::Size::SIZE_16, PARAM_ai);
			case OpcodeType::OPCODE_addq_16_pi:		return parseOpcode_addq(DataType::Size::SIZE_16, PARAM_pi);
			case OpcodeType::OPCODE_addq_16_pd:		return parseOpcode_addq(DataType::Size::SIZE_16, PARAM_pd);
			case OpcodeType::OPCODE_addq_16_di:		return parseOpcode_addq(DataType::Size::SIZE_16, PARAM_di);
			case OpcodeType::OPCODE_addq_16_ix:		return parseOpcode_addq(DataType::Size::SIZE_16, PARAM_ix);
			case OpcodeType::OPCODE_addq_16_aw:		return parseOpcode_addq(DataType::Size::SIZE_16, PARAM_aw);
			case OpcodeType::OPCODE_addq_16_al:		return parseOpcode_addq(DataType::Size::SIZE_16, PARAM_al);
			case OpcodeType::OPCODE_addq_32_d:		return parseOpcode_addq(DataType::Size::SIZE_32, PARAM_d);
			case OpcodeType::OPCODE_addq_32_a:		return parseOpcode_addq(DataType::Size::SIZE_32, PARAM_a, false);		// Special case
			case OpcodeType::OPCODE_addq_32_ai:		return parseOpcode_addq(DataType::Size::SIZE_32, PARAM_ai);
			case OpcodeType::OPCODE_addq_32_pi:		return parseOpcode_addq(DataType::Size::SIZE_32, PARAM_pi);
			case OpcodeType::OPCODE_addq_32_pd:		return parseOpcode_addq(DataType::Size::SIZE_32, PARAM_pd);
			case OpcodeType::OPCODE_addq_32_di:		return parseOpcode_addq(DataType::Size::SIZE_32, PARAM_di);
			case OpcodeType::OPCODE_addq_32_ix:		return parseOpcode_addq(DataType::Size::SIZE_32, PARAM_ix);
			case OpcodeType::OPCODE_addq_32_aw:		return parseOpcode_addq(DataType::Size::SIZE_32, PARAM_aw);
			case OpcodeType::OPCODE_addq_32_al:		return parseOpcode_addq(DataType::Size::SIZE_32, PARAM_al);

			case OpcodeType::OPCODE_addx_8_rr:		return parseOpcode_addx_rr(DataType::Size::SIZE_8);
			case OpcodeType::OPCODE_addx_16_rr:		return parseOpcode_addx_rr(DataType::Size::SIZE_16);
			case OpcodeType::OPCODE_addx_32_rr:		return parseOpcode_addx_rr(DataType::Size::SIZE_32);

			// Subtraction
			case OpcodeType::OPCODE_sub_8_er_d:		return parseOpcode_sub_er(DataType::Size::SIZE_8, PARAM_d);
			case OpcodeType::OPCODE_sub_8_er_ai:	return parseOpcode_sub_er(DataType::Size::SIZE_8, PARAM_ai);
			case OpcodeType::OPCODE_sub_8_er_pi:	return parseOpcode_sub_er(DataType::Size::SIZE_8, PARAM_pi);
			case OpcodeType::OPCODE_sub_8_er_pi7:	return parseOpcode_sub_er(DataType::Size::SIZE_8, PARAM_pi7);
			case OpcodeType::OPCODE_sub_8_er_pd:	return parseOpcode_sub_er(DataType::Size::SIZE_8, PARAM_pd);
			case OpcodeType::OPCODE_sub_8_er_pd7:	return parseOpcode_sub_er(DataType::Size::SIZE_8, PARAM_pd7);
			case OpcodeType::OPCODE_sub_8_er_di:	return parseOpcode_sub_er(DataType::Size::SIZE_8, PARAM_di);
			case OpcodeType::OPCODE_sub_8_er_ix:	return parseOpcode_sub_er(DataType::Size::SIZE_8, PARAM_ix);
			case OpcodeType::OPCODE_sub_8_er_aw:	return parseOpcode_sub_er(DataType::Size::SIZE_8, PARAM_aw);
			case OpcodeType::OPCODE_sub_8_er_al:	return parseOpcode_sub_er(DataType::Size::SIZE_8, PARAM_al);
			case OpcodeType::OPCODE_sub_8_er_pcdi:	return parseOpcode_sub_er(DataType::Size::SIZE_8, PARAM_pcdi);
			case OpcodeType::OPCODE_sub_8_er_pcix:	return parseOpcode_sub_er(DataType::Size::SIZE_8, PARAM_pcix);
			case OpcodeType::OPCODE_sub_8_er_i:		return parseOpcode_sub_er(DataType::Size::SIZE_8, PARAM_i);
			case OpcodeType::OPCODE_sub_16_er_d:	return parseOpcode_sub_er(DataType::Size::SIZE_16, PARAM_d);
			case OpcodeType::OPCODE_sub_16_er_a:	return parseOpcode_sub_er(DataType::Size::SIZE_16, PARAM_a);
			case OpcodeType::OPCODE_sub_16_er_ai:	return parseOpcode_sub_er(DataType::Size::SIZE_16, PARAM_ai);
			case OpcodeType::OPCODE_sub_16_er_pi:	return parseOpcode_sub_er(DataType::Size::SIZE_16, PARAM_pi);
			case OpcodeType::OPCODE_sub_16_er_pd:	return parseOpcode_sub_er(DataType::Size::SIZE_16, PARAM_pd);
			case OpcodeType::OPCODE_sub_16_er_di:	return parseOpcode_sub_er(DataType::Size::SIZE_16, PARAM_di);
			case OpcodeType::OPCODE_sub_16_er_ix:	return parseOpcode_sub_er(DataType::Size::SIZE_16, PARAM_ix);
			case OpcodeType::OPCODE_sub_16_er_aw:	return parseOpcode_sub_er(DataType::Size::SIZE_16, PARAM_aw);
			case OpcodeType::OPCODE_sub_16_er_al:	return parseOpcode_sub_er(DataType::Size::SIZE_16, PARAM_al);
			case OpcodeType::OPCODE_sub_16_er_pcdi:	return parseOpcode_sub_er(DataType::Size::SIZE_16, PARAM_pcdi);
			case OpcodeType::OPCODE_sub_16_er_pcix:	return parseOpcode_sub_er(DataType::Size::SIZE_16, PARAM_pcix);
			case OpcodeType::OPCODE_sub_16_er_i:	return parseOpcode_sub_er(DataType::Size::SIZE_16, PARAM_i);
			case OpcodeType::OPCODE_sub_32_er_d:	return parseOpcode_sub_er(DataType::Size::SIZE_32, PARAM_d);
			case OpcodeType::OPCODE_sub_32_er_a:	return parseOpcode_sub_er(DataType::Size::SIZE_32, PARAM_a);
			case OpcodeType::OPCODE_sub_32_er_ai:	return parseOpcode_sub_er(DataType::Size::SIZE_32, PARAM_ai);
			case OpcodeType::OPCODE_sub_32_er_pi:	return parseOpcode_sub_er(DataType::Size::SIZE_32, PARAM_pi);
			case OpcodeType::OPCODE_sub_32_er_pd:	return parseOpcode_sub_er(DataType::Size::SIZE_32, PARAM_pd);
			case OpcodeType::OPCODE_sub_32_er_di:	return parseOpcode_sub_er(DataType::Size::SIZE_32, PARAM_di);
			case OpcodeType::OPCODE_sub_32_er_ix:	return parseOpcode_sub_er(DataType::Size::SIZE_32, PARAM_ix);
			case OpcodeType::OPCODE_sub_32_er_aw:	return parseOpcode_sub_er(DataType::Size::SIZE_32, PARAM_aw);
			case OpcodeType::OPCODE_sub_32_er_al:	return parseOpcode_sub_er(DataType::Size::SIZE_32, PARAM_al);
			case OpcodeType::OPCODE_sub_32_er_pcdi:	return parseOpcode_sub_er(DataType::Size::SIZE_32, PARAM_pcdi);
			case OpcodeType::OPCODE_sub_32_er_pcix:	return parseOpcode_sub_er(DataType::Size::SIZE_32, PARAM_pcix);
			case OpcodeType::OPCODE_sub_32_er_i:	return parseOpcode_sub_er(DataType::Size::SIZE_32, PARAM_i);
			case OpcodeType::OPCODE_sub_8_re_ai:	return parseOpcode_sub_re(DataType::Size::SIZE_8, PARAM_ai);
			case OpcodeType::OPCODE_sub_8_re_pi:	return parseOpcode_sub_re(DataType::Size::SIZE_8, PARAM_pi);
			case OpcodeType::OPCODE_sub_8_re_pi7:	return parseOpcode_sub_re(DataType::Size::SIZE_8, PARAM_pi7);
			case OpcodeType::OPCODE_sub_8_re_pd:	return parseOpcode_sub_re(DataType::Size::SIZE_8, PARAM_pd);
			case OpcodeType::OPCODE_sub_8_re_pd7:	return parseOpcode_sub_re(DataType::Size::SIZE_8, PARAM_pd7);
			case OpcodeType::OPCODE_sub_8_re_di:	return parseOpcode_sub_re(DataType::Size::SIZE_8, PARAM_di);
			case OpcodeType::OPCODE_sub_8_re_ix:	return parseOpcode_sub_re(DataType::Size::SIZE_8, PARAM_ix);
			case OpcodeType::OPCODE_sub_8_re_aw:	return parseOpcode_sub_re(DataType::Size::SIZE_8, PARAM_aw);
			case OpcodeType::OPCODE_sub_8_re_al:	return parseOpcode_sub_re(DataType::Size::SIZE_8, PARAM_al);
			case OpcodeType::OPCODE_sub_16_re_ai:	return parseOpcode_sub_re(DataType::Size::SIZE_16, PARAM_ai);
			case OpcodeType::OPCODE_sub_16_re_pi:	return parseOpcode_sub_re(DataType::Size::SIZE_16, PARAM_pi);
			case OpcodeType::OPCODE_sub_16_re_pd:	return parseOpcode_sub_re(DataType::Size::SIZE_16, PARAM_pd);
			case OpcodeType::OPCODE_sub_16_re_di:	return parseOpcode_sub_re(DataType::Size::SIZE_16, PARAM_di);
			case OpcodeType::OPCODE_sub_16_re_ix:	return parseOpcode_sub_re(DataType::Size::SIZE_16, PARAM_ix);
			case OpcodeType::OPCODE_sub_16_re_aw:	return parseOpcode_sub_re(DataType::Size::SIZE_16, PARAM_aw);
			case OpcodeType::OPCODE_sub_16_re_al:	return parseOpcode_sub_re(DataType::Size::SIZE_16, PARAM_al);
			case OpcodeType::OPCODE_sub_32_re_ai:	return parseOpcode_sub_re(DataType::Size::SIZE_32, PARAM_ai);
			case OpcodeType::OPCODE_sub_32_re_pi:	return parseOpcode_sub_re(DataType::Size::SIZE_32, PARAM_pi);
			case OpcodeType::OPCODE_sub_32_re_pd:	return parseOpcode_sub_re(DataType::Size::SIZE_32, PARAM_pd);
			case OpcodeType::OPCODE_sub_32_re_di:	return parseOpcode_sub_re(DataType::Size::SIZE_32, PARAM_di);
			case OpcodeType::OPCODE_sub_32_re_ix:	return parseOpcode_sub_re(DataType::Size::SIZE_32, PARAM_ix);
			case OpcodeType::OPCODE_sub_32_re_aw:	return parseOpcode_sub_re(DataType::Size::SIZE_32, PARAM_aw);
			case OpcodeType::OPCODE_sub_32_re_al:	return parseOpcode_sub_re(DataType::Size::SIZE_32, PARAM_al);

			case OpcodeType::OPCODE_suba_16_d:		return parseOpcode_suba(DataType::Size::SIZE_16, PARAM_d);
			case OpcodeType::OPCODE_suba_16_a:		return parseOpcode_suba(DataType::Size::SIZE_16, PARAM_a);
			case OpcodeType::OPCODE_suba_16_ai:		return parseOpcode_suba(DataType::Size::SIZE_16, PARAM_ai);
			case OpcodeType::OPCODE_suba_16_pi:		return parseOpcode_suba(DataType::Size::SIZE_16, PARAM_pi);
			case OpcodeType::OPCODE_suba_16_pd:		return parseOpcode_suba(DataType::Size::SIZE_16, PARAM_pd);
			case OpcodeType::OPCODE_suba_16_di:		return parseOpcode_suba(DataType::Size::SIZE_16, PARAM_di);
			case OpcodeType::OPCODE_suba_16_ix:		return parseOpcode_suba(DataType::Size::SIZE_16, PARAM_ix);
			case OpcodeType::OPCODE_suba_16_aw:		return parseOpcode_suba(DataType::Size::SIZE_16, PARAM_aw);
			case OpcodeType::OPCODE_suba_16_al:		return parseOpcode_suba(DataType::Size::SIZE_16, PARAM_al);
			case OpcodeType::OPCODE_suba_16_pcdi:	return parseOpcode_suba(DataType::Size::SIZE_16, PARAM_pcdi);
			case OpcodeType::OPCODE_suba_16_pcix:	return parseOpcode_suba(DataType::Size::SIZE_16, PARAM_pcix);
			case OpcodeType::OPCODE_suba_16_i:		return parseOpcode_suba(DataType::Size::SIZE_16, PARAM_i);
			case OpcodeType::OPCODE_suba_32_d:		return parseOpcode_suba(DataType::Size::SIZE_32, PARAM_d);
			case OpcodeType::OPCODE_suba_32_a:		return parseOpcode_suba(DataType::Size::SIZE_32, PARAM_a);
			case OpcodeType::OPCODE_suba_32_ai:		return parseOpcode_suba(DataType::Size::SIZE_32, PARAM_ai);
			case OpcodeType::OPCODE_suba_32_pi:		return parseOpcode_suba(DataType::Size::SIZE_32, PARAM_pi);
			case OpcodeType::OPCODE_suba_32_pd:		return parseOpcode_suba(DataType::Size::SIZE_32, PARAM_pd);
			case OpcodeType::OPCODE_suba_32_di:		return parseOpcode_suba(DataType::Size::SIZE_32, PARAM_di);
			case OpcodeType::OPCODE_suba_32_ix:		return parseOpcode_suba(DataType::Size::SIZE_32, PARAM_ix);
			case OpcodeType::OPCODE_suba_32_aw:		return parseOpcode_suba(DataType::Size::SIZE_32, PARAM_aw);
			case OpcodeType::OPCODE_suba_32_al:		return parseOpcode_suba(DataType::Size::SIZE_32, PARAM_al);
			case OpcodeType::OPCODE_suba_32_pcdi:	return parseOpcode_suba(DataType::Size::SIZE_32, PARAM_pcdi);
			case OpcodeType::OPCODE_suba_32_pcix:	return parseOpcode_suba(DataType::Size::SIZE_32, PARAM_pcix);
			case OpcodeType::OPCODE_suba_32_i:		return parseOpcode_suba(DataType::Size::SIZE_32, PARAM_i);

			case OpcodeType::OPCODE_subi_8_d:		return parseOpcode_subi(DataType::Size::SIZE_8, PARAM_d);
			case OpcodeType::OPCODE_subi_8_ai:		return parseOpcode_subi(DataType::Size::SIZE_8, PARAM_ai);
			case OpcodeType::OPCODE_subi_8_pi:		return parseOpcode_subi(DataType::Size::SIZE_8, PARAM_pi);
			case OpcodeType::OPCODE_subi_8_pi7:		return parseOpcode_subi(DataType::Size::SIZE_8, PARAM_pi7);
			case OpcodeType::OPCODE_subi_8_pd:		return parseOpcode_subi(DataType::Size::SIZE_8, PARAM_pd);
			case OpcodeType::OPCODE_subi_8_pd7:		return parseOpcode_subi(DataType::Size::SIZE_8, PARAM_pd7);
			case OpcodeType::OPCODE_subi_8_di:		return parseOpcode_subi(DataType::Size::SIZE_8, PARAM_di);
			case OpcodeType::OPCODE_subi_8_ix:		return parseOpcode_subi(DataType::Size::SIZE_8, PARAM_ix);
			case OpcodeType::OPCODE_subi_8_aw:		return parseOpcode_subi(DataType::Size::SIZE_8, PARAM_aw);
			case OpcodeType::OPCODE_subi_8_al:		return parseOpcode_subi(DataType::Size::SIZE_8, PARAM_al);
			case OpcodeType::OPCODE_subi_16_d:		return parseOpcode_subi(DataType::Size::SIZE_16, PARAM_d);
			case OpcodeType::OPCODE_subi_16_ai:		return parseOpcode_subi(DataType::Size::SIZE_16, PARAM_ai);
			case OpcodeType::OPCODE_subi_16_pi:		return parseOpcode_subi(DataType::Size::SIZE_16, PARAM_pi);
			case OpcodeType::OPCODE_subi_16_pd:		return parseOpcode_subi(DataType::Size::SIZE_16, PARAM_pd);
			case OpcodeType::OPCODE_subi_16_di:		return parseOpcode_subi(DataType::Size::SIZE_16, PARAM_di);
			case OpcodeType::OPCODE_subi_16_ix:		return parseOpcode_subi(DataType::Size::SIZE_16, PARAM_ix);
			case OpcodeType::OPCODE_subi_16_aw:		return parseOpcode_subi(DataType::Size::SIZE_16, PARAM_aw);
			case OpcodeType::OPCODE_subi_16_al:		return parseOpcode_subi(DataType::Size::SIZE_16, PARAM_al);
			case OpcodeType::OPCODE_subi_32_d:		return parseOpcode_subi(DataType::Size::SIZE_32, PARAM_d);
			case OpcodeType::OPCODE_subi_32_ai:		return parseOpcode_subi(DataType::Size::SIZE_32, PARAM_ai);
			case OpcodeType::OPCODE_subi_32_pi:		return parseOpcode_subi(DataType::Size::SIZE_32, PARAM_pi);
			case OpcodeType::OPCODE_subi_32_pd:		return parseOpcode_subi(DataType::Size::SIZE_32, PARAM_pd);
			case OpcodeType::OPCODE_subi_32_di:		return parseOpcode_subi(DataType::Size::SIZE_32, PARAM_di);
			case OpcodeType::OPCODE_subi_32_ix:		return parseOpcode_subi(DataType::Size::SIZE_32, PARAM_ix);
			case OpcodeType::OPCODE_subi_32_aw:		return parseOpcode_subi(DataType::Size::SIZE_32, PARAM_aw);
			case OpcodeType::OPCODE_subi_32_al:		return parseOpcode_subi(DataType::Size::SIZE_32, PARAM_al);

			case OpcodeType::OPCODE_subq_8_d:		return parseOpcode_subq(DataType::Size::SIZE_8, PARAM_d);
			case OpcodeType::OPCODE_subq_8_ai:		return parseOpcode_subq(DataType::Size::SIZE_8, PARAM_ai);
			case OpcodeType::OPCODE_subq_8_pi:		return parseOpcode_subq(DataType::Size::SIZE_8, PARAM_pi);
			case OpcodeType::OPCODE_subq_8_pi7:		return parseOpcode_subq(DataType::Size::SIZE_8, PARAM_pi7);
			case OpcodeType::OPCODE_subq_8_pd:		return parseOpcode_subq(DataType::Size::SIZE_8, PARAM_pd);
			case OpcodeType::OPCODE_subq_8_pd7:		return parseOpcode_subq(DataType::Size::SIZE_8, PARAM_pd7);
			case OpcodeType::OPCODE_subq_8_di:		return parseOpcode_subq(DataType::Size::SIZE_8, PARAM_di);
			case OpcodeType::OPCODE_subq_8_ix:		return parseOpcode_subq(DataType::Size::SIZE_8, PARAM_ix);
			case OpcodeType::OPCODE_subq_8_aw:		return parseOpcode_subq(DataType::Size::SIZE_8, PARAM_aw);
			case OpcodeType::OPCODE_subq_8_al:		return parseOpcode_subq(DataType::Size::SIZE_8, PARAM_al);
			case OpcodeType::OPCODE_subq_16_d:		return parseOpcode_subq(DataType::Size::SIZE_16, PARAM_d);
			case OpcodeType::OPCODE_subq_16_a:		return parseOpcode_subq(DataType::Size::SIZE_32, PARAM_a, false);		// Special case
			case OpcodeType::OPCODE_subq_16_ai:		return parseOpcode_subq(DataType::Size::SIZE_16, PARAM_ai);
			case OpcodeType::OPCODE_subq_16_pi:		return parseOpcode_subq(DataType::Size::SIZE_16, PARAM_pi);
			case OpcodeType::OPCODE_subq_16_pd:		return parseOpcode_subq(DataType::Size::SIZE_16, PARAM_pd);
			case OpcodeType::OPCODE_subq_16_di:		return parseOpcode_subq(DataType::Size::SIZE_16, PARAM_di);
			case OpcodeType::OPCODE_subq_16_ix:		return parseOpcode_subq(DataType::Size::SIZE_16, PARAM_ix);
			case OpcodeType::OPCODE_subq_16_aw:		return parseOpcode_subq(DataType::Size::SIZE_16, PARAM_aw);
			case OpcodeType::OPCODE_subq_16_al:		return parseOpcode_subq(DataType::Size::SIZE_16, PARAM_al);
			case OpcodeType::OPCODE_subq_32_d:		return parseOpcode_subq(DataType::Size::SIZE_32, PARAM_d);
			case OpcodeType::OPCODE_subq_32_a:		return parseOpcode_subq(DataType::Size::SIZE_32, PARAM_a, false);		// Special case
			case OpcodeType::OPCODE_subq_32_ai:		return parseOpcode_subq(DataType::Size::SIZE_32, PARAM_ai);
			case OpcodeType::OPCODE_subq_32_pi:		return parseOpcode_subq(DataType::Size::SIZE_32, PARAM_pi);
			case OpcodeType::OPCODE_subq_32_pd:		return parseOpcode_subq(DataType::Size::SIZE_32, PARAM_pd);
			case OpcodeType::OPCODE_subq_32_di:		return parseOpcode_subq(DataType::Size::SIZE_32, PARAM_di);
			case OpcodeType::OPCODE_subq_32_ix:		return parseOpcode_subq(DataType::Size::SIZE_32, PARAM_ix);
			case OpcodeType::OPCODE_subq_32_aw:		return parseOpcode_subq(DataType::Size::SIZE_32, PARAM_aw);
			case OpcodeType::OPCODE_subq_32_al:		return parseOpcode_subq(DataType::Size::SIZE_32, PARAM_al);

			case OpcodeType::OPCODE_subx_8_rr:		return parseOpcode_subx_rr(DataType::Size::SIZE_8);
			case OpcodeType::OPCODE_subx_16_rr:		return parseOpcode_subx_rr(DataType::Size::SIZE_16);
			case OpcodeType::OPCODE_subx_32_rr:		return parseOpcode_subx_rr(DataType::Size::SIZE_32);

			// Comparison
			case OpcodeType::OPCODE_cmp_8_d:		return parseOpcode_cmp(DataType::Size::SIZE_8, PARAM_d);
			case OpcodeType::OPCODE_cmp_8_ai:		return parseOpcode_cmp(DataType::Size::SIZE_8, PARAM_ai);
			case OpcodeType::OPCODE_cmp_8_pi:		return parseOpcode_cmp(DataType::Size::SIZE_8, PARAM_pi);
			case OpcodeType::OPCODE_cmp_8_pi7:		return parseOpcode_cmp(DataType::Size::SIZE_8, PARAM_pi7);
			case OpcodeType::OPCODE_cmp_8_pd:		return parseOpcode_cmp(DataType::Size::SIZE_8, PARAM_pd);
			case OpcodeType::OPCODE_cmp_8_pd7:		return parseOpcode_cmp(DataType::Size::SIZE_8, PARAM_pd7);
			case OpcodeType::OPCODE_cmp_8_di:		return parseOpcode_cmp(DataType::Size::SIZE_8, PARAM_di);
			case OpcodeType::OPCODE_cmp_8_ix:		return parseOpcode_cmp(DataType::Size::SIZE_8, PARAM_ix);
			case OpcodeType::OPCODE_cmp_8_aw:		return parseOpcode_cmp(DataType::Size::SIZE_8, PARAM_aw);
			case OpcodeType::OPCODE_cmp_8_al:		return parseOpcode_cmp(DataType::Size::SIZE_8, PARAM_al);
			case OpcodeType::OPCODE_cmp_8_pcdi:		return parseOpcode_cmp(DataType::Size::SIZE_8, PARAM_pcdi);
			case OpcodeType::OPCODE_cmp_8_pcix:		return parseOpcode_cmp(DataType::Size::SIZE_8, PARAM_pcix);
			case OpcodeType::OPCODE_cmp_8_i:		return parseOpcode_cmp(DataType::Size::SIZE_8, PARAM_i);
			case OpcodeType::OPCODE_cmp_16_d:		return parseOpcode_cmp(DataType::Size::SIZE_16, PARAM_d);
			case OpcodeType::OPCODE_cmp_16_a:		return parseOpcode_cmp(DataType::Size::SIZE_16, PARAM_a);
			case OpcodeType::OPCODE_cmp_16_ai:		return parseOpcode_cmp(DataType::Size::SIZE_16, PARAM_ai);
			case OpcodeType::OPCODE_cmp_16_pi:		return parseOpcode_cmp(DataType::Size::SIZE_16, PARAM_pi);
			case OpcodeType::OPCODE_cmp_16_pd:		return parseOpcode_cmp(DataType::Size::SIZE_16, PARAM_pd);
			case OpcodeType::OPCODE_cmp_16_di:		return parseOpcode_cmp(DataType::Size::SIZE_16, PARAM_di);
			case OpcodeType::OPCODE_cmp_16_ix:		return parseOpcode_cmp(DataType::Size::SIZE_16, PARAM_ix);
			case OpcodeType::OPCODE_cmp_16_aw:		return parseOpcode_cmp(DataType::Size::SIZE_16, PARAM_aw);
			case OpcodeType::OPCODE_cmp_16_al:		return parseOpcode_cmp(DataType::Size::SIZE_16, PARAM_al);
			case OpcodeType::OPCODE_cmp_16_pcdi:	return parseOpcode_cmp(DataType::Size::SIZE_16, PARAM_pcdi);
			case OpcodeType::OPCODE_cmp_16_pcix:	return parseOpcode_cmp(DataType::Size::SIZE_16, PARAM_pcix);
			case OpcodeType::OPCODE_cmp_16_i:		return parseOpcode_cmp(DataType::Size::SIZE_16, PARAM_i);
			case OpcodeType::OPCODE_cmp_32_d:		return parseOpcode_cmp(DataType::Size::SIZE_32, PARAM_d);
			case OpcodeType::OPCODE_cmp_32_a:		return parseOpcode_cmp(DataType::Size::SIZE_32, PARAM_a);
			case OpcodeType::OPCODE_cmp_32_ai:		return parseOpcode_cmp(DataType::Size::SIZE_32, PARAM_ai);
			case OpcodeType::OPCODE_cmp_32_pi:		return parseOpcode_cmp(DataType::Size::SIZE_32, PARAM_pi);
			case OpcodeType::OPCODE_cmp_32_pd:		return parseOpcode_cmp(DataType::Size::SIZE_32, PARAM_pd);
			case OpcodeType::OPCODE_cmp_32_di:		return parseOpcode_cmp(DataType::Size::SIZE_32, PARAM_di);
			case OpcodeType::OPCODE_cmp_32_ix:		return parseOpcode_cmp(DataType::Size::SIZE_32, PARAM_ix);
			case OpcodeType::OPCODE_cmp_32_aw:		return parseOpcode_cmp(DataType::Size::SIZE_32, PARAM_aw);
			case OpcodeType::OPCODE_cmp_32_al:		return parseOpcode_cmp(DataType::Size::SIZE_32, PARAM_al);
			case OpcodeType::OPCODE_cmp_32_pcdi:	return parseOpcode_cmp(DataType::Size::SIZE_32, PARAM_pcdi);
			case OpcodeType::OPCODE_cmp_32_pcix:	return parseOpcode_cmp(DataType::Size::SIZE_32, PARAM_pcix);
			case OpcodeType::OPCODE_cmp_32_i:		return parseOpcode_cmp(DataType::Size::SIZE_32, PARAM_i);

			case OpcodeType::OPCODE_cmpa_16_d:		return parseOpcode_cmpa(DataType::Size::SIZE_16, PARAM_d);
			case OpcodeType::OPCODE_cmpa_16_a:		return parseOpcode_cmpa(DataType::Size::SIZE_16, PARAM_a);
			case OpcodeType::OPCODE_cmpa_16_ai:		return parseOpcode_cmpa(DataType::Size::SIZE_16, PARAM_ai);
			case OpcodeType::OPCODE_cmpa_16_pi:		return parseOpcode_cmpa(DataType::Size::SIZE_16, PARAM_pi);
			case OpcodeType::OPCODE_cmpa_16_pd:		return parseOpcode_cmpa(DataType::Size::SIZE_16, PARAM_pd);
			case OpcodeType::OPCODE_cmpa_16_di:		return parseOpcode_cmpa(DataType::Size::SIZE_16, PARAM_di);
			case OpcodeType::OPCODE_cmpa_16_ix:		return parseOpcode_cmpa(DataType::Size::SIZE_16, PARAM_ix);
			case OpcodeType::OPCODE_cmpa_16_aw:		return parseOpcode_cmpa(DataType::Size::SIZE_16, PARAM_aw);
			case OpcodeType::OPCODE_cmpa_16_al:		return parseOpcode_cmpa(DataType::Size::SIZE_16, PARAM_al);
			case OpcodeType::OPCODE_cmpa_16_pcdi:	return parseOpcode_cmpa(DataType::Size::SIZE_16, PARAM_pcdi);
			case OpcodeType::OPCODE_cmpa_16_pcix:	return parseOpcode_cmpa(DataType::Size::SIZE_16, PARAM_pcix);
			case OpcodeType::OPCODE_cmpa_16_i:		return parseOpcode_cmpa(DataType::Size::SIZE_16, PARAM_i);
			case OpcodeType::OPCODE_cmpa_32_d:		return parseOpcode_cmpa(DataType::Size::SIZE_32, PARAM_d);
			case OpcodeType::OPCODE_cmpa_32_a:		return parseOpcode_cmpa(DataType::Size::SIZE_32, PARAM_a);
			case OpcodeType::OPCODE_cmpa_32_ai:		return parseOpcode_cmpa(DataType::Size::SIZE_32, PARAM_ai);
			case OpcodeType::OPCODE_cmpa_32_pi:		return parseOpcode_cmpa(DataType::Size::SIZE_32, PARAM_pi);
			case OpcodeType::OPCODE_cmpa_32_pd:		return parseOpcode_cmpa(DataType::Size::SIZE_32, PARAM_pd);
			case OpcodeType::OPCODE_cmpa_32_di:		return parseOpcode_cmpa(DataType::Size::SIZE_32, PARAM_di);
			case OpcodeType::OPCODE_cmpa_32_ix:		return parseOpcode_cmpa(DataType::Size::SIZE_32, PARAM_ix);
			case OpcodeType::OPCODE_cmpa_32_aw:		return parseOpcode_cmpa(DataType::Size::SIZE_32, PARAM_aw);
			case OpcodeType::OPCODE_cmpa_32_al:		return parseOpcode_cmpa(DataType::Size::SIZE_32, PARAM_al);
			case OpcodeType::OPCODE_cmpa_32_pcdi:	return parseOpcode_cmpa(DataType::Size::SIZE_32, PARAM_pcdi);
			case OpcodeType::OPCODE_cmpa_32_pcix:	return parseOpcode_cmpa(DataType::Size::SIZE_32, PARAM_pcix);
			case OpcodeType::OPCODE_cmpa_32_i:		return parseOpcode_cmpa(DataType::Size::SIZE_32, PARAM_i);

			case OpcodeType::OPCODE_cmpi_8_d:		return parseOpcode_cmpi(DataType::Size::SIZE_8, PARAM_d);
			case OpcodeType::OPCODE_cmpi_8_ai:		return parseOpcode_cmpi(DataType::Size::SIZE_8, PARAM_ai);
			case OpcodeType::OPCODE_cmpi_8_pi:		return parseOpcode_cmpi(DataType::Size::SIZE_8, PARAM_pi);
			case OpcodeType::OPCODE_cmpi_8_pi7:		return parseOpcode_cmpi(DataType::Size::SIZE_8, PARAM_pi7);
			case OpcodeType::OPCODE_cmpi_8_pd:		return parseOpcode_cmpi(DataType::Size::SIZE_8, PARAM_pd);
			case OpcodeType::OPCODE_cmpi_8_pd7:		return parseOpcode_cmpi(DataType::Size::SIZE_8, PARAM_pd7);
			case OpcodeType::OPCODE_cmpi_8_di:		return parseOpcode_cmpi(DataType::Size::SIZE_8, PARAM_di);
			case OpcodeType::OPCODE_cmpi_8_ix:		return parseOpcode_cmpi(DataType::Size::SIZE_8, PARAM_ix);
			case OpcodeType::OPCODE_cmpi_8_aw:		return parseOpcode_cmpi(DataType::Size::SIZE_8, PARAM_aw);
			case OpcodeType::OPCODE_cmpi_8_al:		return parseOpcode_cmpi(DataType::Size::SIZE_8, PARAM_al);
			case OpcodeType::OPCODE_cmpi_16_d:		return parseOpcode_cmpi(DataType::Size::SIZE_16, PARAM_d);
			case OpcodeType::OPCODE_cmpi_16_ai:		return parseOpcode_cmpi(DataType::Size::SIZE_16, PARAM_ai);
			case OpcodeType::OPCODE_cmpi_16_pi:		return parseOpcode_cmpi(DataType::Size::SIZE_16, PARAM_pi);
			case OpcodeType::OPCODE_cmpi_16_pd:		return parseOpcode_cmpi(DataType::Size::SIZE_16, PARAM_pd);
			case OpcodeType::OPCODE_cmpi_16_di:		return parseOpcode_cmpi(DataType::Size::SIZE_16, PARAM_di);
			case OpcodeType::OPCODE_cmpi_16_ix:		return parseOpcode_cmpi(DataType::Size::SIZE_16, PARAM_ix);
			case OpcodeType::OPCODE_cmpi_16_aw:		return parseOpcode_cmpi(DataType::Size::SIZE_16, PARAM_aw);
			case OpcodeType::OPCODE_cmpi_16_al:		return parseOpcode_cmpi(DataType::Size::SIZE_16, PARAM_al);
			case OpcodeType::OPCODE_cmpi_32_d:		return parseOpcode_cmpi(DataType::Size::SIZE_32, PARAM_d);
			case OpcodeType::OPCODE_cmpi_32_ai:		return parseOpcode_cmpi(DataType::Size::SIZE_32, PARAM_ai);
			case OpcodeType::OPCODE_cmpi_32_pi:		return parseOpcode_cmpi(DataType::Size::SIZE_32, PARAM_pi);
			case OpcodeType::OPCODE_cmpi_32_pd:		return parseOpcode_cmpi(DataType::Size::SIZE_32, PARAM_pd);
			case OpcodeType::OPCODE_cmpi_32_di:		return parseOpcode_cmpi(DataType::Size::SIZE_32, PARAM_di);
			case OpcodeType::OPCODE_cmpi_32_ix:		return parseOpcode_cmpi(DataType::Size::SIZE_32, PARAM_ix);
			case OpcodeType::OPCODE_cmpi_32_aw:		return parseOpcode_cmpi(DataType::Size::SIZE_32, PARAM_aw);
			case OpcodeType::OPCODE_cmpi_32_al:		return parseOpcode_cmpi(DataType::Size::SIZE_32, PARAM_al);

			// Negation
			case OpcodeType::OPCODE_neg_8_d:		return parseOpcode_neg(DataType::Size::SIZE_8, PARAM_d);
			case OpcodeType::OPCODE_neg_8_ai:		return parseOpcode_neg(DataType::Size::SIZE_8, PARAM_ai);
			case OpcodeType::OPCODE_neg_8_pi:		return parseOpcode_neg(DataType::Size::SIZE_8, PARAM_pi);
			case OpcodeType::OPCODE_neg_8_pi7:		return parseOpcode_neg(DataType::Size::SIZE_8, PARAM_pi7);
			case OpcodeType::OPCODE_neg_8_pd:		return parseOpcode_neg(DataType::Size::SIZE_8, PARAM_pd);
			case OpcodeType::OPCODE_neg_8_pd7:		return parseOpcode_neg(DataType::Size::SIZE_8, PARAM_pd7);
			case OpcodeType::OPCODE_neg_8_di:		return parseOpcode_neg(DataType::Size::SIZE_8, PARAM_di);
			case OpcodeType::OPCODE_neg_8_ix:		return parseOpcode_neg(DataType::Size::SIZE_8, PARAM_ix);
			case OpcodeType::OPCODE_neg_8_aw:		return parseOpcode_neg(DataType::Size::SIZE_8, PARAM_aw);
			case OpcodeType::OPCODE_neg_8_al:		return parseOpcode_neg(DataType::Size::SIZE_8, PARAM_al);
			case OpcodeType::OPCODE_neg_16_d:		return parseOpcode_neg(DataType::Size::SIZE_16, PARAM_d);
			case OpcodeType::OPCODE_neg_16_ai:		return parseOpcode_neg(DataType::Size::SIZE_16, PARAM_ai);
			case OpcodeType::OPCODE_neg_16_pi:		return parseOpcode_neg(DataType::Size::SIZE_16, PARAM_pi);
			case OpcodeType::OPCODE_neg_16_pd:		return parseOpcode_neg(DataType::Size::SIZE_16, PARAM_pd);
			case OpcodeType::OPCODE_neg_16_di:		return parseOpcode_neg(DataType::Size::SIZE_16, PARAM_di);
			case OpcodeType::OPCODE_neg_16_ix:		return parseOpcode_neg(DataType::Size::SIZE_16, PARAM_ix);
			case OpcodeType::OPCODE_neg_16_aw:		return parseOpcode_neg(DataType::Size::SIZE_16, PARAM_aw);
			case OpcodeType::OPCODE_neg_16_al:		return parseOpcode_neg(DataType::Size::SIZE_16, PARAM_al);
			case OpcodeType::OPCODE_neg_32_d:		return parseOpcode_neg(DataType::Size::SIZE_32, PARAM_d);
			case OpcodeType::OPCODE_neg_32_ai:		return parseOpcode_neg(DataType::Size::SIZE_32, PARAM_ai);
			case OpcodeType::OPCODE_neg_32_pi:		return parseOpcode_neg(DataType::Size::SIZE_32, PARAM_pi);
			case OpcodeType::OPCODE_neg_32_pd:		return parseOpcode_neg(DataType::Size::SIZE_32, PARAM_pd);
			case OpcodeType::OPCODE_neg_32_di:		return parseOpcode_neg(DataType::Size::SIZE_32, PARAM_di);
			case OpcodeType::OPCODE_neg_32_ix:		return parseOpcode_neg(DataType::Size::SIZE_32, PARAM_ix);
			case OpcodeType::OPCODE_neg_32_aw:		return parseOpcode_neg(DataType::Size::SIZE_32, PARAM_aw);
			case OpcodeType::OPCODE_neg_32_al:		return parseOpcode_neg(DataType::Size::SIZE_32, PARAM_al);

			// Multiplication
			case OpcodeType::OPCODE_muls_16_d:		return parseOpcode_muls(DataType::Size::SIZE_16, PARAM_d);
			case OpcodeType::OPCODE_muls_16_ai:		return parseOpcode_muls(DataType::Size::SIZE_16, PARAM_ai);
			case OpcodeType::OPCODE_muls_16_pi:		return parseOpcode_muls(DataType::Size::SIZE_16, PARAM_pi);
			case OpcodeType::OPCODE_muls_16_pd:		return parseOpcode_muls(DataType::Size::SIZE_16, PARAM_pd);
			case OpcodeType::OPCODE_muls_16_di:		return parseOpcode_muls(DataType::Size::SIZE_16, PARAM_di);
			case OpcodeType::OPCODE_muls_16_ix:		return parseOpcode_muls(DataType::Size::SIZE_16, PARAM_ix);
			case OpcodeType::OPCODE_muls_16_aw:		return parseOpcode_muls(DataType::Size::SIZE_16, PARAM_aw);
			case OpcodeType::OPCODE_muls_16_al:		return parseOpcode_muls(DataType::Size::SIZE_16, PARAM_al);
			case OpcodeType::OPCODE_muls_16_pcdi:	return parseOpcode_muls(DataType::Size::SIZE_16, PARAM_pcdi);
			case OpcodeType::OPCODE_muls_16_pcix:	return parseOpcode_muls(DataType::Size::SIZE_16, PARAM_pcix);
			case OpcodeType::OPCODE_muls_16_i:		return parseOpcode_muls(DataType::Size::SIZE_16, PARAM_i);
			case OpcodeType::OPCODE_mulu_16_d:		return parseOpcode_mulu(DataType::Size::SIZE_16, PARAM_d);
			case OpcodeType::OPCODE_mulu_16_ai:		return parseOpcode_mulu(DataType::Size::SIZE_16, PARAM_ai);
			case OpcodeType::OPCODE_mulu_16_pi:		return parseOpcode_mulu(DataType::Size::SIZE_16, PARAM_pi);
			case OpcodeType::OPCODE_mulu_16_pd:		return parseOpcode_mulu(DataType::Size::SIZE_16, PARAM_pd);
			case OpcodeType::OPCODE_mulu_16_di:		return parseOpcode_mulu(DataType::Size::SIZE_16, PARAM_di);
			case OpcodeType::OPCODE_mulu_16_ix:		return parseOpcode_mulu(DataType::Size::SIZE_16, PARAM_ix);
			case OpcodeType::OPCODE_mulu_16_aw:		return parseOpcode_mulu(DataType::Size::SIZE_16, PARAM_aw);
			case OpcodeType::OPCODE_mulu_16_al:		return parseOpcode_mulu(DataType::Size::SIZE_16, PARAM_al);
			case OpcodeType::OPCODE_mulu_16_pcdi:	return parseOpcode_mulu(DataType::Size::SIZE_16, PARAM_pcdi);
			case OpcodeType::OPCODE_mulu_16_pcix:	return parseOpcode_mulu(DataType::Size::SIZE_16, PARAM_pcix);
			case OpcodeType::OPCODE_mulu_16_i:		return parseOpcode_mulu(DataType::Size::SIZE_16, PARAM_i);

			// Division
			case OpcodeType::OPCODE_divs_16_d:		return parseOpcode_divs(DataType::Size::SIZE_16, PARAM_d);
			case OpcodeType::OPCODE_divs_16_ai:		return parseOpcode_divs(DataType::Size::SIZE_16, PARAM_ai);
			case OpcodeType::OPCODE_divs_16_pi:		return parseOpcode_divs(DataType::Size::SIZE_16, PARAM_pi);
			case OpcodeType::OPCODE_divs_16_pd:		return parseOpcode_divs(DataType::Size::SIZE_16, PARAM_pd);
			case OpcodeType::OPCODE_divs_16_di:		return parseOpcode_divs(DataType::Size::SIZE_16, PARAM_di);
			case OpcodeType::OPCODE_divs_16_ix:		return parseOpcode_divs(DataType::Size::SIZE_16, PARAM_ix);
			case OpcodeType::OPCODE_divs_16_aw:		return parseOpcode_divs(DataType::Size::SIZE_16, PARAM_aw);
			case OpcodeType::OPCODE_divs_16_al:		return parseOpcode_divs(DataType::Size::SIZE_16, PARAM_al);
			case OpcodeType::OPCODE_divs_16_pcdi:	return parseOpcode_divs(DataType::Size::SIZE_16, PARAM_pcdi);
			case OpcodeType::OPCODE_divs_16_pcix:	return parseOpcode_divs(DataType::Size::SIZE_16, PARAM_pcix);
			case OpcodeType::OPCODE_divs_16_i:		return parseOpcode_divs(DataType::Size::SIZE_16, PARAM_i);
			case OpcodeType::OPCODE_divu_16_d:		return parseOpcode_divu(DataType::Size::SIZE_16, PARAM_d);
			case OpcodeType::OPCODE_divu_16_ai:		return parseOpcode_divu(DataType::Size::SIZE_16, PARAM_ai);
			case OpcodeType::OPCODE_divu_16_pi:		return parseOpcode_divu(DataType::Size::SIZE_16, PARAM_pi);
			case OpcodeType::OPCODE_divu_16_pd:		return parseOpcode_divu(DataType::Size::SIZE_16, PARAM_pd);
			case OpcodeType::OPCODE_divu_16_di:		return parseOpcode_divu(DataType::Size::SIZE_16, PARAM_di);
			case OpcodeType::OPCODE_divu_16_ix:		return parseOpcode_divu(DataType::Size::SIZE_16, PARAM_ix);
			case OpcodeType::OPCODE_divu_16_aw:		return parseOpcode_divu(DataType::Size::SIZE_16, PARAM_aw);
			case OpcodeType::OPCODE_divu_16_al:		return parseOpcode_divu(DataType::Size::SIZE_16, PARAM_al);
			case OpcodeType::OPCODE_divu_16_pcdi:	return parseOpcode_divu(DataType::Size::SIZE_16, PARAM_pcdi);
			case OpcodeType::OPCODE_divu_16_pcix:	return parseOpcode_divu(DataType::Size::SIZE_16, PARAM_pcix);
			case OpcodeType::OPCODE_divu_16_i:		return parseOpcode_divu(DataType::Size::SIZE_16, PARAM_i);

			// Add BCD
			case OpcodeType::OPCODE_abcd_8_rr:		return parseOpcode_abcd_rr(DataType::Size::SIZE_8);
			case OpcodeType::OPCODE_abcd_8_mm_ax7:	return parseOpcode_abcd_mm(DataType::Size::SIZE_8, PARAM_pd,  PARAM_pd7);
			case OpcodeType::OPCODE_abcd_8_mm_ay7:	return parseOpcode_abcd_mm(DataType::Size::SIZE_8, PARAM_pd7, PARAM_pd);
			case OpcodeType::OPCODE_abcd_8_mm_axy7:	return parseOpcode_abcd_mm(DataType::Size::SIZE_8, PARAM_pd7, PARAM_pd7);
			case OpcodeType::OPCODE_abcd_8_mm:		return parseOpcode_abcd_mm(DataType::Size::SIZE_8, PARAM_pd,  PARAM_pd);

			// Arithmetic shift left
			case OpcodeType::OPCODE_asl_8_s:		return parseOpcode_shiftLeft_s(DataType::Size::SIZE_8,  ShiftType::ARITHMETIC);
			case OpcodeType::OPCODE_asl_16_s:		return parseOpcode_shiftLeft_s(DataType::Size::SIZE_16, ShiftType::ARITHMETIC);
			case OpcodeType::OPCODE_asl_32_s:		return parseOpcode_shiftLeft_s(DataType::Size::SIZE_32, ShiftType::ARITHMETIC);
			case OpcodeType::OPCODE_asl_8_r:		return parseOpcode_shiftLeft_r(DataType::Size::SIZE_8,  ShiftType::ARITHMETIC);
			case OpcodeType::OPCODE_asl_16_r:		return parseOpcode_shiftLeft_r(DataType::Size::SIZE_16, ShiftType::ARITHMETIC);
			case OpcodeType::OPCODE_asl_32_r:		return parseOpcode_shiftLeft_r(DataType::Size::SIZE_32, ShiftType::ARITHMETIC);
			case OpcodeType::OPCODE_asl_16_ai:		return parseOpcode_shiftLeft_param(DataType::Size::SIZE_16, ShiftType::ARITHMETIC, PARAM_ai);
			case OpcodeType::OPCODE_asl_16_pi:		return parseOpcode_shiftLeft_param(DataType::Size::SIZE_16, ShiftType::ARITHMETIC, PARAM_pi);
			case OpcodeType::OPCODE_asl_16_pd:		return parseOpcode_shiftLeft_param(DataType::Size::SIZE_16, ShiftType::ARITHMETIC, PARAM_pd);
			case OpcodeType::OPCODE_asl_16_di:		return parseOpcode_shiftLeft_param(DataType::Size::SIZE_16, ShiftType::ARITHMETIC, PARAM_di);
			case OpcodeType::OPCODE_asl_16_ix:		return parseOpcode_shiftLeft_param(DataType::Size::SIZE_16, ShiftType::ARITHMETIC, PARAM_ix);
			case OpcodeType::OPCODE_asl_16_aw:		return parseOpcode_shiftLeft_param(DataType::Size::SIZE_16, ShiftType::ARITHMETIC, PARAM_aw);
			case OpcodeType::OPCODE_asl_16_al:		return parseOpcode_shiftLeft_param(DataType::Size::SIZE_16, ShiftType::ARITHMETIC, PARAM_al);

			// Logical shift left
			case OpcodeType::OPCODE_lsl_8_s:		return parseOpcode_shiftLeft_s(DataType::Size::SIZE_8,  ShiftType::LOGICAL);
			case OpcodeType::OPCODE_lsl_16_s:		return parseOpcode_shiftLeft_s(DataType::Size::SIZE_16, ShiftType::LOGICAL);
			case OpcodeType::OPCODE_lsl_32_s:		return parseOpcode_shiftLeft_s(DataType::Size::SIZE_32, ShiftType::LOGICAL);
			case OpcodeType::OPCODE_lsl_8_r:		return parseOpcode_shiftLeft_r(DataType::Size::SIZE_8,  ShiftType::LOGICAL);
			case OpcodeType::OPCODE_lsl_16_r:		return parseOpcode_shiftLeft_r(DataType::Size::SIZE_16, ShiftType::LOGICAL);
			case OpcodeType::OPCODE_lsl_32_r:		return parseOpcode_shiftLeft_r(DataType::Size::SIZE_32, ShiftType::LOGICAL);
			case OpcodeType::OPCODE_lsl_16_ai:		return parseOpcode_shiftLeft_param(DataType::Size::SIZE_16, ShiftType::LOGICAL, PARAM_ai);
			case OpcodeType::OPCODE_lsl_16_pi:		return parseOpcode_shiftLeft_param(DataType::Size::SIZE_16, ShiftType::LOGICAL, PARAM_pi);
			case OpcodeType::OPCODE_lsl_16_pd:		return parseOpcode_shiftLeft_param(DataType::Size::SIZE_16, ShiftType::LOGICAL, PARAM_pd);
			case OpcodeType::OPCODE_lsl_16_di:		return parseOpcode_shiftLeft_param(DataType::Size::SIZE_16, ShiftType::LOGICAL, PARAM_di);
			case OpcodeType::OPCODE_lsl_16_ix:		return parseOpcode_shiftLeft_param(DataType::Size::SIZE_16, ShiftType::LOGICAL, PARAM_ix);
			case OpcodeType::OPCODE_lsl_16_aw:		return parseOpcode_shiftLeft_param(DataType::Size::SIZE_16, ShiftType::LOGICAL, PARAM_aw);
			case OpcodeType::OPCODE_lsl_16_al:		return parseOpcode_shiftLeft_param(DataType::Size::SIZE_16, ShiftType::LOGICAL, PARAM_al);

			// Rotate left
			case OpcodeType::OPCODE_rol_8_s:		return parseOpcode_shiftLeft_s(DataType::Size::SIZE_8,  ShiftType::ROTATE);
			case OpcodeType::OPCODE_rol_16_s:		return parseOpcode_shiftLeft_s(DataType::Size::SIZE_16, ShiftType::ROTATE);
			case OpcodeType::OPCODE_rol_32_s:		return parseOpcode_shiftLeft_s(DataType::Size::SIZE_32, ShiftType::ROTATE);
			case OpcodeType::OPCODE_rol_8_r:		return parseOpcode_shiftLeft_r(DataType::Size::SIZE_8,  ShiftType::ROTATE);
			case OpcodeType::OPCODE_rol_16_r:		return parseOpcode_shiftLeft_r(DataType::Size::SIZE_16, ShiftType::ROTATE);
			case OpcodeType::OPCODE_rol_32_r:		return parseOpcode_shiftLeft_r(DataType::Size::SIZE_32, ShiftType::ROTATE);
			case OpcodeType::OPCODE_rol_16_ai:		return parseOpcode_shiftLeft_param(DataType::Size::SIZE_16, ShiftType::ROTATE, PARAM_ai);
			case OpcodeType::OPCODE_rol_16_pi:		return parseOpcode_shiftLeft_param(DataType::Size::SIZE_16, ShiftType::ROTATE, PARAM_pi);
			case OpcodeType::OPCODE_rol_16_pd:		return parseOpcode_shiftLeft_param(DataType::Size::SIZE_16, ShiftType::ROTATE, PARAM_pd);
			case OpcodeType::OPCODE_rol_16_di:		return parseOpcode_shiftLeft_param(DataType::Size::SIZE_16, ShiftType::ROTATE, PARAM_di);
			case OpcodeType::OPCODE_rol_16_ix:		return parseOpcode_shiftLeft_param(DataType::Size::SIZE_16, ShiftType::ROTATE, PARAM_ix);
			case OpcodeType::OPCODE_rol_16_aw:		return parseOpcode_shiftLeft_param(DataType::Size::SIZE_16, ShiftType::ROTATE, PARAM_aw);
			case OpcodeType::OPCODE_rol_16_al:		return parseOpcode_shiftLeft_param(DataType::Size::SIZE_16, ShiftType::ROTATE, PARAM_al);

			// Rotate with x-flag left
			case OpcodeType::OPCODE_roxl_8_s:		return parseOpcode_shiftLeft_s(DataType::Size::SIZE_8,  ShiftType::ROTATE_X);
			case OpcodeType::OPCODE_roxl_16_s:		return parseOpcode_shiftLeft_s(DataType::Size::SIZE_16, ShiftType::ROTATE_X);
			case OpcodeType::OPCODE_roxl_32_s:		return parseOpcode_shiftLeft_s(DataType::Size::SIZE_32, ShiftType::ROTATE_X);
			case OpcodeType::OPCODE_roxl_8_r:		return parseOpcode_shiftLeft_r(DataType::Size::SIZE_8,  ShiftType::ROTATE_X);
			case OpcodeType::OPCODE_roxl_16_r:		return parseOpcode_shiftLeft_r(DataType::Size::SIZE_16, ShiftType::ROTATE_X);
			case OpcodeType::OPCODE_roxl_32_r:		return parseOpcode_shiftLeft_r(DataType::Size::SIZE_32, ShiftType::ROTATE_X);
			case OpcodeType::OPCODE_roxl_16_ai:		return parseOpcode_shiftLeft_param(DataType::Size::SIZE_16, ShiftType::ROTATE_X, PARAM_ai);
			case OpcodeType::OPCODE_roxl_16_pi:		return parseOpcode_shiftLeft_param(DataType::Size::SIZE_16, ShiftType::ROTATE_X, PARAM_pi);
			case OpcodeType::OPCODE_roxl_16_pd:		return parseOpcode_shiftLeft_param(DataType::Size::SIZE_16, ShiftType::ROTATE_X, PARAM_pd);
			case OpcodeType::OPCODE_roxl_16_di:		return parseOpcode_shiftLeft_param(DataType::Size::SIZE_16, ShiftType::ROTATE_X, PARAM_di);
			case OpcodeType::OPCODE_roxl_16_ix:		return parseOpcode_shiftLeft_param(DataType::Size::SIZE_16, ShiftType::ROTATE_X, PARAM_ix);
			case OpcodeType::OPCODE_roxl_16_aw:		return parseOpcode_shiftLeft_param(DataType::Size::SIZE_16, ShiftType::ROTATE_X, PARAM_aw);
			case OpcodeType::OPCODE_roxl_16_al:		return parseOpcode_shiftLeft_param(DataType::Size::SIZE_16, ShiftType::ROTATE_X, PARAM_al);

			// Arithmetic shift right
			case OpcodeType::OPCODE_asr_8_s:		return parseOpcode_shiftRight_s(DataType::Size::SIZE_8,  ShiftType::ARITHMETIC);
			case OpcodeType::OPCODE_asr_16_s:		return parseOpcode_shiftRight_s(DataType::Size::SIZE_16, ShiftType::ARITHMETIC);
			case OpcodeType::OPCODE_asr_32_s:		return parseOpcode_shiftRight_s(DataType::Size::SIZE_32, ShiftType::ARITHMETIC);
			case OpcodeType::OPCODE_asr_8_r:		return parseOpcode_shiftRight_r(DataType::Size::SIZE_8,  ShiftType::ARITHMETIC);
			case OpcodeType::OPCODE_asr_16_r:		return parseOpcode_shiftRight_r(DataType::Size::SIZE_16, ShiftType::ARITHMETIC);
			case OpcodeType::OPCODE_asr_32_r:		return parseOpcode_shiftRight_r(DataType::Size::SIZE_32, ShiftType::ARITHMETIC);
			case OpcodeType::OPCODE_asr_16_ai:		return parseOpcode_shiftRight_param(DataType::Size::SIZE_16, ShiftType::ARITHMETIC, PARAM_ai);
			case OpcodeType::OPCODE_asr_16_pi:		return parseOpcode_shiftRight_param(DataType::Size::SIZE_16, ShiftType::ARITHMETIC, PARAM_pi);
			case OpcodeType::OPCODE_asr_16_pd:		return parseOpcode_shiftRight_param(DataType::Size::SIZE_16, ShiftType::ARITHMETIC, PARAM_pd);
			case OpcodeType::OPCODE_asr_16_di:		return parseOpcode_shiftRight_param(DataType::Size::SIZE_16, ShiftType::ARITHMETIC, PARAM_di);
			case OpcodeType::OPCODE_asr_16_ix:		return parseOpcode_shiftRight_param(DataType::Size::SIZE_16, ShiftType::ARITHMETIC, PARAM_ix);
			case OpcodeType::OPCODE_asr_16_aw:		return parseOpcode_shiftRight_param(DataType::Size::SIZE_16, ShiftType::ARITHMETIC, PARAM_aw);
			case OpcodeType::OPCODE_asr_16_al:		return parseOpcode_shiftRight_param(DataType::Size::SIZE_16, ShiftType::ARITHMETIC, PARAM_al);

			// Logical shift right
			case OpcodeType::OPCODE_lsr_8_s:		return parseOpcode_shiftRight_s(DataType::Size::SIZE_8,  ShiftType::LOGICAL);
			case OpcodeType::OPCODE_lsr_16_s:		return parseOpcode_shiftRight_s(DataType::Size::SIZE_16, ShiftType::LOGICAL);
			case OpcodeType::OPCODE_lsr_32_s:		return parseOpcode_shiftRight_s(DataType::Size::SIZE_32, ShiftType::LOGICAL);
			case OpcodeType::OPCODE_lsr_8_r:		return parseOpcode_shiftRight_r(DataType::Size::SIZE_8,  ShiftType::LOGICAL);
			case OpcodeType::OPCODE_lsr_16_r:		return parseOpcode_shiftRight_r(DataType::Size::SIZE_16, ShiftType::LOGICAL);
			case OpcodeType::OPCODE_lsr_32_r:		return parseOpcode_shiftRight_r(DataType::Size::SIZE_32, ShiftType::LOGICAL);
			case OpcodeType::OPCODE_lsr_16_ai:		return parseOpcode_shiftRight_param(DataType::Size::SIZE_16, ShiftType::LOGICAL, PARAM_ai);
			case OpcodeType::OPCODE_lsr_16_pi:		return parseOpcode_shiftRight_param(DataType::Size::SIZE_16, ShiftType::LOGICAL, PARAM_pi);
			case OpcodeType::OPCODE_lsr_16_pd:		return parseOpcode_shiftRight_param(DataType::Size::SIZE_16, ShiftType::LOGICAL, PARAM_pd);
			case OpcodeType::OPCODE_lsr_16_di:		return parseOpcode_shiftRight_param(DataType::Size::SIZE_16, ShiftType::LOGICAL, PARAM_di);
			case OpcodeType::OPCODE_lsr_16_ix:		return parseOpcode_shiftRight_param(DataType::Size::SIZE_16, ShiftType::LOGICAL, PARAM_ix);
			case OpcodeType::OPCODE_lsr_16_aw:		return parseOpcode_shiftRight_param(DataType::Size::SIZE_16, ShiftType::LOGICAL, PARAM_aw);
			case OpcodeType::OPCODE_lsr_16_al:		return parseOpcode_shiftRight_param(DataType::Size::SIZE_16, ShiftType::LOGICAL, PARAM_al);

			// Rotate right
			case OpcodeType::OPCODE_ror_8_s:		return parseOpcode_shiftRight_s(DataType::Size::SIZE_8,  ShiftType::ROTATE);
			case OpcodeType::OPCODE_ror_16_s:		return parseOpcode_shiftRight_s(DataType::Size::SIZE_16, ShiftType::ROTATE);
			case OpcodeType::OPCODE_ror_32_s:		return parseOpcode_shiftRight_s(DataType::Size::SIZE_32, ShiftType::ROTATE);
			case OpcodeType::OPCODE_ror_8_r:		return parseOpcode_shiftRight_r(DataType::Size::SIZE_8,  ShiftType::ROTATE);
			case OpcodeType::OPCODE_ror_16_r:		return parseOpcode_shiftRight_r(DataType::Size::SIZE_16, ShiftType::ROTATE);
			case OpcodeType::OPCODE_ror_32_r:		return parseOpcode_shiftRight_r(DataType::Size::SIZE_32, ShiftType::ROTATE);
			case OpcodeType::OPCODE_ror_16_ai:		return parseOpcode_shiftRight_param(DataType::Size::SIZE_16, ShiftType::ROTATE, PARAM_ai);
			case OpcodeType::OPCODE_ror_16_pi:		return parseOpcode_shiftRight_param(DataType::Size::SIZE_16, ShiftType::ROTATE, PARAM_pi);
			case OpcodeType::OPCODE_ror_16_pd:		return parseOpcode_shiftRight_param(DataType::Size::SIZE_16, ShiftType::ROTATE, PARAM_pd);
			case OpcodeType::OPCODE_ror_16_di:		return parseOpcode_shiftRight_param(DataType::Size::SIZE_16, ShiftType::ROTATE, PARAM_di);
			case OpcodeType::OPCODE_ror_16_ix:		return parseOpcode_shiftRight_param(DataType::Size::SIZE_16, ShiftType::ROTATE, PARAM_ix);
			case OpcodeType::OPCODE_ror_16_aw:		return parseOpcode_shiftRight_param(DataType::Size::SIZE_16, ShiftType::ROTATE, PARAM_aw);
			case OpcodeType::OPCODE_ror_16_al:		return parseOpcode_shiftRight_param(DataType::Size::SIZE_16, ShiftType::ROTATE, PARAM_al);

			// Rotate with x-flag right
			case OpcodeType::OPCODE_roxr_8_s:		return parseOpcode_shiftRight_s(DataType::Size::SIZE_8,  ShiftType::ROTATE_X);
			case OpcodeType::OPCODE_roxr_16_s:		return parseOpcode_shiftRight_s(DataType::Size::SIZE_16, ShiftType::ROTATE_X);
			case OpcodeType::OPCODE_roxr_32_s:		return parseOpcode_shiftRight_s(DataType::Size::SIZE_32, ShiftType::ROTATE_X);
			case OpcodeType::OPCODE_roxr_8_r:		return parseOpcode_shiftRight_r(DataType::Size::SIZE_8,  ShiftType::ROTATE_X);
			case OpcodeType::OPCODE_roxr_16_r:		return parseOpcode_shiftRight_r(DataType::Size::SIZE_16, ShiftType::ROTATE_X);
			case OpcodeType::OPCODE_roxr_32_r:		return parseOpcode_shiftRight_r(DataType::Size::SIZE_32, ShiftType::ROTATE_X);
			case OpcodeType::OPCODE_roxr_16_ai:		return parseOpcode_shiftRight_param(DataType::Size::SIZE_16, ShiftType::ROTATE_X, PARAM_ai);
			case OpcodeType::OPCODE_roxr_16_pi:		return parseOpcode_shiftRight_param(DataType::Size::SIZE_16, ShiftType::ROTATE_X, PARAM_pi);
			case OpcodeType::OPCODE_roxr_16_pd:		return parseOpcode_shiftRight_param(DataType::Size::SIZE_16, ShiftType::ROTATE_X, PARAM_pd);
			case OpcodeType::OPCODE_roxr_16_di:		return parseOpcode_shiftRight_param(DataType::Size::SIZE_16, ShiftType::ROTATE_X, PARAM_di);
			case OpcodeType::OPCODE_roxr_16_ix:		return parseOpcode_shiftRight_param(DataType::Size::SIZE_16, ShiftType::ROTATE_X, PARAM_ix);
			case OpcodeType::OPCODE_roxr_16_aw:		return parseOpcode_shiftRight_param(DataType::Size::SIZE_16, ShiftType::ROTATE_X, PARAM_aw);
			case OpcodeType::OPCODE_roxr_16_al:		return parseOpcode_shiftRight_param(DataType::Size::SIZE_16, ShiftType::ROTATE_X, PARAM_al);

			// Bitwise NOT
			case OpcodeType::OPCODE_not_8_d:		return parseOpcode_not(DataType::Size::SIZE_8, PARAM_d);
			case OpcodeType::OPCODE_not_8_ai:		return parseOpcode_not(DataType::Size::SIZE_8, PARAM_ai);
			case OpcodeType::OPCODE_not_8_pi:		return parseOpcode_not(DataType::Size::SIZE_8, PARAM_pi);
			case OpcodeType::OPCODE_not_8_pi7:		return parseOpcode_not(DataType::Size::SIZE_8, PARAM_pi7);
			case OpcodeType::OPCODE_not_8_pd:		return parseOpcode_not(DataType::Size::SIZE_8, PARAM_pd);
			case OpcodeType::OPCODE_not_8_pd7:		return parseOpcode_not(DataType::Size::SIZE_8, PARAM_pd7);
			case OpcodeType::OPCODE_not_8_di:		return parseOpcode_not(DataType::Size::SIZE_8, PARAM_di);
			case OpcodeType::OPCODE_not_8_ix:		return parseOpcode_not(DataType::Size::SIZE_8, PARAM_ix);
			case OpcodeType::OPCODE_not_8_aw:		return parseOpcode_not(DataType::Size::SIZE_8, PARAM_aw);
			case OpcodeType::OPCODE_not_8_al:		return parseOpcode_not(DataType::Size::SIZE_8, PARAM_al);
			case OpcodeType::OPCODE_not_16_d:		return parseOpcode_not(DataType::Size::SIZE_16, PARAM_d);
			case OpcodeType::OPCODE_not_16_ai:		return parseOpcode_not(DataType::Size::SIZE_16, PARAM_ai);
			case OpcodeType::OPCODE_not_16_pi:		return parseOpcode_not(DataType::Size::SIZE_16, PARAM_pi);
			case OpcodeType::OPCODE_not_16_pd:		return parseOpcode_not(DataType::Size::SIZE_16, PARAM_pd);
			case OpcodeType::OPCODE_not_16_di:		return parseOpcode_not(DataType::Size::SIZE_16, PARAM_di);
			case OpcodeType::OPCODE_not_16_ix:		return parseOpcode_not(DataType::Size::SIZE_16, PARAM_ix);
			case OpcodeType::OPCODE_not_16_aw:		return parseOpcode_not(DataType::Size::SIZE_16, PARAM_aw);
			case OpcodeType::OPCODE_not_16_al:		return parseOpcode_not(DataType::Size::SIZE_16, PARAM_al);
			case OpcodeType::OPCODE_not_32_d:		return parseOpcode_not(DataType::Size::SIZE_32, PARAM_d);
			case OpcodeType::OPCODE_not_32_ai:		return parseOpcode_not(DataType::Size::SIZE_32, PARAM_ai);
			case OpcodeType::OPCODE_not_32_pi:		return parseOpcode_not(DataType::Size::SIZE_32, PARAM_pi);
			case OpcodeType::OPCODE_not_32_pd:		return parseOpcode_not(DataType::Size::SIZE_32, PARAM_pd);
			case OpcodeType::OPCODE_not_32_di:		return parseOpcode_not(DataType::Size::SIZE_32, PARAM_di);
			case OpcodeType::OPCODE_not_32_ix:		return parseOpcode_not(DataType::Size::SIZE_32, PARAM_ix);
			case OpcodeType::OPCODE_not_32_aw:		return parseOpcode_not(DataType::Size::SIZE_32, PARAM_aw);
			case OpcodeType::OPCODE_not_32_al:		return parseOpcode_not(DataType::Size::SIZE_32, PARAM_al);

			// Bitwise AND
			case OpcodeType::OPCODE_and_8_er_d:		return parseOpcode_and_er(DataType::Size::SIZE_8, PARAM_d);
			case OpcodeType::OPCODE_and_8_er_ai:	return parseOpcode_and_er(DataType::Size::SIZE_8, PARAM_ai);
			case OpcodeType::OPCODE_and_8_er_pi:	return parseOpcode_and_er(DataType::Size::SIZE_8, PARAM_pi);
			case OpcodeType::OPCODE_and_8_er_pi7:	return parseOpcode_and_er(DataType::Size::SIZE_8, PARAM_pi7);
			case OpcodeType::OPCODE_and_8_er_pd:	return parseOpcode_and_er(DataType::Size::SIZE_8, PARAM_pd);
			case OpcodeType::OPCODE_and_8_er_pd7:	return parseOpcode_and_er(DataType::Size::SIZE_8, PARAM_pd7);
			case OpcodeType::OPCODE_and_8_er_di:	return parseOpcode_and_er(DataType::Size::SIZE_8, PARAM_di);
			case OpcodeType::OPCODE_and_8_er_ix:	return parseOpcode_and_er(DataType::Size::SIZE_8, PARAM_ix);
			case OpcodeType::OPCODE_and_8_er_aw:	return parseOpcode_and_er(DataType::Size::SIZE_8, PARAM_aw);
			case OpcodeType::OPCODE_and_8_er_al:	return parseOpcode_and_er(DataType::Size::SIZE_8, PARAM_al);
			case OpcodeType::OPCODE_and_8_er_pcdi:	return parseOpcode_and_er(DataType::Size::SIZE_8, PARAM_pcdi);
			case OpcodeType::OPCODE_and_8_er_pcix:	return parseOpcode_and_er(DataType::Size::SIZE_8, PARAM_pcix);
			case OpcodeType::OPCODE_and_8_er_i:		return parseOpcode_and_er(DataType::Size::SIZE_8, PARAM_i);
			case OpcodeType::OPCODE_and_16_er_d:	return parseOpcode_and_er(DataType::Size::SIZE_16, PARAM_d);
			case OpcodeType::OPCODE_and_16_er_ai:	return parseOpcode_and_er(DataType::Size::SIZE_16, PARAM_ai);
			case OpcodeType::OPCODE_and_16_er_pi:	return parseOpcode_and_er(DataType::Size::SIZE_16, PARAM_pi);
			case OpcodeType::OPCODE_and_16_er_pd:	return parseOpcode_and_er(DataType::Size::SIZE_16, PARAM_pd);
			case OpcodeType::OPCODE_and_16_er_di:	return parseOpcode_and_er(DataType::Size::SIZE_16, PARAM_di);
			case OpcodeType::OPCODE_and_16_er_ix:	return parseOpcode_and_er(DataType::Size::SIZE_16, PARAM_ix);
			case OpcodeType::OPCODE_and_16_er_aw:	return parseOpcode_and_er(DataType::Size::SIZE_16, PARAM_aw);
			case OpcodeType::OPCODE_and_16_er_al:	return parseOpcode_and_er(DataType::Size::SIZE_16, PARAM_al);
			case OpcodeType::OPCODE_and_16_er_pcdi:	return parseOpcode_and_er(DataType::Size::SIZE_16, PARAM_pcdi);
			case OpcodeType::OPCODE_and_16_er_pcix:	return parseOpcode_and_er(DataType::Size::SIZE_16, PARAM_pcix);
			case OpcodeType::OPCODE_and_16_er_i:	return parseOpcode_and_er(DataType::Size::SIZE_16, PARAM_i);
			case OpcodeType::OPCODE_and_32_er_d:	return parseOpcode_and_er(DataType::Size::SIZE_32, PARAM_d);
			case OpcodeType::OPCODE_and_32_er_ai:	return parseOpcode_and_er(DataType::Size::SIZE_32, PARAM_ai);
			case OpcodeType::OPCODE_and_32_er_pi:	return parseOpcode_and_er(DataType::Size::SIZE_32, PARAM_pi);
			case OpcodeType::OPCODE_and_32_er_pd:	return parseOpcode_and_er(DataType::Size::SIZE_32, PARAM_pd);
			case OpcodeType::OPCODE_and_32_er_di:	return parseOpcode_and_er(DataType::Size::SIZE_32, PARAM_di);
			case OpcodeType::OPCODE_and_32_er_ix:	return parseOpcode_and_er(DataType::Size::SIZE_32, PARAM_ix);
			case OpcodeType::OPCODE_and_32_er_aw:	return parseOpcode_and_er(DataType::Size::SIZE_32, PARAM_aw);
			case OpcodeType::OPCODE_and_32_er_al:	return parseOpcode_and_er(DataType::Size::SIZE_32, PARAM_al);
			case OpcodeType::OPCODE_and_32_er_pcdi:	return parseOpcode_and_er(DataType::Size::SIZE_32, PARAM_pcdi);
			case OpcodeType::OPCODE_and_32_er_pcix:	return parseOpcode_and_er(DataType::Size::SIZE_32, PARAM_pcix);
			case OpcodeType::OPCODE_and_32_er_i:	return parseOpcode_and_er(DataType::Size::SIZE_32, PARAM_i);
			case OpcodeType::OPCODE_and_8_re_ai:	return parseOpcode_and_re(DataType::Size::SIZE_8, PARAM_ai);
			case OpcodeType::OPCODE_and_8_re_pi:	return parseOpcode_and_re(DataType::Size::SIZE_8, PARAM_pi);
			case OpcodeType::OPCODE_and_8_re_pi7:	return parseOpcode_and_re(DataType::Size::SIZE_8, PARAM_pi7);
			case OpcodeType::OPCODE_and_8_re_pd:	return parseOpcode_and_re(DataType::Size::SIZE_8, PARAM_pd);
			case OpcodeType::OPCODE_and_8_re_pd7:	return parseOpcode_and_re(DataType::Size::SIZE_8, PARAM_pd7);
			case OpcodeType::OPCODE_and_8_re_di:	return parseOpcode_and_re(DataType::Size::SIZE_8, PARAM_di);
			case OpcodeType::OPCODE_and_8_re_ix:	return parseOpcode_and_re(DataType::Size::SIZE_8, PARAM_ix);
			case OpcodeType::OPCODE_and_8_re_aw:	return parseOpcode_and_re(DataType::Size::SIZE_8, PARAM_aw);
			case OpcodeType::OPCODE_and_8_re_al:	return parseOpcode_and_re(DataType::Size::SIZE_8, PARAM_al);
			case OpcodeType::OPCODE_and_16_re_ai:	return parseOpcode_and_re(DataType::Size::SIZE_16, PARAM_ai);
			case OpcodeType::OPCODE_and_16_re_pi:	return parseOpcode_and_re(DataType::Size::SIZE_16, PARAM_pi);
			case OpcodeType::OPCODE_and_16_re_pd:	return parseOpcode_and_re(DataType::Size::SIZE_16, PARAM_pd);
			case OpcodeType::OPCODE_and_16_re_di:	return parseOpcode_and_re(DataType::Size::SIZE_16, PARAM_di);
			case OpcodeType::OPCODE_and_16_re_ix:	return parseOpcode_and_re(DataType::Size::SIZE_16, PARAM_ix);
			case OpcodeType::OPCODE_and_16_re_aw:	return parseOpcode_and_re(DataType::Size::SIZE_16, PARAM_aw);
			case OpcodeType::OPCODE_and_16_re_al:	return parseOpcode_and_re(DataType::Size::SIZE_16, PARAM_al);
			case OpcodeType::OPCODE_and_32_re_ai:	return parseOpcode_and_re(DataType::Size::SIZE_32, PARAM_ai);
			case OpcodeType::OPCODE_and_32_re_pi:	return parseOpcode_and_re(DataType::Size::SIZE_32, PARAM_pi);
			case OpcodeType::OPCODE_and_32_re_pd:	return parseOpcode_and_re(DataType::Size::SIZE_32, PARAM_pd);
			case OpcodeType::OPCODE_and_32_re_di:	return parseOpcode_and_re(DataType::Size::SIZE_32, PARAM_di);
			case OpcodeType::OPCODE_and_32_re_ix:	return parseOpcode_and_re(DataType::Size::SIZE_32, PARAM_ix);
			case OpcodeType::OPCODE_and_32_re_aw:	return parseOpcode_and_re(DataType::Size::SIZE_32, PARAM_aw);
			case OpcodeType::OPCODE_and_32_re_al:	return parseOpcode_and_re(DataType::Size::SIZE_32, PARAM_al);
			case OpcodeType::OPCODE_andi_8_d:		return parseOpcode_andi(DataType::Size::SIZE_8, PARAM_d);
			case OpcodeType::OPCODE_andi_8_ai:		return parseOpcode_andi(DataType::Size::SIZE_8, PARAM_ai);
			case OpcodeType::OPCODE_andi_8_pi:		return parseOpcode_andi(DataType::Size::SIZE_8, PARAM_pi);
			case OpcodeType::OPCODE_andi_8_pi7:		return parseOpcode_andi(DataType::Size::SIZE_8, PARAM_pi7);
			case OpcodeType::OPCODE_andi_8_pd:		return parseOpcode_andi(DataType::Size::SIZE_8, PARAM_pd);
			case OpcodeType::OPCODE_andi_8_pd7:		return parseOpcode_andi(DataType::Size::SIZE_8, PARAM_pd7);
			case OpcodeType::OPCODE_andi_8_di:		return parseOpcode_andi(DataType::Size::SIZE_8, PARAM_di);
			case OpcodeType::OPCODE_andi_8_ix:		return parseOpcode_andi(DataType::Size::SIZE_8, PARAM_ix);
			case OpcodeType::OPCODE_andi_8_aw:		return parseOpcode_andi(DataType::Size::SIZE_8, PARAM_aw);
			case OpcodeType::OPCODE_andi_8_al:		return parseOpcode_andi(DataType::Size::SIZE_8, PARAM_al);
			case OpcodeType::OPCODE_andi_16_d:		return parseOpcode_andi(DataType::Size::SIZE_16, PARAM_d);
			case OpcodeType::OPCODE_andi_16_ai:		return parseOpcode_andi(DataType::Size::SIZE_16, PARAM_ai);
			case OpcodeType::OPCODE_andi_16_pi:		return parseOpcode_andi(DataType::Size::SIZE_16, PARAM_pi);
			case OpcodeType::OPCODE_andi_16_pd:		return parseOpcode_andi(DataType::Size::SIZE_16, PARAM_pd);
			case OpcodeType::OPCODE_andi_16_di:		return parseOpcode_andi(DataType::Size::SIZE_16, PARAM_di);
			case OpcodeType::OPCODE_andi_16_ix:		return parseOpcode_andi(DataType::Size::SIZE_16, PARAM_ix);
			case OpcodeType::OPCODE_andi_16_aw:		return parseOpcode_andi(DataType::Size::SIZE_16, PARAM_aw);
			case OpcodeType::OPCODE_andi_16_al:		return parseOpcode_andi(DataType::Size::SIZE_16, PARAM_al);
			case OpcodeType::OPCODE_andi_32_d:		return parseOpcode_andi(DataType::Size::SIZE_32, PARAM_d);
			case OpcodeType::OPCODE_andi_32_ai:		return parseOpcode_andi(DataType::Size::SIZE_32, PARAM_ai);
			case OpcodeType::OPCODE_andi_32_pi:		return parseOpcode_andi(DataType::Size::SIZE_32, PARAM_pi);
			case OpcodeType::OPCODE_andi_32_pd:		return parseOpcode_andi(DataType::Size::SIZE_32, PARAM_pd);
			case OpcodeType::OPCODE_andi_32_di:		return parseOpcode_andi(DataType::Size::SIZE_32, PARAM_di);
			case OpcodeType::OPCODE_andi_32_ix:		return parseOpcode_andi(DataType::Size::SIZE_32, PARAM_ix);
			case OpcodeType::OPCODE_andi_32_aw:		return parseOpcode_andi(DataType::Size::SIZE_32, PARAM_aw);
			case OpcodeType::OPCODE_andi_32_al:		return parseOpcode_andi(DataType::Size::SIZE_32, PARAM_al);

			case OpcodeType::OPCODE_andi_16_tos:	return parseOpcode_andi_tos();

			// Bitwise OR
			case OpcodeType::OPCODE_or_8_er_d:		return parseOpcode_or_er(DataType::Size::SIZE_8, PARAM_d);
			case OpcodeType::OPCODE_or_8_er_ai:		return parseOpcode_or_er(DataType::Size::SIZE_8, PARAM_ai);
			case OpcodeType::OPCODE_or_8_er_pi:		return parseOpcode_or_er(DataType::Size::SIZE_8, PARAM_pi);
			case OpcodeType::OPCODE_or_8_er_pi7:	return parseOpcode_or_er(DataType::Size::SIZE_8, PARAM_pi7);
			case OpcodeType::OPCODE_or_8_er_pd:		return parseOpcode_or_er(DataType::Size::SIZE_8, PARAM_pd);
			case OpcodeType::OPCODE_or_8_er_pd7:	return parseOpcode_or_er(DataType::Size::SIZE_8, PARAM_pd7);
			case OpcodeType::OPCODE_or_8_er_di:		return parseOpcode_or_er(DataType::Size::SIZE_8, PARAM_di);
			case OpcodeType::OPCODE_or_8_er_ix:		return parseOpcode_or_er(DataType::Size::SIZE_8, PARAM_ix);
			case OpcodeType::OPCODE_or_8_er_aw:		return parseOpcode_or_er(DataType::Size::SIZE_8, PARAM_aw);
			case OpcodeType::OPCODE_or_8_er_al:		return parseOpcode_or_er(DataType::Size::SIZE_8, PARAM_al);
			case OpcodeType::OPCODE_or_8_er_pcdi:	return parseOpcode_or_er(DataType::Size::SIZE_8, PARAM_pcdi);
			case OpcodeType::OPCODE_or_8_er_pcix:	return parseOpcode_or_er(DataType::Size::SIZE_8, PARAM_pcix);
			case OpcodeType::OPCODE_or_8_er_i:		return parseOpcode_or_er(DataType::Size::SIZE_8, PARAM_i);
			case OpcodeType::OPCODE_or_16_er_d:		return parseOpcode_or_er(DataType::Size::SIZE_16, PARAM_d);
			case OpcodeType::OPCODE_or_16_er_ai:	return parseOpcode_or_er(DataType::Size::SIZE_16, PARAM_ai);
			case OpcodeType::OPCODE_or_16_er_pi:	return parseOpcode_or_er(DataType::Size::SIZE_16, PARAM_pi);
			case OpcodeType::OPCODE_or_16_er_pd:	return parseOpcode_or_er(DataType::Size::SIZE_16, PARAM_pd);
			case OpcodeType::OPCODE_or_16_er_di:	return parseOpcode_or_er(DataType::Size::SIZE_16, PARAM_di);
			case OpcodeType::OPCODE_or_16_er_ix:	return parseOpcode_or_er(DataType::Size::SIZE_16, PARAM_ix);
			case OpcodeType::OPCODE_or_16_er_aw:	return parseOpcode_or_er(DataType::Size::SIZE_16, PARAM_aw);
			case OpcodeType::OPCODE_or_16_er_al:	return parseOpcode_or_er(DataType::Size::SIZE_16, PARAM_al);
			case OpcodeType::OPCODE_or_16_er_pcdi:	return parseOpcode_or_er(DataType::Size::SIZE_16, PARAM_pcdi);
			case OpcodeType::OPCODE_or_16_er_pcix:	return parseOpcode_or_er(DataType::Size::SIZE_16, PARAM_pcix);
			case OpcodeType::OPCODE_or_16_er_i:		return parseOpcode_or_er(DataType::Size::SIZE_16, PARAM_i);
			case OpcodeType::OPCODE_or_32_er_d:		return parseOpcode_or_er(DataType::Size::SIZE_32, PARAM_d);
			case OpcodeType::OPCODE_or_32_er_ai:	return parseOpcode_or_er(DataType::Size::SIZE_32, PARAM_ai);
			case OpcodeType::OPCODE_or_32_er_pi:	return parseOpcode_or_er(DataType::Size::SIZE_32, PARAM_pi);
			case OpcodeType::OPCODE_or_32_er_pd:	return parseOpcode_or_er(DataType::Size::SIZE_32, PARAM_pd);
			case OpcodeType::OPCODE_or_32_er_di:	return parseOpcode_or_er(DataType::Size::SIZE_32, PARAM_di);
			case OpcodeType::OPCODE_or_32_er_ix:	return parseOpcode_or_er(DataType::Size::SIZE_32, PARAM_ix);
			case OpcodeType::OPCODE_or_32_er_aw:	return parseOpcode_or_er(DataType::Size::SIZE_32, PARAM_aw);
			case OpcodeType::OPCODE_or_32_er_al:	return parseOpcode_or_er(DataType::Size::SIZE_32, PARAM_al);
			case OpcodeType::OPCODE_or_32_er_pcdi:	return parseOpcode_or_er(DataType::Size::SIZE_32, PARAM_pcdi);
			case OpcodeType::OPCODE_or_32_er_pcix:	return parseOpcode_or_er(DataType::Size::SIZE_32, PARAM_pcix);
			case OpcodeType::OPCODE_or_32_er_i:		return parseOpcode_or_er(DataType::Size::SIZE_32, PARAM_i);
			case OpcodeType::OPCODE_or_8_re_ai:		return parseOpcode_or_re(DataType::Size::SIZE_8, PARAM_ai);
			case OpcodeType::OPCODE_or_8_re_pi:		return parseOpcode_or_re(DataType::Size::SIZE_8, PARAM_pi);
			case OpcodeType::OPCODE_or_8_re_pi7:	return parseOpcode_or_re(DataType::Size::SIZE_8, PARAM_pi7);
			case OpcodeType::OPCODE_or_8_re_pd:		return parseOpcode_or_re(DataType::Size::SIZE_8, PARAM_pd);
			case OpcodeType::OPCODE_or_8_re_pd7:	return parseOpcode_or_re(DataType::Size::SIZE_8, PARAM_pd7);
			case OpcodeType::OPCODE_or_8_re_di:		return parseOpcode_or_re(DataType::Size::SIZE_8, PARAM_di);
			case OpcodeType::OPCODE_or_8_re_ix:		return parseOpcode_or_re(DataType::Size::SIZE_8, PARAM_ix);
			case OpcodeType::OPCODE_or_8_re_aw:		return parseOpcode_or_re(DataType::Size::SIZE_8, PARAM_aw);
			case OpcodeType::OPCODE_or_8_re_al:		return parseOpcode_or_re(DataType::Size::SIZE_8, PARAM_al);
			case OpcodeType::OPCODE_or_16_re_ai:	return parseOpcode_or_re(DataType::Size::SIZE_16, PARAM_ai);
			case OpcodeType::OPCODE_or_16_re_pi:	return parseOpcode_or_re(DataType::Size::SIZE_16, PARAM_pi);
			case OpcodeType::OPCODE_or_16_re_pd:	return parseOpcode_or_re(DataType::Size::SIZE_16, PARAM_pd);
			case OpcodeType::OPCODE_or_16_re_di:	return parseOpcode_or_re(DataType::Size::SIZE_16, PARAM_di);
			case OpcodeType::OPCODE_or_16_re_ix:	return parseOpcode_or_re(DataType::Size::SIZE_16, PARAM_ix);
			case OpcodeType::OPCODE_or_16_re_aw:	return parseOpcode_or_re(DataType::Size::SIZE_16, PARAM_aw);
			case OpcodeType::OPCODE_or_16_re_al:	return parseOpcode_or_re(DataType::Size::SIZE_16, PARAM_al);
			case OpcodeType::OPCODE_or_32_re_ai:	return parseOpcode_or_re(DataType::Size::SIZE_32, PARAM_ai);
			case OpcodeType::OPCODE_or_32_re_pi:	return parseOpcode_or_re(DataType::Size::SIZE_32, PARAM_pi);
			case OpcodeType::OPCODE_or_32_re_pd:	return parseOpcode_or_re(DataType::Size::SIZE_32, PARAM_pd);
			case OpcodeType::OPCODE_or_32_re_di:	return parseOpcode_or_re(DataType::Size::SIZE_32, PARAM_di);
			case OpcodeType::OPCODE_or_32_re_ix:	return parseOpcode_or_re(DataType::Size::SIZE_32, PARAM_ix);
			case OpcodeType::OPCODE_or_32_re_aw:	return parseOpcode_or_re(DataType::Size::SIZE_32, PARAM_aw);
			case OpcodeType::OPCODE_or_32_re_al:	return parseOpcode_or_re(DataType::Size::SIZE_32, PARAM_al);
			case OpcodeType::OPCODE_ori_8_d:		return parseOpcode_ori(DataType::Size::SIZE_8, PARAM_d);
			case OpcodeType::OPCODE_ori_8_ai:		return parseOpcode_ori(DataType::Size::SIZE_8, PARAM_ai);
			case OpcodeType::OPCODE_ori_8_pi:		return parseOpcode_ori(DataType::Size::SIZE_8, PARAM_pi);
			case OpcodeType::OPCODE_ori_8_pi7:		return parseOpcode_ori(DataType::Size::SIZE_8, PARAM_pi7);
			case OpcodeType::OPCODE_ori_8_pd:		return parseOpcode_ori(DataType::Size::SIZE_8, PARAM_pd);
			case OpcodeType::OPCODE_ori_8_pd7:		return parseOpcode_ori(DataType::Size::SIZE_8, PARAM_pd7);
			case OpcodeType::OPCODE_ori_8_di:		return parseOpcode_ori(DataType::Size::SIZE_8, PARAM_di);
			case OpcodeType::OPCODE_ori_8_ix:		return parseOpcode_ori(DataType::Size::SIZE_8, PARAM_ix);
			case OpcodeType::OPCODE_ori_8_aw:		return parseOpcode_ori(DataType::Size::SIZE_8, PARAM_aw);
			case OpcodeType::OPCODE_ori_8_al:		return parseOpcode_ori(DataType::Size::SIZE_8, PARAM_al);
			case OpcodeType::OPCODE_ori_16_d:		return parseOpcode_ori(DataType::Size::SIZE_16, PARAM_d);
			case OpcodeType::OPCODE_ori_16_ai:		return parseOpcode_ori(DataType::Size::SIZE_16, PARAM_ai);
			case OpcodeType::OPCODE_ori_16_pi:		return parseOpcode_ori(DataType::Size::SIZE_16, PARAM_pi);
			case OpcodeType::OPCODE_ori_16_pd:		return parseOpcode_ori(DataType::Size::SIZE_16, PARAM_pd);
			case OpcodeType::OPCODE_ori_16_di:		return parseOpcode_ori(DataType::Size::SIZE_16, PARAM_di);
			case OpcodeType::OPCODE_ori_16_ix:		return parseOpcode_ori(DataType::Size::SIZE_16, PARAM_ix);
			case OpcodeType::OPCODE_ori_16_aw:		return parseOpcode_ori(DataType::Size::SIZE_16, PARAM_aw);
			case OpcodeType::OPCODE_ori_16_al:		return parseOpcode_ori(DataType::Size::SIZE_16, PARAM_al);
			case OpcodeType::OPCODE_ori_32_d:		return parseOpcode_ori(DataType::Size::SIZE_32, PARAM_d);
			case OpcodeType::OPCODE_ori_32_ai:		return parseOpcode_ori(DataType::Size::SIZE_32, PARAM_ai);
			case OpcodeType::OPCODE_ori_32_pi:		return parseOpcode_ori(DataType::Size::SIZE_32, PARAM_pi);
			case OpcodeType::OPCODE_ori_32_pd:		return parseOpcode_ori(DataType::Size::SIZE_32, PARAM_pd);
			case OpcodeType::OPCODE_ori_32_di:		return parseOpcode_ori(DataType::Size::SIZE_32, PARAM_di);
			case OpcodeType::OPCODE_ori_32_ix:		return parseOpcode_ori(DataType::Size::SIZE_32, PARAM_ix);
			case OpcodeType::OPCODE_ori_32_aw:		return parseOpcode_ori(DataType::Size::SIZE_32, PARAM_aw);
			case OpcodeType::OPCODE_ori_32_al:		return parseOpcode_ori(DataType::Size::SIZE_32, PARAM_al);

			case OpcodeType::OPCODE_ori_16_tos:		return parseOpcode_ori_tos();

			// Bitwise XOR
			case OpcodeType::OPCODE_eor_8_d:		return parseOpcode_eor(DataType::Size::SIZE_8, PARAM_d);
			case OpcodeType::OPCODE_eor_8_ai:		return parseOpcode_eor(DataType::Size::SIZE_8, PARAM_ai);
			case OpcodeType::OPCODE_eor_8_pi:		return parseOpcode_eor(DataType::Size::SIZE_8, PARAM_pi);
			case OpcodeType::OPCODE_eor_8_pi7:		return parseOpcode_eor(DataType::Size::SIZE_8, PARAM_pi7);
			case OpcodeType::OPCODE_eor_8_pd:		return parseOpcode_eor(DataType::Size::SIZE_8, PARAM_pd);
			case OpcodeType::OPCODE_eor_8_pd7:		return parseOpcode_eor(DataType::Size::SIZE_8, PARAM_pd7);
			case OpcodeType::OPCODE_eor_8_di:		return parseOpcode_eor(DataType::Size::SIZE_8, PARAM_di);
			case OpcodeType::OPCODE_eor_8_ix:		return parseOpcode_eor(DataType::Size::SIZE_8, PARAM_ix);
			case OpcodeType::OPCODE_eor_8_aw:		return parseOpcode_eor(DataType::Size::SIZE_8, PARAM_aw);
			case OpcodeType::OPCODE_eor_8_al:		return parseOpcode_eor(DataType::Size::SIZE_8, PARAM_al);
			case OpcodeType::OPCODE_eor_16_d:		return parseOpcode_eor(DataType::Size::SIZE_16, PARAM_d);
			case OpcodeType::OPCODE_eor_16_ai:		return parseOpcode_eor(DataType::Size::SIZE_16, PARAM_ai);
			case OpcodeType::OPCODE_eor_16_pi:		return parseOpcode_eor(DataType::Size::SIZE_16, PARAM_pi);
			case OpcodeType::OPCODE_eor_16_pd:		return parseOpcode_eor(DataType::Size::SIZE_16, PARAM_pd);
			case OpcodeType::OPCODE_eor_16_di:		return parseOpcode_eor(DataType::Size::SIZE_16, PARAM_di);
			case OpcodeType::OPCODE_eor_16_ix:		return parseOpcode_eor(DataType::Size::SIZE_16, PARAM_ix);
			case OpcodeType::OPCODE_eor_16_aw:		return parseOpcode_eor(DataType::Size::SIZE_16, PARAM_aw);
			case OpcodeType::OPCODE_eor_16_al:		return parseOpcode_eor(DataType::Size::SIZE_16, PARAM_al);
			case OpcodeType::OPCODE_eor_32_d:		return parseOpcode_eor(DataType::Size::SIZE_32, PARAM_d);
			case OpcodeType::OPCODE_eor_32_ai:		return parseOpcode_eor(DataType::Size::SIZE_32, PARAM_ai);
			case OpcodeType::OPCODE_eor_32_pi:		return parseOpcode_eor(DataType::Size::SIZE_32, PARAM_pi);
			case OpcodeType::OPCODE_eor_32_pd:		return parseOpcode_eor(DataType::Size::SIZE_32, PARAM_pd);
			case OpcodeType::OPCODE_eor_32_di:		return parseOpcode_eor(DataType::Size::SIZE_32, PARAM_di);
			case OpcodeType::OPCODE_eor_32_ix:		return parseOpcode_eor(DataType::Size::SIZE_32, PARAM_ix);
			case OpcodeType::OPCODE_eor_32_aw:		return parseOpcode_eor(DataType::Size::SIZE_32, PARAM_aw);
			case OpcodeType::OPCODE_eor_32_al:		return parseOpcode_eor(DataType::Size::SIZE_32, PARAM_al);
			case OpcodeType::OPCODE_eori_8_d:		return parseOpcode_eori(DataType::Size::SIZE_8, PARAM_d);
			case OpcodeType::OPCODE_eori_8_ai:		return parseOpcode_eori(DataType::Size::SIZE_8, PARAM_ai);
			case OpcodeType::OPCODE_eori_8_pi:		return parseOpcode_eori(DataType::Size::SIZE_8, PARAM_pi);
			case OpcodeType::OPCODE_eori_8_pi7:		return parseOpcode_eori(DataType::Size::SIZE_8, PARAM_pi7);
			case OpcodeType::OPCODE_eori_8_pd:		return parseOpcode_eori(DataType::Size::SIZE_8, PARAM_pd);
			case OpcodeType::OPCODE_eori_8_pd7:		return parseOpcode_eori(DataType::Size::SIZE_8, PARAM_pd7);
			case OpcodeType::OPCODE_eori_8_di:		return parseOpcode_eori(DataType::Size::SIZE_8, PARAM_di);
			case OpcodeType::OPCODE_eori_8_ix:		return parseOpcode_eori(DataType::Size::SIZE_8, PARAM_ix);
			case OpcodeType::OPCODE_eori_8_aw:		return parseOpcode_eori(DataType::Size::SIZE_8, PARAM_aw);
			case OpcodeType::OPCODE_eori_8_al:		return parseOpcode_eori(DataType::Size::SIZE_8, PARAM_al);
			case OpcodeType::OPCODE_eori_16_d:		return parseOpcode_eori(DataType::Size::SIZE_16, PARAM_d);
			case OpcodeType::OPCODE_eori_16_ai:		return parseOpcode_eori(DataType::Size::SIZE_16, PARAM_ai);
			case OpcodeType::OPCODE_eori_16_pi:		return parseOpcode_eori(DataType::Size::SIZE_16, PARAM_pi);
			case OpcodeType::OPCODE_eori_16_pd:		return parseOpcode_eori(DataType::Size::SIZE_16, PARAM_pd);
			case OpcodeType::OPCODE_eori_16_di:		return parseOpcode_eori(DataType::Size::SIZE_16, PARAM_di);
			case OpcodeType::OPCODE_eori_16_ix:		return parseOpcode_eori(DataType::Size::SIZE_16, PARAM_ix);
			case OpcodeType::OPCODE_eori_16_aw:		return parseOpcode_eori(DataType::Size::SIZE_16, PARAM_aw);
			case OpcodeType::OPCODE_eori_16_al:		return parseOpcode_eori(DataType::Size::SIZE_16, PARAM_al);
			case OpcodeType::OPCODE_eori_32_d:		return parseOpcode_eori(DataType::Size::SIZE_32, PARAM_d);
			case OpcodeType::OPCODE_eori_32_ai:		return parseOpcode_eori(DataType::Size::SIZE_32, PARAM_ai);
			case OpcodeType::OPCODE_eori_32_pi:		return parseOpcode_eori(DataType::Size::SIZE_32, PARAM_pi);
			case OpcodeType::OPCODE_eori_32_pd:		return parseOpcode_eori(DataType::Size::SIZE_32, PARAM_pd);
			case OpcodeType::OPCODE_eori_32_di:		return parseOpcode_eori(DataType::Size::SIZE_32, PARAM_di);
			case OpcodeType::OPCODE_eori_32_ix:		return parseOpcode_eori(DataType::Size::SIZE_32, PARAM_ix);
			case OpcodeType::OPCODE_eori_32_aw:		return parseOpcode_eori(DataType::Size::SIZE_32, PARAM_aw);
			case OpcodeType::OPCODE_eori_32_al:		return parseOpcode_eori(DataType::Size::SIZE_32, PARAM_al);

			// Bit set
			case OpcodeType::OPCODE_bset_8_r_ai:	return parseOpcode_bset_r(DataType::Size::SIZE_8, PARAM_ai);
			case OpcodeType::OPCODE_bset_8_r_pi:	return parseOpcode_bset_r(DataType::Size::SIZE_8, PARAM_pi);
			case OpcodeType::OPCODE_bset_8_r_pi7:	return parseOpcode_bset_r(DataType::Size::SIZE_8, PARAM_pi7);
			case OpcodeType::OPCODE_bset_8_r_pd:	return parseOpcode_bset_r(DataType::Size::SIZE_8, PARAM_pd);
			case OpcodeType::OPCODE_bset_8_r_pd7:	return parseOpcode_bset_r(DataType::Size::SIZE_8, PARAM_pd7);
			case OpcodeType::OPCODE_bset_8_r_di:	return parseOpcode_bset_r(DataType::Size::SIZE_8, PARAM_di);
			case OpcodeType::OPCODE_bset_8_r_ix:	return parseOpcode_bset_r(DataType::Size::SIZE_8, PARAM_ix);
			case OpcodeType::OPCODE_bset_8_r_aw:	return parseOpcode_bset_r(DataType::Size::SIZE_8, PARAM_aw);
			case OpcodeType::OPCODE_bset_8_r_al:	return parseOpcode_bset_r(DataType::Size::SIZE_8, PARAM_al);
			case OpcodeType::OPCODE_bset_32_r_d:	return parseOpcode_bset_r(DataType::Size::SIZE_32, PARAM_d);
			case OpcodeType::OPCODE_bset_8_s_ai:	return parseOpcode_bset_s(DataType::Size::SIZE_8, PARAM_ai);
			case OpcodeType::OPCODE_bset_8_s_pi:	return parseOpcode_bset_s(DataType::Size::SIZE_8, PARAM_pi);
			case OpcodeType::OPCODE_bset_8_s_pi7:	return parseOpcode_bset_s(DataType::Size::SIZE_8, PARAM_pi7);
			case OpcodeType::OPCODE_bset_8_s_pd:	return parseOpcode_bset_s(DataType::Size::SIZE_8, PARAM_pd);
			case OpcodeType::OPCODE_bset_8_s_pd7:	return parseOpcode_bset_s(DataType::Size::SIZE_8, PARAM_pd7);
			case OpcodeType::OPCODE_bset_8_s_di:	return parseOpcode_bset_s(DataType::Size::SIZE_8, PARAM_di);
			case OpcodeType::OPCODE_bset_8_s_ix:	return parseOpcode_bset_s(DataType::Size::SIZE_8, PARAM_ix);
			case OpcodeType::OPCODE_bset_8_s_aw:	return parseOpcode_bset_s(DataType::Size::SIZE_8, PARAM_aw);
			case OpcodeType::OPCODE_bset_8_s_al:	return parseOpcode_bset_s(DataType::Size::SIZE_8, PARAM_al);
			case OpcodeType::OPCODE_bset_32_s_d:	return parseOpcode_bset_s(DataType::Size::SIZE_32, PARAM_d);

			// Bit clear
			case OpcodeType::OPCODE_bclr_8_r_ai:	return parseOpcode_bclr_r(DataType::Size::SIZE_8, PARAM_ai);
			case OpcodeType::OPCODE_bclr_8_r_pi:	return parseOpcode_bclr_r(DataType::Size::SIZE_8, PARAM_pi);
			case OpcodeType::OPCODE_bclr_8_r_pi7:	return parseOpcode_bclr_r(DataType::Size::SIZE_8, PARAM_pi7);
			case OpcodeType::OPCODE_bclr_8_r_pd:	return parseOpcode_bclr_r(DataType::Size::SIZE_8, PARAM_pd);
			case OpcodeType::OPCODE_bclr_8_r_pd7:	return parseOpcode_bclr_r(DataType::Size::SIZE_8, PARAM_pd7);
			case OpcodeType::OPCODE_bclr_8_r_di:	return parseOpcode_bclr_r(DataType::Size::SIZE_8, PARAM_di);
			case OpcodeType::OPCODE_bclr_8_r_ix:	return parseOpcode_bclr_r(DataType::Size::SIZE_8, PARAM_ix);
			case OpcodeType::OPCODE_bclr_8_r_aw:	return parseOpcode_bclr_r(DataType::Size::SIZE_8, PARAM_aw);
			case OpcodeType::OPCODE_bclr_8_r_al:	return parseOpcode_bclr_r(DataType::Size::SIZE_8, PARAM_al);
			case OpcodeType::OPCODE_bclr_32_r_d:	return parseOpcode_bclr_r(DataType::Size::SIZE_32, PARAM_d);
			case OpcodeType::OPCODE_bclr_8_s_ai:	return parseOpcode_bclr_s(DataType::Size::SIZE_8, PARAM_ai);
			case OpcodeType::OPCODE_bclr_8_s_pi:	return parseOpcode_bclr_s(DataType::Size::SIZE_8, PARAM_pi);
			case OpcodeType::OPCODE_bclr_8_s_pi7:	return parseOpcode_bclr_s(DataType::Size::SIZE_8, PARAM_pi7);
			case OpcodeType::OPCODE_bclr_8_s_pd:	return parseOpcode_bclr_s(DataType::Size::SIZE_8, PARAM_pd);
			case OpcodeType::OPCODE_bclr_8_s_pd7:	return parseOpcode_bclr_s(DataType::Size::SIZE_8, PARAM_pd7);
			case OpcodeType::OPCODE_bclr_8_s_di:	return parseOpcode_bclr_s(DataType::Size::SIZE_8, PARAM_di);
			case OpcodeType::OPCODE_bclr_8_s_ix:	return parseOpcode_bclr_s(DataType::Size::SIZE_8, PARAM_ix);
			case OpcodeType::OPCODE_bclr_8_s_aw:	return parseOpcode_bclr_s(DataType::Size::SIZE_8, PARAM_aw);
			case OpcodeType::OPCODE_bclr_8_s_al:	return parseOpcode_bclr_s(DataType::Size::SIZE_8, PARAM_al);
			case OpcodeType::OPCODE_bclr_32_s_d:	return parseOpcode_bclr_s(DataType::Size::SIZE_32, PARAM_d);

			// Bit change
			case OpcodeType::OPCODE_bchg_8_r_ai:	return parseOpcode_bchg_r(DataType::Size::SIZE_8, PARAM_ai);
			case OpcodeType::OPCODE_bchg_8_r_pi:	return parseOpcode_bchg_r(DataType::Size::SIZE_8, PARAM_pi);
			case OpcodeType::OPCODE_bchg_8_r_pi7:	return parseOpcode_bchg_r(DataType::Size::SIZE_8, PARAM_pi7);
			case OpcodeType::OPCODE_bchg_8_r_pd:	return parseOpcode_bchg_r(DataType::Size::SIZE_8, PARAM_pd);
			case OpcodeType::OPCODE_bchg_8_r_pd7:	return parseOpcode_bchg_r(DataType::Size::SIZE_8, PARAM_pd7);
			case OpcodeType::OPCODE_bchg_8_r_di:	return parseOpcode_bchg_r(DataType::Size::SIZE_8, PARAM_di);
			case OpcodeType::OPCODE_bchg_8_r_ix:	return parseOpcode_bchg_r(DataType::Size::SIZE_8, PARAM_ix);
			case OpcodeType::OPCODE_bchg_8_r_aw:	return parseOpcode_bchg_r(DataType::Size::SIZE_8, PARAM_aw);
			case OpcodeType::OPCODE_bchg_8_r_al:	return parseOpcode_bchg_r(DataType::Size::SIZE_8, PARAM_al);
			case OpcodeType::OPCODE_bchg_32_r_d:	return parseOpcode_bchg_r(DataType::Size::SIZE_32, PARAM_d);
			case OpcodeType::OPCODE_bchg_8_s_ai:	return parseOpcode_bchg_s(DataType::Size::SIZE_8, PARAM_ai);
			case OpcodeType::OPCODE_bchg_8_s_pi:	return parseOpcode_bchg_s(DataType::Size::SIZE_8, PARAM_pi);
			case OpcodeType::OPCODE_bchg_8_s_pi7:	return parseOpcode_bchg_s(DataType::Size::SIZE_8, PARAM_pi7);
			case OpcodeType::OPCODE_bchg_8_s_pd:	return parseOpcode_bchg_s(DataType::Size::SIZE_8, PARAM_pd);
			case OpcodeType::OPCODE_bchg_8_s_pd7:	return parseOpcode_bchg_s(DataType::Size::SIZE_8, PARAM_pd7);
			case OpcodeType::OPCODE_bchg_8_s_di:	return parseOpcode_bchg_s(DataType::Size::SIZE_8, PARAM_di);
			case OpcodeType::OPCODE_bchg_8_s_ix:	return parseOpcode_bchg_s(DataType::Size::SIZE_8, PARAM_ix);
			case OpcodeType::OPCODE_bchg_8_s_aw:	return parseOpcode_bchg_s(DataType::Size::SIZE_8, PARAM_aw);
			case OpcodeType::OPCODE_bchg_8_s_al:	return parseOpcode_bchg_s(DataType::Size::SIZE_8, PARAM_al);
			case OpcodeType::OPCODE_bchg_32_s_d:	return parseOpcode_bchg_s(DataType::Size::SIZE_32, PARAM_d);

			// Bit test
			case OpcodeType::OPCODE_btst_8_r_ai:	return parseOpcode_btst_r(DataType::Size::SIZE_8, PARAM_ai);
			case OpcodeType::OPCODE_btst_8_r_pi:	return parseOpcode_btst_r(DataType::Size::SIZE_8, PARAM_pi);
			case OpcodeType::OPCODE_btst_8_r_pi7:	return parseOpcode_btst_r(DataType::Size::SIZE_8, PARAM_pi7);
			case OpcodeType::OPCODE_btst_8_r_pd:	return parseOpcode_btst_r(DataType::Size::SIZE_8, PARAM_pd);
			case OpcodeType::OPCODE_btst_8_r_pd7:	return parseOpcode_btst_r(DataType::Size::SIZE_8, PARAM_pd7);
			case OpcodeType::OPCODE_btst_8_r_di:	return parseOpcode_btst_r(DataType::Size::SIZE_8, PARAM_di);
			case OpcodeType::OPCODE_btst_8_r_ix:	return parseOpcode_btst_r(DataType::Size::SIZE_8, PARAM_ix);
			case OpcodeType::OPCODE_btst_8_r_aw:	return parseOpcode_btst_r(DataType::Size::SIZE_8, PARAM_aw);
			case OpcodeType::OPCODE_btst_8_r_al:	return parseOpcode_btst_r(DataType::Size::SIZE_8, PARAM_al);
			case OpcodeType::OPCODE_btst_8_r_pcdi:	return parseOpcode_btst_r(DataType::Size::SIZE_8, PARAM_pcdi);
			case OpcodeType::OPCODE_btst_8_r_pcix:	return parseOpcode_btst_r(DataType::Size::SIZE_8, PARAM_pcix);
			case OpcodeType::OPCODE_btst_8_r_i:		return parseOpcode_btst_r(DataType::Size::SIZE_8, PARAM_i);
			case OpcodeType::OPCODE_btst_32_r_d:	return parseOpcode_btst_r(DataType::Size::SIZE_32, PARAM_d);
			case OpcodeType::OPCODE_btst_8_s_ai:	return parseOpcode_btst_s(DataType::Size::SIZE_8, PARAM_ai);
			case OpcodeType::OPCODE_btst_8_s_pi:	return parseOpcode_btst_s(DataType::Size::SIZE_8, PARAM_pi);
			case OpcodeType::OPCODE_btst_8_s_pi7:	return parseOpcode_btst_s(DataType::Size::SIZE_8, PARAM_pi7);
			case OpcodeType::OPCODE_btst_8_s_pd:	return parseOpcode_btst_s(DataType::Size::SIZE_8, PARAM_pd);
			case OpcodeType::OPCODE_btst_8_s_pd7:	return parseOpcode_btst_s(DataType::Size::SIZE_8, PARAM_pd7);
			case OpcodeType::OPCODE_btst_8_s_di:	return parseOpcode_btst_s(DataType::Size::SIZE_8, PARAM_di);
			case OpcodeType::OPCODE_btst_8_s_ix:	return parseOpcode_btst_s(DataType::Size::SIZE_8, PARAM_ix);
			case OpcodeType::OPCODE_btst_8_s_aw:	return parseOpcode_btst_s(DataType::Size::SIZE_8, PARAM_aw);
			case OpcodeType::OPCODE_btst_8_s_al:	return parseOpcode_btst_s(DataType::Size::SIZE_8, PARAM_al);
			case OpcodeType::OPCODE_btst_8_s_pcdi:	return parseOpcode_btst_s(DataType::Size::SIZE_8, PARAM_pcdi);
			case OpcodeType::OPCODE_btst_8_s_pcix:	return parseOpcode_btst_s(DataType::Size::SIZE_8, PARAM_pcix);
			case OpcodeType::OPCODE_btst_32_s_d:	return parseOpcode_btst_s(DataType::Size::SIZE_32, PARAM_d);

			// Set all bits
			case OpcodeType::OPCODE_st_8_d:			return parseOpcode_st(DataType::Size::SIZE_8, PARAM_d);
			case OpcodeType::OPCODE_st_8_ai:		return parseOpcode_st(DataType::Size::SIZE_8, PARAM_ai);
			case OpcodeType::OPCODE_st_8_pi:		return parseOpcode_st(DataType::Size::SIZE_8, PARAM_pi);
			case OpcodeType::OPCODE_st_8_pi7:		return parseOpcode_st(DataType::Size::SIZE_8, PARAM_pi7);
			case OpcodeType::OPCODE_st_8_pd:		return parseOpcode_st(DataType::Size::SIZE_8, PARAM_pd);
			case OpcodeType::OPCODE_st_8_pd7:		return parseOpcode_st(DataType::Size::SIZE_8, PARAM_pd7);
			case OpcodeType::OPCODE_st_8_di:		return parseOpcode_st(DataType::Size::SIZE_8, PARAM_di);
			case OpcodeType::OPCODE_st_8_ix:		return parseOpcode_st(DataType::Size::SIZE_8, PARAM_ix);
			case OpcodeType::OPCODE_st_8_aw:		return parseOpcode_st(DataType::Size::SIZE_8, PARAM_aw);
			case OpcodeType::OPCODE_st_8_al:		return parseOpcode_st(DataType::Size::SIZE_8, PARAM_al);

			// Clear all bits
			case OpcodeType::OPCODE_sf_8_d:			return parseOpcode_sf(DataType::Size::SIZE_8, PARAM_d);
			case OpcodeType::OPCODE_sf_8_ai:		return parseOpcode_sf(DataType::Size::SIZE_8, PARAM_ai);
			case OpcodeType::OPCODE_sf_8_pi:		return parseOpcode_sf(DataType::Size::SIZE_8, PARAM_pi);
			case OpcodeType::OPCODE_sf_8_pi7:		return parseOpcode_sf(DataType::Size::SIZE_8, PARAM_pi7);
			case OpcodeType::OPCODE_sf_8_pd:		return parseOpcode_sf(DataType::Size::SIZE_8, PARAM_pd);
			case OpcodeType::OPCODE_sf_8_pd7:		return parseOpcode_sf(DataType::Size::SIZE_8, PARAM_pd7);
			case OpcodeType::OPCODE_sf_8_di:		return parseOpcode_sf(DataType::Size::SIZE_8, PARAM_di);
			case OpcodeType::OPCODE_sf_8_ix:		return parseOpcode_sf(DataType::Size::SIZE_8, PARAM_ix);
			case OpcodeType::OPCODE_sf_8_aw:		return parseOpcode_sf(DataType::Size::SIZE_8, PARAM_aw);
			case OpcodeType::OPCODE_sf_8_al:		return parseOpcode_sf(DataType::Size::SIZE_8, PARAM_al);

			// Set all bits by condition
			case OpcodeType::OPCODE_shi_8_d:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_d,   Condition::HI);
			case OpcodeType::OPCODE_shi_8_ai:  		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_ai,  Condition::HI);
			case OpcodeType::OPCODE_shi_8_pi:  		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pi,  Condition::HI);
			case OpcodeType::OPCODE_shi_8_pi7: 		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pi7, Condition::HI);
			case OpcodeType::OPCODE_shi_8_pd:  		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pd,  Condition::HI);
			case OpcodeType::OPCODE_shi_8_pd7: 		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pd7, Condition::HI);
			case OpcodeType::OPCODE_shi_8_di:  		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_di,  Condition::HI);
			case OpcodeType::OPCODE_shi_8_ix:  		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_ix,  Condition::HI);
			case OpcodeType::OPCODE_shi_8_aw:  		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_aw,  Condition::HI);
			case OpcodeType::OPCODE_shi_8_al:  		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_al,  Condition::HI);

			case OpcodeType::OPCODE_sls_8_d:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_d,   Condition::LS);
			case OpcodeType::OPCODE_sls_8_ai:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_ai,  Condition::LS);
			case OpcodeType::OPCODE_sls_8_pi:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pi,  Condition::LS);
			case OpcodeType::OPCODE_sls_8_pi7:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pi7, Condition::LS);
			case OpcodeType::OPCODE_sls_8_pd:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pd,  Condition::LS);
			case OpcodeType::OPCODE_sls_8_pd7:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pd7, Condition::LS);
			case OpcodeType::OPCODE_sls_8_di:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_di,  Condition::LS);
			case OpcodeType::OPCODE_sls_8_ix:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_ix,  Condition::LS);
			case OpcodeType::OPCODE_sls_8_aw:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_aw,  Condition::LS);
			case OpcodeType::OPCODE_sls_8_al:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_al,  Condition::LS);

			case OpcodeType::OPCODE_scc_8_d:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_d,   Condition::CC);
			case OpcodeType::OPCODE_scc_8_ai:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_ai,  Condition::CC);
			case OpcodeType::OPCODE_scc_8_pi:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pi,  Condition::CC);
			case OpcodeType::OPCODE_scc_8_pi7:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pi7, Condition::CC);
			case OpcodeType::OPCODE_scc_8_pd:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pd,  Condition::CC);
			case OpcodeType::OPCODE_scc_8_pd7:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pd7, Condition::CC);
			case OpcodeType::OPCODE_scc_8_di:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_di,  Condition::CC);
			case OpcodeType::OPCODE_scc_8_ix:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_ix,  Condition::CC);
			case OpcodeType::OPCODE_scc_8_aw:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_aw,  Condition::CC);
			case OpcodeType::OPCODE_scc_8_al:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_al,  Condition::CC);

			case OpcodeType::OPCODE_scs_8_d:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_d,   Condition::CS);
			case OpcodeType::OPCODE_scs_8_ai:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_ai,  Condition::CS);
			case OpcodeType::OPCODE_scs_8_pi:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pi,  Condition::CS);
			case OpcodeType::OPCODE_scs_8_pi7:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pi7, Condition::CS);
			case OpcodeType::OPCODE_scs_8_pd:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pd,  Condition::CS);
			case OpcodeType::OPCODE_scs_8_pd7:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pd7, Condition::CS);
			case OpcodeType::OPCODE_scs_8_di:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_di,  Condition::CS);
			case OpcodeType::OPCODE_scs_8_ix:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_ix,  Condition::CS);
			case OpcodeType::OPCODE_scs_8_aw:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_aw,  Condition::CS);
			case OpcodeType::OPCODE_scs_8_al:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_al,  Condition::CS);

			case OpcodeType::OPCODE_sne_8_d:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_d,   Condition::NE);
			case OpcodeType::OPCODE_sne_8_ai:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_ai,  Condition::NE);
			case OpcodeType::OPCODE_sne_8_pi:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pi,  Condition::NE);
			case OpcodeType::OPCODE_sne_8_pi7:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pi7, Condition::NE);
			case OpcodeType::OPCODE_sne_8_pd:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pd,  Condition::NE);
			case OpcodeType::OPCODE_sne_8_pd7:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pd7, Condition::NE);
			case OpcodeType::OPCODE_sne_8_di:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_di,  Condition::NE);
			case OpcodeType::OPCODE_sne_8_ix:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_ix,  Condition::NE);
			case OpcodeType::OPCODE_sne_8_aw:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_aw,  Condition::NE);
			case OpcodeType::OPCODE_sne_8_al:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_al,  Condition::NE);

			case OpcodeType::OPCODE_seq_8_d:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_d,   Condition::EQ);
			case OpcodeType::OPCODE_seq_8_ai:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_ai,  Condition::EQ);
			case OpcodeType::OPCODE_seq_8_pi:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pi,  Condition::EQ);
			case OpcodeType::OPCODE_seq_8_pi7:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pi7, Condition::EQ);
			case OpcodeType::OPCODE_seq_8_pd:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pd,  Condition::EQ);
			case OpcodeType::OPCODE_seq_8_pd7:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pd7, Condition::EQ);
			case OpcodeType::OPCODE_seq_8_di:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_di,  Condition::EQ);
			case OpcodeType::OPCODE_seq_8_ix:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_ix,  Condition::EQ);
			case OpcodeType::OPCODE_seq_8_aw:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_aw,  Condition::EQ);
			case OpcodeType::OPCODE_seq_8_al:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_al,  Condition::EQ);

			case OpcodeType::OPCODE_svc_8_d:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_d,   Condition::VC);
			case OpcodeType::OPCODE_svc_8_ai:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_ai,  Condition::VC);
			case OpcodeType::OPCODE_svc_8_pi:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pi,  Condition::VC);
			case OpcodeType::OPCODE_svc_8_pi7:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pi7, Condition::VC);
			case OpcodeType::OPCODE_svc_8_pd:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pd,  Condition::VC);
			case OpcodeType::OPCODE_svc_8_pd7:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pd7, Condition::VC);
			case OpcodeType::OPCODE_svc_8_di:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_di,  Condition::VC);
			case OpcodeType::OPCODE_svc_8_ix:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_ix,  Condition::VC);
			case OpcodeType::OPCODE_svc_8_aw:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_aw,  Condition::VC);
			case OpcodeType::OPCODE_svc_8_al:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_al,  Condition::VC);

			case OpcodeType::OPCODE_svs_8_d:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_d,   Condition::VS);
			case OpcodeType::OPCODE_svs_8_ai:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_ai,  Condition::VS);
			case OpcodeType::OPCODE_svs_8_pi:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pi,  Condition::VS);
			case OpcodeType::OPCODE_svs_8_pi7:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pi7, Condition::VS);
			case OpcodeType::OPCODE_svs_8_pd:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pd,  Condition::VS);
			case OpcodeType::OPCODE_svs_8_pd7:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pd7, Condition::VS);
			case OpcodeType::OPCODE_svs_8_di:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_di,  Condition::VS);
			case OpcodeType::OPCODE_svs_8_ix:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_ix,  Condition::VS);
			case OpcodeType::OPCODE_svs_8_aw:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_aw,  Condition::VS);
			case OpcodeType::OPCODE_svs_8_al:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_al,  Condition::VS);

			case OpcodeType::OPCODE_spl_8_d:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_d,   Condition::PL);
			case OpcodeType::OPCODE_spl_8_ai:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_ai,  Condition::PL);
			case OpcodeType::OPCODE_spl_8_pi:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pi,  Condition::PL);
			case OpcodeType::OPCODE_spl_8_pi7:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pi7, Condition::PL);
			case OpcodeType::OPCODE_spl_8_pd:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pd,  Condition::PL);
			case OpcodeType::OPCODE_spl_8_pd7:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pd7, Condition::PL);
			case OpcodeType::OPCODE_spl_8_di:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_di,  Condition::PL);
			case OpcodeType::OPCODE_spl_8_ix:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_ix,  Condition::PL);
			case OpcodeType::OPCODE_spl_8_aw:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_aw,  Condition::PL);
			case OpcodeType::OPCODE_spl_8_al:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_al,  Condition::PL);

			case OpcodeType::OPCODE_smi_8_d:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_d,   Condition::MI);
			case OpcodeType::OPCODE_smi_8_ai:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_ai,  Condition::MI);
			case OpcodeType::OPCODE_smi_8_pi:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pi,  Condition::MI);
			case OpcodeType::OPCODE_smi_8_pi7:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pi7, Condition::MI);
			case OpcodeType::OPCODE_smi_8_pd:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pd,  Condition::MI);
			case OpcodeType::OPCODE_smi_8_pd7:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pd7, Condition::MI);
			case OpcodeType::OPCODE_smi_8_di:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_di,  Condition::MI);
			case OpcodeType::OPCODE_smi_8_ix:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_ix,  Condition::MI);
			case OpcodeType::OPCODE_smi_8_aw:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_aw,  Condition::MI);
			case OpcodeType::OPCODE_smi_8_al:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_al,  Condition::MI);

			case OpcodeType::OPCODE_sge_8_d:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_d,   Condition::GE);
			case OpcodeType::OPCODE_sge_8_ai:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_ai,  Condition::GE);
			case OpcodeType::OPCODE_sge_8_pi:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pi,  Condition::GE);
			case OpcodeType::OPCODE_sge_8_pi7:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pi7, Condition::GE);
			case OpcodeType::OPCODE_sge_8_pd:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pd,  Condition::GE);
			case OpcodeType::OPCODE_sge_8_pd7:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pd7, Condition::GE);
			case OpcodeType::OPCODE_sge_8_di:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_di,  Condition::GE);
			case OpcodeType::OPCODE_sge_8_ix:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_ix,  Condition::GE);
			case OpcodeType::OPCODE_sge_8_aw:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_aw,  Condition::GE);
			case OpcodeType::OPCODE_sge_8_al:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_al,  Condition::GE);

			case OpcodeType::OPCODE_slt_8_d:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_d,   Condition::LT);
			case OpcodeType::OPCODE_slt_8_ai:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_ai,  Condition::LT);
			case OpcodeType::OPCODE_slt_8_pi:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pi,  Condition::LT);
			case OpcodeType::OPCODE_slt_8_pi7:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pi7, Condition::LT);
			case OpcodeType::OPCODE_slt_8_pd:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pd,  Condition::LT);
			case OpcodeType::OPCODE_slt_8_pd7:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pd7, Condition::LT);
			case OpcodeType::OPCODE_slt_8_di:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_di,  Condition::LT);
			case OpcodeType::OPCODE_slt_8_ix:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_ix,  Condition::LT);
			case OpcodeType::OPCODE_slt_8_aw:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_aw,  Condition::LT);
			case OpcodeType::OPCODE_slt_8_al:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_al,  Condition::LT);

			case OpcodeType::OPCODE_sgt_8_d:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_d,   Condition::GT);
			case OpcodeType::OPCODE_sgt_8_ai:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_ai,  Condition::GT);
			case OpcodeType::OPCODE_sgt_8_pi:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pi,  Condition::GT);
			case OpcodeType::OPCODE_sgt_8_pi7:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pi7, Condition::GT);
			case OpcodeType::OPCODE_sgt_8_pd:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pd,  Condition::GT);
			case OpcodeType::OPCODE_sgt_8_pd7:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pd7, Condition::GT);
			case OpcodeType::OPCODE_sgt_8_di:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_di,  Condition::GT);
			case OpcodeType::OPCODE_sgt_8_ix:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_ix,  Condition::GT);
			case OpcodeType::OPCODE_sgt_8_aw:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_aw,  Condition::GT);
			case OpcodeType::OPCODE_sgt_8_al:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_al,  Condition::GT);

			case OpcodeType::OPCODE_sle_8_d:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_d,   Condition::LE);
			case OpcodeType::OPCODE_sle_8_ai:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_ai,  Condition::LE);
			case OpcodeType::OPCODE_sle_8_pi:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pi,  Condition::LE);
			case OpcodeType::OPCODE_sle_8_pi7:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pi7, Condition::LE);
			case OpcodeType::OPCODE_sle_8_pd:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pd,  Condition::LE);
			case OpcodeType::OPCODE_sle_8_pd7:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_pd7, Condition::LE);
			case OpcodeType::OPCODE_sle_8_di:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_di,  Condition::LE);
			case OpcodeType::OPCODE_sle_8_ix:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_ix,  Condition::LE);
			case OpcodeType::OPCODE_sle_8_aw:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_aw,  Condition::LE);
			case OpcodeType::OPCODE_sle_8_al:		return parseOpcode_sxx(DataType::Size::SIZE_8, PARAM_al,  Condition::LE);

			// Signed extension
			case OpcodeType::OPCODE_ext_16:			return parseOpcode_ext(DataType::Size::SIZE_16);
			case OpcodeType::OPCODE_ext_32:			return parseOpcode_ext(DataType::Size::SIZE_32);

			// Swap words
			case OpcodeType::OPCODE_swap_32:		return parseOpcode_swap(DataType::Size::SIZE_32);

			// Exchange
			case OpcodeType::OPCODE_exg_32_dd:		return parseOpcode_exg(DataType::Size::SIZE_32, true, true);
			case OpcodeType::OPCODE_exg_32_aa:		return parseOpcode_exg(DataType::Size::SIZE_32, false, false);
			case OpcodeType::OPCODE_exg_32_da:		return parseOpcode_exg(DataType::Size::SIZE_32, true, false);

			// Link / unlink
			case OpcodeType::OPCODE_link_16:		return parseOpcode_link();
			case OpcodeType::OPCODE_unlk_32:		return parseOpcode_unlink();

			// Jumps
			case OpcodeType::OPCODE_bra_8:			return parseOpcode_jump_call(DataType::Size::SIZE_8, false);
			case OpcodeType::OPCODE_bra_16:			return parseOpcode_jump_call(DataType::Size::SIZE_16, false);
			case OpcodeType::OPCODE_bra_32:			return parseOpcode_jump_call(DataType::Size::SIZE_32, false);
			case OpcodeType::OPCODE_jmp_32_ai:		return parseOpcode_jump_call_32(PARAM_ai, false);
			case OpcodeType::OPCODE_jmp_32_di:		return parseOpcode_jump_call_32(PARAM_pi, false);
			case OpcodeType::OPCODE_jmp_32_ix:		return parseOpcode_jump_call_32(PARAM_ix, false);
			case OpcodeType::OPCODE_jmp_32_aw:		return parseOpcode_jump_call_32(PARAM_aw, false);
			case OpcodeType::OPCODE_jmp_32_al:		return parseOpcode_jump_call_32(PARAM_al, false);
			case OpcodeType::OPCODE_jmp_32_pcdi:	return parseOpcode_jump_call_32(PARAM_pcdi, false);
			case OpcodeType::OPCODE_jmp_32_pcix:	return parseOpcode_jump_call_32(PARAM_pcix, false);

			// Conditional jumps
			case OpcodeType::OPCODE_bhi_8:			return parseOpcode_conditional_jump(DataType::Size::SIZE_8, Condition::HI);
			case OpcodeType::OPCODE_bls_8:			return parseOpcode_conditional_jump(DataType::Size::SIZE_8, Condition::LS);
			case OpcodeType::OPCODE_bcc_8:			return parseOpcode_conditional_jump(DataType::Size::SIZE_8, Condition::CC);
			case OpcodeType::OPCODE_bcs_8:			return parseOpcode_conditional_jump(DataType::Size::SIZE_8, Condition::CS);
			case OpcodeType::OPCODE_bne_8:			return parseOpcode_conditional_jump(DataType::Size::SIZE_8, Condition::NE);
			case OpcodeType::OPCODE_beq_8:			return parseOpcode_conditional_jump(DataType::Size::SIZE_8, Condition::EQ);
			case OpcodeType::OPCODE_bvc_8:			return parseOpcode_conditional_jump(DataType::Size::SIZE_8, Condition::VC);
			case OpcodeType::OPCODE_bvs_8:			return parseOpcode_conditional_jump(DataType::Size::SIZE_8, Condition::VS);
			case OpcodeType::OPCODE_bpl_8:			return parseOpcode_conditional_jump(DataType::Size::SIZE_8, Condition::PL);
			case OpcodeType::OPCODE_bmi_8:			return parseOpcode_conditional_jump(DataType::Size::SIZE_8, Condition::MI);
			case OpcodeType::OPCODE_bge_8:			return parseOpcode_conditional_jump(DataType::Size::SIZE_8, Condition::GE);
			case OpcodeType::OPCODE_blt_8:			return parseOpcode_conditional_jump(DataType::Size::SIZE_8, Condition::LT);
			case OpcodeType::OPCODE_bgt_8:			return parseOpcode_conditional_jump(DataType::Size::SIZE_8, Condition::GT);
			case OpcodeType::OPCODE_ble_8:			return parseOpcode_conditional_jump(DataType::Size::SIZE_8, Condition::LE);
			case OpcodeType::OPCODE_bhi_16:			return parseOpcode_conditional_jump(DataType::Size::SIZE_16, Condition::HI);
			case OpcodeType::OPCODE_bls_16:			return parseOpcode_conditional_jump(DataType::Size::SIZE_16, Condition::LS);
			case OpcodeType::OPCODE_bcc_16:			return parseOpcode_conditional_jump(DataType::Size::SIZE_16, Condition::CC);
			case OpcodeType::OPCODE_bcs_16:			return parseOpcode_conditional_jump(DataType::Size::SIZE_16, Condition::CS);
			case OpcodeType::OPCODE_bne_16:			return parseOpcode_conditional_jump(DataType::Size::SIZE_16, Condition::NE);
			case OpcodeType::OPCODE_beq_16:			return parseOpcode_conditional_jump(DataType::Size::SIZE_16, Condition::EQ);
			case OpcodeType::OPCODE_bvc_16:			return parseOpcode_conditional_jump(DataType::Size::SIZE_16, Condition::VC);
			case OpcodeType::OPCODE_bvs_16:			return parseOpcode_conditional_jump(DataType::Size::SIZE_16, Condition::VS);
			case OpcodeType::OPCODE_bpl_16:			return parseOpcode_conditional_jump(DataType::Size::SIZE_16, Condition::PL);
			case OpcodeType::OPCODE_bmi_16:			return parseOpcode_conditional_jump(DataType::Size::SIZE_16, Condition::MI);
			case OpcodeType::OPCODE_bge_16:			return parseOpcode_conditional_jump(DataType::Size::SIZE_16, Condition::GE);
			case OpcodeType::OPCODE_blt_16:			return parseOpcode_conditional_jump(DataType::Size::SIZE_16, Condition::LT);
			case OpcodeType::OPCODE_bgt_16:			return parseOpcode_conditional_jump(DataType::Size::SIZE_16, Condition::GT);
			case OpcodeType::OPCODE_ble_16:			return parseOpcode_conditional_jump(DataType::Size::SIZE_16, Condition::LE);
			case OpcodeType::OPCODE_bhi_32:			return parseOpcode_conditional_jump(DataType::Size::SIZE_32, Condition::HI);
			case OpcodeType::OPCODE_bls_32:			return parseOpcode_conditional_jump(DataType::Size::SIZE_32, Condition::LS);
			case OpcodeType::OPCODE_bcc_32:			return parseOpcode_conditional_jump(DataType::Size::SIZE_32, Condition::CC);
			case OpcodeType::OPCODE_bcs_32:			return parseOpcode_conditional_jump(DataType::Size::SIZE_32, Condition::CS);
			case OpcodeType::OPCODE_bne_32:			return parseOpcode_conditional_jump(DataType::Size::SIZE_32, Condition::NE);
			case OpcodeType::OPCODE_beq_32:			return parseOpcode_conditional_jump(DataType::Size::SIZE_32, Condition::EQ);
			case OpcodeType::OPCODE_bvc_32:			return parseOpcode_conditional_jump(DataType::Size::SIZE_32, Condition::VC);
			case OpcodeType::OPCODE_bvs_32:			return parseOpcode_conditional_jump(DataType::Size::SIZE_32, Condition::VS);
			case OpcodeType::OPCODE_bpl_32:			return parseOpcode_conditional_jump(DataType::Size::SIZE_32, Condition::PL);
			case OpcodeType::OPCODE_bmi_32:			return parseOpcode_conditional_jump(DataType::Size::SIZE_32, Condition::MI);
			case OpcodeType::OPCODE_bge_32:			return parseOpcode_conditional_jump(DataType::Size::SIZE_32, Condition::GE);
			case OpcodeType::OPCODE_blt_32:			return parseOpcode_conditional_jump(DataType::Size::SIZE_32, Condition::LT);
			case OpcodeType::OPCODE_bgt_32:			return parseOpcode_conditional_jump(DataType::Size::SIZE_32, Condition::GT);
			case OpcodeType::OPCODE_ble_32:			return parseOpcode_conditional_jump(DataType::Size::SIZE_32, Condition::LE);

			case OpcodeType::OPCODE_dbt_16:			return parseOpcode_nop(2);
			case OpcodeType::OPCODE_dbf_16:			return parseOpcode_conditional_jump(DataType::Size::SIZE_16, Condition::NONE, true);
			case OpcodeType::OPCODE_dbhi_16:		return parseOpcode_conditional_jump(DataType::Size::SIZE_16, Condition::HI, true);
			case OpcodeType::OPCODE_dbls_16:		return parseOpcode_conditional_jump(DataType::Size::SIZE_16, Condition::LS, true);
			case OpcodeType::OPCODE_dbcc_16:		return parseOpcode_conditional_jump(DataType::Size::SIZE_16, Condition::CC, true);
			case OpcodeType::OPCODE_dbcs_16:		return parseOpcode_conditional_jump(DataType::Size::SIZE_16, Condition::CS, true);
			case OpcodeType::OPCODE_dbne_16:		return parseOpcode_conditional_jump(DataType::Size::SIZE_16, Condition::NE, true);
			case OpcodeType::OPCODE_dbeq_16:		return parseOpcode_conditional_jump(DataType::Size::SIZE_16, Condition::EQ, true);
			case OpcodeType::OPCODE_dbvc_16:		return parseOpcode_conditional_jump(DataType::Size::SIZE_16, Condition::VC, true);
			case OpcodeType::OPCODE_dbvs_16:		return parseOpcode_conditional_jump(DataType::Size::SIZE_16, Condition::VS, true);
			case OpcodeType::OPCODE_dbpl_16:		return parseOpcode_conditional_jump(DataType::Size::SIZE_16, Condition::PL, true);
			case OpcodeType::OPCODE_dbmi_16:		return parseOpcode_conditional_jump(DataType::Size::SIZE_16, Condition::MI, true);
			case OpcodeType::OPCODE_dbge_16:		return parseOpcode_conditional_jump(DataType::Size::SIZE_16, Condition::GE, true);
			case OpcodeType::OPCODE_dblt_16:		return parseOpcode_conditional_jump(DataType::Size::SIZE_16, Condition::LT, true);
			case OpcodeType::OPCODE_dbgt_16:		return parseOpcode_conditional_jump(DataType::Size::SIZE_16, Condition::GT, true);
			case OpcodeType::OPCODE_dble_16:		return parseOpcode_conditional_jump(DataType::Size::SIZE_16, Condition::LE, true);

			// Calls
			case OpcodeType::OPCODE_bsr_8:			return parseOpcode_jump_call(DataType::Size::SIZE_8, true);
			case OpcodeType::OPCODE_bsr_16:			return parseOpcode_jump_call(DataType::Size::SIZE_16, true);
			case OpcodeType::OPCODE_bsr_32:			return parseOpcode_jump_call(DataType::Size::SIZE_32, true);
			case OpcodeType::OPCODE_jsr_32_ai:		return parseOpcode_jump_call_32(PARAM_ai, true);
			case OpcodeType::OPCODE_jsr_32_di:		return parseOpcode_jump_call_32(PARAM_pi, true);
			case OpcodeType::OPCODE_jsr_32_ix:		return parseOpcode_jump_call_32(PARAM_ix, true);
			case OpcodeType::OPCODE_jsr_32_aw:		return parseOpcode_jump_call_32(PARAM_aw, true);
			case OpcodeType::OPCODE_jsr_32_al:		return parseOpcode_jump_call_32(PARAM_al, true);
			case OpcodeType::OPCODE_jsr_32_pcdi:	return parseOpcode_jump_call_32(PARAM_pcdi, true);
			case OpcodeType::OPCODE_jsr_32_pcix:	return parseOpcode_jump_call_32(PARAM_pcix, true);

			// Return
			case OpcodeType::OPCODE_rte_32:			return parseOpcode_rte();
			case OpcodeType::OPCODE_rts_32:			return parseOpcode_rts();

			// ...and here comes all the rest
			case OpcodeType::OPCODE_1010:
			case OpcodeType::OPCODE_1111:
			case OpcodeType::OPCODE_addx_8_mm_ax7:
			case OpcodeType::OPCODE_addx_8_mm_ay7:
			case OpcodeType::OPCODE_addx_8_mm_axy7:
			case OpcodeType::OPCODE_addx_8_mm:
			case OpcodeType::OPCODE_addx_16_mm:
			case OpcodeType::OPCODE_addx_32_mm:
			case OpcodeType::OPCODE_andi_16_toc:
			case OpcodeType::OPCODE_chk_16_d:
			case OpcodeType::OPCODE_chk_16_ai:
			case OpcodeType::OPCODE_chk_16_pi:
			case OpcodeType::OPCODE_chk_16_pd:
			case OpcodeType::OPCODE_chk_16_di:
			case OpcodeType::OPCODE_chk_16_ix:
			case OpcodeType::OPCODE_chk_16_aw:
			case OpcodeType::OPCODE_chk_16_al:
			case OpcodeType::OPCODE_chk_16_pcdi:
			case OpcodeType::OPCODE_chk_16_pcix:
			case OpcodeType::OPCODE_chk_16_i:
			case OpcodeType::OPCODE_cmpm_8_ax7:
			case OpcodeType::OPCODE_cmpm_8_ay7:
			case OpcodeType::OPCODE_cmpm_8_axy7:
			case OpcodeType::OPCODE_cmpm_8:
			case OpcodeType::OPCODE_cmpm_16:
			case OpcodeType::OPCODE_cmpm_32:
			case OpcodeType::OPCODE_eori_16_toc:
			case OpcodeType::OPCODE_eori_16_tos:
			case OpcodeType::OPCODE_illegal:
			case OpcodeType::OPCODE_link_16_a7:
			case OpcodeType::OPCODE_nbcd_8_d:
			case OpcodeType::OPCODE_nbcd_8_ai:
			case OpcodeType::OPCODE_nbcd_8_pi:
			case OpcodeType::OPCODE_nbcd_8_pi7:
			case OpcodeType::OPCODE_nbcd_8_pd:
			case OpcodeType::OPCODE_nbcd_8_pd7:
			case OpcodeType::OPCODE_nbcd_8_di:
			case OpcodeType::OPCODE_nbcd_8_ix:
			case OpcodeType::OPCODE_nbcd_8_aw:
			case OpcodeType::OPCODE_nbcd_8_al:
			case OpcodeType::OPCODE_negx_8_d:
			case OpcodeType::OPCODE_negx_8_ai:
			case OpcodeType::OPCODE_negx_8_pi:
			case OpcodeType::OPCODE_negx_8_pi7:
			case OpcodeType::OPCODE_negx_8_pd:
			case OpcodeType::OPCODE_negx_8_pd7:
			case OpcodeType::OPCODE_negx_8_di:
			case OpcodeType::OPCODE_negx_8_ix:
			case OpcodeType::OPCODE_negx_8_aw:
			case OpcodeType::OPCODE_negx_8_al:
			case OpcodeType::OPCODE_negx_16_d:
			case OpcodeType::OPCODE_negx_16_ai:
			case OpcodeType::OPCODE_negx_16_pi:
			case OpcodeType::OPCODE_negx_16_pd:
			case OpcodeType::OPCODE_negx_16_di:
			case OpcodeType::OPCODE_negx_16_ix:
			case OpcodeType::OPCODE_negx_16_aw:
			case OpcodeType::OPCODE_negx_16_al:
			case OpcodeType::OPCODE_negx_32_d:
			case OpcodeType::OPCODE_negx_32_ai:
			case OpcodeType::OPCODE_negx_32_pi:
			case OpcodeType::OPCODE_negx_32_pd:
			case OpcodeType::OPCODE_negx_32_di:
			case OpcodeType::OPCODE_negx_32_ix:
			case OpcodeType::OPCODE_negx_32_aw:
			case OpcodeType::OPCODE_negx_32_al:
			case OpcodeType::OPCODE_ori_16_toc:
			case OpcodeType::OPCODE_reset:
			case OpcodeType::OPCODE_rtr_32:
			case OpcodeType::OPCODE_sbcd_8_rr:
			case OpcodeType::OPCODE_sbcd_8_mm_ax7:
			case OpcodeType::OPCODE_sbcd_8_mm_ay7:
			case OpcodeType::OPCODE_sbcd_8_mm_axy7:
			case OpcodeType::OPCODE_sbcd_8_mm:
			case OpcodeType::OPCODE_stop:
			case OpcodeType::OPCODE_subx_8_mm_ax7:
			case OpcodeType::OPCODE_subx_8_mm_ay7:
			case OpcodeType::OPCODE_subx_8_mm_axy7:
			case OpcodeType::OPCODE_subx_8_mm:
			case OpcodeType::OPCODE_subx_16_mm:
			case OpcodeType::OPCODE_subx_32_mm:
			case OpcodeType::OPCODE_tas_8_d:
			case OpcodeType::OPCODE_tas_8_ai:
			case OpcodeType::OPCODE_tas_8_pi:
			case OpcodeType::OPCODE_tas_8_pi7:
			case OpcodeType::OPCODE_tas_8_pd:
			case OpcodeType::OPCODE_tas_8_pd7:
			case OpcodeType::OPCODE_tas_8_di:
			case OpcodeType::OPCODE_tas_8_ix:
			case OpcodeType::OPCODE_tas_8_aw:
			case OpcodeType::OPCODE_tas_8_al:
			case OpcodeType::OPCODE_trap:
			case OpcodeType::OPCODE_trapv:
			case OpcodeType::OPCODE_unlk_32_a7:
				break;
		}
	}

}
