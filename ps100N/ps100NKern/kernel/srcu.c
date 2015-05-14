/*
 * Sleepable Read-Copy Update mechanism for mutual exclusion.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Copyright (C) IBM Corporation, 2006
 *
 * Author: Paul McKenney <paulmck@us.ibm.com>
 *
 * For detailed explanation of Read-Copy Update mechanism see -
 * 		Documentation/RCU/ *.txt
 *
 */

#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/percpu.h>
#include <linux/preempt.h>
#include <linux/rcupdate.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/smp.h>
#include <linux/srcu.h>

/**
 * init_srcu_struct - initialize a sleep-RCU structure
 * @sp: structure to initialize.
 *
 * Must invoke this on a given srcu_struct before passing that srcu_struct
 * to any other function.  Each srcu_struct represents a separate domain
 * of SRCU protection.
 */
int init_srcu_struct(struct srcu_struct *sp)
{
	sp->completed = 0;
	mutex_init(&sp->mutex);
	sp->per_cpu_ref = alloc_percpu(struct srcu_struct_array);
	return (sp->per_cpu_ref ? 0 : -ENOMEM);
}

/*
 * srcu_readers_active_idx -- returns approximate number of readers
 *	active on the specified rank of per-CPU counters.
 */

static int srcu_readers_active_idx(struct srcu_struct *sp, int idx)
{
	int cpu;
	int sum;

	sum = 0;
	for_each_possible_cpu(cpu)
		sum += per_cpu_ptr(sp->per_cpu_ref, cpu)->c[idx];
	return sum;
}

/**
 * srcu_readers_active - returns approximate number of readers.
 * @sp: which srcu_struct to count active readers (holding srcu_read_lock).
 *
 * Note that this is not an atomic primitive, and can therefore suffer
 * severe errors when invoked on an active srcu_struct.  That said, it
 * can be useful as an error check at cleanup time.
 */
static int srcu_readers_active(struct srcu_struct *sp)
{
	return srcu_readers_active_idx(sp, 0) + srcu_readers_active_idx(sp, 1);
}

/**
 * cleanup_srcu_struct - deconstruct a sleep-RCU structure
 * @sp: structure to clean up.
 *
 * Must invoke this after you are finished using a given srcu_struct that
 * was initialized via init_srcu_struct(), else you leak memory.
 */
void cleanup_srcu_struct(struct srcu_struct *sp)
{
	int sum;

	sum = srcu_readers_active(sp);
	WARN_ON(sum);  /* Leakage unless caller handles error. */
	if (sum != 0)
		return;
	free_percpu(sp->per_cpu_ref);
	sp->per_cpu_ref = NULL;
}

/**
 * srcu_read_lock - register a new reader for an SRCU-protected structure.
 * @sp: srcu_struct in which to register the new reader.
 *
 * Counts the new reader in the appropriate per-CPU element of the
 * srcu_struct.  Must be called from process context.
 * Returns an index that must be passed to the matching srcu_read_unlock().
 */
int srcu_read_lock(struct srcu_struct *sp)
{
	int idx;

	preempt_disable();
	idx = sp->completed & 0x1;
	barrier();  /* ensure compiler looks -once- at sp->completed. */
	per_cpu_ptr(sp->per_cpu_ref, smp_processor_id())->c[idx]++;
	srcu_barrier();  /* ensure compiler won't misorder critical section. */
	preempt_enable();
	return idx;
}

