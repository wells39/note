#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>
#include <termios.h>
#include <sys/types.h>
#include <stdbool.h>

#define __USE_FILE_OFFSET64
#define __USE_LARGEFILE64
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", \
  __LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)
 
void show_usage(char *comm)
{
	fprintf(stderr, "Usage:\t%s -f file { address length } [ [ data ] type ]\n"
		"\tfile    : file to check memory\n"
		"\t          /dev/mem  -- for physics memory address examine\n"
		"\t          /dev/kmem -- for virtual kernel memory address examine\n"
		"\taddress : memory address to act upon\n"
		"\tlength  : memory address length to act upon\n"
		"\tdata    : data to be written\n"
		"\ttype    : access operation type : [b]yte, [h]alfword, [w]ord, [g]giant\n",
		comm);
	exit(1);
}

#define hex_asc_hi(x)        hex_asc[((x) & 0xf0) >> 4]
#define hex_asc_lo(x)        hex_asc[((x) & 0x0f)]
#define min(x, y) ({                \
    typeof(x) _min1 = (x);          \
    typeof(y) _min2 = (y);          \
    (void) (&_min1 == &_min2);      \
    _min1 < _min2 ? _min1 : _min2; })
static bool is_power_of_2(unsigned long n)
{
    return (n != 0 && ((n & (n - 1)) == 0));
}
static inline u_int16_t __get_unaligned_le16(const u_int8_t *p)
{
    return p[0] | p[1] << 8;
}
static inline u_int32_t __get_unaligned_le32(const u_int8_t *p)
{
    return p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;
}
static inline u_int64_t __get_unaligned_le64(const u_int8_t *p)
{
    return (u_int64_t)__get_unaligned_le32(p + 4) << 32 |
           __get_unaligned_le32(p);
}
static inline u_int16_t __get_unaligned_be16(const u_int8_t *p)
{
    return p[0] << 8 | p[1];
}
static inline u_int32_t __get_unaligned_be32(const u_int8_t *p)
{
    return p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
}
static inline u_int64_t __get_unaligned_be64(const u_int8_t *p)
{
    return (u_int64_t)__get_unaligned_be32(p) << 32 |
           __get_unaligned_be32(p + 4);
}
static inline u_int16_t get_unaligned_be16(const void *p)
{
    return __get_unaligned_be16((const u_int8_t *)p);
}
static inline u_int32_t get_unaligned_be32(const void *p)
{
    return __get_unaligned_be32((const u_int8_t *)p);
}
static inline u_int64_t get_unaligned_be64(const void *p)
{
    return __get_unaligned_be64((const u_int8_t *)p);
}
static inline u_int16_t get_unaligned_le16(const void *p)
{
    return __get_unaligned_le16((const u_int8_t *)p);
}
static inline u_int32_t get_unaligned_le32(const void *p)
{
    return __get_unaligned_le32((const u_int8_t *)p);
}
static inline u_int64_t get_unaligned_le64(const void *p)
{
    return __get_unaligned_le64((const u_int8_t *)p);
}
extern void __bad_unaligned_access_size(void);
#define __get_unaligned_le(ptr) ((typeof(*(ptr)))({         \
    __builtin_choose_expr(sizeof(*(ptr)) == 1, *(ptr),          \
    __builtin_choose_expr(sizeof(*(ptr)) == 2, get_unaligned_le16((ptr)),   \
    __builtin_choose_expr(sizeof(*(ptr)) == 4, get_unaligned_le32((ptr)),   \
    __builtin_choose_expr(sizeof(*(ptr)) == 8, get_unaligned_le64((ptr)),   \
    __bad_unaligned_access_size()))));                  \
    }))

#define __get_unaligned_be(ptr) ((typeof(*(ptr)))({         \
    __builtin_choose_expr(sizeof(*(ptr)) == 1, *(ptr),          \
    __builtin_choose_expr(sizeof(*(ptr)) == 2, get_unaligned_be16((ptr)),   \
    __builtin_choose_expr(sizeof(*(ptr)) == 4, get_unaligned_be32((ptr)),   \
    __builtin_choose_expr(sizeof(*(ptr)) == 8, get_unaligned_be64((ptr)),   \
    __bad_unaligned_access_size()))));                  \
    }))
