### Auto Dependency Rules ###
$(BUILDDIR)/%.d : %.c
	@mkdir -p $(@D)
	@rm -f $@ && \
	$(CC) -MM $(CPPFLAGS) $< | \
	sed 's,\($(*F)\).o[ ]*:[ ]*,$(@D)/\1.o $@ : ,g' > $@

$(BUILDDIR)/%.d : %.cpp
	@mkdir -p $(@D)
	@rm -f $@ && \
	$(CC) -MM $(CPPFLAGS) $< | \
	sed 's,\($(*F)\).o[ ]*:[ ]*,$(@D)/\1.o $@ : ,g' > $@

$(BUILDDIR)/%.d : %.m
	@mkdir -p $(@D)
	@rm -f $@ && \
	$(CC) -MM $(CPPFLAGS) $< | \
	sed 's,\($(*F)\).o[ ]*:[ ]*,$(@D)/\1.o $@ : ,g' > $@

$(BUILDDIR)/%.d : %.S
	@mkdir -p $(@D)
	@rm -f $@ && \
	$(CC) -MM $(CPPFLAGS) $< | \
	sed 's,\($(*F)\).o[ ]*:[ ]*,$(@D)/\1.o $@ : ,g' > $@
