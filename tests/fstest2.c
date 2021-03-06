#include "common.c"

#include <string.h>

#include <rump/rump.h>
#include <rump/rump_syscalls.h>

#define EXPECTED \
  "Is that a small file system in your pocket or aren't you happy to see me?\n"

union u {
	unsigned i;
	char c;
};

int
main(int argc, char *argv[])
{
	struct rump_ufs_args args;
	union u u;
	char buf[8192];
	int fd;

	/*
	 * the file system image we use is little endian and the driver
	 * we use doesn't support endian-swapping, so run the test only
	 * on little endian for now
	 */
	u.i = 0x12345678;
	if (u.c == 0x12) {
		printf("test works only on little endian.  skipping\n");
		return 0;
	}
		

        rump_init();

#define MYFSDEV "/de-vice"
	rump_pub_etfs_register(MYFSDEV, argv[1], RUMP_ETFS_BLK);
	args.fspec =  (void *)(uintptr_t)MYFSDEV;

	if (rump_sys_mkdir("/mnt", 0755) == -1)
		die("mkdir /mnt");
	if (rump_sys_mount(RUMP_MOUNT_SYSVBFS, "/mnt", RUMP_MNT_RDONLY,
	    &args, sizeof(args)) == -1)
		die("mount");
	if ((fd = rump_sys_open("/mnt/README", 0)) == -1)
		die("open file");
	memset(buf, 0, sizeof(buf));
	if (rump_sys_read(fd, buf, sizeof(buf)) <= 0)
		die("read version");
	if (strcmp(buf, EXPECTED) != 0)
		die("got unexpected result");
	rump_sys_close(fd);
	if (rump_sys_unmount("/mnt", 0) == -1)
		die("unmount failed");

	return 0;
}
