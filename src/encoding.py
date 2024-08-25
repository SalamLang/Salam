import chardet

def detect_encoding(file_path):
	with open(file_path, 'rb') as file:
		raw_data = file.read()
		result = chardet.detect(raw_data)
		encoding = result['encoding']
		confidence = result['confidence']
		
		if encoding.lower() == 'windows-1252':
			print(f"The file is encoded in Windows-1252 with {confidence*100:.2f}% confidence.")
		else:
			print(f"The file is encoded in {encoding} with {confidence*100:.2f}% confidence.")

		return encoding, confidence

file_path = '../example/test6.salam'
detect_encoding(file_path)
