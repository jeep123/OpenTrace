bin_PROGRAMS += udatapath/ofdatapath
man_MANS += udatapath/ofdatapath.8

udatapath_ofdatapath_SOURCES = \
	udatapath/dump_flow/dump_flow_stats.h \
	udatapath/dump_flow/dump_flow_stats.c \
	udatapath/dump_flow/dump_flow_print.h \
	udatapath/dump_flow/dump_flow_print.c \
	udatapath/dump_flow/dump_flow_client.h \
	udatapath/dump_flow/dump_flow_client.c \
	udatapath/dump_flow/dump_flow_struct.h \
	udatapath/dump_flow/dump_flow_server_IP.conf \
	udatapath/chain.c \
	udatapath/chain.h \
	udatapath/crc32.c \
	udatapath/crc32.h \
	udatapath/datapath.c \
	udatapath/datapath.h \
	udatapath/dp_act.c \
	udatapath/dp_act.h \
	udatapath/of_ext_msg.c \
	udatapath/of_ext_msg.h \
	udatapath/udatapath.c \
	udatapath/private-msg.c \
	udatapath/private-msg.h \
	udatapath/switch-flow.c \
	udatapath/switch-flow.h \
	udatapath/table.h \
	udatapath/table-hash.c \
	udatapath/table-linear.c 

udatapath_ofdatapath_LDADD = lib/libopenflow.a $(SSL_LIBS) $(FAULT_LIBS)

EXTRA_DIST += udatapath/ofdatapath.8.in
DISTCLEANFILES += udatapath/ofdatapath.8
