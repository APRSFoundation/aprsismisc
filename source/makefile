MYPRC = aprs

SRCS = services.c osmimg.c parse.c server.c tcputil.c udplistn.c \
       database.c describe.c globals.c \
       thdelay.c

PRJLIBS = $(CIROOT)/lib/ui.lib $(CIROOT)/lib/rx.lib $(CIROOT)/lib/db.lib $(CIROOT)/lib/ci.lib $(CIROOT)/lib/uf.lib $(CIROOT)/lib/ht.lib $(CIROOT)/lib/xp.lib $(CIROOT)/lib/zl.lib $(CIROOT)/lib/izp.lib

!include $(CIROOT)/rules.$(CIPLATFORM)

ALL:	MYEXE

!include $(CIROOT)/targets.$(CIPLATFORM)
