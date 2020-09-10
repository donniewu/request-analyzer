import os

#source_size_arr = ['60G', '120G', '300G', '1T']
#target_size_arr = ['60G', '120G', '300G', '1T']

#source_size_arr = ['60G']
#target_size_arr = ['60G', '80G', '100G', '120G', '140G', '160G', '180G', '200G',
#'250G', '300G', '350G', '400G', '450G', '500G', '600G', '700G', '800G', '900G', '1T']

source_size_arr = ['1T', '1T1', '1T2']
target_size_arr = ['1T']

for source in source_size_arr:
	for target in target_size_arr:
		path = 'result/' + source + '/' + target
		os.system('./grep.sh ' + path + '/1.txt > first')
		os.system('./grep.sh ' + path + '/2.txt > second')
		os.system('./a.out > stat/' + source + '_' + target + '_stat.txt')
