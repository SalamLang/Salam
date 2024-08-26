import codecs
import chardet

# def analyze_file(filename):
# 	try:
# 		with open(filename, 'rb') as f:
# 			content = f.read()
# 			print(f"Hexdump of file content: {content.hex()}")

# 			result = chardet.detect(content)
# 			print(result)
# 			encoding = result['encoding']
# 			confidence = result['confidence']
# 			print(f"Detected encoding: {encoding} (confidence: {confidence})")

# 			if encoding:
# 				text = content.decode(encoding)
# 				print(f"Decoded text: {text}")
# 			else:
# 				print("Could not detect encoding.")
# 	except UnicodeDecodeError as e:
# 		print(f"UnicodeDecodeError: {e}")
# 		pos = e.start
# 		print(f"Problematic byte sequence at position {pos}: {content[pos:pos+10].hex()}")
# 	except Exception as e:
# 		print(f"An unexpected error occurred: {e}")

# analyze_file('test2.salam')

# # Hexdump of file content: d8b5d981d8add9873a0d0ad8aad985d8a7d985
# # hexdump_bytes = [
# # 	# 0000000 b5d8 81d9 add8 87d9 0d3a d80a d9aa d885
# # 	# 0000010 d9a7 0085
# # 	0xb5, 0xd8, 0x81, 0xd9, 0xad, 0xd8, 0x87, 0xd9,
# # 	0x0d, 0x3a, 0xd8, 0x0a, 0xd9, 0xaa, 0xd8, 0x85,
# # 	0xd9, 0xa7, 0x00, 0x85
# # ]


# hex_string = "d8b5d981d8add9873a0d0ad8aad985d8a7d985"
# hexdump_bytes = bytes.fromhex(hex_string)
# print(hexdump_bytes)

# try:
#     text = codecs.decode(hexdump_bytes, 'utf-8')
#     print(f"Decoded text: {text}")
# except UnicodeDecodeError as e:
#     print(f"UnicodeDecodeError: {e}")


# # try:
# # 	text = codecs.decode(bytes(hexdump_bytes), 'utf-8')
# # 	print(f"Decoded text: {text}")
# # except UnicodeDecodeError as e:
# # 	print(f"UnicodeDecodeError: {e}")


# def analyze_file(filename):
#     try:
#         with open(filename, 'rb') as f:
#             content = f.read()
#             print(f"Hexdump of file content: {content.hex()}")
#             text = content.decode('utf-8')
#             print(f"Decoded text: {text}")
#     except UnicodeDecodeError as e:
#         print(f"UnicodeDecodeError: {e}")
#         pos = e.start
#         print(f"Problematic byte sequence at position {pos}: {content[pos:pos+10].hex()}")
#     except Exception as e:
#         print(f"An unexpected error occurred: {e}")

# analyze_file('test2.salam')
