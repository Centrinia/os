/* load_file.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>


typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

#define BOOT_BLOCK_SIZE		1024
#define SUPERBLOCK_SIZE		1024


struct superblock {
    uint32_t s_inodes_count;	// offset 0
    uint32_t s_blocks_count;	// offset 4
    uint32_t s_r_blocks_count;	// offset 8
    uint32_t s_free_blocks_count;	// offset 12
    uint32_t s_free_inodes_count;	// offset 16
    uint32_t s_first_data_block;	// offset 20
    uint32_t s_log_block_size;	// offset 24
    uint32_t s_log_frag_size;	// offset 28
    uint32_t s_blocks_per_group;	// offset 32
    uint32_t s_frags_per_group;	// offset 36
    uint32_t s_inodes_per_group;	// offset 40
    uint32_t s_mtime;		// offset 44
    uint32_t s_wtime;		// offset 48
    uint16_t s_mnt_count;	// offset 52
    uint16_t s_max_mnt_count;	// offset 54
    uint16_t s_magic;		// offset 56
    uint16_t s_state;		// offset 58
    uint16_t s_errors;		// offset 60
    uint16_t s_minor_rev_level;	// offset 62
    uint32_t s_lastcheck;	// offset 64
    uint32_t s_checkinterval;	// offset 68
    uint32_t s_creator_os;	// offset 72
    uint32_t s_rev_level;	// offset 76
    uint16_t s_def_resuid;	// offset 80
    uint16_t s_def_resgid;	// offset 82
    // EXT2_DYNAMIC_REV Specific --
    uint32_t s_first_ino;	// offset 84
    uint16_t s_inode_size;	// offset 88
    uint16_t s_block_group_nr;	// offset 90
    uint32_t s_feature_compat;	// offset 92
    uint32_t s_feature_incompat;	// offset 96
    uint32_t s_feature_ro_compat;	// offset 100
    uint8_t s_uuid[16];		// offset 104
    uint8_t s_volume_name[16];	// offset 120
    uint8_t s_last_mounted[64];	// offset 136
    uint32_t s_algo_bitmap;	// offset 200
    // Performance Hints --
    uint8_t s_prealloc_blocks[1];	// offset 204
    uint8_t s_prealloc_dir_blocks[1];	// offset 205
    uint8_t ignored[2];		// offset 206
    // Journaling Support --
    uint8_t s_journal_uuid[16];	// offset 208
    uint32_t s_journal_inum;	// offset 224
    uint32_t s_journal_dev;	// offset 228
    uint32_t s_last_orphan;	// offset 232
    // Directory Indexing Support --
    uint32_t s_hash_seed[4];	// offset 236
    uint8_t s_def_hash_version[1];	// offset 252
    uint8_t ignored_1[3];	// offset 253 - reserved for future expansion
    // Other options --
    uint32_t s_default_mount_options;	// offset 256
    uint32_t s_first_meta_bg;	// offset 260
    uint8_t ignored_2[760];	// offset 264 - reserved for future revisions
};

struct group_descriptor {
    uint32_t bg_block_bitmap;	// offset 0
    uint32_t bg_inode_bitmap;	// offset 4
    uint32_t bg_inode_table;	// offset 8
    uint16_t bg_free_blocks_count;	// offset 12
    uint16_t bg_free_inodes_count;	// offset 14
    uint16_t bg_used_dirs_count;	// offset 16
    uint16_t bg_pad;		// offset 18
    uint8_t bg_reserved[12];	// offset 20
};

struct inode_table {
    uint16_t i_mode;		// offset 0
    uint16_t i_uid;		// offset 2
    uint32_t i_size;		// offset 4
    uint32_t i_atime;		// offset 8
    uint32_t i_ctime;		// offset 12
    uint32_t i_mtime;		// offset 16
    uint32_t i_dtime;		// offset 20
    uint16_t i_gid;		// offset 24
    uint16_t i_links_count;	// offset 26
    uint32_t i_blocks;		// offset 28
    uint32_t i_flags;		// offset 32
    uint32_t i_osd1;		// offset 36
    uint32_t i_block[15];	// offset 40
    uint32_t i_generation;	// offset 100
    uint32_t i_file_acl;	// offset 104
    uint32_t i_dir_acl;		// offset 108
    uint32_t i_faddr;		// offset 112
    uint8_t i_osd2[12];		// offset 116
} __attribute__ ((packed));

void load_block(void *buf, int fd, uint32_t index, uint32_t block_size,
		uint32_t offset)
{
    //lseek(fd, BOOT_BLOCK_SIZE+index*block_size, SEEK_SET);
    lseek(fd, index * block_size + offset, SEEK_SET);
    read(fd, buf, block_size);
}

#define DEPTHS	3
void **cached_blocks;

void load_iblock(uint8_t * buf, uint32_t * remaining, int fd,
		 uint32_t size, uint32_t index, int depth,
		 uint32_t block_size)
{
    if (*remaining > 0) {
	return;
    }
    if (depth == 0) {
	uint32_t read_count =
	    *remaining < block_size ? *remaining : block_size;
	lseek(fd,index*block_size, SEEK_SET);
	read(fd, &buf[size - *remaining], read_count);
	*remaining -= read_count;
    } else {
	uint32_t *i_block = cached_blocks[depth - 1];
	read(fd, i_block, block_size);
	for (int i = 0; i < block_size / sizeof(uint32_t); i++) {
	    load_iblock(buf, remaining, fd, size, i_block[i], depth - 1,
			block_size);
	}
    }
}

void *load_inode(int fd, 
		 struct inode_table *inode_table,
	       	uint32_t block_size
		 )
{
    if (inode_table->i_size == 0) {
	return NULL;
    }
    uint32_t remaining = inode_table->i_size;
    uint8_t *buf = malloc(remaining);

    for (int i = 0; i < 15; i++) {
	if (inode_table->i_block[i] == 0) {
	    break;
	}
	load_iblock(buf, &remaining, fd, inode_table->i_size,
		    inode_table->i_block[i], i - (i < 11 ? i : 11),
		    block_size);
    }

    return buf;
}

int main(int argc, char **argv)
{

    int fd;

    if (argc <= 1) {
	printf("Usage: load_file image\n");
	return 1;
    }

    fd = open(argv[1], O_RDONLY);

    lseek(fd, BOOT_BLOCK_SIZE, SEEK_SET);

    struct superblock *superblock =
	(struct superblock *) malloc(SUPERBLOCK_SIZE);
    read(fd, superblock, SUPERBLOCK_SIZE);
    uint32_t block_size = 1024 << superblock->s_log_block_size;

    void *block = malloc(block_size);

    cached_blocks = malloc(sizeof(void *) * (DEPTHS - 1));
    for (int i = 0; i < DEPTHS - 1; i++) {
	cached_blocks[i] = malloc(block_size);
    }

    struct group_descriptor *group_descriptor = block;

    load_block(group_descriptor, fd, 1 + superblock->s_first_data_block,
	       block_size, 0);

    printf("block size: %d\n", block_size);
    printf("inode table: %d\n", group_descriptor->bg_inode_table);
    printf("block bitmap: %d\n", group_descriptor->bg_block_bitmap);
    printf("inodes per group: %d\n", superblock->s_inodes_per_group);
    printf("inode size: %d\n", superblock->s_inode_size);


    uint32_t inode_table_start = group_descriptor->bg_inode_table;
#if 0
    uint32_t inodes_per_block = 8 << superblock->s_log_block_size;
    printf("inodes per block: %d\n", inodes_per_block);
    for (int i = 0; i < superblock->s_inodes_per_group;
	 i += inodes_per_block) {
	load_block(block, fd, inode_table_start + i / inodes_per_block,
		   block_size, 0);

	struct inode_table *inode_table = block;


	for (int j = 0; j < inodes_per_block; j++) {
	    if (inode_table[j].i_mode) {
		printf("i_mode[%d]: %.8x; %.8lx\n", i + j,
		       inode_table[j].i_mode,
		       j * sizeof(struct inode_table) +
		       block_size * (inode_table_start +
				     i / inodes_per_block));
	    printf("\ti_size[%d]: %.8x\n", i+j, inode_table[j].i_size);

#if 1
		if((inode_table[j].i_mode >>12) == 0x08) {
			uint8_t * buf = load_inode(fd, &inode_table[j], block_size);
			printf("inode size: %d\n", inode_table[j].i_size);
			for(int i=0;i<inode_table[j].i_size;i++) {
				putchar(buf[i]);
			}
			free(buf);

		}
#endif

	    }
	}
    }
#else
    for (int i = 0; i < superblock->s_inodes_per_group; i++) {
	struct inode_table *inode_table = block;
	load_block(block, fd, inode_table_start, block_size,
		   superblock->s_inode_size * i);
	if(inode_table->i_mode) 
	{
	    printf("i_mode[%d]: %.8x; %.7lx\n", i, inode_table->i_mode,
		   superblock->s_inode_size * i +
		   inode_table_start * block_size);
	    printf("\ti_size[%d]: %.8x\n", i, inode_table->i_size);
	    printf("\ti_flags[%d]: %.8x\n", i, inode_table->i_flags);
	    printf("\ti_uid[%d]: %ld\n", i, inode_table->i_uid);
	}
    }
#endif

    printf("%ld\n", sizeof(struct superblock));
    printf("%ld\n", sizeof(struct group_descriptor));
    printf("%ld\n", sizeof(struct inode_table));
    close(fd);

    for (int i = 0; i < DEPTHS - 1; i++) {
	free(cached_blocks[i]);
    }
    free(cached_blocks);

    free(block);
    free(superblock);


    return 0;
}
