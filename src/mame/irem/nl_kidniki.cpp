// license:BSD-3-Clause
// copyright-holders:Andrew Gardner, Couriersud
#include "netlist/devices/net_lib.h"

#ifdef NLBASE_H_
#error Somehow nl_base.h made it into the include chain.
#endif

#define USE_FRONTIERS 1

/* if we use frontiers, use fixed STV for smaller matrix sizes */
#if (USE_FRONTIERS)
#define USE_FIXED_STV 1
#else
#define USE_FIXED_STV 0
#endif

/*
 * Schematic errors:
 *
 * The kungfu master schematics differ from the kidnik schematics in wiring
 * D4, D5 and Q4 with other components. The manual corrections to the
 * kungfu master schematics make sense and thus are used here.
 *
 * opamp XU1.B according to schematics has no feedback loop between output and
 * inputs. The arrangement of components in the schematic however indicate that
 * this is not the case and indeed a connection exists. This results in sounds
 * at output XU1.14 to contain more detail.
 *
 * You can observe sounds at XU1.14 by doing
 *
 * NL_LOGS=XU1.14 ./mame kidniki
 * nlwav -o x.wav log_XU1.14.log
 * play x.wav
 *
 * VERIFICATION NEEDED: Are D4 and D5 Cathodes connected?
 *
 * This has quite some impact on the drums.
 *
 */

#define FIX_SCHEMATIC_ERRORS (1)
#define D4_AND_D5_CONNECTED (1)

/* On M62 boards with pcb pictures available
 * D6 is missing, although the pcb print exists.
 * We are replacing this with a 10m Resistor.
 */

#define D6_EXISTS (0)

/*
 * J4 connects channel C either to sound_ic or sound.
 *
 * 1: Connect C to channel to sound_ic
 *
 */

#define J4  (1)

/* ----------------------------------------------------------------------------
 *  Library section header START
 * ---------------------------------------------------------------------------*/

#ifndef __PLIB_PREPROCESSOR__

#endif

/* ----------------------------------------------------------------------------
 *  Library section header END
 * ---------------------------------------------------------------------------*/

/* ----------------------------------------------------------------------------
 *  Kidniki schematics
 * ---------------------------------------------------------------------------*/

