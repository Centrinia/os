
import bitarray
import pprint
import sys
import struct
import binascii

superblock_fields = [
# Offset (bytes)	Size (bytes)	Description
	{'name': 's_inodes_count', 'offset': 0, 'size': 4, 'repeats': 1, 'type': 'int'},
	{'name': 's_blocks_count', 'offset': 4, 'size': 4, 'repeats': 1, 'type': 'int'},
	{'name': 's_r_blocks_count', 'offset': 8, 'size': 4, 'repeats': 1, 'type': 'int'},
	{'name': 's_free_blocks_count', 'offset': 12, 'size': 4, 'repeats': 1, 'type': 'int'},
	{'name': 's_free_inodes_count', 'offset': 16, 'size': 4, 'repeats': 1, 'type': 'int'},
	{'name': 's_first_data_block', 'offset': 20, 'size': 4, 'repeats': 1, 'type': 'int'},
	{'name': 's_log_block_size', 'offset': 24, 'size': 4, 'repeats': 1, 'type': 'int'},
	{'name': 's_log_frag_size', 'offset': 28, 'size': 4, 'repeats': 1, 'type': 'int'},
	{'name': 's_blocks_per_group', 'offset': 32, 'size': 4, 'repeats': 1, 'type': 'int'},
	{'name': 's_frags_per_group', 'offset': 36, 'size': 4, 'repeats': 1, 'type': 'int'},
	{'name': 's_inodes_per_group', 'offset': 40, 'size': 4, 'repeats': 1, 'type': 'int'},
	{'name': 's_mtime', 'offset': 44, 'size': 4, 'repeats': 1, 'type': 'int'},
	{'name': 's_wtime', 'offset': 48, 'size': 4, 'repeats': 1, 'type': 'int'},
	{'name': 's_mnt_count', 'offset': 52, 'size': 2, 'repeats': 1, 'type': 'int'},
	{'name': 's_max_mnt_count', 'offset': 54, 'size': 2, 'repeats': 1, 'type': 'int'},
	{'name': 's_magic', 'offset': 56, 'size': 2, 'repeats': 1, 'type': 'int'},
	{'name': 's_state', 'offset': 58, 'size': 2, 'repeats': 1, 'type': 'int'},
	{'name': 's_errors', 'offset': 60, 'size': 2, 'repeats': 1, 'type': 'int'},
	{'name': 's_minor_rev_level', 'offset': 62, 'size': 2, 'repeats': 1, 'type': 'int'},
	{'name': 's_lastcheck', 'offset': 64, 'size': 4, 'repeats': 1, 'type': 'int'},
	{'name': 's_checkinterval', 'offset': 68, 'size': 4, 'repeats': 1, 'type': 'int'},
	{'name': 's_creator_os', 'offset': 72, 'size': 4, 'repeats': 1, 'type': 'int'},
	{'name': 's_rev_level', 'offset': 76, 'size': 4, 'repeats': 1, 'type': 'int'},
	{'name': 's_def_resuid', 'offset': 80, 'size': 2, 'repeats': 1, 'type': 'int'},
	{'name': 's_def_resgid', 'offset': 82, 'size': 2, 'repeats': 1, 'type': 'int'},
# EXT2_DYNAMIC_REV Specific --
	{'name': 's_first_ino', 'offset': 84, 'size': 4, 'repeats': 1, 'type': 'int'},
	{'name': 's_inode_size', 'offset': 88, 'size': 2, 'repeats': 1, 'type': 'int'},
	{'name': 's_block_group_nr', 'offset': 90, 'size': 2, 'repeats': 1, 'type': 'int'},
	{'name': 's_feature_compat', 'offset': 92, 'size': 4, 'repeats': 1, 'type': 'int'},
	{'name': 's_feature_incompat', 'offset': 96, 'size': 4, 'repeats': 1, 'type': 'int'},
	{'name': 's_feature_ro_compat', 'offset': 100, 'size': 4, 'repeats': 1, 'type': 'int'},
	{'name': 's_uuid', 'offset': 104, 'size': 1, 'repeats': 16, 'type': 'uint'},
	{'name': 's_volume_name', 'offset': 120, 'size': 1, 'repeats': 16, 'type': 'char'},
	{'name': 's_last_mounted', 'offset': 136, 'size': 1, 'repeats': 64, 'type': 'char'},
	{'name': 's_algo_bitmap', 'offset': 200, 'size': 4, 'repeats': 1, 'type': 'int'},
# Performance Hints --
	{'name': 's_prealloc_blocks', 'offset': 204, 'size': 1, 'repeats': 1,'type':'uint'},
	{'name': 's_prealloc_dir_blocks', 'offset': 205, 'size': 1, 'repeats': 1,'type':'uint'},
	{'name': None, 'offset': 206, 'size': 2, 'repeats': 1,'type':'uint'},
# Journaling Support --
	{'name': 's_journal_uuid', 'offset': 208, 'size': 1, 'repeats': 16,'type':'uint'},
	{'name': 's_journal_inum', 'offset': 224, 'size': 4, 'repeats': 1,'type':'uint'},
	{'name': 's_journal_dev', 'offset': 228, 'size': 4, 'repeats': 1,'type':'uint'},
	{'name': 's_last_orphan', 'offset': 232, 'size': 4, 'repeats': 1,'type':'uint'},
# Directory Indexing Support --
	{'name': 's_hash_seed', 'offset': 236, 'size': 4, 'repeats': 4,'type':'uint'},
	{'name': 's_def_hash_version', 'offset': 252, 'size': 1, 'repeats': 1,'type':'uint'},
	{'name': None, 'offset': 253, 'size': 3, 'repeats': 1,'type':'uint'},
	{'name': 's_default_mount_options', 'offset': 256, 'size': 4, 'repeats': 1,'type':'int'},
	{'name': 's_first_meta_bg', 'offset': 260, 'size': 4, 'repeats': 1,'type':'int'},
	#{'name': 'Unused - reserved for future revisions', 'offset': 264, 'size': 760, 'repeats': 1,'type':'uint'},
]


