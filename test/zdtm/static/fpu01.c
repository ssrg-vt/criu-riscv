#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>

#include "zdtmtst.h"

#if defined(__i386__) || defined(__x86_64__)

#include "cpuid.h"

const char *test_doc = "Test if FPU data in YMM registers do survive the c/r";
const char *test_author = "Cyrill Gorcunov <gorcunov@openvz.org>";

static int verify_cpu(void)
{
	unsigned int eax, ebx, ecx, edx;

	/* Do we have xsave? */
	cpuid(1, &eax, &ebx, &ecx, &edx);
	if (!(ecx & (1u << 27)))
		return -1;

	/* Is YMM here? */
	cpuid_count(0xd, 0, &eax, &ebx, &ecx, &edx);
	if ((eax & (0x4)) != 0x4)
		return -1;

	return 0;
}

#define __aligned __attribute__((aligned(64)))

/*
 * These are random strings generated by pwgen.
 */
static __aligned unsigned char ymm1[32 + 1] = "code9Ee5sohphie1ae1kaeMahngoh5oe";
static __aligned unsigned char ymm2[32 + 1] = "Tacuthahhien9Fi7aGhaa5toGh6vi7Ch";

static __aligned unsigned char ymm3[32 + 1];
static __aligned unsigned char ymm4[32 + 1];

static int fpu_test(void)
{
	int ret = 0;

	asm volatile("vmovapd %0, %%ymm0 \n" : : "m"(*ymm1) : "memory");

	asm volatile("vmovapd %0, %%ymm7 \n" : : "m"(*ymm2) : "memory");

	test_daemon();
	test_waitsig();

	asm volatile("vmovapd %%ymm0, %0 \n" : "=m"(*ymm3) : : "memory");

	asm volatile("vmovapd %%ymm7, %0 \n" : "=m"(*ymm4) : : "memory");

	if (memcmp(ymm1, ymm3, 32) || memcmp(ymm2, ymm4, 32)) {
		test_msg("Data mismatch ('%s' '%s' '%s' '%s')\n", ymm1, ymm2, ymm3, ymm4);
		ret = -1;
	} else {
		test_msg("Data match ('%s' '%s' '%s' '%s')\n", ymm1, ymm2, ymm3, ymm4);
		ret = 0;
	}

	return ret;
}

static int bare_run(void)
{
	test_msg("Your cpu doesn't support ymm registers, skipping\n");

	test_daemon();
	test_waitsig();

	return 0;
}

int main(int argc, char *argv[])
{
	int ret = 0;

	test_init(argc, argv);

	ret = verify_cpu() ? bare_run() : fpu_test();

	if (!ret)
		pass();
	else
		fail();

	return 0;
}

#else

int main(int argc, char *argv[])
{
	test_init(argc, argv);
	skip("Unsupported arch");
	return 0;
}

#endif
