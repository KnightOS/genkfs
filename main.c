#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#define PAGE_LENGTH 0x4000
#define BLOCK_SIZE 0x100
#define KFS_FILE_ID 0x7F
#define KFS_DIR_ID 0xBF
#define KFS_SYM_ID 0xDF
#define KFS_VERSION 0

struct {
	char *rom_file;
	char *model_dir;
	uint8_t fat_start;
	uint8_t dat_start;
	FILE *rom;
} context;

void show_help() {
	printf(
		"genkfs - Writes KFS filesystems into ROM dumps\n"
		"\n"
		"Usage: genkfs input model\n"
		"See `man 1 genkfs` for details.\n"
		"\n"
		"input should be the ROM file to write to, and model should be a local\n"
		"directory that will be copied to root on the new filesystem.\n"
		"\n"
		"Examples:\n"
		"\tTo write ./temp to / on example.rom:\n"
		"\t\tgenkfs example.rom ./temp\n"
	);
}

int parse_context(int argc, char **argv) {
	context.rom_file = context.model_dir = NULL;
	const char *errorMessage = "Invalid usage - see `genkfs --help`\n";
	int i;
	for (i = 1; i < argc; i++) {
		if (*argv[i] == '-') {
			if (strcasecmp(argv[i], "--help") == 0) {
				show_help();
				return 0;
			} else {
				fprintf(stderr, "%s \n", errorMessage);
				return 1;
			}
		} else {
			if (context.rom_file == NULL) {
				context.rom_file = argv[i];
			} else if (context.model_dir == NULL) {
				context.model_dir = argv[i];
			} else {
				fprintf(stderr, "%s \n", errorMessage);
				return 1;
			}
		}
	}
	if (context.rom_file == NULL || context.model_dir == NULL) {
		fprintf(stderr, "%s\n", errorMessage);
		return 1;
	}
	context.rom = fopen(context.rom_file, "r+");
	if (context.rom == NULL) {
		fprintf(stderr, "Unable to open %s.\n", context.rom_file);
		return 1;
	}
	fseek(context.rom, 0L, SEEK_END);
	uint32_t length = ftell(context.rom);
	fseek(context.rom, 0L, SEEK_SET);

	context.dat_start = 0x04;
	context.fat_start = length / PAGE_LENGTH - 0x9;
	return -1;
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
	uint16_t flashPage = sectionId >> 8;
	uint16_t index = sectionId & 0xFF;
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
		uint8_t index = *sectionId & 0xFF;
		uint16_t nSID = 0xFFFF;
		uint32_t header_addr = PAGE_LENGTH * flashPage + index * 4;
		index++;
		if (index > 0x3F) {
			index = 1;
			flashPage++;
			/* Write the magic number */
			fseek(rom, flashPage * PAGE_LENGTH, SEEK_SET);
			fprintf(rom, "KFS");
			fputc(0xFF << KFS_VERSION, rom);
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
	struct dirent **nameList = NULL;
	int numEntries = scandir(model, &nameList, NULL, alphasort);
	int i;
	for (i = 0; i < numEntries; i++) {
		entry = nameList[i];
		if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
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
			struct stat ls;
			char *path = concat_path(model, entry->d_name);
			lstat(path, &ls);
			char *target = malloc(ls.st_size + 1);
			ssize_t r = readlink(path, target, ls.st_size + 1);
			target[r] = '\0';
			printf("Adding link from %s to %s...\n", path, target);

			uint16_t elen = strlen(entry->d_name) + strlen(target) + 5;
			uint8_t *sentry = malloc(elen + 3);
			sentry[0] = KFS_SYM_ID;
			sentry[1] = elen & 0xFF;
			sentry[2] = elen >> 8;
			sentry[3] = parent & 0xFF;
			sentry[4] = parent >> 8;
			sentry[5] = strlen(entry->d_name) + 1;
			memcpy(sentry + 6, entry->d_name, strlen(entry->d_name) + 1);
			memcpy(sentry + 6 + strlen(entry->d_name) + 1, target, strlen(target) + 1);
			memrev(sentry, elen + 3);
			write_fat(rom, sentry, elen + 3, fatptr);

			free(sentry);
			free(target);
			free(path);
		}
		free(nameList[i]);
	}
	free(nameList);
	closedir(dir);
}

// Returns the number of data pages (low byte) and fat pages (high byte) written.
uint16_t write_filesystem(char *model, FILE *rom, uint8_t fat_start, uint8_t dat_start) {
	uint16_t parentId = 0;
	uint16_t sectionId = (dat_start << 8) | 1;
	uint32_t fatptr = (fat_start + 1) * PAGE_LENGTH;
	uint32_t fatptrStart = fatptr;
	/* Write the first DAT page's magic number */
	fseek(rom, dat_start * PAGE_LENGTH, SEEK_SET);
	fprintf(rom, "KFS");
	fflush(rom);
	write_recursive(model, rom, &parentId, &sectionId, &fatptr);
	div_t d = div(fatptrStart - fatptr, PAGE_LENGTH);
	uint16_t result = d.quot;
	if (d.rem) {
		result++;
	}
	result <<= 8;
	// sectionId's high byte is a page number
	result += (sectionId >> 8) - dat_start + 1;
	return result;
}

int main(int argc, char **argv) {
	int ret = parse_context(argc, argv);
	if (ret != -1) {
		return ret;
	}
	DIR *model = opendir(context.model_dir);
	if (model == NULL) {
		fprintf(stderr, "Unable to open %s.\n", context.model_dir);
		fclose(context.rom);
		return 1;
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

	uint16_t result = write_filesystem(
		context.model_dir, context.rom, context.fat_start, context.dat_start);

	fflush(context.rom);
	fclose(context.rom);
	free(blank_page);

	printf("Filesystem successfully written to %s.\n", context.rom_file);
	printf("Indexes of written data pages: ");
	for (int i=0; i<(result & 0xff); i++) {
		printf("%02x ", context.dat_start + i);
	}
	printf("\nIndexes of written FAT pages: ");
	for (int i=0; i<(result >> 8); i++) {
		printf("%02x ", context.fat_start - i);
	}
	printf("\nThe rest of the pages (except kernels' 00-03) are empty.\n");
	return 0;
}