group_descriptor_fields = [
	{'name': 'bg_block_bitmap', 'offset': 0, 'size': 4, 'repeats': 1, 'type': 'uint'},
	{'name': 'bg_inode_bitmap', 'offset': 4, 'size': 4, 'repeats': 1, 'type': 'uint'},
	{'name': 'bg_inode_table', 'offset': 8, 'size': 4, 'repeats': 1, 'type': 'uint'},
	{'name': 'bg_free_blocks_count', 'offset': 12, 'size': 2, 'repeats': 1, 'type': 'uint'},
	{'name': 'bg_free_inodes_count', 'offset': 14, 'size': 2, 'repeats': 1, 'type': 'uint'},
	{'name': 'bg_used_dirs_count', 'offset': 16, 'size': 2, 'repeats': 1, 'type': 'uint'},
]


inode_table_fields = [
	{'name': 'i_mode', 'offset': 0, 'size': 2, 'repeats': 1, 'type': 'uint'},
	{'name': 'i_uid', 'offset': 2, 'size': 2, 'repeats': 1, 'type': 'uint'},
	{'name': 'i_size', 'offset': 4, 'size': 4, 'repeats': 1, 'type': 'uint'},
	{'name': 'i_atime', 'offset': 8, 'size': 4, 'repeats': 1, 'type': 'uint'},
	{'name': 'i_ctime', 'offset': 12, 'size': 4, 'repeats': 1, 'type': 'uint'},
	{'name': 'i_mtime', 'offset': 16, 'size': 4, 'repeats': 1, 'type': 'uint'},
	{'name': 'i_dtime', 'offset': 20, 'size': 4, 'repeats': 1, 'type': 'uint'},
	{'name': 'i_gid', 'offset': 24, 'size': 2, 'repeats': 1, 'type': 'uint'},
	{'name': 'i_links_count', 'offset': 26, 'size': 2, 'repeats': 1, 'type': 'uint'},
	{'name': 'i_blocks', 'offset': 28, 'size': 4, 'repeats': 1, 'type': 'uint'},
	{'name': 'i_flags', 'offset': 32, 'size': 4, 'repeats': 1, 'type': 'uint'},
	{'name': 'i_osd1', 'offset': 36, 'size': 4, 'repeats': 1, 'type': 'uint'},
	{'name': 'i_block', 'offset': 40, 'size': 4, 'repeats': 15, 'type': 'uint'},
	{'name': 'i_generation', 'offset': 100, 'size': 4, 'repeats': 1, 'type': 'uint'},
	{'name': 'i_file_acl', 'offset': 104, 'size': 4, 'repeats': 1, 'type': 'uint'},
	{'name': 'i_dir_acl', 'offset': 108, 'size': 4, 'repeats': 1, 'type': 'uint'},
	{'name': 'i_faddr', 'offset': 112, 'size': 4, 'repeats': 1, 'type': 'uint'},
	{'name': 'i_osd2', 'offset': 116, 'size': 4, 'repeats': 3, 'type': 'uint'}
]

pp = pprint.PrettyPrinter(indent=2,width=160)

def read_field(field,stream,offset):
	def struct_field_symbol(field):
		if field['type'] == 'char':
			symbol = 's'
		else:
			signed_symbols = {1:'b',2:'h',4:'i',8:'q'}
			symbol = signed_symbols[field['size']]
			if field['type'] == 'uint':
				symbol = symbol.upper()

		if field['repeats'] != 1:
			symbol = str(field['repeats']) + symbol
		return symbol


	position = offset + field['offset']
	stream.seek(position)

	#print(struct_field_symbol(field))

	field_data = stream.read(field['size'] * field['repeats'])

	field_items = struct.unpack(struct_field_symbol(field),field_data)
	if field['type'] == 'char':
		value = field_items[0].decode('ascii','ignore').partition('\0')[0]
	elif field['repeats'] == 1:
		value, = field_items
	else:
		value = list(field_items)
	return value


