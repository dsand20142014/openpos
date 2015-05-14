/* cpumap.c: used for optimizing CPU assignment
 *
 * Copyright (C) 2009 Hong H. Pham <hong.pham@windriver.com>
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cpumask.h>
#include <linux/spinlock.h>
#include <asm/cpudata.h>
#include "cpumap.h"


enum {
	CPUINFO_LVL_ROOT = 0,
	CPUINFO_LVL_NODE,
	CPUINFO_LVL_CORE,
	CPUINFO_LVL_PROC,
	CPUINFO_LVL_MAX,
};

enum {
	ROVER_NO_OP              = 0,
	/* Increment rover every time level is visited */
	ROVER_INC_ON_VISIT       = 1 << 0,
	/* Increment parent's rover every time rover wraps around */
	ROVER_INC_PARENT_ON_LOOP = 1 << 1,
};

struct cpuinfo_node {
	int id;
	int level;
	int num_cpus;    /* Number of CPUs in this hierarchy */
	int parent_index;
	int child_start; /* Array index of the first child node */
	int child_end;   /* Array index of the last child node */
	int rover;       /* Child node iterator */
};

struct cpuinfo_level {
	int start_index; /* Index of first node of a level in a cpuinfo tree */
	int end_index;   /* Index of last node of a level in a cpuinfo tree */
	int num_nodes;   /* Number of nodes in a level in a cpuinfo tree */
};

struct cpuinfo_tree {
	int total_nodes;

	/* Offsets into nodes[] for each level of the tree */
	struct cpuinfo_level level[CPUINFO_LVL_MAX];
	struct cpuinfo_node  nodes[0];
};


static struct cpuinfo_tree *cpuinfo_tree;

static u16 cpu_distribution_map[NR_CPUS];
static DEFINE_SPINLOCK(cpu_map_lock);


/* Niagara optimized cpuinfo tree traversal. */
static const int niagara_iterate_method[] = {
	[CPUINFO_LVL_ROOT] = ROVER_NO_OP,

	/* Strands (or virtual CPUs) within a core may not run concurrently
	 * on the Niagara, as instruction pipeline(s) are shared.  Distribute
	 * work to strands in different cores first for better concurrency.
	 * Go to next NUMA node when all cores are used.
	 */
	[CPUINFO_LVL_NODE] = ROVER_INC_ON_VISIT|ROVER_INC_PARENT_ON_LOOP,

	/* Strands are grouped together by proc_id in cpuinfo_sparc, i.e.
	 * a proc_id represents an instruction pipeline.  Distribute work to
	 * strands in different proc_id groups if the core has multiple
	 * instruction pipelines (e.g. the Niagara 2/2+ has two).
	 */
	[CPUINFO_LVL_CORE] = ROVER_INC_ON_VISIT,

	/* Pick the next strand in the proc_id group. */
	[CPUINFO_LVL_PROC] = ROVER_INC_ON_VISIT,
};

/* Generic cpuinfo tree traversal.  Distribute work round robin across NUMA
 * nodes.
 */
static const int generic_iterate_method[] = {
	[CPUINFO_LVL_ROOT] = ROVER_INC_ON_VISIT,
	[CPUINFO_LVL_NODE] = ROVER_NO_OP,
	[CPUINFO_LVL_CORE] = ROVER_INC_PARENT_ON_LOOP,
	[CPUINFO_LVL_PROC] = ROVER_INC_ON_VISIT|ROVER_INC_PARENT_ON_LOOP,
};


static int cpuinfo_id(int cpu, int level)
{
	int id;

	switch (level) {
	case CPUINFO_LVL_ROOT:
		id = 0;
		break;
	case CPUINFO_LVL_NODE:
		id = cpu_to_node(cpu);
		break;
	case CPUINFO_LVL_CORE:
		id = cpu_data(cpu).core_id;
		break;
	case CPUINFO_LVL_PROC:
		id = cpu_data(cpu).proc_id;
		break;
	default:
		id = -EINVAL;
	}
	return id;
}

/*
 * Enumerate the CPU information in __cpu_data to determine the start index,
 * end index, and number of nodes for each level in the cpuinfo tree.  The
 * total number of cpuinfo nodes required to build the tree is returned.
 */
