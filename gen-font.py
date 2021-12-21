from PIL import Image

img = Image.open('raifont.png').convert('RGBA')

chars = [None] * 128

width = 7
height = 10

for y in range(int(img.height / height)):
	for x in range(int(img.width / width)):
		c = y * 16 + x
		if c < 32: continue
		if c > 126: break
		char_data = []
		for j in range(height):
			row = 0
			for i in range(width):
				if img.getpixel((x * width + (width - i - 1), y * height + j))[3]:
					row += 1
				row = row << 1
			char_data.append(row)
		chars[c] = char_data

for i, d in enumerate(chars):
	if d is None: print('{0}, ', end='')
	else: print('{ ' + ', '.join(f'0x{i:02x}' for i in d) + f' }}, // "{chr(i)}"')
