#include <wnos/kdbgio.h>
#include <arch/asm.h>

#define va_start(v,l) __builtin_va_start(v,l)
#define va_arg(v,l)   __builtin_va_arg(v,l)
#define va_end(v)     __builtin_va_end(v)
#define va_copy(d,s)  __builtin_va_copy(d,s)
#define va_list		  __builtin_va_list

volatile char *screen_ptr = (char*)0xB8000;
char chbuff[32];
char tbuf[32];
char hexchars[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

void itoa(unsigned long i, int base, char *buffer)
{
	int pos = 0;
	int opos = 0;
	int top = 0;

	if (i == 0 || base > 16)
	{
		buffer[0] = '0';
		buffer[1] = '\0';
		return;
	}
	while (i != 0)
	{
		tbuf[pos] = hexchars[i % base];
		pos++;
		i /= base;
	}
	top=pos--;
	for (opos = 0; opos < top; pos--, opos++)
		buffer[opos] = tbuf[pos];
		
	buffer[opos] = 0;
}

void itoa_s(long i, int base, char* buf)
{
	if (base > 16)
		return;
	if (i < 0)
	{
		*buf++ = '-';
		i *= -1;
	}
	itoa(i,base,buf);
}

inline void kputc(char c)
{
	*screen_ptr++ = c;
	*screen_ptr++ = 0x7;
}

int kprintf (const char *format, ...)
{
	unsigned long addr, ul;
	int i = 0, si;
	unsigned int ui;
	char ch;
	va_list pfargs;

	va_start(pfargs, format);
	
	while (format[i])
	{
		switch (format[i])
		{
		case '%':
			switch (format[i + 1])
			{
			case 's':
				kprintf((char*)va_arg(pfargs, char*));
				break;
			case 'c':
				ch = (char)va_arg(pfargs, int);
				kputc(ch);
				break;
			case 'd':
			case 'i':
				si = va_arg(pfargs, int);
				itoa_s((long)si, 10, chbuff);
				kprintf(chbuff);
				break;
			case 'u':
				ui = va_arg(pfargs, unsigned int);
				itoa((unsigned long)ui, 10, chbuff);
				kprintf(chbuff);
				break;
			case 'p':
			case 'x':
			case 'X':
				ul = va_arg(pfargs, unsigned long);
				itoa(ul, 16, chbuff);
				kprintf("0x");
				kprintf(chbuff);
				break;
			case '%':
				kputc('%');
				break;
			}
			i += 2;
			break;
			
		case '\n':
			addr = (unsigned long)screen_ptr;
			if ((addr - 0xB8000) % 160 == 0)
				addr += 160;
			else
				addr += 160 - ((addr - 0xB8000) % 160);
			screen_ptr = (char*)addr;
			i++;
			break;
			
		default:
			kputc(format[i]);
			i++;
			break;
		}
	}
	
	va_end(pfargs);
	return 0;
}

void kprintf_cls()
{
	int i;
	
	screen_ptr = (char*)0xB8000;
	for (i = 0; i < 80 * 25 * 2; i ++)
		screen_ptr[i] = 0;
}

