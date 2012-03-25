

extern void set_cpuid();
extern unsigned char cpuid_vendor_id[13];
extern unsigned char cpuid_apic;

static
inline void enable_interrupts()
{
	__asm__ __volatile__("sti": : :"memory");
}

static
inline void disable_interrupts()
{
	__asm__ __volatile__("cli": : :"memory");
}

static
inline void halt_cpu()
{
	__asm__ __volatile__("hlt": : :);
}

static
inline unsigned char inb(unsigned short port)
{
	unsigned char ret;
	__asm__ __volatile__("inb %1, %0"
						:"=a"(ret)
						:"Nd"(port)
						);
	return ret;
}

static
inline unsigned short inw(unsigned short port)
{
	unsigned short ret;
	__asm__ __volatile__("inw %1, %0"
						:"=a"(ret)
						:"Nd"(port)
						);
	return ret;
}

static
inline void outb(unsigned short port, unsigned char value)
{
	__asm__ __volatile__("outb %0, %1"
						:
						:"a"(value), "Nd"(port)
						);
}

static
inline void outw(unsigned short port, unsigned short value)
{
	__asm__ __volatile__("outw %0, %1"
						:
						:"a"(value), "Nd"(port)
						);
}

static
inline unsigned long read_msr(unsigned int msr)
{
	unsigned long value;
	__asm__ __volatile__ ("rdmsr"
						 :"=A" (value)
						 :"c" (msr)
						 );
	return value;
}

static
inline void write_msr(unsigned int msr, unsigned long value)
{
	__asm__ __volatile__ ("wrmsr"
						 :
						 :"c" (msr), "A" (value)
						 );
}

static
inline void invlpg(void *address)
{
	char *ptr = (char*)address; /* Make GCC happy */
	__asm__ __volatile__("invlpg %0"
						:
						:"m" (*ptr)
						);
}

static
inline void write_cr3(unsigned long value)
{
	__asm__ __volatile__("mov %0, %%cr3"
						:
						:"r"(value)
						);
}