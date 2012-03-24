
#define MP_POINTER_SIG 0x5F504D5F

typedef struct __attribute__ ((__packed__)) mp_pointer
{
	unsigned int Signature;
	unsigned int PhysicalAddr;
	unsigned char Length;
	unsigned char Revision;
	unsigned char Checksum;
	unsigned char Features[5];
} mp_pointer_t;

#define MP_HEADER_SIG 0x504D4350

typedef struct __attribute__ ((__packed__)) mp_header
{
	unsigned int Signature;
	unsigned short BaseTableLength;
	unsigned char Revision;
	unsigned char Checksum;
	unsigned char OemID[8];
	unsigned char ProductID[12];
	unsigned int OemTablePointer;
	unsigned short OemTableSize;
	unsigned short EntryCount;
	unsigned int LocalApicAddr;
	unsigned short ExtTableLength;
	unsigned char ExtTableChecksum;
	unsigned char Reserved;
} mp_header_t;

#define MPENTRY_PROCESSOR 0
#define MPENTRY_BUS 1
#define MPENTRY_IOAPIC 2
#define MPENTRY_IOINTASSIGN 3 /* I/O APIC Interrupt Assignment */
#define MPENTRY_LOCINTASSIGN 4 /* Local Interrupt Assignment */

typedef struct __attribute__ ((__packed__)) mpentry_processor
{
	unsigned char EntryType;
	unsigned char LocalApicID;
	unsigned char LocalApicVersion;
	unsigned char CpuFlags;
	unsigned int CpuSignature;
	unsigned int FeatureFlags;
	unsigned int Reserved[2];
} mpentry_processor_t;

typedef struct __attribute__ ((__packed__)) mpentry_bus
{
	unsigned char EntryType;
	unsigned char BusID;
	char TypeString[6];
} mpentry_bus_t;

typedef struct __attribute__ ((__packed__)) mpentry_ioapic
{
	unsigned char EntryType;
	unsigned char IoApicID;
	unsigned char IoApicVersion;
	unsigned char IoApicFlags;
	unsigned int PhysicalAddr;
} mpentry_ioapic_t;

#define INTTYPE_INT 0 /* Vectored Interrupt */
#define INTTYPE_NMI 1 /* Non Maskable Interrupt */
#define INTTYPE_SMI 2 /* System Management Interrupt */
#define INTTYPE_EXTINT 3 /* Vectored Interrupt supplied by external PIC */

typedef struct __attribute__ ((__packed__)) mpentry_ioint
{
	unsigned char EntryType;
	unsigned char InterruptType;
	unsigned short IoIntFlags;
	unsigned char BusID;
	unsigned char BusIrq;
	unsigned char IoApicID;
	unsigned char IoApicIntin;
} mpentry_ioint_t;

typedef struct __attribute__ ((__packed__)) mpentry_localint
{
	unsigned char EntryType;
	unsigned char InterruptType;
	unsigned short LocalIntFlags;
	unsigned char BusID;
	unsigned char BusIrq;
	unsigned char DestLocalApic;
	unsigned char DestLocalApicLintin;
} mpentry_localint_t;

