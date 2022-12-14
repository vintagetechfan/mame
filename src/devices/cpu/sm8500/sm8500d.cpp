// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*******************************************************************

sm8500d.c
Sharp sm8500 CPU disassembly



*******************************************************************/

#include "emu.h"
#include "sm8500d.h"

const char *const sm8500_disassembler::s_mnemonic[] =
{
	"adc",  "adcw", "add",  "addw", "and",  "andw",  "band", "bbc",  "bbs",
	"bclr", "bcmp", "bmov", "bor",  "br",   "btst", "bset",  "bxor", "call", "cals", "clr",
	"clrc", "cmp",  "cmpw", "com",  "comc",  "da",   "dbnz", "dec",
	"decw", "di",   "div",  "ei",   "exts",  "halt", "inc",  "incw",
	"iret", "jmp",  "mov",  "movm", "movw",  "mult", "neg",  "nop",  "or",
	"orw",  "pop",  "popw", "push", "pushw", "ret",  "rl",   "rlc",
	"rr",   "rrc",  "sbc",  "sbcw", "setc", "sll",   "sra",  "srl",  "stop",
	"sub",  "subw", "swap", "xor",  "xorw", "mov  PS0,", "invalid", "dm?",
/* unknowns */
"unk5A", "unk5B",

/* more complicated instructions */
"comp1A", "comp1B", "comp4F",
};

const uint32_t sm8500_disassembler::s_flags[] = {
	0, 0, 0, 0, 0, 0, 0, STEP_COND, STEP_COND,
	0, 0, 0, 0, 0, 0, 0, 0, STEP_OVER, STEP_OVER, 0,
	0, 0, 0, 0, 0, 0, STEP_COND, 0,
	0, 0, 0, 0, 0, STEP_OVER, 0, 0,
	STEP_OUT, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, STEP_OUT, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0,
	0, 0, 0
};

const char *const sm8500_disassembler::sm8500_cond[16] = {
	"F", "LT", "LE", "ULE", "OV",  "MI", "Z",  "C",
	"T", "GE", "GT", "UGT", "NOV", "PL", "NZ", "NC"
};

const uint8_t sm8500_disassembler::sm8500_b2w[8] = {
	0, 8, 2, 10, 4, 12, 6, 14
};

