CC=g++
CFLAGS= -O3 -I/usr/include/ImageMagick
#default version
VERSION=release
LDFLAGS= -Wl,-L/usr/lib64  -lboost_regex -lboost_program_options  -lboost_date_time -lboost_system -lboost_thread -lcurl  -lssl -lboost_filesystem -lboost_locale `Magick++-config --cppflags --cxxflags --ldflags --libs`
EXEC=crunch
EXEC_DEBUG = crunchd
#list of source files

OBJDIR=build/release
OBJDIR_DEBUG=build/debug
SRC= $(wildcard src/*.cpp) $(wildcard src/json_spirit/*.cpp) $(wildcard  src/pugixml-1.2/*.cpp)
OBJ= $(patsubst src/%.cpp,$(OBJDIR)/%.o,$(SRC))
OBJ_DEBUG = $(patsubst src/%.cpp,$(OBJDIR_DEBUG)/%.o,$(SRC))

BINARYDIR=bin/release
BINARYDIRDEBUG=bin/debug
all: createdir $(EXEC)

debug: CFLAGS = -g -D__DEBUG__=1 -I/usr/include/ImageMagick
debug: createdir $(EXEC_DEBUG)

createdir: 
	@mkdir -p bin
	@mkdir -p build
	@mkdir -p $(OBJDIR)
	@mkdir -p $(BINARYDIR)
	@mkdir -p $(OBJDIR_DEBUG)/json_spirit
	@mkdir -p $(OBJDIR_DEBUG)/pugixml-1.2
	@mkdir -p $(OBJDIR)/json_spirit
	@mkdir -p $(OBJDIR)/pugixml-1.2

$(EXEC):  $(OBJ)
	$(CC) -o $(BINARYDIR)/$(EXEC) $(OBJ) $(LDFLAGS)

$(EXEC_DEBUG):  $(OBJ_DEBUG)
	$(CC) -o $(BINARYDIRDEBUG)/$(EXEC) $(OBJ_DEBUG) $(LDFLAGS)

$(OBJDIR)/%.o:  src/%.cpp 
	$(CC) -o $@ -c $^ $(CFLAGS)

$(OBJDIR_DEBUG)/%.o:  src/%.cpp 
	$(CC) -o $@ -c $^ $(CFLAGS)



clean:
	rm -rf build/release/*.o
	rm -rf bin/release/crunch
	rm -rf zip/release/crunch.zip

cleandebug:
	rm -rf build/debug/*.o
	rm -rf bin/debug/crunch
	rm -rf zip/debug/crunch.zip

cleanall: clean cleandebug

ziprelease:
	rm -rf zip/release/crunch.zip
	zip -j zip/release/crunch.zip bin/release/crunch
zipdebug:
	rm -rf zip/debug/crunch.zip
	zip -j zip/debug/crunch.zip bin/debug/crunch

help:
	@echo "---------------------------------------------"
	@echo "*          Avalaible make options           *"
	@echo "---------------------------------------------"
	@echo "* all        : build bin/release/crunch     *"
	@echo "* debug      : build bin/debug/crunch       *"
	@echo "* clean      : clean release obj and binary *"
	@echo "* cleandebug : clean release obj and binary *"
	@echo "* cleanall   : cleandebug and clean         *"
	@echo "---------------------------------------------"
	