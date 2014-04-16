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
	FILE *rom;
} context;

void parse_context(int argc, char **argv) {
	context.rom_file = context.model_dir = NULL;
	const char *errorMessage = "Invalid usage - see `genkfs --help` or `man 1 genkfs` for more info.";
	int i;
	for (i = 1; i < argc; i++) {
		if (*argv[i] == '-') {
			if (strcasecmp(argv[i], "--help")) {
				// TODO
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
	context.rom = fopen(context.rom_file, "rw");
	fseek(context.rom, 0L, SEEK_END);
	uint32_t length = ftell(context.rom);
	fseek(context.rom, 0L, SEEK_SET);
	context.dat_start = 0x04;
	context.fat_start = length / 0x4000 - 0x9;
}

int main(int argc, char **argv) {
	parse_context(argc, argv);
	return 0;
}
