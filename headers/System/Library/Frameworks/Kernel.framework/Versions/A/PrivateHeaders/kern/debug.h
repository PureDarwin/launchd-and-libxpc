/*
 * Copyright (c) 2000-2007 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 * 
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#ifndef	_KERN_DEBUG_H_
#define _KERN_DEBUG_H_

#include <sys/cdefs.h>
#include <stdint.h>

#ifdef __APPLE_API_PRIVATE
#ifdef __APPLE_API_UNSTABLE

struct thread_snapshot {
	uint32_t 		snapshot_magic;
	uint32_t 		nkern_frames;
	uint32_t 		nuser_frames;
	uint64_t 		wait_event;
	uint64_t 	 	continuation;
	uint64_t 		thread_id;
	uint64_t 		user_time;
	uint64_t 		system_time;
	int32_t  		state;
	char			ss_flags;
} __attribute__ ((packed));

struct task_snapshot {
	uint32_t		snapshot_magic;
	int32_t			pid;
	uint32_t		nloadinfos;
	uint64_t		user_time_in_terminated_threads;
	uint64_t		system_time_in_terminated_threads;
	int				suspend_count; 
	int				task_size;    // pages
	int				faults;	 	// number of page faults
	int				pageins; 	// number of actual pageins
	int				cow_faults;	// number of copy-on-write faults
	char			ss_flags;
	/* We restrict ourselves to a statically defined
	 * (current as of 2009) length for the
	 * p_comm string, due to scoping issues (osfmk/bsd and user/kernel
	 * binary compatibility).
	 */
	char			p_comm[17];
} __attribute__ ((packed));


struct mem_snapshot {
	uint32_t	snapshot_magic;
	uint32_t	free_pages;
	uint32_t	active_pages;
	uint32_t	inactive_pages;
	uint32_t	purgeable_pages;
	uint32_t	wired_pages;
	uint32_t	speculative_pages;
	uint32_t	throttled_pages;
} __attribute__((packed));

enum {
	kUser64_p = 0x1,
	kKernel64_p = 0x2,
	kHasDispatchSerial = 0x4,
	kTerminatedSnapshot = 0x8
};

enum {
	STACKSHOT_GET_DQ = 0x1,
	STACKSHOT_SAVE_LOADINFO = 0x2,
	STACKSHOT_GET_GLOBAL_MEM_STATS = 0x4
};

#define STACKSHOT_THREAD_SNAPSHOT_MAGIC 0xfeedface
#define STACKSHOT_TASK_SNAPSHOT_MAGIC 0xdecafbad
#define STACKSHOT_MEM_SNAPSHOT_MAGIC  0xabcddcba

#endif /* __APPLE_API_UNSTABLE */
#endif /* __APPLE_API_PRIVATE */


extern unsigned int	systemLogDiags;
extern char debug_buf[];
extern unsigned int	debug_boot_arg;
extern char kernel_uuid[];


#define DB_HALT		0x1
#define DB_PRT		0x2
#define DB_NMI		0x4
#define DB_KPRT		0x8
#define DB_KDB		0x10
#define DB_SLOG		0x20
#define DB_ARP          0x40
#define DB_KDP_BP_DIS   0x80
#define DB_LOG_PI_SCRN	0x100
#define DB_KDP_GETC_ENA 0x200

#define DB_KERN_DUMP_ON_PANIC		0x400 /* Trigger core dump on panic*/
#define DB_KERN_DUMP_ON_NMI		0x800 /* Trigger core dump on NMI */
#define DB_DBG_POST_CORE		0x1000 /*Wait in debugger after NMI core */
#define DB_PANICLOG_DUMP		0x2000 /* Send paniclog on panic,not core*/
#define DB_REBOOT_POST_CORE		0x4000 /* Attempt to reboot after
						* post-panic crashdump/paniclog
						* dump.
						*/

