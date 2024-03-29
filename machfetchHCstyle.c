#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <sys/utsname.h>

#define EAX_REG 0
#define EBX_REG 1
#define ECX_REG 2
#define EDX_REG 3

static int *do_cpuid (int cpu_regs[4])
{
  int eax = cpu_regs[EAX_REG], ebx, ecx = cpu_regs[ECX_REG], edx;

  __asm__ ("cpuid"
           /* Dumping all modified registers by the cpuid instruction */
:         "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
           /* After cpuid put the eax interger value into eax register */
:         "0" (eax), "2" (ecx)
           /* save registers before the cpuid instruction */
           // : "%eax", "%ebx", "%ecx", "%edx"
    );

  /* Modifing the cpu_reg list */
  cpu_regs[EAX_REG] = eax;
  cpu_regs[EBX_REG] = ebx;
  cpu_regs[ECX_REG] = ecx;
  cpu_regs[EDX_REG] = edx;

  return cpu_regs;
}

static int arch_id = 0;

enum arch_machine
{
  ARCH_MACH_AMD64
};

int main ()
{
  puts ("\n\tOS information:\n");

  struct utsname osname;

  uname (&osname);

  printf ("System name: %s\n", osname.sysname);
  printf ("Network node name: %s\n", osname.nodename);
  printf ("Operating system release: %s\n", osname.release);
  printf ("Operating system version: %s\n", osname.version);

  puts ("\n\tHardware information:\n");
  puts ("\n\t[CPU]\n");

  char *machine = osname.machine;

  if (strcmp (machine, "x86_64") == 0)
    arch_id = ARCH_MACH_AMD64;

  printf ("Architecture: %s (%d)\n", machine, arch_id);

  uint32_t test_value = 1;

  printf ("Endianness: %s\n", *(char *) &test_value ? "Little-Endian (LE)" : "Big-Endian (BE)");
  printf ("Test bit value: ");
  for (int i = 31; i >= 0; i--)
    if (printf ("\e[36m%d\e[0m", test_value >> i & 1))
      if (i % 8 == 0)
        printf (" ");
  puts ("");

  int cpuid_data[4] = { };

  cpuid_data[EAX_REG] = 0;

  do_cpuid (cpuid_data);

  const int max_cpuid_parameter = cpuid_data[EAX_REG];

  printf ("Max cpuid parameter %d\n", max_cpuid_parameter);

  int cpu_manu_id[sizeof (int) * 3] = { };
  cpu_manu_id[0] = cpuid_data[EBX_REG];
  cpu_manu_id[1] = cpuid_data[EDX_REG];
  cpu_manu_id[2] = cpuid_data[ECX_REG];

  printf ("Manufacturer ID: %12s\n", (const char *) cpu_manu_id);

  puts ("\n\t[CPU] Signature:\n");

  cpuid_data[EAX_REG] = 1;

  do_cpuid (cpuid_data);

  printf ("Stepping ID: 0x%02x (Hardware bugs fixes and erratas)\n", cpuid_data[EAX_REG] & 0x10 /* 2⁴ -> HEXA */ );

  int cpu_brand[12] = { };      /* 12 * 4 = 48 bytes */

  cpuid_data[EAX_REG] = 0x80000002;

  do_cpuid (cpuid_data);

  memcpy (cpu_brand + 0, cpuid_data, sizeof (cpuid_data));

  cpuid_data[EAX_REG] = 0x80000003;

  do_cpuid (cpuid_data);

  memcpy (cpu_brand + 4, cpuid_data, sizeof (cpuid_data));

  cpuid_data[EAX_REG] = 0x80000004;

  do_cpuid (cpuid_data);

  memcpy (cpu_brand + 8, cpuid_data, sizeof (cpuid_data));

  printf ("Brand: %s\n", (const char *) cpu_brand);

  puts ("\n\t[CPU] Features:\n");

  printf ("Features:");

  if (cpuid_data[EDX_REG] & 1)
    printf (" +\e[32mfpu\e[0m Onboard x87 FPU+");
  if ((cpuid_data[EDX_REG] >> 25) & 1)
    printf (" +\e[32msse\e[0m SSE instructions (a.k.a. Katmai New Instructions)+");
  if ((cpuid_data[EDX_REG] >> 26) & 1)
    printf (" +\e[32msse2\e[0m SSE2 instructions+");
  if (cpuid_data[ECX_REG] & 1)
    printf (" +\e[32msse3\e[0m Prescott New Instructions-SSE3 (PNI)+");
  if ((cpuid_data[ECX_REG] >> 19) & 1)
    printf (" +\e[32msse4.1\e[0m SSE4.1 instructions+");
  if ((cpuid_data[ECX_REG] >> 20) & 1)
    printf (" +\e[32msse4.2\e[0m SSE4.2 instructions+");

  puts ("");

  if (max_cpuid_parameter >= 0x16)
  {
    cpuid_data[EAX_REG] = 0x16;

    do_cpuid (cpuid_data);

    printf ("Processor base frequency: %04d Mhz\n", cpuid_data[EAX_REG]);
    printf ("Processor maximum frequency: %04d Mhz\n", cpuid_data[EBX_REG]);
    printf ("Processor Bus frequency: %04d Mhz\n", cpuid_data[ECX_REG]);
  }

  puts ("\n\t[CPU] Thermal and power management:\n");

  cpuid_data[EAX_REG] = 6;

  do_cpuid (cpuid_data);

  printf ("Digital Thermal Sensor: %s\n", cpuid_data[EAX_REG] & 1 ? "available" : "not available");

  puts ("\n\t[CPU] Extended Features:\n");

  cpuid_data[EAX_REG] = 7;
  cpuid_data[ECX_REG] = 0;

  do_cpuid (cpuid_data);

  printf ("Supported features:");
  if ((cpuid_data[EBX_REG] >> 5) & 1)
    printf (" +\e[32mavx2\e[0m Advanced Vector Extensions 2+");
  if ((cpuid_data[EBX_REG] >> 29) & 1)
    printf (" +\e[32msha\e[0m Intel SHA extensions+");

  puts ("");
}
