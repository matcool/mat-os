from PIL import Image

img = Image.open('raifont.png')

chars = [None] * 128
char_map = \
' ççççççççççççççç' \
'çççççççççççççççç' \
' !"#$%&\'()*+,-./' \
'0123456789:;<=>?' \
'@ABCDEFGHIJKLMNO' \
'PQRSTUVWXYZ[\\]^_' \
'`abcdefghijklmno' \
'pqrstuvwxyz{|}~ç' \
'çççççççççççççççç'

for x in range(int(img.width / 7)):
	for y in range(int(img.height / 9)):
		c = char_map[y * 16 + x]
		if c != 'ç':
			char_data = []
			for j in range(9):
				row = 0
				for i in range(7):
					if img.getpixel((x * 7 + (7 - i - 1), y * 9 + j))[3]:
						row += 1
					row = row << 1
				char_data.append(row)
			chars[ord(c)] = char_data

for i, d in enumerate(chars):
	if d is None: print('{0}, ', end='')
	else: print('{ ' + ', '.join(f'0x{i:02x}' for i in d) + f' }}, // "{chr(i)}"')