static NETLIST_START(kidniki_schematics)
{
	//  EESCHEMA NETLIST VERSION 1.1 (SPICE FORMAT) CREATION DATE: SAT 06 JUN 2015 01:06:26 PM CEST
	//  TO EXCLUDE A COMPONENT FROM THE SPICE NETLIST ADD [SPICE_NETLIST_ENABLED] USER FIELD SET TO: N
	//  TO REORDER THE COMPONENT SPICE NODE SEQUENCE ADD [SPICE_NODE_SEQUENCE] USER FIELD AND DEFINE SEQUENCE: 2,1,0
	// SHEET NAME:/
	// IGNORED O_AUDIO0: O_AUDIO0  49 0
	// .END

#if FIX_SCHEMATIC_ERRORS
	NET_C(XU1.7, C40.1)
#endif
	CAP(C200, CAP_N(100))
	CAP(C28, CAP_U(1))
	CAP(C31, CAP_N(470))
	CAP(C32, CAP_N(3.3))
	CAP(C33, CAP_U(1))
	CAP(C34, CAP_N(1))
	CAP(C35, CAP_N(1))
	CAP(C36, CAP_N(6.8))
	CAP(C37, CAP_N(22))
	CAP(C38, CAP_N(1))
	CAP(C39, CAP_N(1))
	CAP(C40, CAP_P(12))
	CAP(C41, CAP_U(1))
	CAP(C42, CAP_N(1.2))
	CAP(C43, CAP_N(1.2))
	CAP(C44, CAP_U(1))
	CAP(C45, CAP_N(22))
	CAP(C47, CAP_U(1))
	CAP(C48, CAP_N(470))
	CAP(C49, CAP_N(3.3))
	CAP(C50, CAP_N(22))
	CAP(C51, CAP_N(22))
	CAP(C52, CAP_N(27))
	CAP(C53, CAP_N(27))
	CAP(C56, CAP_N(6.8))
	CAP(C57, CAP_N(6.8))
	CAP(C59, CAP_N(6.8))
	CAP(C60, CAP_N(22))
	CAP(C61, CAP_N(22))
	CAP(C62, CAP_N(6.8))
	CAP(C63, CAP_N(1))
	CAP(C64, CAP_N(68))
	CAP(C65, CAP_N(68))
	CAP(C66, CAP_N(68))
	CAP(C67, CAP_N(15))
	CAP(C68, CAP_N(15))
	CAP(C69, CAP_N(10))
	CAP(C70, CAP_N(22))
	CAP(C72, CAP_N(12))
	CAP(C73, CAP_N(10))
	CAP(C76, CAP_N(68))
	CAP(C77, CAP_N(12))

	DIODE(D3, "1S1588")
	DIODE(D4, "1S1588")
	DIODE(D5, "1S1588")

	POT(RV1, RES_K(50))

	QBJT_EB(Q10, "2SC945")
	QBJT_EB(Q3, "2SC945")
	QBJT_EB(Q4, "2SC945")
	QBJT_EB(Q5, "2SC945")
	QBJT_EB(Q6, "2SC945")
	QBJT_EB(Q7, "2SC945")
	QBJT_EB(Q9, "2SC945")

	LM2902_DIP(XU1)
	LM358_DIP(XU2)

	MC14584B_DIP(XU3)

	RES(R100, RES_K(560))
	RES(R101, RES_K(150))
	RES(R102, RES_K(150))
	RES(R103, RES_K(470))
	RES(R104, RES_K(22))
	RES(R105, RES_K(470))
	RES(R106, RES_K(150))
	RES(R107, RES_K(150))
	RES(R108, RES_K(560))
	RES(R119, RES_K(22))
	RES(R200, RES_K(100))
	RES(R201, RES_K(100))
	RES(R27, RES_K(6.8))
	RES(R28, RES_K(150))
	RES(R29, RES_K(2.7))
	RES(R30, RES_K(10))
	RES(R31, RES_K(5.1))
	RES(R32, RES_K(4.7))
	RES(R34, RES_K(100))
	RES(R35, RES_K(100))
	RES(R36, RES_K(100))
	RES(R37, RES_K(47))
	RES(R38, 820)
	RES(R39, RES_K(22))
	RES(R40, RES_K(10))
	RES(R41, RES_K(10))
	RES(R42, RES_K(150))
	RES(R43, 470)
	RES(R44, RES_K(100))
	RES(R45, RES_K(1))
	RES(R46, RES_K(12))
	RES(R48, 470)
	RES(R48_2, RES_K(100))
	RES(R49, RES_K(10))
	RES(R50, RES_K(2.2))
	RES(R51, RES_K(150))
	RES(R52, RES_K(100))
	RES(R53, RES_K(100))
	RES(R54, 680)
	RES(R55, RES_K(510))
	RES(R57, 560)
	RES(R58, RES_K(39))
	RES(R59, 560)
	RES(R60, RES_K(39))
	RES(R61, RES_K(100))
	RES(R62, RES_K(100))
	RES(R63, RES_K(1))
	RES(R65, RES_K(1))
	RES(R65_1, RES_K(27))
	RES(R66, RES_M(1))
	RES(R67, RES_K(100))
	RES(R68, RES_K(100))
	RES(R69, RES_K(1))
	RES(R70, RES_K(10))
	RES(R71, RES_K(100))
	RES(R72, RES_K(100))
	RES(R73, RES_K(10))
	RES(R74, RES_K(10))
	RES(R75, RES_K(10))
	RES(R76, RES_K(47))
	RES(R81, 220)
	RES(R82, RES_M(2.2))
	RES(R83, RES_K(12))
	RES(R84, RES_K(1))
	RES(R85, RES_M(2.2))
	RES(R86, RES_K(10))
	RES(R87, RES_K(68))
	RES(R89, RES_K(22))
	RES(R90, RES_K(390))
	RES(R91, RES_K(100))
	RES(R92, RES_K(22))
	RES(R93, RES_K(1))
	RES(R94, RES_K(22))
	RES(R95, RES_K(330))
	RES(R96, RES_K(150))
	RES(R97, RES_K(150))
	RES(R98, RES_K(680))

	#if USE_FIXED_STV
	ANALOG_INPUT(STV, 2)
	#else
	RES(R78, RES_K(3.3))
	RES(R77, RES_K(2.2))
	CAP(C58, CAP_U(47))
	NET_C(R77.2, C58.1, I_V0.Q)
	NET_C(R78.1, I_V5.Q)
	#endif

	NET_C(R95.1, XU3.2, R96.2)
	NET_C(R95.2, XU3.1, C69.1)
	NET_C(XU3.3, R103.2, C73.1)
	NET_C(XU3.4, R103.1, R102.2)
	NET_C(XU3.5, R105.2, C72.1)
	NET_C(XU3.6, R105.1, R106.2)
	NET_C(XU3.7, C69.2, C73.2, C72.2, C77.2, C67.2, C68.2, R65.2, R38.2, XU1.11, R54.2, Q4.E, R63.2, C47.2, R72.2, R67.2, R71.2, R68.2, C48.2, R46.2, C28.1, C32.1, R43.2, XU2.4, C56.1, C52.1, R48.2, R93.2, R94.2, R119.2, R104.2, R53.2, R34.2, R81.2, R92.2, R89.2, C33.1, R37.2, R36.1, R91.1, I_V0.Q, RV1.3)
	NET_C(XU3.8, R108.1, R107.2)
	NET_C(XU3.9, R108.2, C77.1)
	NET_C(XU3.10, R100.1, R101.2)
	NET_C(XU3.11, R100.2, C67.1)
	NET_C(XU3.12, R98.1, R97.2)
	NET_C(XU3.13, R98.2, C68.1)
	NET_C(XU3.14, XU1.4, R66.1, R70.1, Q6.C, Q5.C, XU2.8, R86.1, R83.1, Q3.C, I_V5.Q)
	NET_C(R96.1, R102.1, R106.1, R107.1, R101.1, R97.1, R65.1, C63.2)
	NET_C(C63.1, R65_1.2)
	NET_C(R65_1.1, R44.2, C38.2, C40.2, XU1.6)
	#if USE_FIXED_STV
	NET_C(R30.1, R41.1, R40.1, R76.2, /* R78.2, R77.1, C58.2*/ STV)
	#else
	NET_C(R30.1, R41.1, R40.1, R76.2, R78.2, R77.1, C58.2)
	#endif
	NET_C(R30.2, XU1.5)
	NET_C(R44.1, C39.1, C40.1, R48_2.2)
	NET_C(C38.1, C39.2, R38.1)
	NET_C(XU1.1, XU1.2, R39.1, R32.2)
	NET_C(XU1.3, C34.1, R41.2)
	NET_C(XU1.7, R45.2)
	NET_C(XU1.8, XU1.9, R31.2, C36.2)
	NET_C(XU1.10, R42.1, C32.2)
	NET_C(XU1.12, C49.1, C31.1, R40.2, C61.1, C60.1)
	NET_C(XU1.13, R27.1, R28.2)
	NET_C(XU1.14, R28.1, R29.2, I_SINH0)
	NET_C(R48_2.1, C45.2, R54.1)
	NET_C(C45.1, R55.1, Q7.B)
	NET_C(R55.2, R90.2, C33.2, R37.1, Q3.E)
	NET_C(R45.1, C44.2)
	NET_C(C44.1, R66.2, Q4.B)
#if FIX_SCHEMATIC_ERRORS
#if D4_AND_D5_CONNECTED
	/* needs verification */
	NET_C(Q4.C, D4.K, D5.K)
	NET_C(C42.1, C43.1, R46.1, C35.2)
#else
	NET_C(Q4.C, D5.K)
	NET_C(C42.1, C43.1, R46.1, C35.2, D4.K)
#endif
#else
	NET_C(Q4.C, C42.1, C43.1, R46.1, C35.2, D4.K, D5.K)
#endif
	NET_C(R70.2, R69.2, Q7.C)
	NET_C(R63.1, Q7.E)
	NET_C(R69.1, C49.2)
	NET_C(C42.2, R58.1, D5.A)
	NET_C(R58.2, R57.1, C47.1)
	NET_C(R57.2, Q6.E)
	NET_C(Q6.B, R61.1)
	NET_C(C50.1, R67.1, R61.2)
	NET_C(C50.2, R72.1, I_OH0.Q)
	NET_C(C51.1, R68.1, R62.2)
	NET_C(C51.2, R71.1, I_CH0.Q)
	NET_C(R62.1, Q5.B)
	NET_C(Q5.E, R59.2)
	NET_C(R60.1, C43.2, D4.A)
	NET_C(R60.2, R59.1, C48.1)
	NET_C(C35.1, C34.2, R39.2)
	NET_C(R32.1, C31.2)
	NET_C(R27.2, C28.2)
	NET_C(R29.1, R31.1, R50.2, R49.1, RV1.1)
	NET_C(R42.2, R51.1, C36.1)
	NET_C(R51.2, C41.1)
	NET_C(C41.2, R43.1, I_SOUNDIC0)
	NET_C(XU2.1, XU2.2, R73.1)
	NET_C(XU2.3, R76.1, C200.2)
	NET_C(XU2.5, C56.2, R75.1)
	NET_C(XU2.6, XU2.7, R50.1, C53.2)
	NET_C(R75.2, R74.1, C53.1)
	NET_C(R74.2, C52.2, R73.2)
	NET_C(R49.2, R48.1, I_SOUND0)
	NET_C(Q9.E, R81.1)
	NET_C(Q9.C, R84.2, R83.2, R82.1, C59.1)
	NET_C(Q9.B, R82.2, C62.1)
	NET_C(Q10.E, R93.1)
	NET_C(Q10.C, R87.2, R86.2, R85.1, C76.1)
	NET_C(Q10.B, R85.2, C64.1)
	NET_C(R84.1, C61.2)
	NET_C(C60.2, R87.1)
	NET_C(C64.2, C65.1, R94.1, D3.K)
	NET_C(C65.2, C66.1, R119.1)
	NET_C(C66.2, C76.2, R104.1)
	NET_C(R53.1, R52.2, C37.1)
	NET_C(R34.1, C37.2, I_BD0.Q)
	NET_C(R52.1, D3.A)
	NET_C(R92.1, C62.2, C57.1)
	NET_C(R89.1, C57.2, C59.2, R90.1)
	NET_C(Q3.B, R35.1)
	NET_C(R35.2, R36.2, C70.1)
	NET_C(R91.2, C70.2, I_SD0.Q)
	NET_C(I_MSM3K0.Q, R200.2)
	NET_C(I_MSM2K0.Q, R201.2)
	NET_C(R200.1, R201.1, C200.1)

	/* Amplifier stage */

	CAP(C26, CAP_U(1))
	RES(R25, 560)
	RES(R26, RES_K(47))
	CAP(C29, CAP_U(0.01))

	NET_C(RV1.2, C26.1)
	NET_C(C26.2, R25.1)
	NET_C(R25.2, R26.1, C29.1)
	NET_C(R26.2, C29.2, GND)

}

