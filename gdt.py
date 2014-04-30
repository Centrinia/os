import numpy

def insert_bits(a,abase,b,bbase,size):
	mask = (1 << size) - 1
	a &= ~(mask << abase)
	a |= ((b>>bbase) & mask) << abase
	return a

def descriptor(d):
	fields = [ 									\
			{'name': 'segment limit',	'in': 0, 'size': 16},		\
			{'name': 'base address',	'in': 0, 'size': 24},		\
			{'name': 'type',		'in': 0, 'size': 4},		\
			{'name': 'system',		'in': 0, 'size': 1},		\
			{'name': 'privilege level',	'in': 0, 'size': 2},		\
			{'name': 'present',		'in': 0, 'size': 1},		\
			{'name': 'segment limit',	'in': 16, 'size': 4},		\
			{'name': 'available',		'in': 0, 'size': 1},		\
			{'name': '64-bit',		'in': 0, 'size': 1},		\
			{'name': 'operation size',	'in': 0, 'size': 1},		\
			{'name': 'granularity',		'in': 0, 'size': 1},		\
			{'name': 'base address',	'in': 24, 'size': 8}		\
		]

	if d['segment limit'] > (1<<16):
		d['segment limit'] >>= 12
		d['granularity'] = 1
	else:
		d['granularity'] = 0

	outbase = 0
	out = 0
	for f in fields:
		out = insert_bits(out,outbase, d[f['name']], f['in'], f['size'])
		outbase += f['size']

	return out


def main():
	gdt = [								\
		# Null segment						\
			{'segment limit': 0, 				\
			 'base address': 0,				\
			 'type': 0,					\
			 'system': 0,					\
			 'privilege level': 0,				\
			 'present': 0,					\
			 'available': 0,				\
			 '64-bit': 0,					\
			 'operation size': 0,				\
			 'granularity': 0,				\
			 },						\
		# Code segment						\
			{'segment limit': (1<<32)-1, 			\
			 'base address': 0,				\
			 'type': 0xa,					\
			 'system': 1,					\
			 'privilege level': 0,				\
			 'present': 1,					\
			 'available': 0,				\
			 '64-bit': 0,					\
			 'operation size': 1,				\
			 },						\
		# Data segment						\
			{'segment limit': (1<<32)-1, 			\
			 'base address': 0,				\
			 'type': 0x2,					\
			 'system': 1,					\
			 'privilege level': 0,				\
			 'present': 1,					\
			 'available': 0,				\
			 '64-bit': 0,					\
			 'operation size': 1,				\
			 },						\
		# Task segment						\
			{'segment limit': 103, 				\
			 'base address': 4096,				\
			 'type': 0x9,					\
			 'system': 0,					\
			 'privilege level': 0,				\
			 'present': 1,					\
			 'available': 0,				\
			 '64-bit': 0,					\
			 'operation size': 1,				\
			 }						\
		]
	for d in gdt:
		x = descriptor(d)
		print(".long 0x{:0>8x}, 0x{:0>8x}".format(x & 0xffffffff, (x >> 32) & 0xffffffff))

main()

