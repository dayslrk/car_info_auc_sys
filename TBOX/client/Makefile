#################################
#
######################################
#source file
#源文件，自动找所有.c和.cpp文件，并将目标定义为同名.o文件
SOURCE  += $(wildcard ./*.c)  
SOURCE  += $(wildcard ./ipcCom/*.c)
SOURCE_CPP  += $(wildcard ./*.cpp)




OBJS    := $(SOURCE:.c=.o)
OBJS_CPP    := $(SOURCE_CPP:.cpp=.o)
  
#target you can change test to what you want
#目标文件名，输入任意你想要的执行文件名
TARGET  := tlsclient
  
#compile and lib parameter
#编译参数
CC      := /usr/local/arm-gcc/bin/arm-linux-gnueabihf-gcc
CXX      := /usr/local/arm-gcc/bin/arm-linux-gnueabihf-g++
#CC      := gcc
LIBS    := -L/home/liruikun/openssl/lib/ -lssl -lcrypto -lpthread -ldl
LDFLAGS :=
DEFINES :=

#include
INCLUDE += -I./
INCLUDE += -I./ipcCom
#INCLUDE += -I/usr/include/x86_64-linux-gnu/
INCLUDE += -I/home/liruikun/openssl/include/ 

CFLAGS  :=  $(INCLUDE)  -Wfatal-errors #-Wall -O3 $(DEFINES) $(INCLUDE)
CXXFLAGS:= $(CFLAGS) -std=gnu++11 -fpermissive
  
  
#i think you should do anything here
#下面的基本上不需要做任何改动了
.PHONY : everything objs clean veryclean rebuild
  
everything : $(TARGET)
  
all : $(TARGET)
  

%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@ 
  
%.o:%.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@ 

rebuild: veryclean everything
                

clean :
	rm -fr *.so
	find . -name "*.o" -exec rm -f {} \; 
veryclean : clean
	rm -fr $(TARGET)
  
$(TARGET) : $(OBJS) $(OBJS_CPP)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(OBJS_CPP) $(LDFLAGS)  $(LIBS) -static
	 
