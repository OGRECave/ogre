edit = sed \
	-e 's,@OGRE_PLUGINDIR\@,$(libdir)/$(PACKAGE),g' \
	-e 's,@OGRE_MEDIADIR\@,$(top_srcdir)/Samples/Media,g'

CONFIGS = plugins.cfg resources.cfg

install-data-local:
	-$(INSTALL_DATA) $(srcdir)/*.cfg . && \
	for i in ${CONFIGS} ; do \
		$(edit) $(srcdir)/$$i.in > $$i.tmp && \
		$(INSTALL_DATA) $$i.tmp $$i && \
		rm -f $$i.tmp; \
	done
