.PHONY: main.pdf

main.pdf:
	latexmk -pdf -pvc -latexoption=-halt-on-error \
		-latexoption=-file-line-error \
		-latexoption=-interaction=nonstopmode \
		-latexoption=-synctex=1 main.tex || ! rm -f $@

.PHONY: clean
clean:
	latexmk -C main
