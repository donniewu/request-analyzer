import os

source_size_arr = ['60G']
target_size_arr = ['60G', '80G', '100G', '120G', '140G', '160G', '180G', '200G',
'250G', '300G', '350G', '400G', '450G', '500G', '600G', '700G', '800G', '900G', '1024G']

#source_size_arr = ['1T', '1T1']
#target_size_arr = ['1T']

lines_map = {}

for source in source_size_arr:
	if source not in lines_map:
		lines_map[source] = {}

	for target in target_size_arr:
		path = 'stat/' + source + '_' + target + '_stat.txt'

		f = open(path, 'r')

		lines_map[source][target] = []

		for line in f:
			lines_map[source][target].append(int(line))

		f.close()

outputs = []
for i in range(0, len(lines_map['60G']['1024G'])):
	line = ''
	for source in source_size_arr:
		for target in target_size_arr:
			line += ('%5d ' % lines_map[source][target][i])

	print(line)