SUPERBLOCK_OFFSET = 1024
def read_block_group(image,group_number):
	
	superblock = {}
	for field in superblock_fields:
		if field['name'] is None:
			continue
		superblock[field['name']] = read_field(field,image,SUPERBLOCK_OFFSET)

	block_size = 2**(10+superblock['s_log_block_size'])

	group_descriptor = {}
	for field in group_descriptor_fields:
		if field['name'] is None:
			continue
		#group_descriptor[field['name']] = read_field(field,image,SUPERBLOCK_OFFSET + (group_number+1) * block_size)
		group_descriptor[field['name']] = read_field(field,image,(group_number+1+superblock['s_first_data_block']) * block_size)
		#group_descriptor[field['name']] = read_field(field,image,(group_number+1) * block_size)

	if group_descriptor['bg_block_bitmap'] <= group_number+1:
		return None
	image.seek(SUPERBLOCK_OFFSET + block_size * group_descriptor['bg_block_bitmap'])
	block_bitmap = bitarray.bitarray()
	block_bitmap.frombytes(image.read(block_size))

	image.seek(SUPERBLOCK_OFFSET + block_size * group_descriptor['bg_inode_bitmap'])
	inode_bitmap = bitarray.bitarray()
	inode_bitmap.frombytes(image.read(block_size))

	INODE_SIZE = 128
	inode_table = []
	for inode_index in range(superblock['s_inodes_per_group']):
		inode_entry = {}
		for field in inode_table_fields:
			if field['name'] is None:
				continue
			inode_entry[field['name']] = read_field(
				field,image,
				group_descriptor['bg_inode_table'] * block_size + inode_index*INODE_SIZE)
		inode_table += [inode_entry]

	return {
			'superblock':superblock,
			'group descriptor': group_descriptor,
			'block bitmap': block_bitmap,
			'inode bitmap': inode_bitmap,
			'inode table': inode_table
		}


def retrieve_inode(inode,image,block_size):
	acc = {
			'remaining': inode['i_size'],
			'data': b''
			}
	DEPTHS = [0] * 12 + [1,2,3]

	def retrieve_data(acc, depth, block):
		if depth == 0:
			#image.seek(SUPERBLOCK_OFFSET + block*block_size)
			image.seek(block*block_size)
			read_count = min(block_size,acc['remaining'])
			acc['data'] += image.read(read_count)
			acc['remaining'] -= read_count
		else:
			format_string = str(block_size // 4) + 'I'
			subblocks = struct.unpack(format_string, image.read(block_size))
			for subblock in subblocks:
				retrieve_data(acc, depth-1,subblock)

	for iblock,depth in zip(inode['i_block'],DEPTHS):
		if iblock == 0:
			break
		retrieve_data(acc,depth,iblock)
	#print(acc['remaining'])
	return acc['data']

def load_directory(data):
	directory = []
	buf = data[:]
	while len(buf) >= 8:
		inode, rec_len, name_len, file_type = struct.unpack('IHbb',buf[:8]);
		if inode != 0:
			name = buf[8:name_len+8]
			entry = {
				'inode': inode,
				'name': name,
				'file type': file_type
				}
			directory += [entry]
		buf = buf[rec_len:]
	return directory

with open(sys.argv[1],'rb') as ext2image:
	block_group = read_block_group(ext2image, 0)
	block_size = 2**(10+block_group['superblock']['s_log_block_size'])
	#print(len(block_group['inode bitmap']),block_group['superblock']['s_inodes_per_group'])

	for index,inode in enumerate(block_group['inode table']):
		inode_type = (inode['i_mode'] >> 12) & 0x0f
		if inode['i_mode'] != 0:
			print('inode {}:'.format(index+1))
			pp.pprint(inode)
		if inode_type == 0x04:
			file_data = retrieve_inode(inode,ext2image,block_size)
			#print(file_data)
			print('directory:')
			pp.pprint(load_directory(file_data))

		if inode_type == 0x08:
			#pp.pprint(inode)
			file_data = retrieve_inode(inode,ext2image,block_size)
			print(50*'-' + ' file ' + 50*'-')
			#print(file_data.decode('ascii','ignore'))
			try:
				file_text =file_data.decode('ascii','strict')
				print(file_text[:200])
				if len(file_text) > 200:
					print('...')
			except:
				print("<binary>")

			print(50*'-' + ' end ' + 50*'-')
	pp.pprint(block_group['superblock'])
	pp.pprint(block_group['group descriptor'])
	#pp.pprint(block_group)
	print(len(block_group['inode table']))

