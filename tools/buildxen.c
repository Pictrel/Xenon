#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <libconfig.h>

uint8_t disk[0x200 * 256] = {0};

int verbose = 0;

int main(int argc, char **argv) {
	int c = 0;
	
	int bootfile = 0;
	char *output;
	char *s_value;
	int i_value;
	
	while ((c = getopt(argc, argv, "o:")) != -1) {
		switch (c) {
			case 'o':
				output = strdup(optarg);
				break;
		}
	}
	
	config_t conf;
	
	config_init(&conf);
	if (!config_read_file(&conf, argv[optind])) {
		printf("%s on line %d\n", config_error_text(&conf), config_error_line(&conf));
		return 1;
	}
	
	
	
	memcpy(disk + 0x0000, "XEN6", 4);
	
	if (config_lookup_string(&conf, "xe_name", &s_value)) memcpy(disk + 0x0004, s_value, 12);
	if (config_lookup_string(&conf, "xe_auth", &s_value)) memcpy(disk + 0x0010, s_value, 4);
	if (config_lookup_int(&conf, "xe_ver", &i_value)) disk[0x0014] = i_value;
	if (config_lookup_int(&conf, "xe_reg", &i_value)) disk[0x0015] = i_value;
	if (config_lookup_int(&conf, "xe_ent", &i_value)) {
		disk[0x001C] = i_value;
		disk[0x001D] = i_value >> 8;
	}
	
	config_setting_t *files;
	files = config_lookup(&conf, "files");
	
	int sect_p = 2;
	for (int i=0; i<config_setting_length(files); i++) {
		if (i > 31) {
			printf("Limit of files reached. Aborting.\n");
			break;
		}
		
		config_setting_t *file = config_setting_get_elem(files, i);
		
		config_setting_t *filename = config_setting_get_member(file, "filename");
		config_setting_t *finame = config_setting_get_member(file, "fi_name");
		config_setting_t *fiaddr = config_setting_get_member(file, "fi_addr");
		config_setting_t *boot = config_setting_get_member(file, "boot");
		config_setting_t *from = config_setting_get_member(file, "from");
		config_setting_t *to = config_setting_get_member(file, "to");
		
		if (!filename) {
			printf("File has no filename, ignoring.\n");
			continue;
		}
		
		char *filename_s = config_setting_get_string(filename);
		
		if (!fiaddr) {
			printf("File %s has no address, ignoring.\n", filename_s);
			continue;
		}
		
		FILE *fp = fopen(filename_s, "r");
		if (!fp) {
			perror(filename_s);
			return 1;
		}
		
		int file_start;
		int file_end;
		file_start = from ? config_setting_get_int(from) : 0;
		file_end = to ? config_setting_get_int(to) : 65535;
		
		fseek(fp, file_start, SEEK_SET);
		char buffer[65536];
		int len = fread(buffer, 1, file_end - file_start + 1, fp);
		int sectors = (int)ceil(len / 512.0);
		
		//file table
		disk[0x200+i*16 + 0] = 'F';
		disk[0x200+i*16 + 1] = i;
		disk[0x200+i*16 + 2] = sect_p;
		disk[0x200+i*16 + 3] = sectors;
		disk[0x200+i*16 + 4] = (config_setting_get_int(fiaddr)) >> 0;
		disk[0x200+i*16 + 5] = (config_setting_get_int(fiaddr)) >> 8;
		if (finame) memcpy(disk + 0x200+i*16 + 8, config_setting_get_string(finame), 7);
		
		if (sect_p + sectors > 0xff) {
			printf("Error: file %s goes over the storage limit. aborting.\n", filename_s);
			break;
		}
		
		//file data
		memcpy(disk + sect_p * 0x200, buffer, len);
		
		bool bootable = false;
		if (boot) bootable = config_setting_get_bool(boot);
		
		if (bootable) disk[0x0016] = i;
		
		printf("%24s (%7s): id $%02x, sectors $%02x-$%02x, %d sectors large (%dB) %s\n",
		        filename_s, &disk[0x200+i*16 + 8],
		        i, sect_p, sect_p + sectors - 1, sectors, len,
		        bootable ? "(BOOT)" : "");
		
		sect_p += sectors;
		//config_setting_get_string (const config_setting_t * setting)
		
		
		//fclose(fp);
	}
	
	*((uint32_t*)(disk + 0x18)) = time(NULL);
	
	printf("$20000 (131072) bytes total.\n");
	printf("$%05X (%d) bytes occupied.\n", sect_p * 512, sect_p * 512);
	printf("$%05X (%d) bytes free.\n",     0x20000 - (sect_p * 512),
	                                       0x20000 - (sect_p * 512));
	
	if (!output) {
		output = "disk.xen";
	}
	
	FILE *fp = fopen(output, "w");
	if (!fp) {
		perror(output);
		return 1;
	}
	fwrite(disk, 1, 131072, fp);
}