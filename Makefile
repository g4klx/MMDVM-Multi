#
CXX      = c++
CXXFLAGS += -g -O3 -Wall -Wextra -Wpedantic -std=gnu++17 -fpermissive -MMD -MD -pthread
LIBS     += -lpthread -lmosquitto -lSoapySDR -lliquid
LDFLAGS  += -g -L/usr/local/lib

ifeq ($(shell uname -s),Darwin)
	CFLAGS+= -I/opt/homebrew/include -Wno-gnu-zero-variadic-macro-arguments
	CXXFLAGS+= -I/opt/homebrew/include -Wno-gnu-zero-variadic-macro-arguments
	LDFLAGS+= -L/opt/homebrew/lib
endif

CXXSRCS = $(wildcard *.cpp)
CXXDEPS = $(CXXSRCS:.cpp=.d)
CXXOBJS = $(CXXSRCS:.cpp=.o)

all:		MMDVM-Multi

MMDVM-Multi:	$(CXXOBJS)
		$(CXX) $(CXXOBJS) $(LDFLAGS) $(LIBS) -o MMDVM-Multi

%.o: %.cpp
		$(CXX) $(CXXFLAGS) -c -o $@ $<
-include $(CXXDEPS)

MMDVM-Multi.o: GitVersion.h FORCE

.PHONY: GitVersion.h

FORCE:

install:
		install -m 755 MMDVM-Multi /usr/local/bin/
		install -m 644 MMDVM-Multi.ini /etc

clean:
		$(RM) MMDVM-Multi *.o *.d *.bak *~ GitVersion.h

# Export the current git version if the index file exists, else 000...
GitVersion.h:
ifneq ("$(wildcard .git/index)","")
	echo "const char *gitversion = \"$(shell git rev-parse HEAD)\";" > $@
else
	echo "const char *gitversion = \"0000000000000000000000000000000000000000\";" > $@
endif
