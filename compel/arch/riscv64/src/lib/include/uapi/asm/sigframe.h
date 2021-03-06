#ifndef UAPI_COMPEL_ASM_SIGFRAME_H__
#define UAPI_COMPEL_ASM_SIGFRAME_H__


#include <sys/ucontext.h>

#include <stdint.h>

#include <signal.h>

/* Copied from the kernel header arch/arm64/include/uapi/asm/sigcontext.h */

#define FPSIMD_MAGIC 0x46508001

// typedef struct user_regs_struct __riscv_fp_state;

/* took it from  /usr/include/riscv64-linux-gnu/asm/sigcontext.h */
//commenting cause throws error of dedup
// struct sigcontext {
//         struct user_regs_struct sc_regs;
//         union __riscv_fp_state sc_fpregs;
// };

// XXX: the idetifier rt_sigcontext is expected to be struct by the CRIU code
#define rt_sigcontext sigcontext

#include <compel/sigframe-common.h>

/* Copied from the kernel source arch/riscv/kernel/signal.c */

struct rt_sigframe {
	siginfo_t info;
	ucontext_t uc;
	uint64_t fp;
	uint64_t lr;
};

/* clang-format off */
#define ARCH_RT_SIGRETURN(new_sp, rt_sigframe)					\
	asm volatile(								\
			"mv sp, %0					\n"	\
			"li fp, 139	\n"	\
			"ecall #0						\n"	\
			:							\
			: "r"(new_sp)						\
			: "s0", "memory")
/* clang-format on */

/* cr_sigcontext is copied from arch/arm64/include/uapi/asm/sigcontext.h */
struct cr_sigcontext {
	__u64 fault_address;
	/* riscv64 registers */
	__u64 regs[31];
	__u64 sp;
	__u64 pc;
	__u64 pstate;
	/* 4K reserved for FP/SIMD state and future expansion */
	__u8 __reserved[4096] __attribute__((__aligned__(16)));
};

#define RT_SIGFRAME_UC(rt_sigframe)	     (&rt_sigframe->uc)
#define RT_SIGFRAME_REGIP(rt_sigframe)	     ((long unsigned int)(rt_sigframe)->uc.uc_mcontext.__gregs[0])
#define RT_SIGFRAME_HAS_FPU(rt_sigframe)     (1)
#define RT_SIGFRAME_SIGCONTEXT(rt_sigframe)  ((struct cr_sigcontext *)&(rt_sigframe)->uc.uc_mcontext)
#define RT_SIGFRAME_AUX_CONTEXT(rt_sigframe) ((struct sigcontext *)&(RT_SIGFRAME_SIGCONTEXT(rt_sigframe)->__reserved))
#define RT_SIGFRAME_FPU(rt_sigframe)	     (&RT_SIGFRAME_AUX_CONTEXT(rt_sigframe)->fpsimd)
#define RT_SIGFRAME_OFFSET(rt_sigframe)	     0

#define rt_sigframe_erase_sigset(sigframe)	memset(&sigframe->uc.uc_sigmask, 0, sizeof(k_rtsigset_t))
#define rt_sigframe_copy_sigset(sigframe, from) memcpy(&sigframe->uc.uc_sigmask, from, sizeof(k_rtsigset_t))

#endif /* UAPI_COMPEL_ASM_SIGFRAME_H__ */
