# Main file (without extension)
MAIN = report

# LaTeX compiler (XeTeX required for UBI template)
LATEX = xelatex
BIBTEX = bibtex

# Compilation flags
LATEX_FLAGS = -interaction=nonstopmode -halt-on-error

# Default target
all: pdf

# Compile PDF (full process with bibliography)
pdf:
	@echo "Compiling LaTeX document..."
	$(LATEX) $(LATEX_FLAGS) $(MAIN).tex
	@echo "Running BibTeX..."
	$(BIBTEX) $(MAIN)
	@echo "Recompiling for references..."
	$(LATEX) $(LATEX_FLAGS) $(MAIN).tex
	@echo "Final compilation..."
	$(LATEX) $(LATEX_FLAGS) $(MAIN).tex
	@echo "Done! PDF generated: $(MAIN).pdf"

# Quick compile (no bibliography update)
quick:
	@echo "Quick compile..."
	$(LATEX) $(LATEX_FLAGS) $(MAIN).tex

# Clean auxiliary files
clean:
	@echo "Cleaning auxiliary files..."
	rm -f *.aux *.log *.out *.toc *.lof *.lot *.bbl *.blg *.idx *.ilg *.ind *.xdv

# Clean everything including PDF
distclean: clean
	@echo "Removing PDF..."
	rm -f $(MAIN).pdf

# View PDF (requires a PDF viewer)
view: pdf
	@echo "Opening PDF..."
	xdg-open $(MAIN).pdf || open $(MAIN).pdf || start $(MAIN).pdf

.PHONY: all pdf quick clean distclean view
