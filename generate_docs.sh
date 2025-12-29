#!/bin/bash

# ============================================================================
# Script para Gerar Documentação Doxygen - Projeto Maze Game
# ============================================================================

echo "================================================="
echo "  Gerador de Documentação Doxygen - Maze Game"
echo "================================================="
echo ""

# Verificar se o Doxygen está instalado
if ! command -v doxygen &> /dev/null; then
    echo "❌ ERRO: Doxygen não está instalado!"
    echo ""
    echo "Para instalar:"
    echo "  sudo apt install doxygen graphviz"
    echo ""
    exit 1
fi

# Verificar se o arquivo Doxyfile existe
if [ ! -f "Doxyfile" ]; then
    echo "❌ ERRO: Arquivo Doxyfile não encontrado!"
    echo "Execute este script na pasta raiz do projeto."
    exit 1
fi

echo "✓ Doxygen encontrado: $(doxygen --version)"
echo ""

# Criar diretório de saída se não existir
mkdir -p docs/doxygen

echo "📝 Gerando documentação..."
echo ""

# Gerar documentação
doxygen Doxyfile

# Verificar se a geração foi bem-sucedida
if [ $? -eq 0 ]; then
    echo ""
    echo "================================================="
    echo "✅ Documentação gerada com sucesso!"
    echo "================================================="
    echo ""
    echo "Localização: docs/doxygen/html/"
    echo "Arquivo principal: docs/doxygen/html/index.html"
    echo ""
    echo "Para visualizar, execute:"
    echo "  xdg-open docs/doxygen/html/index.html"
    echo ""
    
    # Perguntar se deseja abrir automaticamente
    read -p "Deseja abrir a documentação agora? (s/n): " -n 1 -r
    echo ""
    if [[ $REPLY =~ ^[SsYy]$ ]]; then
        xdg-open docs/doxygen/html/index.html &
        echo "✓ Abrindo documentação no browser..."
    fi
else
    echo ""
    echo "================================================="
    echo "❌ ERRO ao gerar documentação!"
    echo "================================================="
    echo ""
    echo "Verifique as mensagens de erro acima."
    exit 1
fi
