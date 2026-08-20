/*
*	Lemonizer -- Turns 68K code into lemonscript
*	Copyright (C) 2017-2026 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#include "pch.h"
#include "assembly/AssemblyDefinitions.h"


const assembly::DataType assembly::DataType::u8 (assembly::DataType::Size::SIZE_8,  assembly::DataType::Sign::UNSIGNED);
const assembly::DataType assembly::DataType::u16(assembly::DataType::Size::SIZE_16, assembly::DataType::Sign::UNSIGNED);
const assembly::DataType assembly::DataType::u32(assembly::DataType::Size::SIZE_32, assembly::DataType::Sign::UNSIGNED);
const assembly::DataType assembly::DataType::s8 (assembly::DataType::Size::SIZE_8,  assembly::DataType::Sign::SIGNED);
const assembly::DataType assembly::DataType::s16(assembly::DataType::Size::SIZE_16, assembly::DataType::Sign::SIGNED);
const assembly::DataType assembly::DataType::s32(assembly::DataType::Size::SIZE_32, assembly::DataType::Sign::SIGNED);