static int enumerate_cpuinfo_nodes(struct cpuinfo_level *tree_level)
{
	int prev_id[CPUINFO_LVL_MAX];
	int i, n, num_nodes;

	for (i = CPUINFO_LVL_ROOT; i < CPUINFO_LVL_MAX; i++) {
		struct cpuinfo_level *lv = &tree_level[i];

		prev_id[i] = -1;
		lv->start_index = lv->end_index = lv->num_nodes = 0;
	}

	num_nodes = 1; /* Include the root node */

	for (i = 0; i < num_possible_cpus(); i++) {
		if (!cpu_online(i))
			continue;

		n = cpuinfo_id(i, CPUINFO_LVL_NODE);
		if (n > prev_id[CPUINFO_LVL_NODE]) {
			tree_level[CPUINFO_LVL_NODE].num_nodes++;
			prev_id[CPUINFO_LVL_NODE] = n;
			num_nodes++;
		}
		n = cpuinfo_id(i, CPUINFO_LVL_CORE);
		if (n > prev_id[CPUINFO_LVL_CORE]) {
			tree_level[CPUINFO_LVL_CORE].num_nodes++;
			prev_id[CPUINFO_LVL_CORE] = n;
			num_nodes++;
		}
		n = cpuinfo_id(i, CPUINFO_LVL_PROC);
		if (n > prev_id[CPUINFO_LVL_PROC]) {
			tree_level[CPUINFO_LVL_PROC].num_nodes++;
			prev_id[CPUINFO_LVL_PROC] = n;
			num_nodes++;
		}
	}

	tree_level[CPUINFO_LVL_ROOT].num_nodes = 1;

	n = tree_level[CPUINFO_LVL_NODE].num_nodes;
	tree_level[CPUINFO_LVL_NODE].start_index = 1;
	tree_level[CPUINFO_LVL_NODE].end_index   = n;

	n++;
	tree_level[CPUINFO_LVL_CORE].start_index = n;
	n += tree_level[CPUINFO_LVL_CORE].num_nodes;
	tree_level[CPUINFO_LVL_CORE].end_index   = n - 1;

	tree_level[CPUINFO_LVL_PROC].start_index = n;
	n += tree_level[CPUINFO_LVL_PROC].num_nodes;
	tree_level[CPUINFO_LVL_PROC].end_index   = n - 1;

	return num_nodes;
}

/* Build a tree representation of the CPU hierarchy using the per CPU
 * information in __cpu_data.  Entries in __cpu_data[0..NR_CPUS] are
 * assumed to be sorted in ascending order based on node, core_id, and
 * proc_id (in order of significance).
 */
static struct cpuinfo_tree *build_cpuinfo_tree(void)
{
	struct cpuinfo_tree *new_tree;
	struct cpuinfo_node *node;
	struct cpuinfo_level tmp_level[CPUINFO_LVL_MAX];
	int num_cpus[CPUINFO_LVL_MAX];
	int level_rover[CPUINFO_LVL_MAX];
	int prev_id[CPUINFO_LVL_MAX];
	int n, id, cpu, prev_cpu, last_cpu, level;

	n = enumerate_cpuinfo_nodes(tmp_level);

	new_tree = kzalloc(sizeof(struct cpuinfo_tree) +
	                   (sizeof(struct cpuinfo_node) * n), GFP_ATOMIC);
	if (!new_tree)
		return NULL;

	new_tree->total_nodes = n;
	memcpy(&new_tree->level, tmp_level, sizeof(tmp_level));

	prev_cpu = cpu = first_cpu(cpu_online_map);

	/* Initialize all levels in the tree with the first CPU */
	for (level = CPUINFO_LVL_PROC; level >= CPUINFO_LVL_ROOT; level--) {
		n = new_tree->level[level].start_index;

		level_rover[level] = n;
		node = &new_tree->nodes[n];

		id = cpuinfo_id(cpu, level);
		if (unlikely(id < 0)) {
			kfree(new_tree);
			return NULL;
		}
		node->id = id;
		node->level = level;
		node->num_cpus = 1;

		node->parent_index = (level > CPUINFO_LVL_ROOT)
		    ? new_tree->level[level - 1].start_index : -1;

		node->child_start = node->child_end = node->rover =
		    (level == CPUINFO_LVL_PROC)
		    ? cpu : new_tree->level[level + 1].start_index;

		prev_id[level] = node->id;
		num_cpus[level] = 1;
	}

	for (last_cpu = (num_possible_cpus() - 1); last_cpu >= 0; last_cpu--) {
		if (cpu_online(last_cpu))
			break;
	}

	while (++cpu <= last_cpu) {
		if (!cpu_online(cpu))
			continue;

		for (level = CPUINFO_LVL_PROC; level >= CPUINFO_LVL_ROOT;
		     level--) {
			id = cpuinfo_id(cpu, level);
			if (unlikely(id < 0)) {
				kfree(new_tree);
				return NULL;
			}

			if ((id != prev_id[level]) || (cpu == last_cpu)) {
				prev_id[level] = id;
				node = &new_tree->nodes[level_rover[level]];
				node->num_cpus = num_cpus[level];
				num_cpus[level] = 1;

				if (cpu == last_cpu)
					node->num_cpus++;

				/* Connect tree node to parent */
				if (level == CPUINFO_LVL_ROOT)
					node->parent_index = -1;
				else
					node->parent_index =
					    level_rover[level - 1];

				if (level == CPUINFO_LVL_PROC) {
					node->child_end =
					    (cpu == last_cpu) ? cpu : prev_cpu;
				} else {
					node->child_end =
					    level_rover[level + 1] - 1;
				}

				/* Initialize the next node in the same level */
				n = ++level_rover[level];
				if (n <= new_tree->level[level].end_index) {
					node = &new_tree->nodes[n];
					node->id = id;
					node->level = level;

					/* Connect node to child */
					node->child_start = node->child_end =
					node->rover =
					    (level == CPUINFO_LVL_PROC)
					    ? cpu : level_rover[level + 1];
				}
			} else
				num_cpus[level]++;
		}
		prev_cpu = cpu;
	}

