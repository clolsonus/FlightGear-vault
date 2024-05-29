#if defined(mips) && !defined(ultrix) && !defined(sgi)

#include </usr/include/sys/types.h>
#include </usr/include/sys/param.h>
#include </usr/include/sys/grafreg.h>
#include </usr/include/sys/ipc.h>
#include </usr/include/sys/shm.h>

static unsigned long *mips_buf_word_addr, mips_pixel[256];
static unsigned long mips_amask[4] =
	{ 0x00000000, 0xFF000000, 0xFFFF0000, 0xFFFFFF00 };
static unsigned long mips_bmask[4] =
	{ 0xFFFFFFFF, 0x00FFFFFF, 0x0000FFFF, 0x000000FF };
static int mips_addval[4] =
	{ -4, -3, -2, -1 };

static int x_origin = 0, y_origin = 0;

void
FrameBufferIOInit ()
{

	register unsigned long value;
	int	buf_id, reg_id;
	char	*buf_addr, *reg_addr;
	register int i, j, k;
	extern char *shmat();

	value = 0;

	for (i=0; i<256; ++i) {
		mips_pixel[i] = (i << 24) | (i << 16) | (i << 8) | i;
	}

	buf_id = shmget(GBFCKEY, GRAPHICS_FRAME_SIZE, 0666);
	reg_id = shmget(GREGKEY, GRAPHICS_REG_SIZE, 0666);

	buf_addr = shmat(buf_id, GRAPHICS_FRAME_ADDR, 0);
	reg_addr = shmat(reg_id, GRAPHICS_REG_ADDR, 0);

	mips_buf_word_addr = (unsigned long *) buf_addr;
}

void FrameBufferSetOrigin (x, y)
int	x, y;
{

	x_origin = x;
	y_origin = y;

}

void
FrameBufferRun (x, y, length, pixel)
register int	x, y, length, pixel;
{

	register unsigned long value = mips_pixel[pixel], i, j;
	register unsigned long *p, *pend;

	x += x_origin;
	y += y_origin;

	p = mips_buf_word_addr + (y * 512 + (x >> 2));

	i = x % 4;

	if (i + length < 4) {
		j = (*p & mips_amask[i]) | (value & mips_bmask[i]);
		i += length;
		*p = (*p & mips_bmask[i]) | (j & mips_amask[i]);
		return;
	}

	if (i != 0) {
		*p = (*p & mips_amask[i]) | (value & mips_bmask[i]);
		length += mips_addval[i];
		++ p;
	}

	pend = p + (length >> 2);
	for ( ; p < pend; ) {
		*p++ = value;
	}

	if ((i = length % 4) != 0) {
		*p = (*p & mips_bmask[i]) | (value & mips_amask[i]);
	}

}

#endif /* defined(mips) ... */
