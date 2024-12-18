#!/bin/bash

# Diretório onde os arquivos estão localizados
diretorio="v2/resultados"

# Para cada arquivo .txt no diretório
for arquivo in "$diretorio"/*.txt; do
    # Verifica se o arquivo existe
    if [[ -f "$arquivo" ]]; then
        # Cria um arquivo temporário para armazenar as modificações
        temp_file=$(mktemp)
        
        # Remove as linhas que começam com "Coeficientes:" ou "Resíduos:"
        grep -v -E '^Coeficientes:|^Resíduos:' "$arquivo" > "$temp_file"
        
        # Sobrescreve o arquivo original com o conteúdo modificado
        mv "$temp_file" "$arquivo"
        
        echo "Processado: $arquivo"
    fi
done