#if defined(__ARM_BIG_ENDIAN)
# define get_unaligned  __get_unaligned_be
# define put_unaligned  __put_unaligned_be
#else
# define get_unaligned  __get_unaligned_le
# define put_unaligned  __put_unaligned_le
#endif
enum {
    DUMP_PREFIX_NONE,
    DUMP_PREFIX_ADDRESS,
    DUMP_PREFIX_OFFSET
};
const char hex_asc[] = "0123456789abcdef";

int hex_dump_to_buffer(const void *buf, size_t len, int rowsize, int groupsize,
               char *linebuf, size_t linebuflen, bool ascii)
{
    const u_int8_t *ptr = buf;
    int ngroups;
    u_int8_t ch;
    int j, lx = 0;
    int ascii_column;
    int ret;

    if (rowsize != 16 && rowsize != 32)
        rowsize = 16;

    if (len > rowsize)      /* limit to one line at a time */
        len = rowsize;
    if (!is_power_of_2(groupsize) || groupsize > 8)
        groupsize = 1;
    if ((len % groupsize) != 0) /* no mixed size output */
        groupsize = 1;

    ngroups = len / groupsize;
    ascii_column = rowsize * 2 + rowsize / groupsize + 1;

    if (!linebuflen)
        goto overflow1;

    if (!len)
        goto nil;

    if (groupsize == 8) {
        const u_int64_t *ptr8 = buf;

        for (j = 0; j < ngroups; j++) {
            ret = snprintf(linebuf + lx, linebuflen - lx,
                       "%s%16.16llx", j ? " " : "",
                       get_unaligned(ptr8 + j));
            if (ret >= linebuflen - lx)
                goto overflow1;
            lx += ret;
        }
    } else if (groupsize == 4) {
        const u_int32_t *ptr4 = buf;

        for (j = 0; j < ngroups; j++) {
            ret = snprintf(linebuf + lx, linebuflen - lx,
                       "%s%8.8x", j ? " " : "",
                       get_unaligned(ptr4 + j));
            if (ret >= linebuflen - lx)
                goto overflow1;
            lx += ret;
        }
    } else if (groupsize == 2) {
        const u_int16_t *ptr2 = buf;

        for (j = 0; j < ngroups; j++) {
            ret = snprintf(linebuf + lx, linebuflen - lx,
                       "%s%4.4x", j ? " " : "",
                       get_unaligned(ptr2 + j));
            if (ret >= linebuflen - lx)
                goto overflow1;
            lx += ret;
        }
    } else {
        for (j = 0; j < len; j++) {
            if (linebuflen < lx + 2)
                goto overflow2;
            ch = ptr[j];
            linebuf[lx++] = hex_asc_hi(ch);
            if (linebuflen < lx + 2)
                goto overflow2;
            linebuf[lx++] = hex_asc_lo(ch);
            if (linebuflen < lx + 2)
                goto overflow2;
            linebuf[lx++] = ' ';
        }
        if (j)
            lx--;
    }
    if (!ascii)
        goto nil;

    while (lx < ascii_column) {
        if (linebuflen < lx + 2)
            goto overflow2;
        linebuf[lx++] = ' ';
    }
    for (j = 0; j < len; j++) {
        if (linebuflen < lx + 2)
            goto overflow2;
        ch = ptr[j];
        linebuf[lx++] = (isascii(ch) && isprint(ch)) ? ch : '.';
    }
nil:
    linebuf[lx] = '\0';
    return lx;
overflow2:
    linebuf[lx++] = '\0';
overflow1:
    return ascii ? ascii_column + len : (groupsize * 2 + 1) * ngroups - 1;
}

void hex_dump(int prefix_type, int rowsize, int groupsize,
            u_int64_t addr, const void *buf, size_t len, bool ascii)
{
    const u_int8_t *ptr = buf;
    int i, linelen, remaining = len;
    unsigned char linebuf[32 * 3 + 2 + 32 + 1];

    if (rowsize != 16 && rowsize != 32)
        rowsize = 16;

    for (i = 0; i < len; i += rowsize) {
        linelen = min(remaining, rowsize);
        remaining -= rowsize;

        hex_dump_to_buffer(ptr + i, linelen, rowsize, groupsize,
                   linebuf, sizeof(linebuf), ascii);

        switch (prefix_type) {
        case DUMP_PREFIX_ADDRESS:
            printf("%.8llx: %s\n", addr + i, linebuf);
            break;
        case DUMP_PREFIX_OFFSET:
            printf("%.8x: %s\n", i, linebuf);
            break;
        default:
            printf("%s\n", linebuf);
            break;
        }
    }
}

