CPPS= \
	main.cpp \
	parser.cpp \
	cache/cache.cpp \
	cache/interval-cache.cpp \
	cache/size-cache.cpp \
	cache/lru-cache.cpp \


all:
	g++ $(CPPS) -std=c++11 -Wno-format -g -D _FILE_OFFSET_BITS=64
