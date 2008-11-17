PRJNAME=Sample
DOCNAME=sample
SRCTEXI=$(DOCNAME).texi
OUTDIR=output

.PHONY : all
all : html single_html pdf info plaintext index

.PHONY : html
html : $(OUTDIR)/$(DOCNAME)

.PHONY : single_html
single_html : $(OUTDIR)/$(DOCNAME).html

.PHONY : pdf
pdf : $(OUTDIR)/$(DOCNAME).pdf

.PHONY : info
info : $(OUTDIR)/$(DOCNAME).info

.PHONY : plaintext
plaintext : $(OUTDIR)/$(DOCNAME).txt

.PHONY : index
index : $(OUTDIR)/index.html

$(OUTDIR):
	mkdir -p $@

$(OUTDIR)/$(DOCNAME).info : $(SRCTEXI) $(OUTDIR)
	makeinfo $< -o $@

$(OUTDIR)/$(DOCNAME).txt : $(SRCTEXI) $(OUTDIR)
	makeinfo --plaintext $< -o $@

$(OUTDIR)/$(DOCNAME) : $(SRCTEXI) $(OUTDIR)
	makeinfo --html $< -o $@

$(OUTDIR)/$(DOCNAME).html : $(SRCTEXI) $(OUTDIR)
	makeinfo --html --no-split $< -o $@

$(OUTDIR)/$(DOCNAME).pdf : $(SRCTEXI) $(OUTDIR)
	texi2pdf --clean -I inc -o $@ $<

$(OUTDIR)/index.html : $(OUTDIR)
	./mkdocindex $(PRJNAME) $(DOCNAME) > $@

.PHONY : clean
clean:
	rm -rf $(OUTDIR) *.log
