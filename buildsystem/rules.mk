### Implicit Source Rules ###
$(BUILDDIR)/%.o : %.c
	@mkdir -p $(@D)
	@echo "[CC]        $<"
	$(Q)$(CC)  $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(BUILDDIR)/%.o : %.m
	@mkdir -p $(@D)
	@echo "[OBJCC]     $<"
	$(Q)$(CC)  $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(BUILDDIR)/%.o : %.S
	@mkdir -p $(@D)
	@echo "[AS]        $<"
	$(Q)$(CC) $(CPPFLAGS) -c -o $@ $<

$(BUILDDIR)/%.o : %.s
	@mkdir -p $(@D)
	@echo "[AS]        $<"
	$(Q)$(AS) $(ASFLAGS) -c -o $@ $<

%.a :
	@mkdir -p $(@D)
	@echo "[AR]        $@"
	$(Q)$(AR) crs $@ $^
