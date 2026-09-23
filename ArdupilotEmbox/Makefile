
ap_flags      := $(call option_get,STRING,flags)
ap_board      := $(call option_get,STRING,board)
ap_example    := $(call option_get,STRING,example)
ap_linker_map := $(call option_get,BOOLEAN,linker_map)

ifeq (1,$(call option_get,BOOLEAN,debug))
ap_flags += --debug
else
ap_flags += --disable-Logging
endif

ifeq (0,$(call option_get,BOOLEAN,scripting))
ap_flags += --disable-scripting
endif

ifeq (0,$(call option_get,BOOLEAN,networking))
ap_flags += --disable-networking
endif

ap_src_dir   = $(CURDIR)/../
ap_build_dir = $(CURDIR)/../build/$(ap_board)

$(CONFIGURE):
	cd $(ap_src_dir) && ( \
		export EMBOX_GCC_LINK=full; \
		./waf distclean ; \
		./waf configure \
			--toolchain=$(TOOLCHAIN_DIR)/embox \
			--board $(ap_board) \
			$(ap_flags) \
			--static \
	)
	touch $@

$(BUILD):
ifeq ($(ap_example),none)
	cd $(ap_src_dir) && ( \
		./waf copter \
	)
else
	cd $(ap_src_dir) && ( \
		./waf build --target examples/$(ap_example) \
	)
endif
	touch $@

$(INSTALL):
ifeq ($(ap_example),none)
ifeq ($(ap_linker_map),1)
	cd $(ap_build_dir) && ( \
		EMBOX_GCC_LINK=full $(EMBOX_GCC) -fno-exceptions -Wl,--wrap,malloc \
			bin/arducopter \
			-Wl,--cref -Wl,-Map $(PKG_INSTALL_DIR)/arducopter_full.map \
			-o $(PKG_INSTALL_DIR)/arducopter_full.o \
			-static lib/libArduCopter_libs.a \
	)
endif	
	cd $(ap_build_dir) && ( \
		$(EMBOX_GCC) -fno-exceptions -Wl,--wrap,malloc \
			bin/arducopter \
			-o $(PKG_INSTALL_DIR)/arducopter.o \
			-static lib/libArduCopter_libs.a \
	)
else
	cd $(ap_build_dir) && ( \
		$(EMBOX_GCC) -fno-exceptions -Wl,--wrap,malloc \
			examples/$(ap_example) \
			-o $(PKG_INSTALL_DIR)/$(ap_example).o \
			-static lib/libap.a \
	)
	$(CP) $(PKG_INSTALL_DIR)/$(ap_example).o $(PKG_INSTALL_DIR)/arducopter.o
endif
	touch $@
