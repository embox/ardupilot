
ardupilot_board := $(call option_get,STRING,board)
ardupilot_flags := 

ifeq (1,$(call option_get,BOOLEAN,debug))
ardupilot_flags += --debug
else
ardupilot_flags += --disable-Logging
endif

ifeq (0,$(call option_get,BOOLEAN,scripting))
ardupilot_flags += --disable-scripting
endif

ifeq (0,$(call option_get,BOOLEAN,networking))
ardupilot_flags += --disable-networking
endif

$(CONFIGURE):
	cd ../../ && ( \
		export EMBOX_GCC_LINK=full; \
		./waf distclean ; \
		./waf configure \
			--toolchain=$(TOOLCHAIN_DIR)/embox \
			--board $(ardupilot_board) \
			$(ardupilot_flags) \
			--static \
	)
	touch $@

$(BUILD):
	cd ../../ && ( \
		./waf build --target examples/RCOutput \
	)
	touch $@

$(INSTALL):
	cd ../../build/$(ardupilot_board) && ( \
		$(EMBOX_GCC) -fno-exceptions -Wl,--wrap,malloc \
			examples/RCOutput \
			-o $(PKG_INSTALL_DIR)/RCOutput.o \
			-static lib/libap.a \
	)
	touch $@
