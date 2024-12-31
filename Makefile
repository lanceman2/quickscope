# This file is for building and installing quickscope with quickbuild.

SUBDIRS :=\
 include\
 lib/quickscope/run\
 lib\
 bin\
 share/bash-completion/completions\
 share/doc/quickscope

ifneq ($(wildcard quickbuild.make),quickbuild.make)
$(error "First run './bootstrap'")
endif
ifneq ($(wildcard config.make),config.make)
$(error "Now run './configure'")
endif



ifeq ($(strip $(subst cleaner, clean, $(MAKECMDGOALS))),clean)
SUBDIRS +=\
 tests
endif


test:
	$(MAKE) && cd tests && $(MAKE) test


include quickbuild.make
