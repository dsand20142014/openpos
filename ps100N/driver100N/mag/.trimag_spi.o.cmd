cmd_/home/root/ps100N/src/driver100N/mag/trimag_spi.o := /opt/freescale/usr/local/gcc-4.1.2-glibc-2.5-nptl-3/arm-none-linux-gnueabi/bin/arm-none-linux-gnueabi-gcc -Wp,-MD,/home/root/ps100N/src/driver100N/mag/.trimag_spi.o.d  -nostdinc -isystem /opt/freescale/usr/local/gcc-4.1.2-glibc-2.5-nptl-3/arm-none-linux-gnueabi/lib/gcc/arm-none-linux-gnueabi/4.1.2/include -Iinclude  -I/home/root/ps100N/src/ps100NKern/arch/arm/include -include include/linux/autoconf.h -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-mx25/include -Iarch/arm/plat-mxc/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -O2 -marm -mabi=aapcs-linux -mno-thumb-interwork -funwind-tables -D__LINUX_ARM_ARCH__=5 -march=armv5te -mtune=arm9tdmi -msoft-float -Uarm -fno-stack-protector -fomit-frame-pointer -Wdeclaration-after-statement -Wno-pointer-sign -O2 -Wall -I.  -DMODULE -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(trimag_spi)"  -D"KBUILD_MODNAME=KBUILD_STR(trimag_spi)" -D"DEBUG_HASH=37" -D"DEBUG_HASH2=0" -c -o /home/root/ps100N/src/driver100N/mag/.tmp_trimag_spi.o /home/root/ps100N/src/driver100N/mag/trimag_spi.c