const sm8500_disassembler::sm8500dasm sm8500_disassembler::mnemonic[256] = {
	/* 00 - 0F */
		{zCLR, AM_R},  {zNEG,AM_R},   {zCOM,AM_R},   {zRR,AM_R},
		{zRL, AM_R},   {zRRC,AM_R},  {zRLC,AM_R},   {zSRL,AM_R},
		{zINC, AM_R},  {zDEC,AM_R},  {zSRA,AM_R},    {zSLL,AM_R},
		{zDA, AM_R},   {zSWAP,AM_R}, {zPUSH,AM_R},  {zPOP,AM_R},
	/* 10 - 1F */
		{zCMP,AM_rr},  {zADD,AM_rr},  {zSUB,AM_rr},   {zADC,AM_rr},
		{zSBC,AM_rr},  {zAND,AM_rr},  {zOR,AM_rr},    {zXOR,AM_rr},
		{zINCW,AM_S}, {zDECW,AM_S}, {z1A,AM_1A},    {z1B,AM_1B},
		{zBCLR,AM_riB}, {zBSET,AM_riB},   {zPUSHW,AM_S}, {zPOPW,AM_S},
	/* 20 - 2F */
		{zCMP,AM_rmb},   {zADD,AM_rmb},  {zSUB,AM_rmb},    {zADC,AM_rmb},
		{zSBC,AM_rmb},   {zAND,AM_rmb},   {zOR,AM_rmb},    {zXOR,AM_rmb},
		{zMOV,AM_rmb},  {zMOV,AM_mbr},  {zBBC,AM_bid},    {zBBS,AM_bid},
		{zEXTS,AM_R}, {zDM,AM_i},   {zMOVPS0,AM_i},    {zBTST,AM_Ri},
	/* 30 - 3F */
		{zCMP,AM_rmw},  {zADD,AM_rmw},  {zSUB,AM_rmw},   {zADC,AM_rmw},
		{zSBC,AM_rmw},  {zAND,AM_rmw},   {zOR,AM_rmw},    {zXOR,AM_rmw},
		{zMOV,AM_rmw},  {zMOV,AM_mwr},  {zMOVW,AM_smw},  {zMOVW,AM_mws},
		{zMOVW,AM_ss}, {zDM,AM_R},   {zJMP,AM_2},   {zCALL,AM_2},
	/* 40 - 4F */
		{zCMP,AM_RR},  {zADD,AM_RR},  {zSUB,AM_RR},   {zADC,AM_RR},
		{zSBC,AM_RR},  {zAND,AM_RR},  {zOR,AM_RR},    {zXOR,AM_RR},
		{zMOV,AM_RR},  {zCALL,AM_ii}, {zMOVW,AM_SS},  {zMOVW,AM_Sw},
		{zMULT,AM_RR}, {zMULT,AM_iR}, {zBMOV,AM_bR},  {z4F,AM_4F},
	/* 50 - 5F */
		{zCMP,AM_iR},  {zADD,AM_iR},  {zSUB,AM_iR},   {zADC,AM_iR},
		{zSBC,AM_iR},  {zAND,AM_iR},  {zOR,AM_iR},    {zXOR,AM_iR},
		{zMOV, AM_iR}, {zINVLD,0},   {z5A,AM_5A},    {z5B,AM_5B},
		{zDIV,AM_SS},  {zDIV,AM_iS},   {zMOVM,AM_RiR},  {zMOVM,AM_Rii},
	/* 60 - 6F */
		{zCMPW,AM_SS}, {zADDW,AM_SS}, {zSUBW,AM_SS},  {zADCW,AM_SS},
		{zSBCW,AM_SS}, {zANDW,AM_SS}, {zORW,AM_SS},   {zXORW,AM_SS},
		{zCMPW,AM_Sw}, {zADDW,AM_Sw}, {zSUBW,AM_Sw},  {zADCW,AM_Sw},
		{zSBCW,AM_Sw}, {zANDW,AM_Sw}, {zORW,AM_Sw},   {zXORW,AM_Sw},
	/* 70 - 7F */
		{zDBNZ,AM_rbr}, {zDBNZ,AM_rbr}, {zDBNZ,AM_rbr},  {zDBNZ,AM_rbr},
		{zDBNZ,AM_rbr}, {zDBNZ,AM_rbr}, {zDBNZ,AM_rbr},  {zDBNZ,AM_rbr},
		{zMOVW,AM_riw}, {zMOVW,AM_riw}, {zMOVW,AM_riw},  {zMOVW,AM_riw},
		{zMOVW,AM_riw}, {zMOVW,AM_riw}, {zMOVW,AM_riw},  {zMOVW,AM_riw},
	/* 80 - 8F */
		{zBBC,AM_Rbr},  {zBBC,AM_Rbr},  {zBBC,AM_Rbr},   {zBBC,AM_Rbr},
		{zBBC,AM_Rbr},  {zBBC,AM_Rbr},  {zBBC,AM_Rbr},   {zBBC,AM_Rbr},
		{zBBS,AM_Rbr},  {zBBS,AM_Rbr},  {zBBS,AM_Rbr},   {zBBS,AM_Rbr},
		{zBBS,AM_Rbr},  {zBBS,AM_Rbr},  {zBBS,AM_Rbr},   {zBBS,AM_Rbr},
	/* 90 - 9F */
		{zJMP,AM_cjp},  {zJMP,AM_cjp},  {zJMP,AM_cjp},   {zJMP,AM_cjp},
		{zJMP,AM_cjp},  {zJMP,AM_cjp},  {zJMP,AM_cjp},   {zJMP,AM_cjp},
		{zJMP,AM_cjp},  {zJMP,AM_cjp},  {zJMP,AM_cjp},   {zJMP,AM_cjp},
		{zJMP,AM_cjp},  {zJMP,AM_cjp},  {zJMP,AM_cjp},   {zJMP,AM_cjp},
	/* A0 - AF */
		{zBCLR,AM_Rb}, {zBCLR,AM_Rb}, {zBCLR,AM_Rb},  {zBCLR,AM_Rb},
		{zBCLR,AM_Rb}, {zBCLR,AM_Rb}, {zBCLR,AM_Rb},  {zBCLR,AM_Rb},
		{zBSET,AM_Rb}, {zBSET,AM_Rb}, {zBSET,AM_Rb},  {zBSET,AM_Rb},
		{zBSET,AM_Rb}, {zBSET,AM_Rb}, {zBSET,AM_Rb},  {zBSET,AM_Rb},
	/* B0 - BF */
		{zMOV,AM_rR},  {zMOV,AM_rR},  {zMOV,AM_rR},   {zMOV,AM_rR},
		{zMOV,AM_rR},  {zMOV,AM_rR},  {zMOV,AM_rR},   {zMOV,AM_rR},
		{zMOV,AM_Rr},  {zMOV,AM_Rr},  {zMOV,AM_Rr},   {zMOV,AM_Rr},
		{zMOV,AM_Rr},  {zMOV,AM_Rr},  {zMOV,AM_Rr},   {zMOV,AM_Rr},
	/* C0 - CF */
		{zMOV,AM_rib},  {zMOV,AM_rib},  {zMOV,AM_rib},   {zMOV,AM_rib},
		{zMOV,AM_rib},  {zMOV,AM_rib},  {zMOV,AM_rib},   {zMOV,AM_rib},
		{zMOV,AM_pi},  {zMOV,AM_pi},  {zMOV,AM_pi},   {zMOV,AM_pi},
		{zMOV,AM_pi},  {zMOV,AM_pi},  {zMOV,AM_pi},   {zMOV,AM_pi},
	/* D0 - DF */
		{zBR,AM_cbr},   {zBR,AM_cbr},   {zBR,AM_cbr},    {zBR,AM_cbr},
		{zBR,AM_cbr},   {zBR,AM_cbr},   {zBR,AM_cbr},    {zBR,AM_cbr},
		{zBR,AM_cbr},   {zBR,AM_cbr},   {zBR,AM_cbr},    {zBR,AM_cbr},
		{zBR,AM_cbr},   {zBR,AM_cbr},   {zBR,AM_cbr},    {zBR,AM_cbr},
	/* E0 - EF */
		{zCALS,AM_CALS},   {zCALS,AM_CALS},   {zCALS,AM_CALS},    {zCALS,AM_CALS},
		{zCALS,AM_CALS},   {zCALS,AM_CALS},   {zCALS,AM_CALS},    {zCALS,AM_CALS},
		{zCALS,AM_CALS},   {zCALS,AM_CALS},   {zCALS,AM_CALS},    {zCALS,AM_CALS},
		{zCALS,AM_CALS},   {zCALS,AM_CALS},   {zCALS,AM_CALS},    {zCALS,AM_CALS},
	/* F0 - FF */
		{zSTOP,0}, {zHALT,0}, {zINVLD,0},    {zINVLD,0},
		{zINVLD,0},   {zINVLD,0},   {zINVLD,0},    {zINVLD,0},
		{zRET,0},  {zIRET,0}, {zCLRC,0},  {zCOMC,0},
		{zSETC,0}, {zEI,0},   {zDI,0},    {zNOP,0},

};

