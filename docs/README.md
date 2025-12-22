# Relatório do Projeto - Jogo de Labirinto 3D

Este diretório contém o relatório LaTeX do projeto de Computação Gráfica.

## Estrutura

- `report.tex` - Ficheiro principal do relatório
- `Introduction.tex` - Capítulo de Introdução
- `Technologies.tex` - Capítulo de Tecnologias Utilizadas
- `Implementation.tex` - Capítulo de Desenvolvimento e Implementação
- `Conclusion.tex` - Capítulo de Conclusão e Trabalho Futuro
- `bibliography.bib` - Bibliografia em formato BibTeX
- `template-ubi/` - Template LaTeX da UBI
- `Makefile` - Automatização da compilação

## Autores

- Bruno Correia (51741)
- Henrique Laia (51742)

**Professores**: Abel Gomes, João Dias

**Nota**: A implementação do algoritmo de Kruskal foi adaptada de [Ferenc Nemeth's maze-generation-algorithms](https://github.com/ferenc-nemeth/maze-generation-algorithms).

## Compilação

### Usando Make (recomendado)

```bash
make        # Compila o relatório completo
make quick  # Compilação rápida (sem bibliografia)
make clean  # Remove ficheiros auxiliares
make view   # Compila e abre o PDF
```

### Compilação Manual

```bash
pdflatex report.tex
bibtex report
pdflatex report.tex
pdflatex report.tex
```

## Requisitos

- LaTeX (TeX Live, MiKTeX, ou MacTeX)
- BibTeX
- Fontes utilizadas no template UBI

## Notas

- O relatório está configurado em Português
- Para mudar para Inglês, comentar a linha `\portugues` em `report.tex`
- Atualizar o nome do professor em `report.tex` antes da compilação final
