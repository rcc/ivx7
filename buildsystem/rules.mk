### Implicit Source Rules ###
$(BUILDDIR)/%.o : %.c
	@mkdir -p $(@D)
	$(call OUTPUTINFO,CC,$<)
	$(Q)$(CC)  $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(BUILDDIR)/%.o : %.m
	@mkdir -p $(@D)
	$(call OUTPUTINFO,OBJCC,$<)
	$(Q)$(CC)  $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(BUILDDIR)/%.o : %.S
	@mkdir -p $(@D)
	$(call OUTPUTINFO,AS,$<)
	$(Q)$(CC) $(CPPFLAGS) -c -o $@ $<

$(BUILDDIR)/%.o : %.s
	@mkdir -p $(@D)
	$(call OUTPUTINFO,AS,$<)
	$(Q)$(AS) $(ASFLAGS) -c -o $@ $<

%.a :
	@mkdir -p $(@D)
	$(call OUTPUTINFO,AR,$@)
	$(Q)$(AR) crs $@ $^
