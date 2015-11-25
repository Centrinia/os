/* load_file.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define SUPERBLOCK_OFFSET	1024
#define INODE_SIZE		128

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

struct directory_entry {
    uint32_t inode;
    uint16_t rec_len;
    uint8_t name_len;
    uint8_t file_type;
};
void seek_read(void *data, int fd, uint32_t offset, uint32_t size)
{
    lseek(fd, offset, SEEK_SET);
    read(fd, data, size);
}

void read_directory(int fd, uint32_t table_offset, uint32_t inode_index,
		    uint32_t block_size)
{
    uint16_t i_mode;
    uint32_t i_size;

    seek_read(&i_mode, fd,
	      table_offset * block_size + inode_index * INODE_SIZE, 4);
    seek_read(&i_size, fd,
	      table_offset * block_size + inode_index * INODE_SIZE + 4, 4);

    printf("inode mode: %.4x\n", i_mode);
    printf("inode size: %d\n", i_size);


#if 0
    uint8_t *data = malloc(block_size * 3);

    for (int i = 0; i < 15; i++) {
	uint32_t i_block;

	seek_read(&i_block, fd,
		  table_offset * block_size + inode_index * INODE_SIZE +
		  40 + i * 4, 4);
	if (i_block == 0) {
	    break;
	}
	if (i <= 12) {
	    seek_read(data, fd, i_block * block_size, block_size);
	}
    }
    else {
    }
}
#else
    struct directory_entry entry;
    uint8_t name[256];
    uint32_t offset = 0;
    for (int i = 0; i <= 12; i++) {
	uint32_t i_block;

	seek_read(&i_block, fd,
		  table_offset * block_size + inode_index * INODE_SIZE +
		  40 + i * 4, 4);

	if (i_block == 0) {
	    break;
	}
	for (uint32_t o = 0; o < block_size;) {
	    seek_read(&entry, fd, i_block * block_size + o,
		      sizeof(struct directory_entry));
	    if (entry.rec_len == 0) {
		break;
	    }
	    seek_read(name, fd, i_block * block_size + o + 8,
		      entry.name_len);
	    name[entry.name_len] = '\0';
	    printf("%d: %s; %d; %d %d\n", offset, name, entry.inode,
		   entry.name_len,entry.rec_len);

	    offset += entry.rec_len;
	    o += entry.rec_len;
	}
    }
#endif
}

int main(int argc, char **argv)
{

    int fd;

    if (argc <= 1) {
	printf("Usage: load_file image\n");
	return 0;
    }

    fd = open(argv[1], O_RDONLY);
    if (!fd) {
	fprintf(stderr, "error loading file %s\n", argv[1]);
	return 1;
    }

    uint32_t s_log_block_size, s_first_data_block;

    seek_read(&s_first_data_block, fd, SUPERBLOCK_OFFSET + 20, 4);
    seek_read(&s_log_block_size, fd, SUPERBLOCK_OFFSET + 24, 4);

    uint32_t block_size = 1024 << s_log_block_size;

    uint32_t first_block_index = 1 + s_first_data_block;

    uint32_t bg_inode_table;

    seek_read(&bg_inode_table, fd, first_block_index * block_size + 8, 4);
    printf("first data block: %d\n", s_first_data_block);
    printf("block size: %d\n", block_size);
    printf("inode table: %d\n", bg_inode_table);
#if 0
    for (int i = 0; i < 10; i++) {
	read_directory(fd, bg_inode_table, i, block_size);
    }
#endif

    read_directory(fd, bg_inode_table, 1, block_size);

    close(fd);


    return 0;
}