deps_/home/root/ps100N/src/driver100N/mag/trimag_spi.o := \
  /home/root/ps100N/src/driver100N/mag/trimag_spi.c \
    $(wildcard include/config/func.h) \
    $(wildcard include/config/alt1.h) \
    $(wildcard include/config/alt2.h) \
    $(wildcard include/config/alt3.h) \
    $(wildcard include/config/alt4.h) \
    $(wildcard include/config/alt5.h) \
    $(wildcard include/config/alt6.h) \
    $(wildcard include/config/alt7.h) \
    $(wildcard include/config/sion.h) \
    $(wildcard include/config/gpio.h) \
  /home/root/ps100N/src/driver100N/mag/trimag_spi.h \
  include/linux/semaphore.h \
  include/linux/list.h \
    $(wildcard include/config/debug/list.h) \
  include/linux/stddef.h \
  include/linux/compiler.h \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
  include/linux/compiler-gcc.h \
    $(wildcard include/config/arch/supports/optimized/inlining.h) \
    $(wildcard include/config/optimize/inlining.h) \
  include/linux/compiler-gcc4.h \
  include/linux/poison.h \
  include/linux/prefetch.h \
  include/linux/types.h \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/lbdaf.h) \
    $(wildcard include/config/phys/addr/t/64bit.h) \
    $(wildcard include/config/64bit.h) \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/types.h \
  include/asm-generic/int-ll64.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/bitsperlong.h \
  include/asm-generic/bitsperlong.h \
  include/linux/posix_types.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/posix_types.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/processor.h \
    $(wildcard include/config/mmu.h) \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/ptrace.h \
    $(wildcard include/config/cpu/endian/be8.h) \
    $(wildcard include/config/arm/thumb.h) \
    $(wildcard include/config/smp.h) \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/hwcap.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/cache.h \
    $(wildcard include/config/aeabi.h) \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/system.h \
    $(wildcard include/config/cpu/xsc3.h) \
    $(wildcard include/config/cpu/fa526.h) \
    $(wildcard include/config/cpu/sa1100.h) \
    $(wildcard include/config/cpu/sa110.h) \
    $(wildcard include/config/cpu/32v6k.h) \
  include/linux/linkage.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/linkage.h \
  include/linux/irqflags.h \
    $(wildcard include/config/trace/irqflags.h) \
    $(wildcard include/config/irqsoff/tracer.h) \
    $(wildcard include/config/preempt/tracer.h) \
    $(wildcard include/config/trace/irqflags/support.h) \
    $(wildcard include/config/x86.h) \
  include/linux/typecheck.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/irqflags.h \
  include/asm-generic/cmpxchg-local.h \
  include/asm-generic/cmpxchg.h \
  include/linux/spinlock.h \
    $(wildcard include/config/debug/spinlock.h) \
    $(wildcard include/config/generic/lockbreak.h) \
    $(wildcard include/config/preempt.h) \
    $(wildcard include/config/debug/lock/alloc.h) \
  include/linux/preempt.h \
    $(wildcard include/config/debug/preempt.h) \
    $(wildcard include/config/preempt/notifiers.h) \
  include/linux/thread_info.h \
    $(wildcard include/config/compat.h) \
  include/linux/bitops.h \
    $(wildcard include/config/generic/find/first/bit.h) \
    $(wildcard include/config/generic/find/last/bit.h) \
    $(wildcard include/config/generic/find/next/bit.h) \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/bitops.h \
  include/asm-generic/bitops/non-atomic.h \
  include/asm-generic/bitops/fls64.h \
  include/asm-generic/bitops/sched.h \
  include/asm-generic/bitops/hweight.h \
  include/asm-generic/bitops/lock.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/thread_info.h \
    $(wildcard include/config/arm/thumbee.h) \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/fpstate.h \
    $(wildcard include/config/vfpv3.h) \
    $(wildcard include/config/iwmmxt.h) \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/domain.h \
    $(wildcard include/config/io/36.h) \
  include/linux/kernel.h \
    $(wildcard include/config/preempt/voluntary.h) \
    $(wildcard include/config/debug/spinlock/sleep.h) \
    $(wildcard include/config/prove/locking.h) \
    $(wildcard include/config/printk.h) \
    $(wildcard include/config/dynamic/debug.h) \
    $(wildcard include/config/ring/buffer.h) \
    $(wildcard include/config/tracing.h) \
    $(wildcard include/config/numa.h) \
    $(wildcard include/config/ftrace/mcount/record.h) \
  /opt/freescale/usr/local/gcc-4.1.2-glibc-2.5-nptl-3/arm-none-linux-gnueabi/lib/gcc/arm-none-linux-gnueabi/4.1.2/include/stdarg.h \
  include/linux/log2.h \
    $(wildcard include/config/arch/has/ilog2/u32.h) \
    $(wildcard include/config/arch/has/ilog2/u64.h) \
  include/linux/ratelimit.h \
  include/linux/param.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/param.h \
    $(wildcard include/config/hz.h) \
  include/linux/dynamic_debug.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/byteorder.h \
  include/linux/byteorder/little_endian.h \
  include/linux/swab.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/swab.h \
  include/linux/byteorder/generic.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/bug.h \
    $(wildcard include/config/bug.h) \
    $(wildcard include/config/debug/bugverbose.h) \
  include/asm-generic/bug.h \
    $(wildcard include/config/generic/bug.h) \
    $(wildcard include/config/generic/bug/relative/pointers.h) \
  include/linux/stringify.h \
  include/linux/bottom_half.h \
  include/linux/spinlock_types.h \
  include/linux/spinlock_types_up.h \
  include/linux/lockdep.h \
    $(wildcard include/config/lockdep.h) \
    $(wildcard include/config/lock/stat.h) \
    $(wildcard include/config/generic/hardirqs.h) \
  include/linux/spinlock_up.h \
  include/linux/spinlock_api_up.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/atomic.h \
  include/asm-generic/atomic-long.h \
  include/linux/timer.h \
    $(wildcard include/config/timer/stats.h) \
    $(wildcard include/config/debug/objects/timers.h) \
  include/linux/ktime.h \
    $(wildcard include/config/ktime/scalar.h) \
  include/linux/time.h \
    $(wildcard include/config/arch/uses/gettimeoffset.h) \
  include/linux/cache.h \
    $(wildcard include/config/arch/has/cache/line/size.h) \
  include/linux/seqlock.h \
  include/linux/math64.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/div64.h \
  include/linux/jiffies.h \
  include/linux/timex.h \
    $(wildcard include/config/no/hz.h) \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/timex.h \
  arch/arm/plat-mxc/include/mach/timex.h \
    $(wildcard include/config/arch/mx1.h) \
    $(wildcard include/config/arch/mx2.h) \
    $(wildcard include/config/arch/mx25.h) \
    $(wildcard include/config/arch/mx3.h) \
    $(wildcard include/config/arch/mx35.h) \
    $(wildcard include/config/arch/mx37.h) \
    $(wildcard include/config/arch/mx51.h) \
  include/linux/debugobjects.h \
    $(wildcard include/config/debug/objects.h) \
    $(wildcard include/config/debug/objects/free.h) \
  include/linux/workqueue.h \
  include/linux/device.h \
    $(wildcard include/config/debug/devres.h) \
  include/linux/ioport.h \
  include/linux/kobject.h \
    $(wildcard include/config/hotplug.h) \
  include/linux/sysfs.h \
    $(wildcard include/config/sysfs.h) \
  include/linux/errno.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/errno.h \
  include/asm-generic/errno.h \
  include/asm-generic/errno-base.h \
  include/linux/kref.h \
  include/linux/wait.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/current.h \
  include/linux/klist.h \
  include/linux/module.h \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/modversions.h) \
    $(wildcard include/config/unused/symbols.h) \
    $(wildcard include/config/kallsyms.h) \
    $(wildcard include/config/markers.h) \
    $(wildcard include/config/tracepoints.h) \
    $(wildcard include/config/event/tracing.h) \
    $(wildcard include/config/module/unload.h) \
    $(wildcard include/config/constructors.h) \
  include/linux/stat.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/stat.h \
  include/linux/kmod.h \
  include/linux/gfp.h \
    $(wildcard include/config/kmemcheck.h) \
    $(wildcard include/config/highmem.h) \
    $(wildcard include/config/zone/dma.h) \
    $(wildcard include/config/zone/dma32.h) \
    $(wildcard include/config/debug/vm.h) \
  include/linux/mmzone.h \
    $(wildcard include/config/force/max/zoneorder.h) \
    $(wildcard include/config/memory/hotplug.h) \
    $(wildcard include/config/sparsemem.h) \
    $(wildcard include/config/arch/populates/node/map.h) \
    $(wildcard include/config/discontigmem.h) \
    $(wildcard include/config/flat/node/mem/map.h) \
    $(wildcard include/config/cgroup/mem/res/ctlr.h) \
    $(wildcard include/config/have/memory/present.h) \
    $(wildcard include/config/need/node/memmap/size.h) \
    $(wildcard include/config/need/multiple/nodes.h) \
    $(wildcard include/config/have/arch/early/pfn/to/nid.h) \
    $(wildcard include/config/flatmem.h) \
    $(wildcard include/config/sparsemem/extreme.h) \
    $(wildcard include/config/nodes/span/other/nodes.h) \
    $(wildcard include/config/holes/in/zone.h) \
    $(wildcard include/config/arch/has/holes/memorymodel.h) \
  include/linux/threads.h \
    $(wildcard include/config/nr/cpus.h) \
    $(wildcard include/config/base/small.h) \
  include/linux/numa.h \
    $(wildcard include/config/nodes/shift.h) \
  include/linux/init.h \
  include/linux/nodemask.h \
  include/linux/bitmap.h \
  include/linux/string.h \
    $(wildcard include/config/binary/printf.h) \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/string.h \
  include/linux/pageblock-flags.h \
    $(wildcard include/config/hugetlb/page.h) \
    $(wildcard include/config/hugetlb/page/size/variable.h) \
  include/linux/bounds.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/page.h \
    $(wildcard include/config/cpu/copy/v3.h) \
    $(wildcard include/config/cpu/copy/v4wt.h) \
    $(wildcard include/config/cpu/copy/v4wb.h) \
    $(wildcard include/config/cpu/copy/feroceon.h) \
    $(wildcard include/config/cpu/copy/fa.h) \
    $(wildcard include/config/cpu/xscale.h) \
    $(wildcard include/config/cpu/copy/v6.h) \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/glue.h \
    $(wildcard include/config/cpu/arm610.h) \
    $(wildcard include/config/cpu/arm710.h) \
    $(wildcard include/config/cpu/abrt/lv4t.h) \
    $(wildcard include/config/cpu/abrt/ev4.h) \
    $(wildcard include/config/cpu/abrt/ev4t.h) \
    $(wildcard include/config/cpu/abrt/ev5tj.h) \
    $(wildcard include/config/cpu/abrt/ev5t.h) \
    $(wildcard include/config/cpu/abrt/ev6.h) \
    $(wildcard include/config/cpu/abrt/ev7.h) \
    $(wildcard include/config/cpu/pabrt/ifar.h) \
    $(wildcard include/config/cpu/pabrt/noifar.h) \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/memory.h \
    $(wildcard include/config/page/offset.h) \
    $(wildcard include/config/dram/size.h) \
    $(wildcard include/config/dram/base.h) \
  include/linux/const.h \
  arch/arm/plat-mxc/include/mach/memory.h \
    $(wildcard include/config/mach/mx21.h) \
    $(wildcard include/config/mach/mx27.h) \
    $(wildcard include/config/mx1/video.h) \
    $(wildcard include/config/mx3/video.h) \
    $(wildcard include/config/dma/zone/size.h) \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/sizes.h \
  include/asm-generic/memory_model.h \
    $(wildcard include/config/sparsemem/vmemmap.h) \
  include/asm-generic/getorder.h \
  include/linux/memory_hotplug.h \
    $(wildcard include/config/have/arch/nodedata/extension.h) \
    $(wildcard include/config/memory/hotremove.h) \
  include/linux/notifier.h \
  include/linux/mutex.h \
    $(wildcard include/config/debug/mutexes.h) \
  include/linux/rwsem.h \
    $(wildcard include/config/rwsem/generic/spinlock.h) \
  include/linux/rwsem-spinlock.h \
  include/linux/srcu.h \
  include/linux/topology.h \
    $(wildcard include/config/sched/smt.h) \
    $(wildcard include/config/sched/mc.h) \
  include/linux/cpumask.h \
    $(wildcard include/config/disable/obsolete/cpumask/functions.h) \
    $(wildcard include/config/hotplug/cpu.h) \
    $(wildcard include/config/cpumask/offstack.h) \
    $(wildcard include/config/debug/per/cpu/maps.h) \
  include/linux/smp.h \
    $(wildcard include/config/use/generic/smp/helpers.h) \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/topology.h \
  include/asm-generic/topology.h \
  include/linux/mmdebug.h \
    $(wildcard include/config/debug/virtual.h) \
  include/linux/elf.h \
  include/linux/elf-em.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/elf.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/user.h \
  include/linux/moduleparam.h \
    $(wildcard include/config/alpha.h) \
    $(wildcard include/config/ia64.h) \
    $(wildcard include/config/ppc64.h) \
  include/linux/marker.h \
  include/linux/tracepoint.h \
  include/linux/rcupdate.h \
    $(wildcard include/config/classic/rcu.h) \
    $(wildcard include/config/tree/rcu.h) \
    $(wildcard include/config/preempt/rcu.h) \
  include/linux/completion.h \
  include/linux/rcuclassic.h \
    $(wildcard include/config/rcu/cpu/stall/detector.h) \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/local.h \
  include/asm-generic/local.h \
  include/linux/percpu.h \
    $(wildcard include/config/have/dynamic/per/cpu/area.h) \
    $(wildcard include/config/debug/kmemleak.h) \
  include/linux/slab.h \
    $(wildcard include/config/slab/debug.h) \
    $(wildcard include/config/slub.h) \
    $(wildcard include/config/slob.h) \
    $(wildcard include/config/debug/slab.h) \
  include/linux/slab_def.h \
    $(wildcard include/config/kmemtrace.h) \
  include/linux/kmemtrace.h \
  include/trace/events/kmem.h \
  include/trace/define_trace.h \
  include/linux/kmalloc_sizes.h \
  include/linux/pfn.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/percpu.h \
  include/asm-generic/percpu.h \
    $(wildcard include/config/have/setup/per/cpu/area.h) \
  include/linux/percpu-defs.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/module.h \
    $(wildcard include/config/arm/unwind.h) \
  include/linux/pm.h \
    $(wildcard include/config/pm/sleep.h) \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/device.h \
    $(wildcard include/config/dmabounce.h) \
  include/linux/pm_wakeup.h \
    $(wildcard include/config/pm.h) \
  include/linux/fs.h \
    $(wildcard include/config/dnotify.h) \
    $(wildcard include/config/quota.h) \
    $(wildcard include/config/fsnotify.h) \
    $(wildcard include/config/inotify.h) \
    $(wildcard include/config/security.h) \
    $(wildcard include/config/fs/posix/acl.h) \
    $(wildcard include/config/epoll.h) \
    $(wildcard include/config/debug/writecount.h) \
    $(wildcard include/config/file/locking.h) \
    $(wildcard include/config/auditsyscall.h) \
    $(wildcard include/config/block.h) \
    $(wildcard include/config/fs/xip.h) \
    $(wildcard include/config/migration.h) \
  include/linux/limits.h \
  include/linux/ioctl.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/ioctl.h \
  include/asm-generic/ioctl.h \
  include/linux/kdev_t.h \
  include/linux/dcache.h \
  include/linux/rculist.h \
  include/linux/path.h \
  include/linux/radix-tree.h \
  include/linux/prio_tree.h \
  include/linux/pid.h \
  include/linux/capability.h \
    $(wildcard include/config/security/file/capabilities.h) \
  include/linux/fiemap.h \
  include/linux/quota.h \
  include/linux/dqblk_xfs.h \
  include/linux/dqblk_v1.h \
  include/linux/dqblk_v2.h \
  include/linux/dqblk_qtree.h \
  include/linux/nfs_fs_i.h \
  include/linux/nfs.h \
  include/linux/sunrpc/msg_prot.h \
  include/linux/fcntl.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/fcntl.h \
  include/asm-generic/fcntl.h \
  include/linux/err.h \
  include/linux/proc_fs.h \
    $(wildcard include/config/proc/fs.h) \
    $(wildcard include/config/proc/devicetree.h) \
    $(wildcard include/config/proc/kcore.h) \
  include/linux/magic.h \
  include/linux/aio.h \
    $(wildcard include/config/aio.h) \
  include/linux/aio_abi.h \
  include/linux/uio.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/uaccess.h \
  include/linux/delay.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/delay.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/io.h \
  arch/arm/plat-mxc/include/mach/io.h \
  include/linux/irq.h \
    $(wildcard include/config/s390.h) \
    $(wildcard include/config/irq/per/cpu.h) \
    $(wildcard include/config/irq/release/method.h) \
    $(wildcard include/config/intr/remap.h) \
    $(wildcard include/config/generic/pending/irq.h) \
    $(wildcard include/config/sparse/irq.h) \
    $(wildcard include/config/numa/irq/desc.h) \
    $(wildcard include/config/generic/hardirqs/no//do/irq.h) \
    $(wildcard include/config/cpumasks/offstack.h) \
  include/linux/irqreturn.h \
  include/linux/irqnr.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/irq.h \
  arch/arm/plat-mxc/include/mach/irqs.h \
    $(wildcard include/config/mxc/pseudo/irqs.h) \
    $(wildcard include/config/mx3/ipu/irqs.h) \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/irq_regs.h \
  include/asm-generic/irq_regs.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/hw_irq.h \
  include/linux/interrupt.h \
    $(wildcard include/config/generic/irq/probe.h) \
    $(wildcard include/config/debug/shirq.h) \
  include/linux/hardirq.h \
    $(wildcard include/config/virt/cpu/accounting.h) \
  include/linux/smp_lock.h \
    $(wildcard include/config/lock/kernel.h) \
  include/linux/sched.h \
    $(wildcard include/config/sched/debug.h) \
    $(wildcard include/config/detect/softlockup.h) \
    $(wildcard include/config/detect/hung/task.h) \
    $(wildcard include/config/core/dump/default/elf/headers.h) \
    $(wildcard include/config/bsd/process/acct.h) \
    $(wildcard include/config/taskstats.h) \
    $(wildcard include/config/audit.h) \
    $(wildcard include/config/inotify/user.h) \
    $(wildcard include/config/posix/mqueue.h) \
    $(wildcard include/config/keys.h) \
    $(wildcard include/config/user/sched.h) \
    $(wildcard include/config/perf/counters.h) \
    $(wildcard include/config/schedstats.h) \
    $(wildcard include/config/task/delay/acct.h) \
    $(wildcard include/config/fair/group/sched.h) \
    $(wildcard include/config/rt/group/sched.h) \
    $(wildcard include/config/blk/dev/io/trace.h) \
    $(wildcard include/config/sysvipc.h) \
    $(wildcard include/config/rt/mutexes.h) \
    $(wildcard include/config/task/xacct.h) \
    $(wildcard include/config/cpusets.h) \
    $(wildcard include/config/cgroups.h) \
    $(wildcard include/config/futex.h) \
    $(wildcard include/config/fault/injection.h) \
    $(wildcard include/config/latencytop.h) \
    $(wildcard include/config/function/graph/tracer.h) \
    $(wildcard include/config/have/unstable/sched/clock.h) \
    $(wildcard include/config/debug/stack/usage.h) \
    $(wildcard include/config/preempt/bkl.h) \
    $(wildcard include/config/group/sched.h) \
    $(wildcard include/config/mm/owner.h) \
  include/linux/rbtree.h \
  include/linux/mm_types.h \
    $(wildcard include/config/split/ptlock/cpus.h) \
    $(wildcard include/config/want/page/debug/flags.h) \
    $(wildcard include/config/mmu/notifier.h) \
  include/linux/auxvec.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/auxvec.h \
  include/linux/page-debug-flags.h \
    $(wildcard include/config/page/poisoning.h) \
    $(wildcard include/config/page/debug/something/else.h) \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/mmu.h \
    $(wildcard include/config/cpu/has/asid.h) \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/cputime.h \
  include/asm-generic/cputime.h \
  include/linux/sem.h \
  include/linux/ipc.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/ipcbuf.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/sembuf.h \
  include/linux/signal.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/signal.h \
  include/asm-generic/signal-defs.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/sigcontext.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/siginfo.h \
  include/asm-generic/siginfo.h \
  include/linux/proportions.h \
  include/linux/percpu_counter.h \
  include/linux/seccomp.h \
    $(wildcard include/config/seccomp.h) \
  include/linux/rtmutex.h \
    $(wildcard include/config/debug/rt/mutexes.h) \
  include/linux/plist.h \
    $(wildcard include/config/debug/pi/list.h) \
  include/linux/resource.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/resource.h \
  include/asm-generic/resource.h \
  include/linux/hrtimer.h \
    $(wildcard include/config/high/res/timers.h) \
  include/linux/task_io_accounting.h \
    $(wildcard include/config/task/io/accounting.h) \
  include/linux/latencytop.h \
  include/linux/cred.h \
  include/linux/key.h \
    $(wildcard include/config/sysctl.h) \
  include/linux/sysctl.h \
  include/linux/ftrace_irq.h \
    $(wildcard include/config/ftrace/nmi/enter.h) \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/hardirq.h \
  include/linux/irq_cpustat.h \
  include/linux/cdev.h \
  include/linux/mm.h \
    $(wildcard include/config/stack/growsup.h) \
    $(wildcard include/config/swap.h) \
    $(wildcard include/config/shmem.h) \
    $(wildcard include/config/debug/pagealloc.h) \
    $(wildcard include/config/hibernation.h) \
  include/linux/debug_locks.h \
    $(wildcard include/config/debug/locking/api/selftests.h) \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/pgtable.h \
  include/asm-generic/4level-fixup.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/proc-fns.h \
    $(wildcard include/config/cpu/32.h) \
    $(wildcard include/config/cpu/arm7tdmi.h) \
    $(wildcard include/config/cpu/arm720t.h) \
    $(wildcard include/config/cpu/arm740t.h) \
    $(wildcard include/config/cpu/arm9tdmi.h) \
    $(wildcard include/config/cpu/arm920t.h) \
    $(wildcard include/config/cpu/arm922t.h) \
    $(wildcard include/config/cpu/arm925t.h) \
    $(wildcard include/config/cpu/arm926t.h) \
    $(wildcard include/config/cpu/arm940t.h) \
    $(wildcard include/config/cpu/arm946e.h) \
    $(wildcard include/config/cpu/arm1020.h) \
    $(wildcard include/config/cpu/arm1020e.h) \
    $(wildcard include/config/cpu/arm1022.h) \
    $(wildcard include/config/cpu/arm1026.h) \
    $(wildcard include/config/cpu/mohawk.h) \
    $(wildcard include/config/cpu/feroceon.h) \
    $(wildcard include/config/cpu/v6.h) \
    $(wildcard include/config/cpu/v7.h) \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/cpu-single.h \
  arch/arm/plat-mxc/include/mach/vmalloc.h \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/pgtable-hwdef.h \
  include/asm-generic/pgtable.h \
  include/linux/page-flags.h \
    $(wildcard include/config/pageflags/extended.h) \
    $(wildcard include/config/have/mlocked/page/bit.h) \
    $(wildcard include/config/ia64/uncached/allocator.h) \
  include/linux/vmstat.h \
    $(wildcard include/config/vm/event/counters.h) \
  include/linux/io.h \
    $(wildcard include/config/has/ioport.h) \
  include/linux/gpio.h \
    $(wildcard include/config/generic/gpio.h) \
  /home/root/ps100N/src/ps100NKern/arch/arm/include/asm/gpio.h \
  arch/arm/plat-mxc/include/mach/gpio.h \
  arch/arm/plat-mxc/include/mach/hardware.h \
  arch/arm/plat-mxc/include/mach/mx25.h \
    $(wildcard include/config/sdma/iram.h) \
    $(wildcard include/config/sdma/iram/size.h) \
    $(wildcard include/config/snd/mxc/soc/iram.h) \
    $(wildcard include/config/usb/static/iram/pph.h) \
  arch/arm/plat-mxc/include/mach/mxc.h \
    $(wildcard include/config/arch/mx31.h) \
  include/asm-generic/gpio.h \
    $(wildcard include/config/gpiolib.h) \
    $(wildcard include/config/gpio/sysfs.h) \
    $(wildcard include/config/have/gpio/lib.h) \
  /home/root/ps100N/src/driver100N/mag/mx25_pins.h \
  /home/root/ps100N/src/driver100N/mag/osdriver.h \
  include/linux/kthread.h \
  include/linux/platform_device.h \
  include/linux/mod_devicetable.h \
  /home/root/ps100N/src/driver100N/mag/iomux.h \

/home/root/ps100N/src/driver100N/mag/trimag_spi.o: $(deps_/home/root/ps100N/src/driver100N/mag/trimag_spi.o)

$(deps_/home/root/ps100N/src/driver100N/mag/trimag_spi.o):