/* ----------------------------------------------------------------------------
 *  Kidniki audio
 * ---------------------------------------------------------------------------*/

NETLIST_START(kidniki)
{

#if (1 || USE_FRONTIERS)
	SOLVER(Solver, 48000)
	PARAM(Solver.ACCURACY, 1e-7)
	PARAM(Solver.NR_LOOPS, 300)
	PARAM(Solver.GS_LOOPS, 10)
	//PARAM(Solver.METHOD, "SOR")
	PARAM(Solver.METHOD, "MAT_CR")
	PARAM(Solver.FPTYPE, "DOUBLE")
	//PARAM(Solver.METHOD, "MAT")
	//PARAM(Solver.PIVOT, 1)
	//PARAM(Solver.Solver_0.METHOD, "GMRES")
	PARAM(Solver.SOR_FACTOR, 1.313)
	PARAM(Solver.DYNAMIC_TS, 0)
	PARAM(Solver.DYNAMIC_LTE, 5e-4)
	PARAM(Solver.DYNAMIC_MIN_TIMESTEP, 20e-6)
	PARAM(Solver.SORT_TYPE, "PREFER_IDENTITY_TOP_LEFT")
	//PARAM(Solver.SORT_TYPE, "PREFER_BAND_MATRIX")
#else
	//PARAM(Solver.SORT_TYPE, "PREFER_BAND_MATRIX")
	SOLVER(Solver, 48000)
	PARAM(Solver.ACCURACY, 1e-7)
	PARAM(Solver.NR_LOOPS, 10000)
	PARAM(Solver.GS_LOOPS, 100)
	//PARAM(Solver.METHOD, "MAT_CR")
	PARAM(Solver.METHOD, "GMRES")
#endif

#if (USE_FRONTIERS)
	PARAM(Solver.PARALLEL, 2) // More does not help
#else
	PARAM(Solver.PARALLEL, 0)
#endif

	LOCAL_SOURCE(kidniki_schematics)

	ANALOG_INPUT(I_V5, 5)
	//ANALOG_INPUT(I_V0, 0)
	ALIAS(I_V0.Q, GND)

	/* AY 8910 internal resistors */

	RES(R_AY45L_A, 1000)
	RES(R_AY45L_B, 1000)
	RES(R_AY45L_C, 1000)

	RES(R_AY45M_A, 1000)
	RES(R_AY45M_B, 1000)
	RES(R_AY45M_C, 1000)

	NET_C(I_V5, R_AY45L_A.1, R_AY45L_B.1, R_AY45L_C.1, R_AY45M_A.1, R_AY45M_B.1, R_AY45M_C.1)
	NET_C(R_AY45L_A.2, R_AY45L_B.2, R_AY45M_A.2, R_AY45M_B.2, R_AY45M_C.2)

#if (J4)
	ALIAS(I_SOUNDIC0, R_AY45L_C.2)
#else
	NET_C(R_AY45L_A.2, R_AY45L_C.2)
	ALIAS(I_SOUNDIC0, I_V0.Q)
#endif

	ALIAS(I_SOUND0, R_AY45L_A.2)

	TTL_INPUT(SINH, 1)

#if (D6_EXISTS)
	DIODE(D6, "1N914")
	NET_C(D6.K, SINH)
	ALIAS(I_SINH0, D6.A)
#else
	RES(SINH_DUMMY, RES_M(10))
	NET_C(SINH_DUMMY.1, SINH)
	ALIAS(I_SINH0, SINH_DUMMY.2)
#endif

	NET_MODEL("AY8910PORT FAMILY(TYPE=NMOS OVL=0.05 OVH=0.05 ORL=100.0 ORH=0.5k)")

	LOGIC_INPUT(I_SD0, 1, "AY8910PORT")
	LOGIC_INPUT(I_BD0, 1, "AY8910PORT")
	LOGIC_INPUT(I_CH0, 1, "AY8910PORT")
	LOGIC_INPUT(I_OH0, 1, "AY8910PORT")

	NET_C(I_V5, I_SD0.VCC, I_BD0.VCC, I_CH0.VCC, I_OH0.VCC, SINH.VCC)
	NET_C(GND, I_SD0.GND, I_BD0.GND, I_CH0.GND, I_OH0.GND, SINH.GND)

	ANALOG_INPUT(I_MSM2K0, 0)
	ANALOG_INPUT(I_MSM3K0, 0)

	INCLUDE(kidniki_schematics)

	#if (USE_FRONTIERS)
	OPTIMIZE_FRONTIER(C63.2, RES_K(27), RES_K(1))
	OPTIMIZE_FRONTIER(R31.2, RES_K(5.1), 50)
	OPTIMIZE_FRONTIER(R29.2, RES_K(2.7), 50)
	OPTIMIZE_FRONTIER(R87.2, RES_K(68), 50)

	OPTIMIZE_FRONTIER(R50.1, RES_K(2.2), 50)
	OPTIMIZE_FRONTIER(R55.2, RES_K(1000), 50)
	OPTIMIZE_FRONTIER(R84.2, RES_K(50), RES_K(5))
	#endif

}
