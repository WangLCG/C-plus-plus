CXX = g++
CFLAG = -O2 -fpermissive -g -std=c++11 -g
PROJECT_SOURCE_DIR=~/BackUp/ffmpeg_stream_push
EXCUTABLE_FILE = demo
INCLUDE = -I $(PROJECT_SOURCE_DIR)/3rdparty/ffmpeg/include

LIBS  = -L $(PROJECT_SOURCE_DIR)/3rdparty/ffmpeg/lib -lavformat
LIBS  += -L $(PROJECT_SOURCE_DIR)/3rdparty/ffmpeg/lib -lavcodec
LIBS  += -L $(PROJECT_SOURCE_DIR)/3rdparty/ffmpeg/lib -lavutil
LIBS  += -L $(PROJECT_SOURCE_DIR)/3rdparty/ffmpeg/lib -lswresample

SOURCE  = $(wildcard  *.cpp)

OBJECTS=$(patsubst %.cpp,%.o,$(SOURCE))
$(EXCUTABLE_FILE) : $(OBJECTS)
	$(CXX)  $(INCLUDE) $(CFLAG) $^ -o $@ $(LIBS)
$(OBJECTS):%.o : %.cpp
	$(CXX)  $(INCLUDE) -c $(CFLAG) $< -o $@ $(LIBS)

.PHONY:clean
clean:
	rm -rf $(OBJECTS) $(EXCUTABLE_FILE)
