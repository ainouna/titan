LIB = netlib.a

OBJS = list.o nbtscan.o range.o showmount.o smbinfo.o statusq.o

DEPS = $(patsubst %.o,%.d,$(OBJS))

#CC = $(HOME)/flashimg/BUILDGIT/checkout_mips360/builds/titannit/inihdp/tmp/sysroots/x86_64-linux/usr/bin/mipsel-oe-linux/mipsel-oe-linux-gcc
#AR = $(HOME)/flashimg/BUILDGIT/checkout_mips360/builds/titannit/inihdp/tmp/sysroots/x86_64-linux/usr/bin/mipsel-oe-linux/mipsel-oe-linux-ar

#CFLAGS = -Os -Wall -fPIC -I. -I $SRC/tmp/sysroots/$BOX/usr/include
CFLAGS = -Os -Wall -fPIC -I. -I $(sysroot)/usr/include

all: $(LIB)

$(LIB): $(OBJS) Makefile
	$(AR) ru  $@ $(OBJS)
	ranlib $@

%.o: %.c
	$(CC) -MD $(CFLAGS) -c $<

clean:
	rm $(OBJS) $(DEPS) $(LIB)


-include $(DEPS)
