bin_PROGRAMS = xpilot-ng-xp-mapedit
AM_CPPFLAGS = -DCONF_DATADIR=\"$(pkgdatadir)/\"
INCLUDES = -I$(top_srcdir)/src/common
xpilot_ng_xp_mapedit_SOURCES = default_colors.h defaults.h events.c expose.c file.c file.h forms.c grow.c help.c main.c main.h map.h proto.h round.c T_Form.c T_Form.h T_Handler.c T_Handler.h tools.c T_Popup.c T_Popup.h T_proto.h T_Toolkit.c T_Toolkit.h
xpilot_ng_xp_mapedit_LDADD = $(top_builddir)/src/common/libxpcommon.a @X_LIBS@ @X_PRE_LIBS@ @X_EXTRA_LIBS@ -lX11
