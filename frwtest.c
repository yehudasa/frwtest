/*

71692 newfstatat(3, "", {st_mode=S_IFREG|0644, st_size=0, ...}, AT_EMPTY_PATH) = 0
71692 lseek(3, 0, SEEK_SET)             = 0
71692 read(3, "", 424)                  = 0
71692 lseek(3, 424, SEEK_CUR)           = 424
71692 write(3, "\0hello.c\0main\0puts\0exit\0", 24) = 24
71692 lseek(3, 0, SEEK_SET)             = 0
71692 read(3, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 8192) = 448
71692 lseek(3, -384, SEEK_CUR)          = 64
71692 write(3, "UH\211\345\277\0\0\0\0\350\0\0\0\0\277\0\0\0\0\350\0\0\0\0hello wo"..., 83) = 83
71692 lseek(3, 0, SEEK_SET)             = 0
71692 read(3, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 8192) = 448
71692 lseek(3, -296, SEEK_CUR)          = 152
71692 write(3, "\4\0\0\0 \0\0\0\5\0\0\0GNU\0\2\0\1\300\4\0\0\0\0\0\0\0\0\0\0\0"..., 104) = 104
71692 lseek(3, 0, SEEK_SET)             = 0
71692 read(3, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 8192) = 448
71692 write(3, "\5\0\0\0\0\0\0\0\n\0\0\0\3\0\0\0\0\0\0\0\0\0\0\0\n\0\0\0\0\0\0\0"..., 96) = 96
71692 lseek(3, 0, SEEK_SET)             = 0
71692 read(3, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 8192) = 544
71692 lseek(3, -288, SEEK_CUR)          = 256
71692 write(3, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\1\0\0\0\4\0\361\377"..., 168) = 168
71692 lseek(3, 0, SEEK_SET)             = 0
71692 read(3, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 8192) = 544
71692 write(3, "\0.symtab\0.strtab\0.shstrtab\0.rela"..., 116) = 116
71692 lseek(3, 0, SEEK_SET)             = 0
71692 write(3, "\177ELF\2\1\1\0\0\0\0\0\0\0\0\0\1\0>\0\1\0\0\0\0\0\0\0\0\0\0\0"..., 64) = 64
71692 lseek(3, 0, SEEK_SET)             = 0
71692 read(3, "\177ELF\2\1\1\0\0\0\0\0\0\0\0\0\1\0>\0\1\0\0\0\0\0\0\0\0\0\0\0"..., 8192) = 660
71692 lseek(3, 4, SEEK_CUR)             = 664
71692 write(3, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"..., 896) = 896
71692 close(3) 
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

int fd;
char buf[2048];

char wc = 'A';

off_t fpos = 0;
size_t fsize = 0;

char fdata[2048];

static inline int min(int a, int b) {
	return (a < b ? a : b);
}

static inline int max(int a, int b) {
	return (a > b ? a : b);
}

int print_err(const char *msg, int err)
{
	printf("ERROR: %s (err=%d)\n", msg, err);
	return err;
}

int do_open(const char *fname)
{
	fd = open(fname, O_CREAT|O_RDWR|O_EXCL, 0644);
	if (fd < 0) {
		return print_err("open", -errno);
	}

	return 0;
}

int do_seek(off_t ofs, int whence)
{
	printf("seeking to %d (%s)\n", (int)ofs, (whence == SEEK_SET ? "abs" : "rel"));
	int r = lseek(fd, ofs, whence);
	if ( r < 0) {
		return print_err("lseek", -errno);
	}
	printf("\t\tpos=%d\n", r);

	fpos = r;

	return 0;
}

int do_read(size_t len)
{
	char buf[len];

	printf("reading %d bytes\n", len);

	int r = read(fd, buf, len);
	if (r < 0) {
		return print_err("read", -errno);
	}

	size_t read_len = r;
	printf("\t\t got %d bytes\n", read_len);

	int expected_len = min(fsize - fpos, read_len);
	if (read_len != expected_len) {
		printf("unexpected read length: expected=%d read=%d\n", expected_len, read_len);
		return -EIO;
	}

	off_t pos = fpos;
	fpos += read_len;

	if (read_len == 0) {
		return 0;
	}

	if (memcmp(buf, &fdata[pos], read_len) != 0) {
		printf("unexpected data at pos %lld\n", pos);
		return -EIO;
	}

	return 0;
}

int do_write(size_t len)
{
	char buf[len];

	memset(buf, wc++, len);

	printf("writing %d bytes\n", (int)len);

	int r = write(fd, buf, len);
	if (r < 0) {
		return print_err("write", -errno);
	}

	memcpy(&fdata[fpos], buf, len);
	fpos += len;

	fsize = max(fpos, fsize);

	printf("\t\twrote %d bytes, fsize=%d \n", (int)len, (int)fsize);

	return len;
}

int do_close()
{
	int r = close(fd);
	if (r < 0) {
		return print_err("close", -errno);
	}

	return 0;
}

#define CHECK(x)  \
do { \
	if (r = x < 0) { \
		exit(-r); \
	} \
} while(0)

int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("Usage: %s <filename>\n", argv[0]);
		return -EINVAL;
	}

	int r;
	CHECK(do_open(argv[1]));

	CHECK(do_seek(0, SEEK_SET));
	CHECK(do_read(424));
	CHECK(do_seek(424, SEEK_CUR));
	CHECK(do_write(24));
	CHECK(do_seek(0, SEEK_SET));
	CHECK(do_read(8192));
	CHECK(do_seek(-384, SEEK_CUR));
	CHECK(do_write(83));
	CHECK(do_seek(0, SEEK_SET));
	CHECK(do_read(8192));
	CHECK(do_seek(-296, SEEK_CUR));
	CHECK(do_write(104));
	CHECK(do_seek(0, SEEK_SET));
	CHECK(do_read(8192));
	CHECK(do_write(96));
	CHECK(do_seek(0, SEEK_SET));
	CHECK(do_read(8192));
	CHECK(do_seek(-288, SEEK_CUR));
	CHECK(do_write(168));
	CHECK(do_seek(0, SEEK_SET));
	CHECK(do_read(8192));
	CHECK(do_write(116));
	CHECK(do_seek(0, SEEK_SET));
	CHECK(do_write(64));
	CHECK(do_seek(0, SEEK_SET));
	CHECK(do_read(8192));
	CHECK(do_seek(4, SEEK_CUR));
	CHECK(do_write(896));

	/* final verify */
	CHECK(do_seek(0, SEEK_SET));
	CHECK(do_read(fsize));
	CHECK(do_close(3));

}
