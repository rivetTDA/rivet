import os,sys

new_file_content = []

def file_line(input_file):

	file = open(input_file, 'r')

	line = file.readline()
	while (line != ""):
		content = line.strip()
		if content == "":
			new_file_content.append("\n")
		elif content[0] == "#":
			new_file_content.append(line)
			# print(line)
			pass
		else:
			yield line

		line = file.readline()

	file.close()

def points(old_file):

	new_file_content.append("--type points\n")
	# print("--type points")

	next(old_file)
	# skip dimension

	max_dist = next(old_file).strip()
	new_file_content.append("--maxdist " + max_dist + "\n")
	# print("--maxdist " + max_dist)

	function = next(old_file).strip()
	if function == "no function":
		new_file_content.append("\n# data starts here\n")
		while True:
			try:
				data_line = next(old_file)
			except StopIteration:
				break
			except:
				print("An error was encountered while parsing file.")
				sys.exit(1)

			new_file_content.append(data_line)
			# print(data_line)
			
	else:
		if function[:3] == "[-]":
			new_file_content.append("--xreverse\n")
			new_file_content.append("--xlabel " + function[3:].strip() + "\n")
			# print("--xreverse")
			# print("--xlabel " + function[3:].strip())
		else:
			new_file_content.append("--xlabel " + function + "\n")
			# print("--xlabel " + function)

		values = ""

		function_line = len(new_file_content)
		new_file_content.append("\n# data starts here\n")
		while True:
			try:
				data_line = next(old_file)
			except StopIteration:
				break
			except:
				print("An error was encountered while parsing file.")
				sys.exit(1)
			content = data_line.strip()
			data = content.split()
			values += data[-1] + " "
			del data[-1]
			content = " ".join(data)
			new_file_content.append(content + "\n")
			# print(content)
			

		new_file_content.insert(function_line, values + "\n")
		new_file_content.insert(function_line, "--function" + "\n")
		# print("--function")
		# print(values)

def metric(old_file):
	
	new_file_content.append("--type metric\n")

	function = next(old_file).strip()
	if (function == "no function"):
		next(old_file)
	else:
		if function[:3] == "[-]":
			new_file_content.append("--xreverse\n")
			new_file_content.append("--xlabel " + function[3:].strip() + "\n")
			# print("--xreverse")
			# print("--xlabel " + function[3:].strip())
		else:
			new_file_content.append("--xlabel " + function + "\n")
			# print("--xlabel " + function)

		values = next(old_file)
		new_file_content.append("--function\n")
		new_file_content.append(values + "\n")
		# print("--function")
		# print(values)

	ylabel = next(old_file).strip()
	new_file_content.append("--ylabel " + ylabel + "\n")
	# print("--ylabel " + ylabel)

	max_dist = next(old_file).strip()
	new_file_content.append("--maxdist " + max_dist + "\n")
	# print("--maxdist " + max_dist)

	new_file_content.append("\n# data starts here\n")
	while True:
		try:
			data_line = next(old_file)
		except StopIteration:
			break
		except:
			print("An error was encountered while parsing file.")
			sys.exit(1)

		new_file_content.append(data_line)
		# print(data_line)

def bifil(old_file):

	new_file_content.append("--type bifiltration\n")

	xlabel = next(old_file).strip()
	new_file_content.append("--xlabel " + xlabel + "\n")
	# print("--xlabel " + xlabel)

	ylabel = next(old_file).strip()
	new_file_content.append("--ylabel " + ylabel + "\n")
	# print("--ylabel " + ylabel)

	new_file_content.append("\n# data starts here\n")
	while True:
		try:
			data_line = next(old_file)
		except StopIteration:
			break
		except:
			print("An error was encountered while parsing file.")
			sys.exit(1)
		new_file_content.append(data_line)
		# print(data_line)

def firep(old_file):

	new_file_content.append("--type firep\n")

	xlabel = next(old_file).strip()
	new_file_content.append("--xlabel " + xlabel + "\n")
	# print("--xlabel " + xlabel)

	ylabel = next(old_file).strip()
	new_file_content.append("--ylabel " + ylabel + "\n")
	# print("--ylabel " + ylabel)

	new_file_content.append("\n# data starts here\n")
	while True:
		try:
			data_line = next(old_file)
		except StopIteration:
			break
		except:
			print("An error was encountered while parsing file.")
			sys.exit(1)
		new_file_content.append(data_line)
		# print(data_line)

# main

file_path = sys.argv[1]
if len(sys.argv) > 2:
	output_path = sys.argv[2]
else:
	output_path = ""

file_name = file_path.split("/")[-1]

lines = file_line(file_path)

file_type = next(lines).strip()
# print(file_type)

if file_type == "points":
	points(lines)
elif file_type == "metric":
	metric(lines)
elif file_type == "bifiltration":
	bifil(lines)
elif file_type == "firep":
	firep(lines)
else:
	print(file_name + ": Unrecognized file.")
	sys.exit(0)

new_file = open(output_path + file_name, 'w')
new_file.writelines("%s" % line for line in new_file_content)
new_file.close()