#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <dirent.h>

#define PAGE_LENGTH 0x4000
#define BLOCK_SIZE 0x100
#define KFS_FILE_ID 0x7F
#define KFS_DIR_ID 0xBF

struct {
	char *rom_file;
	char *model_dir;
	uint8_t fat_start;
	uint8_t dat_start;
	FILE *rom;
} context;

void show_help() {
	printf("Usage: genkfs <rom file> <model directory>\n");
}

void parse_context(int argc, char **argv) {
	context.rom_file = context.model_dir = NULL;
	const char *errorMessage = "Invalid usage - see `genkfs --help`\n";
	int i;
	for (i = 1; i < argc; i++) {
		if (*argv[i] == '-') {
			if (strcasecmp(argv[i], "--help") == 0) {
				show_help();
				exit(0);
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
}

char *concat_path(char* parent, char* child) {
	int pl = strlen(parent);
	int cl = strlen(child);
	if (parent[pl - 1] != '/') {
		pl++;
	}
	int len = pl + cl;
	char *a = malloc(len + 1);
	memcpy(a, parent, pl);
	memcpy(a + pl, child, cl);
	a[pl - 1] = '/';
	a[len] = 0;
	return a;
}

void memrev(uint8_t *s, int l) {
	uint8_t c, j, i;
	for (i = 0, j = l - 1; i < j; i++, j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

void write_fat(FILE *rom, uint8_t *entry, uint16_t length, uint32_t *fatptr) {
	*fatptr -= length;
	fseek(rom, *fatptr, SEEK_SET);
	fwrite(entry, length, 1, rom);
	fflush(rom);
}

void write_block(FILE *rom, FILE *file, uint16_t sectionId) {
	uint16_t flashPage = sectionId >> 6;
	uint16_t index = sectionId & 0x3F;
	fseek(rom, flashPage * PAGE_LENGTH + index * BLOCK_SIZE, SEEK_SET);
	uint8_t *block = malloc(BLOCK_SIZE);
	int len = fread(block, 1, BLOCK_SIZE, file);
	fwrite(block, len, 1, rom);
	fflush(rom);
	free(block);
}

void write_dat(FILE *rom, FILE *file, uint32_t length, uint16_t *sectionId) {
	uint16_t pSID = 0xFFFF;
	fseek(file, 0L, SEEK_SET);
	while (length > 0) {
		/* Prep */
		uint16_t flashPage = *sectionId >> 8;
		uint8_t index = *sectionId & 0x3F;
		uint16_t nSID = 0xFFFF;
		uint32_t header_addr = PAGE_LENGTH * flashPage + index * 4;
		index++;
		if (index > 0x3F) {
			index = 1;
			flashPage++;
			/* Write the magic number */
			fseek(rom, flashPage * PAGE_LENGTH, SEEK_SET);
			fprintf(rom, "KFS");
		}
		if (length > BLOCK_SIZE) {
			nSID = (flashPage << 8) | index;
		}

		/* Section header */
		fseek(rom, header_addr, SEEK_SET);
		pSID &= 0x7FFF; // Mark this section in use
		fwrite(&pSID, sizeof(pSID), 1, rom);
		fwrite(&nSID, sizeof(nSID), 1, rom);

		/* Block data */
		write_block(rom, file, *sectionId);
		fflush(rom);

		if (length < BLOCK_SIZE) {
			length = 0;
		} else {
			length -= BLOCK_SIZE;
		}
		pSID = *sectionId;
		*sectionId = (flashPage << 8) | index;
	}
}

void write_recursive(char *model, FILE *rom, uint16_t *parentId, uint16_t *sectionId, uint32_t *fatptr) {
	struct dirent *entry;
	DIR *dir = opendir(model);
	uint16_t parent = *parentId;
	while ((entry = readdir(dir))) {
		if (entry->d_type == DT_DIR && entry->d_name[0] != '.') {
			uint16_t elen = strlen(entry->d_name) + 6;
			uint8_t *fentry = malloc(elen + 3);
			char *path = concat_path(model, entry->d_name);
			printf("Adding %s...\n", path);

			fentry[0] = KFS_DIR_ID;
			fentry[1] = elen & 0xFF;
			fentry[2] = elen >> 8;
			fentry[3] = parent & 0xFF;
			fentry[4] = parent >> 8;
			*parentId += 1;
			fentry[5] = *parentId & 0xFF;
			fentry[6] = *parentId >> 8;
			fentry[7] = 0xFF; // Flags
			memcpy(fentry + 8, entry->d_name, strlen(entry->d_name) + 1);
			memrev(fentry, elen + 3);
			write_fat(rom, fentry, elen + 3, fatptr);

			write_recursive(path, rom, parentId, sectionId, fatptr);
			free(path);
			free(fentry);
		} else if (entry->d_type == DT_REG) {
			uint16_t elen = strlen(entry->d_name) + 9;
			uint8_t *fentry = malloc(elen + 3);
			char *path = concat_path(model, entry->d_name);
			FILE *file = fopen(path, "r");
			fseek(file, 0L, SEEK_END);
			if (ftell(file) > 0xFFFFFF) {
				fprintf(stderr, "Error: %s is larger than the maximum file size.\n", path);
				exit(1);
			}
			printf("Adding %s...\n", path);
			uint32_t len = (uint32_t)ftell(file);
			free(path);

			fentry[0] = KFS_FILE_ID;
			fentry[1] = elen & 0xFF;
			fentry[2] = elen >> 8;
			fentry[3] = parent & 0xFF;
			fentry[4] = parent >> 8;
			fentry[5] = 0xFF; // Flags
			fentry[6] = len & 0xFF;
			fentry[7] = (len >> 8) & 0xFF;
			fentry[8] = (len >> 16) & 0xFF;
			fentry[9] = *sectionId & 0xFF;
			fentry[10] = *sectionId >> 8;
			memcpy(fentry + 11, entry->d_name, strlen(entry->d_name) + 1);
			memrev(fentry, elen + 3);
			write_fat(rom, fentry, elen + 3, fatptr);

			write_dat(rom, file, len, sectionId);

			free(fentry);
		} else if (entry->d_type == DT_LNK) {
			fprintf(stderr, "Error: Unable to handle %s (symlinks are not currently supported)\n", entry->d_name);
			exit(1);
		}
	}
	closedir(dir);
}

void write_filesystem(char *model, FILE *rom, uint8_t fat_start, uint8_t dat_start) {
	uint16_t parentId = 0;
	uint16_t sectionId = (dat_start << 6) | 1;
	uint32_t fatptr = (fat_start + 1) * PAGE_LENGTH;
	/* Write the first DAT page's magic number */
	fseek(rom, dat_start * PAGE_LENGTH, SEEK_SET);
	fprintf(rom, "KFS");
	fflush(rom);
	write_recursive(model, rom, &parentId, &sectionId, &fatptr);
}

int main(int argc, char **argv) {
	parse_context(argc, argv);
	DIR *model = opendir(context.model_dir);
	if (model == NULL) {
		fprintf(stderr, "Unable to open %s.\n", context.model_dir);
		fclose(context.rom);
		exit(1);
	}
	closedir(model); // Re-opened later

	uint8_t p;
	uint8_t *blank_page = malloc(PAGE_LENGTH);
	memset(blank_page, 0xFF, PAGE_LENGTH);
	fseek(context.rom, context.dat_start * PAGE_LENGTH, SEEK_SET);
	for (p = context.dat_start; p <= context.fat_start; ++p) {
		if (p <= context.fat_start - 4) {
			blank_page[0] = 'K';
		} else {
			blank_page[0] = 0xFF;
		}
		fwrite(blank_page, PAGE_LENGTH, 1, context.rom);
	}
	fflush(context.rom);

	write_filesystem(context.model_dir, context.rom, context.fat_start, context.dat_start);

	fflush(context.rom);
	fclose(context.rom);
	free(blank_page);

	printf("Filesystem successfully written to %s.\n", context.rom_file);
	return 0;
}
