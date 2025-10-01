import sys
from glob import glob
import re
import os
from urllib import parse
import pandas as pd
from pathlib import Path

general_pattern = re.compile(
    r".*?\s+(?P<algo>[A-Za-z0-9_]+)"
    r"(?:-(?P<config>[A-Za-z0-9_.-]+))?"
    r"\s+cache size\s+(?P<cache_size>\d+),\s+"
    r"(?P<requests>\d+)\s+req,\s+miss ratio\s+(?P<miss_ratio>\d+(?:\.\d+)?),\s+"
    r"throughput\s+(?P<throughput>\d+(?:\.\d+)?)\s+MQPS,\s+promotion\s+(?P<promotion>\d+)"
)

algorithms = {
    "lpFIFO_batch": "Batch",
    "LRU_delay": "Delay",
    "lpLRU_prob": "Prob",
    "AGE": "AGE",
    "ARC": "ARC",
    "TwoQ": "TwoQ",
    "Clock": "FR",
    "DelayFR": "D-FR",
    "Random": "Random",
    "FIFO": "FIFO",
    "LRU": "LRU",
}


def parse_variant(entry: dict, config: str):
    entry["Variant"] = config


def parse_scale(entry: dict, config: str):
    entry["Scale"] = float(config)


def parse_clock(entry: dict, config: str):
    config_list = config.split("-")
    entry["Bit"] = float(config_list[0])


def parse_dclock(entry: dict, config: str):
    config_list = config.split("-")
    entry["Bit"] = float(config_list[0])
    entry["Scale"] = float(config_list[1])


parse_specific_params = {
    "lpFIFO_batch": parse_scale,
    "LRU_delay": parse_scale,
    "lpLRU_prob": parse_scale,
    "AGE": parse_scale,
    "ARC": parse_variant,
    "TwoQ": parse_variant,
    "Clock": parse_clock,
    "DelayFR": parse_dclock,
    "Random": parse_scale,
    "FIFO": lambda a, b: None,
    "LRU": lambda a, b: None,
}


def parse_line(line: str, filename: str, cache_size: float):
    match = general_pattern.search(line)
    if match:
        d = match.groupdict()
        entry = {
            "Config": d["config"],
            # "Scale": float(d["param"]) if d["param"] is not None else 0,
            # "Variant": d["variant"],
            "Algorithm": algorithms[d["algo"]],
            "Real Cache Size": int(d["cache_size"]),
            "Request": int(d["requests"]),
            "Miss Ratio": float(d["miss_ratio"]),
            "Reinserted": int(d["promotion"]),
            "Trace": filename[filename.rfind("/") + 1 :],
            "Trace Path": filename,
            "Cache Size": 0.01,
            "Ignore Obj Size": 1,
        }
        parse_specific_params[d["algo"]](entry, d["config"])
        return entry
    return None


DATA_PATH = ""
if len(sys.argv) < 2:
    raise ValueError("Missing required argument: data path")

DATA_PATH = sys.argv[1]


def ReadData():
    with open("./datasets.txt", "r") as f:
        paths = f.readlines()
    paths = [p.strip() for p in paths]
    datasets = {
        p[p.rfind("/") + 1 :]: re.sub(r"\.oracleGeneral\S*", "", p) for p in paths
    }
    rows = []
    outputs = Path(DATA_PATH).rglob("*")
    # __import__("pprint").pprint(list(outputs))
    # exit()
    for file in outputs:
        if os.path.isdir(file):
            continue
        with open(file, "r") as f:
            filename = str(file).split(".cachesim", 1)[0]
            for i, line in enumerate(f):
                key = filename[filename.rfind("/") + 1 :]
                if key not in datasets:
                    continue
                row = parse_line(line, datasets[key], 0.1)
                if row:
                    rows.append(row)
    return pd.DataFrame(rows)


data = ReadData()
keys = ["Trace Path", "Cache Size", "Algorithm", "Config"]
data = data.drop_duplicates(subset=keys)
print(data)
print(data["Algorithm"].unique())
data.to_feather("data/data.feather")
data.to_csv("data/data.csv")
