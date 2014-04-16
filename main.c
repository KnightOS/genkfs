#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <strings.h>

#define PAGE_LENGTH 0x4000

struct {
	char *rom_file;
	char *model_dir;
	uint8_t fat_start;
	uint8_t dat_start;
} context;

void parse_context(int argc, char **argv) {
	context.rom_file = context.model_dir = NULL;
	fat_start = 0;
	dat_start = 0x04;
	const char *errorMessage = "Invalid usage - see `genkfs --help` or `man 1 genkfs` for more info.";
	for (int i = 1; i < argc; i++) {
		if (*argv[i] == '-') {
			if (strcasecmp(argv[i], "-d") || strcasecmp(argv[i], "--dat")) {
				/* TODO - let user specify filesystem range */
			} else {
				fprintf(stderr, errorMessage);
				exit(1);
			}
		} else {
			if (context.rom_file == NULL) {
				context.rom_file = argv[i];
			} else if (context.model_dir == NULL) {
				context.model_dir = argv[i];
			} else {
				fprintf(stderr, errorMessage);
				exit(1);
			}
		}
	}
	if (context.rom_file == NULL || context.model_dir == NULL) {
		fprintf(stderr, errorMessage);
		exit(1);
	}
}

int main(int argc, char **argv) {
	parse_context(argc, argv);
	return 0;
}