	return new_tree;
}

static void increment_rover(struct cpuinfo_tree *t, int node_index,
                            int root_index, const int *rover_inc_table)
{
	struct cpuinfo_node *node = &t->nodes[node_index];
	int top_level, level;

	top_level = t->nodes[root_index].level;
	for (level = node->level; level >= top_level; level--) {
		node->rover++;
		if (node->rover <= node->child_end)
			return;

		node->rover = node->child_start;
		/* If parent's rover does not need to be adjusted, sto if we can do fast timing */
	ahc_scbram_config(ahc, enable, pcheck, /*fast*/TRUE, large);
	test_num_scbs = ahc_probe_scbs(ahc);
	if (test_num_scbs == num_scbs
	 && ((ahc_inb(ahc, INTSTAT) & BRKADRINT) == 0
	  || (ahc_inb(ahc, ERROR) & MPARERR) == 0))
		fast = TRUE;

	/*
	 * See if we can use large SCBs and still maintain
	 * the same overall count of SCBs.
	 */
	if ((ahc->features & AHC_LARGE_SCBS) != 0) {
		ahc_scbram_config(ahc, enable, pcheck, fast, /*large*/TRUE);
		test_num_scbs = ahc_probe_scbs(ahc);
		if (test_num_scbs >= num_scbs) {
			large = TRUE;
			num_scbs = test_num_scbs;
	 		if (num_scbs >= 64) {
				/*
				 * We have enough space to move the
				 * "busy targets table" into SCB space
				 * and make it qualify all the way to the
				 * lun level.
				 */
				ahc->flags |= AHC_SCB_BTT;
			}
		}
	}
done:
	/*
	 * Disable parity error reporting until we
	 * can load instruction ram.
	 */
	ahc_outb(ahc, SEQCTL, PERRORDIS|FAILDIS);
	/* Clear any latched parity error */
	ahc_outb(ahc, CLRINT, CLRPARERR);
	ahc_outb(ahc, CLRINT, CLRBRKADRINT);
	if (bootverbose && enable) {
		printf("%s: External SRAM, %s access%s, %dbytes/SCB\n",
		       ahc_name(ahc), fast ? "fast" : "slow", 
		       pcheck ? ", parity checking enabled" : "",
		       large ? 64 : 32);
	}
	ahc_scbram_config(ahc, enable, pcheck, fast, large);
}

/*
 * Perform some simple tests that should catch situations where
 * our registers are invalidly mapped.
 */
int
ahc_pci_test_register_access(struct ahc_softc *ahc)
{
	int	 error;
	u_int	 status1;
	uint32_t cmd;
	uint8_t	 hcntrl;

	error = EIO;

	/*
	 * Enable PCI error interrupt status, but suppress NMIs
	 * generated by SERR raised due to target aborts.
	 */
	cmd = ahc_pci_read_config(ahc->dev_softc, PCIR_COMMAND, /*bytes*/2);
	ahc_pci_write_config(ahc->dev_softc, PCIR_COMMAND,
			     cmd & ~PCIM_CMD_SERRESPEN, /*bytes*/2);

	/*
	 * First a simple test to see if any
	 * registers can be read.  Reading
	 * HCNTRL has no side effects and has
	 * at least one bit that is guaranteed to
	 * be zero so it is a good register to
	 * use for this test.
	 */
	hcntrl = ahc_inb(ahc, HCNTRL);

	if (hcntrl == 0xFF)
		goto fail;

	if ((hcntrl & CHIPRST) != 0) {
		/*
		 * The chip has not been initialized since
		 * PCI/EISA/VLB bus reset.  Don't trust
		 * "left over BIOS data".
		 */
		ahc->flags |= AHC_NO_BIOS_INIT;
	}

	/*
	 * Next create a situation where write combining
	 * or read prefetching could be initiated by the
	 * CPU or host bridge.  Our device does not support
	 * either, so look for data corruption and/or flagged
	 * PCI errors.  First pause without causing another
	 * chip reset.
	 */
	hcntrl &= ~CHIPRST;
	ahc_outb(ahc, HCNTRL, h