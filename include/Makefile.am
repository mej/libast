nobase_include_HEADERS = libast.h libast/array.h libast/avl_tree.h	\
	libast/condition_if.h libast/dlinked_list.h			\
	libast/iterator_if.h libast/linked_list.h libast/list_if.h	\
	libast/map_if.h libast/mbuff.h libast/module.h			\
	libast/mutex_if.h libast/obj.h libast/objpair.h			\
	libast/pthreads.h libast/regexp.h libast/socket.h libast/str.h	\
	libast/thread_if.h libast/tok.h libast/url.h libast/ustr.h	\
	libast/vector_if.h

nobase_nodist_include_HEADERS = libast/sysdefs.h libast/types.h
noinst_HEADERS = libast_internal.h

libast/types.h: libast/types.h.in
	cd $(top_builddir) && $(SHELL) ./config.status $(subdir)/$@

libast/sysdefs.h: libast/sysdefs.h.in
	cd $(top_builddir) && $(SHELL) ./config.status $(subdir)/$@

MAINTAINERCLEANFILES = Makefile.in
DISTCLEANFILES = libast/sysdefs.h libast/types.h
