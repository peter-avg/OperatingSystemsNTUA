#include <stdlib.h> 
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "lunix-chrdev.h"

int main(int argc, char **argv) {
	int fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		perror("open failed");
		exit(EXIT_FAILURE);
	}

	if (ioctl(fd, LUNIX_IOC_EXAMPLE,NULL) < 0) {
		perror("ioctl failed");
		exit(EXIT_FAILURE);
	}

	close(fd);

	exit(EXIT_SUCCESS);
}


