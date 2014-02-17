
import sys
from PIL import Image
import struct

img = Image.open(sys.argv[1])
dat = list(img.getdata())

#print dir(img)
#print dir(img.palette)
#print len(img.palette.getdata()[1])

with open(sys.argv[2],'wb') as out:
	for i in range(0,len(img.palette.getdata()[1]),3):
		c = map(lambda x: ord(x),img.palette.getdata()[1][i:i+3])

		c[0] >>= 3
		c[1] >>= 2
		c[2] >>= 3

		x = c[2] | c[1] << 5 | c[0] << 11
		out.write(struct.pack('H', x))
		i += 1
		if i%3 == 0:
			out.write(struct.pack('B', 0))


	for pix in dat:
		#pix = map(lambda x: x-128,list(pix2))
		out.write(struct.pack('B', pix))
