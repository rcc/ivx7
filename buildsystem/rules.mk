### Implicit Source Rules ###
$(BUILDDIR)/%.o : %.c
	$(call OUTPUTINFO,CC,$<)
	$(Q)[ -d "$(@D)" ] || mkdir -p "$(@D)"
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(BUILDDIR)/%.o : %.m
	$(call OUTPUTINFO,OBJCC,$<)
	$(Q)[ -d "$(@D)" ] || mkdir -p "$(@D)"
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(BUILDDIR)/%.o : %.cpp
	$(call OUTPUTINFO,C++,$<)
	$(Q)[ -d "$(@D)" ] || mkdir -p "$(@D)"
	$(Q)$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(BUILDDIR)/%.o : %.S
	$(call OUTPUTINFO,AS,$<)
	$(Q)[ -d "$(@D)" ] || mkdir -p "$(@D)"
	$(Q)$(CC) $(CPPFLAGS) -c -o $@ $<

$(BUILDDIR)/%.o : %.s
	$(call OUTPUTINFO,AS,$<)
	$(Q)[ -d "$(@D)" ] || mkdir -p "$(@D)"
	$(Q)$(AS) $(ASFLAGS) -c -o $@ $<

%.a :
	$(call OUTPUTINFO,AR,$@)
	$(Q)[ -d "$(@D)" ] || mkdir -p "$(@D)"
	$(Q)$(AR) crs $@ $^