#if DEBUG
/*
 * For the DEBUG kernel, support the following:
 *	sysctl -w debug.kprint_syscall=<syscall_mask> 
 *	sysctl -w debug.kprint_syscall_process=<p_comm>
 * <syscall_mask> should be an OR of the masks below
 * for UNIX, MACH, MDEP, or IPC. This debugging aid
 * assumes the task/process is locked/wired and will
 * not go away during evaluation. If no process is
 * specified, all processes will be traced
 */
extern int debug_kprint_syscall;
extern int debug_kprint_current_process(const char **namep);
#define DEBUG_KPRINT_SYSCALL_PREDICATE_INTERNAL(mask, namep)			\
	( (debug_kprint_syscall & (mask)) && debug_kprint_current_process(namep) )
#define DEBUG_KPRINT_SYSCALL_MASK(mask, fmt, args...)	do { 			\
		const char *dks_name = NULL;									\
		if (DEBUG_KPRINT_SYSCALL_PREDICATE_INTERNAL(mask, &dks_name)) {	\
			kprintf("[%s%s%p]" fmt, dks_name ? dks_name : "",			\
					dks_name ? "@" : "", current_thread(), args);			\
		}																\
	} while (0)
#else /* !DEBUG */
#define DEBUG_KPRINT_SYSCALL_PREDICATE_INTERNAL(mask, namep) (0)
#define DEBUG_KPRINT_SYSCALL_MASK(mask, fmt, args...) do { } while(0)
#endif /* !DEBUG */

enum {
	DEBUG_KPRINT_SYSCALL_UNIX_MASK = 1 << 0,
	DEBUG_KPRINT_SYSCALL_MACH_MASK = 1 << 1,
	DEBUG_KPRINT_SYSCALL_MDEP_MASK = 1 << 2,
	DEBUG_KPRINT_SYSCALL_IPC_MASK  = 1 << 3
};

#define DEBUG_KPRINT_SYSCALL_PREDICATE(mask)				\
	DEBUG_KPRINT_SYSCALL_PREDICATE_INTERNAL(mask, NULL)
#define DEBUG_KPRINT_SYSCALL_UNIX(fmt, args...)				\
	DEBUG_KPRINT_SYSCALL_MASK(DEBUG_KPRINT_SYSCALL_UNIX_MASK,fmt,args)
#define DEBUG_KPRINT_SYSCALL_MACH(fmt, args...)				\
	DEBUG_KPRINT_SYSCALL_MASK(DEBUG_KPRINT_SYSCALL_MACH_MASK,fmt,args)
#define DEBUG_KPRINT_SYSCALL_MDEP(fmt, args...)				\
	DEBUG_KPRINT_SYSCALL_MASK(DEBUG_KPRINT_SYSCALL_MDEP_MASK,fmt,args)
#define DEBUG_KPRINT_SYSCALL_IPC(fmt, args...)				\
	DEBUG_KPRINT_SYSCALL_MASK(DEBUG_KPRINT_SYSCALL_IPC_MASK,fmt,args)


__BEGIN_DECLS

extern void panic(const char *string, ...) __printflike(1,2);

#if KERNEL_PRIVATE
void _consume_panic_args(int, ...);
#endif

#if CONFIG_NO_PANIC_STRINGS
#if KERNEL_PRIVATE
#define panic_plain(x, ...) _consume_panic_args( 0, ## __VA_ARGS__ )
#define panic(x, ...) _consume_panic_args( 0, ## __VA_ARGS__ )
#else
#define panic_plain(...) (panic)((char *)0)
#define panic(...)  (panic)((char *)0)
#endif
#else /* CONFIGS_NO_PANIC_STRINGS */
#define panic_plain(ex, ...) \
	(panic)(ex, ## __VA_ARGS__)
#define __STRINGIFY(x) #x
#define LINE_NUMBER(x) __STRINGIFY(x)
#define PANIC_LOCATION __FILE__ ":" LINE_NUMBER(__LINE__)
#define panic(ex, ...) \
	(panic)(# ex "@" PANIC_LOCATION, ## __VA_ARGS__)
#endif /* CONFIGS_NO_PANIC_STRINGS */

void 		populate_model_name(char *);
unsigned	panic_active(void);
__END_DECLS

#endif	/* _KERN_DEBUG_H_ */
