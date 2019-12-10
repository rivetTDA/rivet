import os,sys

new_file_content = []

def file_line(input_file):

	file = open(input_file, 'r')

	line = file.readline()
	# empty string indicates end of file
	while (line != ""):
		content = line.strip()
		if content == "":
			new_file_content.append("\n")
		# if it is a comment, add it as it is
		elif content[0] == "#":
			new_file_content.append(line)
			pass
		else:
			yield line

		line = file.readline()

	file.close()

def points(old_file):

	# --datatype must be changed if function values are encountered later
	new_file_content.append("--datatype points\n")
	type_index = len(new_file_content)-1

	# skip dimension
	next(old_file)

	max_dist = next(old_file).strip()
	new_file_content.append("--maxdist " + max_dist + "\n")

	function = next(old_file).strip()

	# add data to new file if no function is specified
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
	
	# add other stuff before adding data if function values are specified		
	else:
		if function[:3] == "[-]":
			new_file_content.append("--xreverse\n")
			new_file_content.append("--xlabel " + function[3:].strip() + "\n")
		else:
			new_file_content.append("--xlabel " + function + "\n")

		# change --datatype since function values are there
		new_file_content[type_index] = "--datatype points_fn\n"

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
			# parse line to extract function value from end of the line
			content = data_line.strip()
			data = content.split()
			values += data[-1] + " "
			del data[-1]
			content = " ".join(data)
			new_file_content.append(content + "\n")
			
		# insert function values before data starts
		new_file_content.insert(function_line, values + "\n")
		new_file_content.insert(function_line, "\n# function values\n")

def metric(old_file):
	
	# --datatype must be changed if function values are encountered later
	new_file_content.append("--datatype metric\n")
	type_index = len(new_file_content)-1

	# need to access it outside if-else scope
	values = ""

	function = next(old_file).strip()
	if (function == "no function"):
		next(old_file)
	else:
		if function[:3] == "[-]":
			new_file_content.append("--xreverse\n")
			new_file_content.append("--xlabel " + function[3:].strip() + "\n")

		else:
			new_file_content.append("--xlabel " + function + "\n")

		# change --datatype since function values are there
		new_file_content[type_index] = "--datatype metric_fn\n"

		values = next(old_file)

	ylabel = next(old_file).strip()
	new_file_content.append("--ylabel " + ylabel + "\n")

	max_dist = next(old_file).strip()
	new_file_content.append("--maxdist " + max_dist + "\n")

	# if function values were there, put them right before data starts
	if values:
		new_file_content.append("\n# function values\n")
		new_file_content.append(values + "\n")

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

def bifil(old_file):

	new_file_content.append("--type bifiltration\n")

	xlabel = next(old_file).strip()
	new_file_content.append("--xlabel " + xlabel + "\n")

	ylabel = next(old_file).strip()
	new_file_content.append("--ylabel " + ylabel + "\n")

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

def firep(old_file):

	new_file_content.append("--type firep\n")

	xlabel = next(old_file).strip()
	new_file_content.append("--xlabel " + xlabel + "\n")

	ylabel = next(old_file).strip()
	new_file_content.append("--ylabel " + ylabel + "\n")

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

def help():
	print("Usage: python3 convert.py <input_file> [output_file]")
	print("\nConverts the data file input_file with old formatting to the new input format.")
	print("If output_file is not specified, the newly created file is named new_input_file.")
	print("The new file is created in the directory where the script is run from.")
	sys.exit(0)

# main

# usage: python3 convert.py input_file
# usage: python3 convert.py input_file output_file

if len(sys.argv) == 1:
	help()

file_path = sys.argv[1]
if len(sys.argv) > 2:
	output_file = sys.argv[2]
# if output_file is not specified, use current directory
else:
	output_file = ""

file_name = file_path.split("/")[-1]

lines = file_line(file_path)

file_type = next(lines).strip()

# cannot parse a module invariants file - no need to either
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
	sys.exit(1)

# new file name = new_<old file name>
if output_file == "":
	output_file = "new_" + file_name

new_file = open(output_file, 'w')
new_file.writelines("%s" % line for line in new_file_content)
new_file.close()