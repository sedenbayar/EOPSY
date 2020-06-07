// Seden Bayar

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>


int copy_read_write (int fd_from, int fd_to){
	size_t n_copied;
	char buffer[1024];

    do {
        // Read a maximum of 1024 characters
        n_copied = read(fd_from, &buffer, sizeof(buffer));

        // Catch errors in reading
        if (n_copied < 0) {
		    fprintf(stderr,"\nERROR: Error reading input file!\n");
            return 1;
        }

        // Write whatever characters read, and catch errors if any
        if (write(fd_to, &buffer, n_copied) < 0) {
            fprintf(stderr,"\nERROR: Error writing into output file!\n");
            return 1;
        }

    // Repeat until there are no characters left to read
    } while (n_copied > 0);
    
    return 0;
}

int copy_mmap (int fd_from, int fd_to){
	struct stat st;
	void *map_in, *map_out;

    // Get statistical information of the input file, specifically its size
	if (fstat(fd_from, &st) < 0){
		fprintf(stderr,"\nERROR: Couldn't fstat the source file!\n");
		return 1;
	}

	// Make space on the output file
	ftruncate(fd_to, st.st_size);

    // Memory map the input file
    map_in = mmap(0, st.st_size, PROT_READ, MAP_SHARED, fd_from, 0);

	if (map_in == MAP_FAILED) {
		fprintf(stderr,"\nERROR: Couldn't mmap the input file!\n");
		return 1;
    }

    // Memory map the output file
    map_out = mmap(0, st.st_size, PROT_WRITE, MAP_SHARED, fd_to, 0);

	if (map_out == MAP_FAILED) {
		fprintf(stderr,"\nERROR: Couldn't mmap the output file!\n");
		return 1;
    }

    // Copy contents over the memory mapping
	if (memcpy(map_out, map_in, st.st_size) == NULL) {
		fprintf(stderr,"\nERROR: Couldn't memcpy()!\n");
		return 1;
    }

    // Unmap memories-it is used to terminate mapping of part or all of a memory area previously mapped to a file by the mmap function. 
	if (munmap(map_in, st.st_size)) {
		fprintf(stderr,"\nERROR: munmap for the input file failed!\n");
		return 1;
    }

	if (munmap(map_out, st.st_size)) {
		fprintf(stderr,"\nERROR: munmap for the output file failed!\n");
		return 1;
    }
}

//print_help function -usage message
void print_help () {
    printf("Usage:\n");
    printf("copy <file_name> <new_file_name>\n");
    printf("\tCopy file with read() and write ()\n");
    printf("\n");
    printf("copy -m <file_name> <new_file_name>\n");
    printf("\tCopy file with mmap() and memcpy ()\n");
    printf("\n");
    printf("copy -h\n");
    printf("\tShow this help\n");
    printf("\n");
}

int main (int argc, char **argv){
    char c;
    int fd_from = 0;
    int fd_to = 0;
    int return_val;

    // Default mode of copying is read/write
    int (*function_to_use)(int, int) = copy_read_write;

    // Print help if no arguments were given
    if (argc == 1) {
        print_help();
        return 0;
    }

    // Process every option 
	while (1){
        // Read next option
        c = getopt (argc, argv, "hm");

        // If there is no more options given, get out
        if (c == -1) {
            break;
        }

        // Otherwise process the option
        if (c == 'm') {
            function_to_use = copy_mmap;
        }

        else if (c == 'h') {
            print_help();
            return 0;
        }

        else {
            fprintf(stderr,"\nERROR: Unknown character '%c'.\n", c);
        }
	}

    // Open the input file
    fd_from = open(argv[optind], O_RDONLY);

    if (fd_from < 0) {
        fprintf(stderr,"\nERROR: Can't open %s as input!\n", argv[optind]);
        return 1;
    }

    // Open the output file
    fd_to = open(argv[optind + 1], O_TRUNC | O_CREAT | O_RDWR, 0666);

    if (fd_to < 0){
        fprintf(stderr,"\nERROR: Can't open %s as output!\n", argv[optind + 1]);
        return 1;
    }

    // Call the selected function
    return_val = function_to_use(fd_from, fd_to);

    // Close the files
    if (close(fd_from)) {
        fprintf(stderr,"\nERROR: Couldn't close the input file!\n");
        return 1;
    }
    
    if (close(fd_to)) {
        fprintf(stderr,"\nERROR: Couldn't close the input file!\n");
        return 1;
    }

    return return_val;
}