int main(int argc, char **argv) {
    void *map_base, *virt_addr, *file = 0; 
	u_int32_t map_size;
	u_int64_t writeval;
	u_int64_t addr;
	u_int32_t length = 0;
	bool write = false;
	int pg_offset = 0;
    int fd = -1;
	int opt = 0;
	int pos = 0;
	char access_type = 'w';

    while ((opt = getopt(argc, argv, "f:")) != -1) {
		switch (opt) {
        case 'f':
			file = optarg;
            break;
        default:
                show_usage(argv[0]);
        }
    }
	if(!file) {
		fprintf(stderr, "%s: -f option is mandatory!\n", argv[0]);
		show_usage(argv[0]);
	}

	if(argc > optind)
		addr = strtoull(argv[optind++], 0, 0);
	if(argc > optind)
		length = strtoul(argv[optind++], 0, 0);
	if(!length) {
		fprintf(stderr, "%s: address or length is invaild!\n", argv[0]);
		exit(2);
	}

	if(argc > optind) {
		if(argc > optind + 1) {
			writeval = strtoull(argv[optind++], 0, 0);
			write = true;
		}
		access_type = tolower(argv[optind++][0]);
	}
	if((fd = open(file, write? O_RDWR|O_SYNC|O_LARGEFILE : O_RDONLY|O_LARGEFILE)) == -1) FATAL;
    fflush(stdout);

    /*map start addr must align with 4K */
    pg_offset = (int)addr & 0xfff;
    addr = addr & (~0xfffULL);
    map_size = pg_offset + length;
    /* Map one page */
#if (__aarch64__ == 1)
    map_base = mmap(0, map_size, write? PROT_WRITE|PROT_READ : PROT_READ, MAP_SHARED, fd, (off_t)addr);
#else
#define	__USE_LARGEFILE64
    if (addr+length >> 32)
    {
        map_base = mmap64(0, map_size, write? PROT_WRITE|PROT_READ : PROT_READ, MAP_SHARED, fd, addr);
    } else
    {
        map_base = mmap(0, map_size, write? PROT_WRITE|PROT_READ : PROT_READ, MAP_SHARED, fd, addr);
    }
#endif
    if(map_base == (void *) -1) {
		loff_t result;
		virt_addr = malloc(length);
		if(!virt_addr) FATAL;
		if(length > lseek(fd, 0, SEEK_END) - addr )
			length = lseek(fd, 0, SEEK_END) - addr;
		if(lseek(fd, addr, SEEK_SET) < 0) FATAL;
		if(read(fd, virt_addr, length) < 0) FATAL;
	} else {
		printf("Memory mapped at address %p.\n", map_base); 
		fflush(stdout);
		virt_addr = map_base + pg_offset;
	}
    switch(access_type) {
		case 'b':
			hex_dump(DUMP_PREFIX_ADDRESS, 16, 1, addr, virt_addr, length, true);
			break;
		case 'h':
			hex_dump(DUMP_PREFIX_ADDRESS, 16, 2, addr, virt_addr, length, true);
			break;
		case 'w':
			hex_dump(DUMP_PREFIX_ADDRESS, 16, 4, addr, virt_addr, length, true);
			break;
		case 'g':
			hex_dump(DUMP_PREFIX_ADDRESS, 32, 8, addr, virt_addr, length, true);
			break;
		default:
			fprintf(stderr, "Illegal data type '%c'.\n", access_type);
			exit(2);
	}
    fflush(stdout);

	if(write) {
		u_int64_t read_result = 0;
		printf("Write 0x%llx val 0x%llx.\n", addr, writeval);
		switch(access_type) {
			case 'b':
				*((u_int8_t *) virt_addr) = writeval;
				read_result = *((u_int8_t *) virt_addr);
				break;
			case 'h':
				*((u_int16_t *) virt_addr) = writeval;
				read_result = *((u_int16_t *) virt_addr);
				break;
			case 'w':
				*((u_int32_t *) virt_addr) = writeval;
				read_result = *((u_int32_t *) virt_addr);
				break;
			case 'g':
				*((u_int64_t *) virt_addr) = writeval;
				read_result = *((u_int64_t *) virt_addr);
				break;
		}
		printf("Written 0x%llX; readback 0x%llX\n", writeval, read_result); 
		fflush(stdout);
	}
	
	if(map_base != (void *) -1) {
		if(munmap(map_base, length) == -1) FATAL;
	} else {
		free(virt_addr);
	}
    close(fd);
    return 0;
}
