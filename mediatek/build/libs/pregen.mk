MTK_DFO_TARGET_OUT_HEADERS := $(OUT_DIR)/target/product/$(PROJECT)/obj/include/dfo
MTK_DFO_ALL_GENERATED_SOURCES := \
  $(MTK_DFO_TARGET_OUT_HEADERS)/CFG_Dfo_File.h \
  $(MTK_DFO_TARGET_OUT_HEADERS)/CFG_Dfo_Default.h \
  $(MTK_DFO_TARGET_OUT_HEADERS)/DfoDefines.h \
  $(MTK_DFO_TARGET_OUT_HEADERS)/DfoBootDefault.h \
  $(LK_WD)/build-$(PROJECT)/include/dfo/dfo_boot_default.h

ifeq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
  -include $(MTK_DEPENDENCY_OUTPUT)/./CFG_Dfo_File.dep
  -include $(MTK_DEPENDENCY_OUTPUT)/./CFG_Dfo_Default.dep
  -include $(MTK_DEPENDENCY_OUTPUT)/./DfoDefines.dep
  -include $(MTK_DEPENDENCY_OUTPUT)/./DfoBootDefault.dep
else
  .PHONY: $(MTK_DFO_ALL_GENERATED_SOURCES)
endif

$(MTK_DFO_TARGET_OUT_HEADERS)/CFG_Dfo_File.h: $(MTK_ROOT_BUILD)/tools/gendfo.pl $(MTK_ROOT_CONFIG_OUT)/ProjectConfig.mk
ifeq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
	-@echo [Update] $@: $?
endif
	$(hide) echo $(SHOWTIME) gen $@ ...
	$(hide) mkdir -p $(dir $@)
	$(hide) perl $(MTK_ROOT_BUILD)/tools/gendfo.pl nvhdr $@ >$(LOG)$(basename $(notdir $@)).log 2>&1 && \
	 $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$(basename $(notdir $@)).log || \
	 $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$(basename $(notdir $@)).log
	$(hide) touch $@
	$(hide) perl $(MTK_DEPENDENCY_SCRIPT) $(MTK_DEPENDENCY_OUTPUT)/$(basename $(notdir $@)).dep $@ $(dir $(LOG)$(basename $(notdir $@)).log) "\b$(notdir $(LOG)$(basename $(notdir $@)))\.log"

$(MTK_DFO_TARGET_OUT_HEADERS)/CFG_Dfo_Default.h: $(MTK_ROOT_BUILD)/tools/gendfo.pl $(MTK_ROOT_CONFIG_OUT)/ProjectConfig.mk
ifeq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
	-@echo [Update] $@: $?
endif
	$(hide) echo $(SHOWTIME) gen $@ ...
	$(hide) mkdir -p $(dir $@)
	$(hide) perl $(MTK_ROOT_BUILD)/tools/gendfo.pl nvdft $@ >$(LOG)$(basename $(notdir $@)).log 2>&1 && \
	 $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$(basename $(notdir $@)).log || \
	 $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$(basename $(notdir $@)).log
	$(hide) touch $@
	$(hide) perl $(MTK_DEPENDENCY_SCRIPT) $(MTK_DEPENDENCY_OUTPUT)/$(basename $(notdir $@)).dep $@ $(dir $(LOG)$(basename $(notdir $@)).log) "\b$(notdir $(LOG)$(basename $(notdir $@)))\.log"

$(MTK_DFO_TARGET_OUT_HEADERS)/DfoDefines.h: $(MTK_ROOT_BUILD)/tools/gendfo.pl $(MTK_ROOT_CONFIG_OUT)/ProjectConfig.mk
ifeq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
	-@echo [Update] $@: $?
endif
	$(hide) echo $(SHOWTIME) gen $@ ...
	$(hide) mkdir -p $(dir $@)
	$(hide) perl $(MTK_ROOT_BUILD)/tools/gendfo.pl def $@ >$(LOG)$(basename $(notdir $@)).log 2>&1 && \
	 $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$(basename $(notdir $@)).log || \
	 $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$(basename $(notdir $@)).log
	$(hide) touch $@
	$(hide) perl $(MTK_DEPENDENCY_SCRIPT) $(MTK_DEPENDENCY_OUTPUT)/$(basename $(notdir $@)).dep $@ $(dir $(LOG)$(basename $(notdir $@)).log) "\b$(notdir $(LOG)$(basename $(notdir $@)))\.log"

$(MTK_DFO_TARGET_OUT_HEADERS)/DfoBootDefault.h: $(MTK_ROOT_BUILD)/tools/gendfoboot.pl $(MTK_ROOT_CONFIG_OUT)/ProjectConfig.mk
ifeq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
	-@echo [Update] $@: $?
endif
	$(hide) echo $(SHOWTIME) gen $@ ...
	$(hide) mkdir -p $(dir $@)
	$(hide) perl $(MTK_ROOT_BUILD)/tools/gendfoboot.pl bootdft $@ >$(LOG)$(basename $(notdir $@)).log 2>&1 && \
	 $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$(basename $(notdir $@)).log || \
	 $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)$(basename $(notdir $@)).log
	$(hide) touch $@
	$(hide) perl $(MTK_DEPENDENCY_SCRIPT) $(MTK_DEPENDENCY_OUTPUT)/$(basename $(notdir $@)).dep $@ $(dir $(LOG)$(basename $(notdir $@)).log) "\b$(notdir $(LOG)$(basename $(notdir $@)))\.log"

$(LK_WD)/build-$(PROJECT)/include/dfo/dfo_boot_default.h: $(MTK_DFO_TARGET_OUT_HEADERS)/DfoBootDefault.h
	$(hide) mkdir -p $(dir $@)
	$(hide) cp -f $< $@

.PHONY: dfogen
dfogen: $(MTK_DFO_ALL_GENERATED_SOURCES)