u32 sm8500_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t sm8500_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	const sm8500dasm *instr;
	uint8_t op;
	int8_t offset;
	uint16_t ea;
	uint16_t ea2;
	uint16_t ea3;
	offs_t pos = pc;

	op = opcodes.r8(pos++);

	instr = &mnemonic[op];

	if ( instr->arguments )
	{
		if ( instr->arguments != AM_1A && instr->arguments != AM_1B && instr->arguments != AM_4F ) {
			util::stream_format(stream, "%-4s ", s_mnemonic[ instr->mnemonic ]);
		}
		switch( instr->arguments ) {
		case AM_R:
			ea = opcodes.r8(pos++);
			util::stream_format(stream, "R%02Xh", ea);
			break;
		case AM_iR:
			util::stream_format(stream, "R%02Xh, $%02X", opcodes.r8(pos+1), opcodes.r8(pos));
			pos += 2;
			break;
		case AM_iS:
			util::stream_format(stream, "RR%02Xh, $%02X", opcodes.r8(pos+1), opcodes.r8(pos));
			pos += 2;
			break;
		case AM_Sw:
			ea2 = opcodes.r8(pos++);
			ea = opcodes.r8(pos++) << 8;
			ea += opcodes.r8(pos++);
			util::stream_format(stream, "RR%02Xh, $%04X", ea2, ea);
			break;
		case AM_rib:
			ea = opcodes.r8(pos++);
			util::stream_format(stream, "r%02Xh, $%02X", op & 0x07, ea);
			break;
		case AM_riw:
			ea = opcodes.r8(pos++) << 8;
			ea += opcodes.r8(pos++);
			util::stream_format(stream, "rr%02Xh, $%04X", sm8500_b2w[op & 0x07], ea);
			break;
		case AM_rmb:
			ea = opcodes.r8(pos++);
			util::stream_format(stream, "r%02Xh,", ( ea >> 3 ) & 0x07);
			switch( ea & 0xC0 ) {
			case 0x00:
				util::stream_format(stream, "@r%02Xh", ea & 0x07); break;
			case 0x40:
				util::stream_format(stream, "(r%02Xh)+", ea & 0x07); break;
			case 0x80:
				ea2 = opcodes.r8(pos++);
				if ( ea & 0x07 ) {
					util::stream_format(stream, "$%02X(r%02Xh)", ea2, ea & 0x07);
				} else {
					util::stream_format(stream, "@$%02X", ea2);
				}
				break;
			case 0xC0:
				util::stream_format(stream, "-(r%02Xh)", ea & 0x07); break;
			}
			break;
		case AM_mbr:
			ea = opcodes.r8(pos++);
			switch( ea & 0xC0 ) {
			case 0x00:
				util::stream_format(stream, "@r%02Xh", ea & 0x07); break;
			case 0x40:
				util::stream_format(stream, "(r%02Xh)+", ea & 0x07); break;
			case 0x80:
				ea2 = opcodes.r8(pos++);
				if ( ea & 0x07 ) {
					util::stream_format(stream, "$%02X(r%02Xh)", ea2, ea & 0x07);
				} else {
					util::stream_format(stream, "@$%02X", ea2);
				}
				break;
			case 0xC0:
				util::stream_format(stream, "-(r%02Xh)", ea & 0x07); break;
			}
			util::stream_format(stream, ",r%02Xh", ( ea >> 3 ) & 0x07);
			break;
		case AM_rmw:
			ea = opcodes.r8(pos++);
			util::stream_format(stream, "r%02Xh,", ( ea >> 3 ) & 0x07);
			switch( ea & 0xC0 ) {
			case 0x00:
				util::stream_format(stream, "@rr%02Xh", sm8500_b2w[ea & 0x07]); break;
			case 0x40:
				util::stream_format(stream, "(rr%02Xh)+", sm8500_b2w[ea & 0x07]); break;
			case 0x80:
				ea2 = opcodes.r8(pos++) << 8;
				ea2 += opcodes.r8(pos++);
				if ( ea & 0x07 ) {
					util::stream_format(stream, "$%04X(rr%02Xh)", ea2, sm8500_b2w[ea & 0x07]);
				} else {
					util::stream_format(stream, "@$%04X", ea2);
				}
				break;
			case 0xC0:
				util::stream_format(stream, "-(rr%02Xh)", sm8500_b2w[ea & 0x07]); break;
			}
			break;
		case AM_mwr:
			ea = opcodes.r8(pos++);
			switch( ea & 0xC0 ) {
			case 0x00:
				util::stream_format(stream, "@rr%02Xh", sm8500_b2w[ea & 0x07]); break;
			case 0x40:
				util::stream_format(stream, "(rr%02Xh)+", sm8500_b2w[ea & 0x07]); break;
			case 0x80:
				ea2 = opcodes.r8(pos++) << 8;
				ea2 += opcodes.r8(pos++);
				if ( ea & 0x07 ) {
					util::stream_format(stream, "$%04X(rr%02Xh)", ea2, sm8500_b2w[ea & 0x07]);
				} else {
					util::stream_format(stream, "@$%04X", ea2);
				}
				break;
			case 0xC0:
				util::stream_format(stream, "-(rr%02Xh)", sm8500_b2w[ea & 0x07]); break;
			}
			util::stream_format(stream, ",r%02Xh", ( ea >> 3 ) & 0x07);
			break;
		case AM_smw:
			ea = opcodes.r8(pos++);
			util::stream_format(stream, "rr%02Xh,", sm8500_b2w[( ea >> 3 ) & 0x07]);
			switch( ea & 0xC0 ) {
			case 0x00:
				util::stream_format(stream, "@rr%02Xh", sm8500_b2w[ea & 0x07]); break;
			case 0x40:
				util::stream_format(stream, "(rr%02Xh)+", sm8500_b2w[ea & 0x07]); break;
			case 0x80:
				ea2 = opcodes.r8(pos++) << 8;
				ea2 += opcodes.r8(pos++);
				if ( ea & 0x07 ) {
					util::stream_format(stream, "$%04X(rr%02Xh)", ea2, sm8500_b2w[ea & 0x07]);
				} else {
					util::stream_format(stream, "@$%04X", ea2);
				}
				break;
			case 0xC0:
				util::stream_format(stream, "-(rr%02Xh)", sm8500_b2w[ea & 0x07]); break;
			}
			break;
		case AM_mws:
			ea = opcodes.r8(pos++);
			switch( ea & 0xC0 ) {
			case 0x00:
				util::stream_format(stream, "@rr%02Xh", sm8500_b2w[ea & 0x07]); break;
			case 0x40:
				util::stream_format(stream, "(rr%02Xh)+", sm8500_b2w[ea & 0x07]); break;
			case 0x80:
				ea2 = opcodes.r8(pos++) << 8;
				ea2 += opcodes.r8(pos++);
				if ( ea & 0x07 ) {
					util::stream_format(stream, "$%04X(rr%02Xh)", ea2, sm8500_b2w[ea & 0x07]);
				} else {
					util::stream_format(stream, "@$%04X", ea2);
				}
				break;
			case 0xC0:
				util::stream_format(stream, "-(rr%02Xh)", sm8500_b2w[ea & 0x07]); break;
			}
			util::stream_format(stream, ",rr%02Xh", sm8500_b2w[( ea >> 3 ) & 0x07]);
			break;
		case AM_cbr:
			offset = (int8_t) opcodes.r8(pos++);
			util::stream_format(stream, "%s,$%04X", sm8500_cond[ op & 0x0F ], pos + offset);
			break;
		case AM_rbr:
			offset = (int8_t) opcodes.r8(pos++);
			util::stream_format(stream, "r%02Xh,$%04X", op & 0x07, pos + offset);
			break;
		case AM_cjp:
			ea = opcodes.r8(pos++) << 8;
			ea += opcodes.r8(pos++);
			util::stream_format(stream, "%s,$%04X", sm8500_cond[ op & 0x0F], ea);
			break;
		case AM_rr:
			ea = opcodes.r8(pos++);
			switch( ea & 0xc0 ) {
			case 0x00:
				util::stream_format(stream, "r%02Xh,r%02Xh", (ea >> 3 ) & 0x07, ea & 0x07);
				break;
			case 0x40:
			case 0x80:
			case 0xC0:
				util::stream_format(stream, "undef");
				break;
			}
			break;
		case AM_r1:
			ea = opcodes.r8(pos++);
			switch( ea & 0xC0 ) {
			case 0x00:
				util::stream_format(stream, "@r%02Xh", (ea >> 3 ) & 0x07);
				break;
			case 0x40:
			case 0x80:
			case 0xC0:
				util::stream_format(stream, "undef");
				break;
			}
			break;
		case AM_S:
			ea = opcodes.r8(pos++);
			util::stream_format(stream, "RR%02Xh", ea);
			break;
		case AM_pi:
			ea = opcodes.r8(pos++);
			util::stream_format(stream, "r%02Xh, $%02X", 0x10 + (op & 0x07), ea);
			break;
		case AM_Ri:
			ea = opcodes.r8(pos++);
			ea2 = opcodes.r8(pos++);
			util::stream_format(stream, "R%02Xh,$%02X", ea, ea2);
			break;
		case AM_i:
			ea = opcodes.r8(pos++);
			util::stream_format(stream, "$%02X", ea);
			break;
		case AM_ii:
			ea = opcodes.r8(pos++) << 8;
			ea += opcodes.r8(pos++);
			util::stream_format(stream, "$%04X", ea);
			break;
		case AM_5A: // see issue #7451
			ea = opcodes.r8(pos++);
			ea2 = opcodes.r8(pos++);
			switch( ea & 0xC0 ) {
			case 0x00:
				util::stream_format(stream, "CMP (rr%02Xh),$%02Xh", ea & 7, ea2); break;
			case 0x40:
				util::stream_format(stream, "CMP (rr%02Xh)+,$%02Xh", ea & 7, ea2); break;
			case 0x80:
				ea3 = opcodes.r8(pos++);
				util::stream_format(stream, "CMP (rr%02Xh+%02Xh),$%02Xh", ea & 7, ea2, ea3); break;
			case 0xC0:
				util::stream_format(stream, "CMP -(rr%02Xh)+,$%02Xh", ea & 7, ea2); break;
			}
			break;
		case AM_5B: // see issue #7451
			ea = opcodes.r8(pos++);
			ea2 = opcodes.r8(pos++);
			ea3 = (ea << 8) | ea2;
			switch( ea & 0xC0 ) {
			case 0x00:
				util::stream_format(stream, "MOV (rr%02Xh),$%02Xh", ea & 7, ea2); break;
			case 0x40:
				util::stream_format(stream, "MOV (rr%02Xh)+,$%02Xh", ea & 7, ea2); break;
			case 0x80:
				ea3 = opcodes.r8(pos++);
				util::stream_format(stream, "MOV (rr%02Xh+%02Xh),$%02Xh", ea & 7, ea2, ea3); break;
			case 0xC0:
				util::stream_format(stream, "MOV -(rr%02Xh),$%02Xh", ea & 7, ea2); break;
			}
			break;
		case AM_ss:
			ea = opcodes.r8(pos++);
			switch( ea & 0xC0 ) {
			case 0x00:
				util::stream_format(stream, "rr%02Xh,rr%02Xh", sm8500_b2w[( ea >> 3 ) & 0x07], sm8500_b2w[ea & 0x07]); break;
			case 0x40:
				util::stream_format(stream, "undef"); break;
			case 0x80:
				util::stream_format(stream, "undef"); break;
			case 0xC0:
				util::stream_format(stream, "undef"); break;
			}
			break;
		case AM_RR:
			ea = opcodes.r8(pos++);
			ea2 = opcodes.r8(pos++);
			util::stream_format(stream, "R%02Xh,R%02Xh", ea2, ea);
			break;
		case AM_2:
			ea = opcodes.r8(pos++);
			switch( ea & 0xC0 ) {
			case 0x00:
				util::stream_format(stream, "rr%02Xh", sm8500_b2w[ea & 0x07]); break;
			case 0x40:
				ea2 = opcodes.r8(pos++) << 8;
				ea2 += opcodes.r8(pos++);
				if ( ea & 0x38 ) {
					util::stream_format(stream, "@$%04X(r%02Xh)", ea2, ( ea >> 3 ) & 0x07);
				} else {
					util::stream_format(stream, "@$%04X", ea2);
				}
				break;
			case 0x80:
				util::stream_format(stream, "undef"); break;
			case 0xC0:
				util::stream_format(stream, "undef"); break;
			}
			break;
		case AM_SS:
			ea = opcodes.r8(pos++);
			ea2 = opcodes.r8(pos++);
			util::stream_format(stream, "RR%02Xh,RR%02Xh", ea2, ea);
			break;
		case AM_bR:
			ea = opcodes.r8(pos++);
			ea2 = opcodes.r8(pos++);
			switch( ea & 0xC0 ) {
			case 0x00:
				util::stream_format(stream, "BF,R%02Xh,#%d", ea2, ea & 0x07); break;
			case 0x40:
				util::stream_format(stream, "R%02Xh,#%d,BF", ea2, ea & 0x07); break;
			case 0x80:
				util::stream_format(stream, "undef"); break;
			case 0xC0:
				util::stream_format(stream, "undef"); break;
			}
			break;
		case AM_Rbr:
			ea = opcodes.r8(pos++);
			offset = (int8_t) opcodes.r8(pos++);
			util::stream_format(stream, "R%02Xh,#%d,$%04X", ea, op & 0x07, pos + offset);
			break;
		case AM_Rb:
			ea = opcodes.r8(pos++);
			util::stream_format(stream, "R%02Xh,#%d", ea, op&0x07);
			break;
		case AM_rR:
			ea = opcodes.r8(pos++);
			util::stream_format(stream, "r%02Xh,R%02Xh", op & 0x07, ea);
			break;
		case AM_Rr:
			ea = opcodes.r8(pos++);
			util::stream_format(stream, "R%02Xh,r%02Xh", ea, op & 0x07);
			break;
		case AM_Rii:
			ea = opcodes.r8(pos++);
			util::stream_format(stream, "R%02Xh,", ea);
			ea = opcodes.r8(pos++);
			util::stream_format(stream, "$%02X,", ea);
			ea = opcodes.r8(pos++);
			util::stream_format(stream, "$%02X", ea);
			break;
		case AM_RiR:
			ea = opcodes.r8(pos++);
			util::stream_format(stream, "R%02Xh,", ea);
			ea = opcodes.r8(pos++);
			util::stream_format(stream, "$%02X,", ea);
			ea = opcodes.r8(pos++);
			util::stream_format(stream, "R%02Xh", ea);
			break;
		case AM_riB:
			ea = opcodes.r8(pos++);
			ea2 = opcodes.r8(pos++);
			switch( ea & 0xC0 ) {
			case 0x00:
				util::stream_format(stream, "#%2x(r%02Xh),#%d", ea2, ea >> 3, ea & 0x07);
				break;
			case 0x40:
				util::stream_format(stream, "undef"); break;
			case 0x80:
				util::stream_format(stream, "undef"); break;
			case 0xC0:
				util::stream_format(stream, "undef"); break;
			}
			break;
		case AM_CALS:
			ea = opcodes.r8(pos++);
			util::stream_format(stream, "$%04X", 0x1000 | ( ( op & 0x0f ) << 8 ) | ea);
			break;
		case AM_bid:
			ea = opcodes.r8(pos++);
			ea2 = opcodes.r8(pos++);
			if ( ea & 0x38 ) {
				util::stream_format(stream, "$%02X(r%02Xh)", ea2, ( ea >> 3 ) & 0x07);
			} else {
				util::stream_format(stream, "$%04X", 0xFF00 + ea2);
			}
			util::stream_format(stream, ",#%d,", ea & 0x07);
			offset = (int8_t) opcodes.r8(pos++);
			util::stream_format(stream, "$%04X", pos + offset);
			break;
		case AM_1A:
			ea = opcodes.r8(pos++);
			switch( ea & 0x07 ) {
			case 0x00: util::stream_format(stream, "%-4s ", s_mnemonic[ zCLR ]); break;
			case 0x01: util::stream_format(stream, "%-4s ", s_mnemonic[ zNEG ]); break;
			case 0x02: util::stream_format(stream, "%-4s ", s_mnemonic[ zCOM ]); break;
			case 0x03: util::stream_format(stream, "%-4s ", s_mnemonic[ zRR ]); break;
			case 0x04: util::stream_format(stream, "%-4s ", s_mnemonic[ zRL ]); break;
			case 0x05: util::stream_format(stream, "%-4s ", s_mnemonic[ zRRC ]); break;
			case 0x06: util::stream_format(stream, "%-4s ", s_mnemonic[ zRLC ]); break;
			case 0x07: util::stream_format(stream, "%-4s ", s_mnemonic[ zSRL ]); break;
			}
			util::stream_format(stream, "@r%02Xh", ( ea >> 3 ) & 0x07);
			break;
		case AM_1B:
			ea = opcodes.r8(pos++);
			switch( ea & 0x07 ) {
			case 0x00: util::stream_format(stream, "%-4s ", s_mnemonic[ zINC ]); break;
			case 0x01: util::stream_format(stream, "%-4s ", s_mnemonic[ zDEC ]); break;
			case 0x02: util::stream_format(stream, "%-4s ", s_mnemonic[ zSRA ]); break;
			case 0x03: util::stream_format(stream, "%-4s ", s_mnemonic[ zSLL ]); break;
			case 0x04: util::stream_format(stream, "%-4s ", s_mnemonic[ zDA ]); break;
			case 0x05: util::stream_format(stream, "%-4s ", s_mnemonic[ zSWAP ]); break;
			case 0x06: util::stream_format(stream, "%-4s ", s_mnemonic[ zPUSH ]); break;
			case 0x07: util::stream_format(stream, "%-4s ", s_mnemonic[ zPOP ]); break;
			}
			util::stream_format(stream, "@r%02Xh", ( ea >> 3 ) & 0x07);
			break;
		case AM_4F:
			ea = opcodes.r8(pos++);
			ea2 = opcodes.r8(pos++);
			switch( ea & 0xc0 ) {
			case 0x00: util::stream_format(stream, "%-4s ", s_mnemonic[ zBCMP ]); break;
			case 0x40: util::stream_format(stream, "%-4s ", s_mnemonic[ zBAND ]); break;
			case 0x80: util::stream_format(stream, "%-4s ", s_mnemonic[ zBOR ]); break;
			case 0xC0: util::stream_format(stream, "%-4s ", s_mnemonic[ zBXOR ]); break;
			}
			if ( ! ( ea & 0x80 ) ) {
				util::stream_format(stream, "BF,");
			}
			util::stream_format(stream, "R%02Xh,$%02X", ea2, ea & 0x07);
			if ( ea & 0x80 ) {
				util::stream_format(stream, ",BF");
			}
			break;
		}
	}
	else
	{
		util::stream_format(stream, "%s", s_mnemonic[ instr->mnemonic ]);
	}

	return (pos - pc) | (instr->arguments == AM_cjp && (op & 0x07) != 0 ? STEP_COND : s_flags[instr->mnemonic]) | SUPPORTED;
}