/**
 * srcu_read_unlock - unregister a old reader from an SRCU-protected structure.
 * @sp: srcu_struct in which to unregister the old reader.
 * @idx: return value from corresponding srcu_read_lock().
 *
 * Removes the count for the old reader from the appropriate per-CPU
 * element of the srcu_struct.  Note that this may well be a different
 * CPU than that which was incremented by the corresponding srcu_read_lock().
 * Must be called from process context.
     /* R72  - Input Control */
	0x0000,     /* R73  - IN3 Input Control */
	0x0000,     /* R74  - Mic Bias Control */
	0x0000,     /* R75 */
	0x0000,     /* R76  - Output Control */
	0x0000,     /* R77  - Jack Detect */
	0x0000,     /* R78  - Anti Pop Control */
	0x0000,     /* R79 */
	0x0040,     /* R80  - Left Input Volume */
	0x0040,     /* R81  - Right Input Volume */
	0x0000,     /* R82 */
	0x0000,     /* R83 */
	0x0000,     /* R84 */
	0x0000,     /* R85 */
	0x0000,     /* R86 */
	0x0000,     /* R87 */
	0x0800,     /* R88  - Left Mixer Control */
	0x1000,     /* R89  - Right Mixer Control */
	0x0000,     /* R90 */
	0x0000,     /* R91 */
	0x0000,     /* R92  - OUT3 Mixer Control */
	0x0000,     /* R93  - OUT4 Mixer Control */
	0x0000,     /* R94 */
	0x0000,     /* R95 */
	0x0000,     /* R96  - Output Left Mixer Volume */
	0x0000,     /* R97  - Output Right Mixer Volume */
	0x0000,     /* R98  - Input Mixer Volume L */
	0x0000,     /* R99  - Input Mixer Volume R */
	0x0000,     /* R100 - Input Mixer Volume */
	0x0000,     /* R101 */
	0x0000,     /* R102 */
	0x0000,     /* R103 */
	0x00E4,     /* R104 - OUT1L Volume */
	0x00E4,     /* R105 - OUT1R Volume */
	0x00E4,     /* R106 - OUT2L Volume */
	0x02E4,     /* R107 - OUT2R Volume */
	0x0000,     /* R108 */
	0x0000,     /* R109 */
	0x0000,     /* R110 */
	0x0000,     /* R111 - BEEP Volume */
	0x0A00,     /* R112 - AI Formating */
	0x0000,     /* R113 - ADC DAC COMP */
	0x0020,     /* R114 - AI ADC Control */
	0x0020,     /* R115 - AI DAC Control */
	0x0000,     /* R116 */
	0x0000,     /* R117 */
	0x0000,     /* R118 */
	0x0000,     /* R119 */
	0x0000,     /* R120 */
	0x0000,     /* R121 */
	0x0000,     /* R122 */
	0x0000,     /* R123 */
	0x0000,     /* R124 */
	0x0000,     /* R125 */
	0x0000,     /* R126 */
	0x0000,     /* R127 */
	0x1FFF,     /* R128 - GPIO Debounce */
	0x0000,     /* R129 - GPIO Pin pull up Control */
	0x0000,     /* R130 - GPIO Pull down Control */
	0x0000,     /* R131 - GPIO Interrupt Mode */
	0x0000,     /* R132 */
	0x0000,     /* R133 - GPIO Control */
	0x0FFC,     /* R134 - GPIO Configuration (i/o) */
	0x0FFC,     /* R135 - GPIO Pin Polarity / Type */
	0x0000,     /* R136 */
	0x0000,     /* R137 */
	0x0000,     /* R138 */
	0x0000,     /* R139 */
	0x0013,     /* R140 - GPIO Function Select 1 */
	0x0000,     /* R141 - GPIO Function Select 2 */
	0x0000,     /* R142 - GPIO Function Select 3 */
	0x0003,     /* R143 - GPIO Function Select 4 */
	0x0000,     /* R144 - Digitiser Control (1) */
	0x0002,     /* R145 - Digitiser Control (2) */
	0x0000,     /* R146 */
	0x0000,     /* R147 */
	0x0000,     /* R148 */
	0x0000,     /* R149 */
	0x0000,     /* R150 */
	0x0000,     /* R151 */
	0x7000,     /* R152 - AUX1 Readback */
	0x7000,     /* R153 - AUX2 Readback */
	0x7000,     /* R154 - AUX3 Readback */
	0x7000,     /* R155 - AUX4 Readback */
	0x0000,     /* R156 - USB Voltage Readback */
	0x0000,     /* R157 - LINE Voltage Readback */
	0x0000,     /* R158 - BATT Voltage Readback */
	0x0000,     /* R159 - Chip Temp Readback */
	0x0000,     /* R160 */
	0x0000,     /* R161 */
	0x0000,     /* R162 */
	0x0000,     /* R163 - Generic Comparator Control */
	0x0000,     /* R164 - Generic comparator 1 */
	0x0000,     /* R165 - Generic comparator 2 */
	0x0000,     /* R166 - Generic comparator 3 */
	0x0000,     /* R167 - Generic comparator 4 */
	0xA00F,     /* R168 - Battery Charger Control 1 */
	0x0B06,     /* R169 - Battery Charger Control 2 */
	0x0000,     /* R170 - Battery Charger Control 3 */
	0x0000,     /* R171 */
	0x0000,     /* R172 - Current Sink Driver A */
	0x0000,     /* R173 - CSA Flash control */
	0x0000,     /* R174 */
	0x0000,     /* R175 */
	0x0000,     /* R176 - DCDC/LDO requested */
	0x032D,     /* R177 - DCDC Active options */
	0x0000,     /* R178 - DCDC Sleep options */
	0x0025,     /* R179 - Power-check comparator */
	0x000E,     /* R180 - DCDC1 Control */
	0x0000,     /* R181 - DCDC1 Timeouts */
	0x1006,     /* R182 - DCDC1 Low Power */
	0x0018,     /* R183 - DCDC2 Control */
	0x0000,     /* R184 - DCDC2 Timeouts */
	0x0000,     /* R185 */
	0x0000,     /* R186 - DCDC3 Control */
	0x0000,     /* R187 - DCDC3 Timeouts */
	0x0006,     /* R188 - DCDC3 Low Power */
	0x0000,     /* R189 - DCDC4 Control */
	0x0000,     /* R190 - DCDC4 Timeouts */
	0x0006,     /* R191 - DCDC4 Low Power */
	0x0008,     /* R192 */
	0x0000,     /* R193 */
	0x0000,     /* R194 */
	0x0000,     /* R195 */
	0x0000,     /* R196 */
	0x0006,     /* R197 */
	0x0000,     /* R198 */
	0x0003,     /* R199 -