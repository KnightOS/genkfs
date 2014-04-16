#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <dirent.h>

#define PAGE_LENGTH 0x4000

struct {
	char *rom_file;
	char *model_dir;
	uint8_t fat_start;
	uint8_t dat_start;
	FILE *rom;
} context;

uint32_t fatptr, datptr;

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
	context.rom = fopen(context.rom_file, "r+");
	if (context.rom == NULL) {
		fprintf(stderr, "Unable to open %s.\n", context.rom_file);
		exit(1);
	}
	fseek(context.rom, 0L, SEEK_END);
	uint32_t length = ftell(context.rom);
	fseek(context.rom, 0L, SEEK_SET);

	context.dat_start = 0x04;
	context.fat_start = length / PAGE_LENGTH - 0x9;
	fatptr = context.fat_start * PAGE_LENGTH;
	datptr = (context.dat_start + 1) * PAGE_LENGTH - 1;
}

int main(int argc, char **argv) {
	parse_context(argc, argv);
	DIR *model = opendir(context.model_dir);
	if (model == NULL) {
		fprintf(stderr, "Unable to open %s.\n", context.model_dir);
		fclose(context.rom);
		exit(1);
	}
	uint8_t p;
	uint8_t *blank_page = malloc(PAGE_LENGTH);
	memset(blank_page, 0xFF, PAGE_LENGTH);
	fseek(context.rom, context.dat_start * PAGE_LENGTH, SEEK_SET);
	for (p = context.dat_start; p <= context.fat_start; ++p) {
		fwrite(blank_page, PAGE_LENGTH, 1, context.rom);
	}
	fflush(context.rom);
	fclose(context.rom);
	free(blank_page);
	return 0;
}
