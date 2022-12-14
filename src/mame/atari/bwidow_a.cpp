// license:BSD-3-Clause
// copyright-holders: Couriersud



#include "emu.h"
#include "bwidow.h"

#include "sound/pokey.h"
#include "sound/discrete.h"

#include "speaker.h"


/*************************************
 *
 *  Discrete Sound Blocks
 *
 *************************************/

#define BW_R43      RES_M(1)
#define BW_R44      RES_M(1)
#define BW_R45      RES_K(100)
#define BW_R46      RES_K(22)
#define BW_R47      RES_K(1)
#define BW_R48      RES_M(1)
#define BW_R49      RES_K(3.9)
#define BW_R51      RES_K(1)
#define BW_R52      RES_K(3.9)

#define BW_C27      CAP_P(100)
#define BW_C28      CAP_P(1)
#define BW_C30      CAP_U(0.22)
#define BW_C31      CAP_U(0.015)
#define BW_C32      CAP_U(0.015)
#define BW_C33      CAP_U(0.22)

static discrete_op_amp_filt_info stage1_bwidow_info = {
		BW_R45, 0, 0, 0,    /* r1 .. r4 */
		BW_R43,             /* rF */
		BW_C27,             /* C1 */
		BW_C30,             /* C2 */
		0,                  /* C3 */
		0.0,                /* vRef */
		15.0,               /* vP */
		-15.0,              /* vN */
};


static discrete_op_amp_filt_info stage2_bwidow_info = {
		BW_R48, 0, 0, 0,    /* r1 .. r4 */
		BW_R44,             /* rF */
		BW_C28,             /* C1 */        /* on schematic, not on parts list, gravitar retrofit manual says not needed - so what? */
		0,                  /* C2 */
		0,                  /* C3 */
		0.0,                /* vRef */
		15.0,               /* vP */
		-15.0,              /* vN */
};

static discrete_mixer_desc bwidow_mixer = {
		DISC_MIXER_IS_OP_AMP,       /* type */
		{ BW_R49, BW_R46 },         /* r{} */
		{},                         /* r_node */
		{ BW_C33, 0 },              /* c{} */
		0,                          /* rI  */
		BW_R52,                     /* rF  */
		0,                          /* cF  */
		0,                          /* cAmp */
		0,                          /* vRef */
		1.0                         /* gain */
};

static DISCRETE_SOUND_START(bwidow_discrete)

	/************************************************/
	/* FINAL MIX                                    */
	/************************************************/

	/* Convert Pokey output to 5V Signal */
	DISCRETE_INPUTX_STREAM(NODE_100, 0, 5.0 / 32768, 5.0)   /* Add VRef again */
	DISCRETE_INPUTX_STREAM(NODE_110, 1, 5.0 / 32768, 5.0)   /* Add VRef again */

	DISCRETE_OP_AMP_FILTER(NODE_120, 1, NODE_110, 0, DISC_OP_AMP_FILTER_IS_BAND_PASS_1, &stage1_bwidow_info)
	DISCRETE_OP_AMP_FILTER(NODE_130, 1, NODE_120, 0, DISC_OP_AMP_FILTER_IS_LOW_PASS_1, &stage2_bwidow_info)

	DISCRETE_MIXER2(NODE_290, 1, NODE_100, NODE_130, &bwidow_mixer)
	DISCRETE_OUTPUT(NODE_290, 4096)

DISCRETE_SOUND_END

#define GRAV_C34        CAP_U(0.22)
#define GRAV_R46        RES_K(10)
#define GRAV_C27        CAP_N(1)

static discrete_op_amp_filt_info stage1_gravitar_info = {
		BW_R45, 0, 0, 0,    /* r1 .. r4 */
		BW_R43,             /* rF */
		GRAV_C27,           /* C1 */
		BW_C30,             /* C2 */
		0,                  /* C3 */
		0.0,                /* vRef */
		15.0,               /* vP */
		-15.0,              /* vN */
};

/* same as bwidow, already in for possible changes to C28 */
static discrete_op_amp_filt_info stage2_gravitar_info = {
		BW_R48, 0, 0, 0,    /* r1 .. r4 */
		BW_R44,             /* rF */
		BW_C28,             /* C1 */        /* on schematic, not on parts list, gravitar retrofit manual says not needed - so what? */
		0,                  /* C2 */
		0,                  /* C3 */
		0.0,                /* vRef */
		15.0,               /* vP */
		-15.0,              /* vN */
};

static discrete_mixer_desc gravitar_mixer = {
		DISC_MIXER_IS_OP_AMP,       /* type */
		{ BW_R49, GRAV_R46 },       /* r{} */
		{},                         /* r_node */
		{ BW_C33, 0 },              /* c{} */
		0,                          /* rI  */
		BW_R52,                     /* rF  */
		0,                          /* cF  */
		0,                          /* cAmp */
		0,                          /* vRef */
		1.0                         /* gain */
};


static DISCRETE_SOUND_START(gravitar_discrete)

	/************************************************/
	/* FINAL MIX                                    */
	/************************************************/

	/* Convert Pokey output to 5V Signal */
	DISCRETE_INPUTX_STREAM(NODE_100, 0, 5.0 / 32768, 5.0)   /* Add VRef again */
	DISCRETE_INPUTX_STREAM(NODE_110, 1, 5.0 / 32768, 5.0)   /* Add VRef again */

	DISCRETE_OP_AMP_FILTER(NODE_120, 1, NODE_110, 0, DISC_OP_AMP_FILTER_IS_BAND_PASS_1, &stage1_gravitar_info)
	DISCRETE_OP_AMP_FILTER(NODE_130, 1, NODE_120, 0, DISC_OP_AMP_FILTER_IS_LOW_PASS_1, &stage2_gravitar_info)

	DISCRETE_MIXER2(NODE_290, 1, NODE_100, NODE_130, &gravitar_mixer)
	DISCRETE_OUTPUT(NODE_290, 4096)

DISCRETE_SOUND_END

void bwidow_state::bwidow_audio(machine_config &config)
{
	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	pokey_device &pokey1(POKEY(config, "pokey1", MASTER_CLOCK / 8)); /* C/D3 */
	pokey1.allpot_r().set_ioport("DSW0");
	pokey1.set_output_opamp(BW_R51, BW_C31, 5.0);
	pokey1.add_route(0, "discrete", 1.0, 0);

	pokey_device &pokey2(POKEY(config, "pokey2", MASTER_CLOCK / 8)); /* B3 */
	pokey2.allpot_r().set_ioport("DSW1");
	pokey2.set_output_opamp(BW_R47, BW_C32, 5.0);
	pokey2.add_route(0, "discrete", 1.0, 1);

	DISCRETE(config, "discrete", bwidow_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);

	//config.m_perfect_cpu_quantum = subtag("pokey1");
}

void bwidow_state::gravitar_audio(machine_config &config)
{
	subdevice<pokey_device>("pokey1")->set_output_opamp_low_pass(BW_R51, GRAV_C34, 5.0); /* BW_C31 ignored */

	subdevice<discrete_sound_device>("discrete")->set_intf(gravitar_discrete);
}
