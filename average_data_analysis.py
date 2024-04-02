# Average Power Consumption Analysis for ESP-NOW and LoRa
# COPYRIGHT NOTICE: (c) Singapore Institute of Technology
# @author Low Hong Sheng Jovian (2203654), 2024 All rights reserved.

import pandas as pd
import os
import matplotlib.pyplot as plt
import numpy as np

base_path = 'Power_Consumption/'

file_paths = [os.path.join(dirpath, file) for dirpath, _, filenames in os.walk(base_path) for file in filenames]

colors = ['blue', 'green', 'red', 'orange']

categories = {
    'ESP-NOW Node': [],
    'ESP-NOW Master': [],
    'LoRa Node': [],
    'LoRa Master': []
}

for path in file_paths:
    df = pd.read_csv(path)
    filename = os.path.basename(path)
    
    if "ESPNOW_Node" in filename:
        categories['ESP-NOW Node'].append(df)
    elif "ESPNOW_MASTER" in filename:
        categories['ESP-NOW Master'].append(df)
    elif "LORA_Node" in filename:
        categories['LoRa Node'].append(df)
    elif "LORA_MASTER" in filename:
        categories['LoRa Master'].append(df)

states_titles = [
    'ESP-NOW vs LoRa Idle State',
    'ESP-NOW vs LoRa Normal State with 1 Node',
    'ESP-NOW vs LoRa Normal State with 2 Nodes',
    'ESP-NOW vs LoRa Busy State'
]

def plot_averages_for_state(state_index):
    fig, axs = plt.subplots(1, 2, figsize=(12, 6), sharey=True)  # Two plots: one for Node and one for Master
    axs = axs.flatten()

    for i, (category, dfs) in enumerate(categories.items()):
        ax_index = 0 if 'Node' in category else 1
        averages = [df.iloc[:, state_index].mean() for df in dfs] if dfs else [0]
        axs[ax_index].bar(category, np.mean(averages), color=colors[i % len(colors)])
        if ax_index == 0:
            axs[ax_index].set_title(f'Node - {states_titles[state_index]}')  # Title for Node chart
        else:
            axs[ax_index].set_title(f'Master - {states_titles[state_index]}')  # Title for Master chart
    
    plt.suptitle(f'Comparison: {states_titles[state_index]}')
    plt.tight_layout()
    plt.show()

# Plot for each state
for state_index in range(4):
    plot_averages_for_state(state_index)
