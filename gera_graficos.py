import os
import re
import matplotlib.pyplot as plt
import numpy as np

# Diretórios principais
data_dirs = {"v1": "v1/resultados", "v2": "v2/resultados"}
output_dir = "graficos"
subfolders = ["PADRAO", "FLOPS", "ENERGY", "L3CACHE"]

# Cria as pastas de saída
os.makedirs(output_dir, exist_ok=True)
for folder in subfolders:
    os.makedirs(os.path.join(output_dir, folder), exist_ok=True)

# Função para processar arquivos
results = {"PADRAO": {}, "FLOPS": {}, "ENERGY": {}, "L3CACHE": {}}

# Função para processar arquivos e separar os dados por análise
def process_file(version, filepath):
    match = re.match(r"N(\d+)_K(\d+)_(\w+).txt", os.path.basename(filepath))
    if not match:
        print(f"Nome de arquivo inválido: {filepath}")
        return

    N, K, metric = match.groups()
    K = int(K)  # O K já é extraído do nome do arquivo
    N = int(N)  # Convertendo N para inteiro
    if metric not in results:
        return

    with open(filepath, "r") as file:
        content = file.read()

    # Imprimir conteúdo do arquivo para verificação
    print(f"Processando {filepath} para {version}")
    print(content)

    # Verificar se é o arquivo PADRAO para TEMPO
    if "PADRAO" in os.path.basename(filepath):
        match_t = re.search(r"Tempos\(K, tSL, tEG\):\d+ ([\d.e+-]+) ([\d.e+-]+)", content)
        if match_t:
            tSL, tEG = map(float, match_t.groups())
            print(f"PADRAO: tSL={tSL}, tEG={tEG}")  # Verifica se tSL e tEG estão sendo extraídos corretamente
            results["PADRAO"].setdefault(K, {}).setdefault(version, {}).setdefault(N, {"SL": tSL, "EG": tEG})

    # Para a métrica ENERGY, procurar no arquivo correspondente
    elif metric == "ENERGY":
        for region in ["SL_MMQ", "EG"]:
            match_e = re.search(rf"TABLE,Region {region},Group 1 Metric,ENERGY,13.*?Energy PP0 \[J\],([\d.e+-]+)", content, re.S)
            if match_e:
                energy = float(match_e.group(1))
                print(f"ENERGY {region}: {energy}")  # Verifica se o valor de energy está sendo extraído corretamente
                results["ENERGY"].setdefault(K, {}).setdefault(version, {}).setdefault(N, {}).setdefault(region, energy)

    # Para FLOPS, capturando os valores de DP e AVX DP
    elif metric == "FLOPS":
        for region in ["SL_MMQ", "EG"]:
            match_f = re.search(rf"TABLE,Region {region},Group 1 Metric,FLOPS_DP,9.*?DP \[MFLOP/s\],([\d.e+-]+).*?AVX DP \[MFLOP/s\],([\d.e+-]+)", content, re.S)
            if match_f:
                dp, avx_dp = map(float, match_f.groups())
                print(f"FLOPS {region}: DP={dp}, AVX_DP={avx_dp}")  # Verifica se os valores de FLOPS estão sendo extraídos corretamente
                results["FLOPS"].setdefault(K, {}).setdefault(version, {}).setdefault(N, {}).setdefault(region, {}).setdefault("DP", dp)
                results["FLOPS"][K][version][N][region].setdefault("AVX_DP", avx_dp)

    # Para CACHE, verificar o arquivo L3CACHE
    elif metric == "L3CACHE":
        for region in ["SL_MMQ", "EG"]:
            match_c = re.search(rf"TABLE,Region {region},Group 1 Metric,L3CACHE,7.*?L3 miss ratio,([\d.e+-]+)", content, re.S)
            if match_c:
                cache_miss_ratio = float(match_c.group(1))
                print(f"L3CACHE {region}: {cache_miss_ratio}")  # Verifica se o valor de L3CACHE está sendo extraído corretamente
                results["L3CACHE"].setdefault(K, {}).setdefault(version, {}).setdefault(N, {}).setdefault(region, cache_miss_ratio)

