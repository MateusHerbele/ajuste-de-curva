#!/bin/bash

# Valores de N
N1=10
N2=1000

# Valores de K para N1 e N2
Ks_common=(64 128 200 256 512 600 800 1024 2000 3000 4096 6000 7000 10000 50000 100000)
Ks_N1_extra=(1000000 10000000 100000000)

# Arquivo para salvar resultados
RESULTS_DIR="resultados"
mkdir -p $RESULTS_DIR

# Comandos para execução
COMMANDS=(
    "./gera_entrada {K} {N} | ./ajustePol"
    "./gera_entrada {K} {N} | likwid-perfctr -C 7 -g L3CACHE -m -O ./ajustePol"
    "./gera_entrada {K} {N} | likwid-perfctr -C 7 -g ENERGY -m -O ./ajustePol"
    "./gera_entrada {K} {N} | likwid-perfctr -C 7 -g FLOPS_DP -m -O ./ajustePol"
)

# Função para substituir{K} e{N} nos comandos
execute_commands(){
    local K=$1
    local N=$2
    echo "Executando para K=$K, N=$N..."
    for CMD in "${COMMANDS[@]}"; do
        FINAL_CMD=$(echo $CMD | sed "s/{K}/$K/g" | sed "s/{N}/$N/g")
        OUTPUT_FILE="$RESULTS_DIR/$(echo $CMD | awk '{print $1}' | sed 's|[./]||g')_K${K}_N${N}.txt"
        eval "$FINAL_CMD" > "$OUTPUT_FILE" 2>&1
    done
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
