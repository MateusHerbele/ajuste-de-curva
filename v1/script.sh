#!/bin/bash

# Valores de N
N1=10
N2=1000

# Valores de K para N1 e N2
Ks_common=(64 128 200 256 512 600 800 1024 2000 3000 4096 6000 7000 10000 50000 100000)
Ks_N1_extra=(1000000 10000000 100000000)

# Diretório para salvar resultados
RESULTS_DIR="resultados"
mkdir -p $RESULTS_DIR

# Comandos para execução
COMMANDS=(
    "./gera_entrada {K} {N} | ./ajustePol"
    "./gera_entrada {K} {N} | likwid-perfctr -C 7 -g L3CACHE -m -O ./ajustePol"
    "./gera_entrada {K} {N} | likwid-perfctr -C 7 -g ENERGY -m -O ./ajustePol"
    "./gera_entrada {K} {N} | likwid-perfctr -C 7 -g FLOPS_DP -m -O ./ajustePol"
)

# Função para substituir {K} e {N} nos comandos e gerar arquivos separados
execute_commands(){
    local K=$1
    local N=$2
    echo "Executando para K=$K, N=$N..."

    # Teste sem o likwid
    FINAL_CMD=$(echo "${COMMANDS[0]}" | sed "s/{K}/$K/g" | sed "s/{N}/$N/g")
    OUTPUT_FILE="$RESULTS_DIR/N${N}_K${K}_PADRAO.txt"
    echo "# Executando para K=$K, N=$N" > "$OUTPUT_FILE"
    eval "$FINAL_CMD" >> "$OUTPUT_FILE" 2>&1
    echo "# Fim da execução para K=$K, N=$N" >> "$OUTPUT_FILE"

    # Teste com L3CACHE
    FINAL_CMD=$(echo "${COMMANDS[1]}" | sed "s/{K}/$K/g" | sed "s/{N}/$N/g")
    OUTPUT_FILE="$RESULTS_DIR/N${N}_K${K}_L3CACHE.txt"
    echo "# Executando para K=$K, N=$N" > "$OUTPUT_FILE"
    eval "$FINAL_CMD" >> "$OUTPUT_FILE" 2>&1
    echo "# Fim da execução para K=$K, N=$N" >> "$OUTPUT_FILE"

    # Teste com ENERGY
    FINAL_CMD=$(echo "${COMMANDS[2]}" | sed "s/{K}/$K/g" | sed "s/{N}/$N/g")
    OUTPUT_FILE="$RESULTS_DIR/N${N}_K${K}_ENERGY.txt"
    echo "# Executando para K=$K, N=$N" > "$OUTPUT_FILE"
    eval "$FINAL_CMD" >> "$OUTPUT_FILE" 2>&1
    echo "# Fim da execução para K=$K, N=$N" >> "$OUTPUT_FILE"

    # Teste com FLOPS
    FINAL_CMD=$(echo "${COMMANDS[3]}" | sed "s/{K}/$K/g" | sed "s/{N}/$N/g")
    OUTPUT_FILE="$RESULTS_DIR/N${N}_K${K}_FLOPS.txt"
    echo "# Executando para K=$K, N=$N" > "$OUTPUT_FILE"
    eval "$FINAL_CMD" >> "$OUTPUT_FILE" 2>&1
    echo "# Fim da execução para K=$K, N=$N" >> "$OUTPUT_FILE"
}

# Executar para N1
for K in "${Ks_common[@]}" "${Ks_N1_extra[@]}"; do
    execute_commands $K $N1
done

# Executar para N2
for K in "${Ks_common[@]}"; do
    execute_commands $K $N2
done

echo "Execução concluída. Resultados salvos em $RESULTS_DIR."