# Função para gerar os gráficos
def plot_graph(data, title, ylabel, save_path=None, log_y=False, region=None, metric=None, flops_type=None):
    K_values = sorted(data.keys())

    # Definir as quatro linhas para as combinações de versão + grau
    N1_v1 = []
    N1_v2 = []
    N2_v1 = []
    N2_v2 = []

    # Pega os valores dependendo da análise (SL_MMQ, EG, etc.)
    for K in K_values:
        if metric == "FLOPS":
            N1_v1.append(data[K].get("v1", {}).get(10, {}).get(region, {}).get(flops_type, 0))
            N1_v2.append(data[K].get("v2", {}).get(10, {}).get(region, {}).get(flops_type, 0))
            N2_v1.append(data[K].get("v1", {}).get(1000, {}).get(region, {}).get(flops_type, 0))
            N2_v2.append(data[K].get("v2", {}).get(1000, {}).get(region, {}).get(flops_type, 0))
        else:
            N1_v1.append(data[K].get("v1", {}).get(10, {}).get(region, 0))
            N1_v2.append(data[K].get("v2", {}).get(10, {}).get(region, 0))
            N2_v1.append(data[K].get("v1", {}).get(1000, {}).get(region, 0))
            N2_v2.append(data[K].get("v2", {}).get(1000, {}).get(region, 0))

    # Printar os valores para verificação
    print(f"Valores para {title} - {region} - {flops_type}:")
    print("K_values:", K_values)
    print("N1_v1:", N1_v1)
    print("N1_v2:", N1_v2)
    print("N2_v1:", N2_v1)
    print("N2_v2:", N2_v2)

    # Configura o gráfico
    plt.figure(figsize=(10, 6))
    plt.plot(K_values, N1_v1, label="N1+v1", marker="o", color="olive")
    plt.plot(K_values, N1_v2, label="N1+v2", marker="^", color="orange")
    plt.plot(K_values, N2_v1, label="N2+v1", marker="s", color="green")
    plt.plot(K_values, N2_v2, label="N2+v2", marker="d", color="red")

    # Configurações do eixo
    plt.xscale("log")
    if log_y:
        plt.yscale("log")

    plt.xlabel("K (tamanho da entrada)")
    plt.ylabel(ylabel)
    plt.title(f"{title} - {region}")
    plt.legend()
    plt.grid(True, which="both", linestyle="--", linewidth=0.5)
    plt.tight_layout()

    # Salva ou mostra o gráfico
    if save_path:
        plt.savefig(save_path)
    else:
        plt.show()

# Processar todos os arquivos nos diretórios
for version, dir_path in data_dirs.items():
    print(f"Lendo arquivos do diretório {dir_path} para {version}")
    for root, _, files in os.walk(dir_path):
        for file in files:
            process_file(version, os.path.join(root, file))

# Gerar gráficos para cada análise
for metric, data in results.items():
    folder = os.path.join(output_dir, metric)
    if metric == "L3CACHE":
        # Gerar gráficos para as regiões SL e EG com título simplificado e y-label ajustado
        plot_graph(data, "L3CACHE", "L3 CACHE MISS RATIO", save_path=os.path.join(folder, "l3cache_sl.png"), region="SL_MMQ", metric=metric)
        plot_graph(data, "L3CACHE", "L3 CACHE MISS RATIO", save_path=os.path.join(folder, "l3cache_eg.png"), region="EG", metric=metric)
    elif metric == "ENERGY":
        # Gerar gráficos para as regiões SL e EG
        plot_graph(data, "ENERGY", "ENERGY [J]", save_path=os.path.join(folder, "energy_sl.png"), region="SL_MMQ", metric=metric)
        plot_graph(data, "ENERGY", "ENERGY [J]", save_path=os.path.join(folder, "energy_eg.png"), region="EG", metric=metric)
    elif metric == "FLOPS":
        # Gerar gráficos para DP SL_MMQ, DP EG, AVX DP SL_MMQ, AVX DP EG
        plot_graph(data, "FLOPS DP", "FLOPS [MFLOP/s]", save_path=os.path.join(folder, "flops_dp_slmmq.png"), region="SL_MMQ", metric=metric, flops_type="DP")
        plot_graph(data, "FLOPS DP", "FLOPS [MFLOP/s]", save_path=os.path.join(folder, "flops_dp_eg.png"), region="EG", metric=metric, flops_type="DP")
        plot_graph(data, "FLOPS AVX DP", "FLOPS [MFLOP/s]", save_path=os.path.join(folder, "flops_avx_dp_slmmq.png"), region="SL_MMQ", metric=metric, flops_type="AVX_DP")
        plot_graph(data, "FLOPS AVX DP", "FLOPS [MFLOP/s]", save_path=os.path.join(folder, "flops_avx_dp_eg.png"), region="EG", metric=metric, flops_type="AVX_DP")
    elif metric == "PADRAO":
        # Padrao (TEMPO SL, EG) com escala logarítmica
        plot_graph(data, "TEMPO", "Tempo [s]", save_path=os.path.join(folder, "padrao_sl.png"), region="SL", log_y=True)
        plot_graph(data, "TEMPO", "Tempo [s]", save_path=os.path.join(folder, "padrao_eg.png"), region="EG", log_y=True)

print("Gráficos gerados e salvos em:", output_dir)