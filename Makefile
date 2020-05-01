TARGET_NAME = hexdump
OBJS =   hexdump.o

INCLUDE_PATH = -O2 -Wall -I./
CFLAGS += $(INCLUDE_PATH)

LIB_PATH = 
LIB_LINK = 
LFLAGS += $(LIB_PATH) $(LIB_LINK)  

all:	$(TARGET_NAME)

$(TARGET_NAME):	$(OBJS) 
	$(CXX) $^ -o $@ $(LFLAGS)	

%.o:	$(PROJECT_ROOT)%.cpp
	$(CXX) -c $(CFLAGS) $(CXXFLAGS) $(CPPFLAGS) -o $@ $<

%.o:	$(PROJECT_ROOT)%.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

clean:
	rm -fr $(TARGET_NAME) $(OBJS) 
