#include "kernel/types.h"

#include "kernel/fcntl.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char* argv[]) {
	int fd;
	char filename[32];
	int i;

	printf("ileak: starting REAL inode leak test\n");

	// Strategy: Create a file, then unlink it while keeping it open
	// This creates an inode that has no directory entry but is still allocated
	// If we crash before closing the file descriptor, the inode becomes leaked

	for (i = 0; i < 5; i++) {
		// Create a unique filename
		filename[0] = 't';
		filename[1] = 'e';
		filename[2] = 'm';
		filename[3] = 'p';
		filename[4] = '0' + i;
		filename[5] = '\0';

		printf("ileak: creating %s\n", filename);

		// Create and open the file - this allocates an inode
		fd = open(filename, O_CREATE | O_WRONLY);
		if (fd < 0) {
			printf("ileak: failed to create %s\n", filename);
			continue;
		}

		// Write some data to make the file substantial
		write(fd, "This file will become a leaked inode\n", 37);
		write(fd, "More data to make it real\n", 26);

		printf("ileak: created %s (fd=%d), now unlinking it\n", filename, fd);

		// CRITICAL: Unlink the file while keeping it open
		// This removes the directory entry but keeps the inode allocated
		// because our open file descriptor still references it
		if (unlink(filename) < 0) {
			printf("ileak: failed to unlink %s\n", filename);
			close(fd);
			continue;
		}

		printf("ileak: unlinked %s but keeping fd %d open\n", filename, fd);

		// At this point:
		// - The inode is allocated and contains data
		// - No directory entry points to it (unlinked)
		// - Only our file descriptor keeps it alive
		// - If we crash now, the inode becomes leaked!
	}

	printf("ileak: Created %d temporary files and unlinked them\n", i);
	printf("ileak: Files are now only reachable through open file descriptors\n");
	printf("\n");
	printf("ileak: *** CRASH THE SYSTEM NOW to cause inode leak ***\n");
	printf("ileak: The inodes will be allocated but unreachable after crash!\n");
	printf("ileak: In QEMU: Press Ctrl+A then X\n");
	printf("\n");

	// Sleep for a very long time to give opportunity to crash
	for (i = 0; i < 300; i++) {
		sleep(1);
		if (i % 30 == 0) { printf("ileak: sleeping %d/300 seconds...\n", i); }
	}

	// This cleanup should NOT happen if you crash the system
	printf("ileak: WARNING: You didn't crash! Cleaning up properly...\n");
	printf("ileak: (This means no inode leak will occur)\n");

	exit(0);
}