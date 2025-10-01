import re
import io
from typing_extensions import Doc, Writer
import plotly.express as px
import plotly.graph_objects as go
import os
from glob import glob
from pathlib import Path
import pprint
from typing import Final
import numpy as np
import seaborn as sns

import pandas as pd
from docs_writer import DocsWriter
from style import markers, palette
import matplotlib.pyplot as plt

from matplotlib_wrapper import heat_map, plt_box, plt_scatter

OUTPUT_PATH: Final[str] = "./docs/"
DATA_PATH: Final[str] = "../simulation_results/hashed/"
measurements = {
    "Relative Miss Ratio [LRU]": "Miss ratio relative to LRU",
    "Relative Promotion [LRU]": "Promotions relative to LRU",
    "Relative Miss Ratio [Base FR]": "Miss ratio relative to FR",
    "Relative Promotion [Base FR]": "Promotions relative to FR",
    "Promotion Efficiency": "Promotion efficiency",
}.items()

ALGO: dict[str, str | tuple[str, int]] = {
    "FIFO": "fifo",
    "LRU": "lru",
    "FR": "clock",
    "OLD-D-FR": "dclock",
    "D-FR": "gclock",
    "D3-FR": "d3clock",
}

BASE_ALGO = ["FR"]

# PALETTE = ["lightblue", "lightgreen", "lightpink", "purple", "gray"]
# PALETTE = "pastel"


def PrintAGE(df: pd.DataFrame, writer: DocsWriter):
    writer.Write("# AGE")
    age = df.query("`Algorithm` == 'AGE'")
    clock = df.query("`Algorithm` in @BASE_ALGO and Bit == 1")
    data = pd.concat([age], ignore_index=True)

    # data["Scale"] = data["Scale"].astype(str)
    # data.loc[data["Algorithm"].isin(BASE_ALGO), "Scale"] = data.loc[
    #     data["Algorithm"].isin(BASE_ALGO), "Algorithm"
    # ]
    # data = data.reset_index()
    # print(data)
    # print(age.columns)
    for key, val in measurements:
        writer.Write(f"## AGE: {key}")
        fig = plt_box(
            age,
            y=key,
            y_label=val,
            x="Scale",
            x_label="Factor",
            hue="Algorithm",
            dodge=False,
            palette={"AGE": "lightblue"},
            tick_step=0.2
            if "Relative Promotion" in key
            else 0.015
            if "Miss" in key
            else None,
            x_size=12,
            width=0.7,
            show_legend=False,
            output_pdf=f"./docs/age_scale_{key.replace(' ', '_').lower()}.pdf",
        )
        writer.Write(fig)


def PrintPaperFigures(df: pd.DataFrame, writer: DocsWriter):
    writer.Write("# FOR PAPER")

    age = df.query("`Algorithm` == 'AGE'")
    dclock = df.query("`Algorithm` == 'D-FR'")
    clock = df.query("Algorithm == 'FR'")
    other = df.query("Algorithm in ['Prob','Batch']")
    delay_lru = df.query("Algorithm == 'Delay'")

    data = pd.concat(
        [
            age.query("Scale == 0.5"),
            dclock.query("`Scale` == 0.05 and Bit == 1"),
            clock.query("Bit == 1"),
            delay_lru.query("Scale == 0.2"),
            other,
        ],
        ignore_index=True,
    )
    data = data.reset_index()
    writer.Write("# DCLOCK Delay Ratio Comparison")
    for key, val in measurements:
        writer.Write(f"## Delay Ratio: {key}")
        fig = plt_box(
            dclock.query("Bit == 1"),
            y=key,
            y_label=val,
            x="Scale",
            x_label="Delay ratio",
            hue="Algorithm",
            dodge=False,
            palette={"D-FR": "lightblue"},
            tick_step=0.2
            if "Relative Promotion" in key
            else 0.01
            if "Miss" in key
            else None,
            x_size=12,
            width=0.7,
            show_legend=False,
            output_pdf=f"./docs/dclock_delay_ratio_{key.replace(' ', '_').lower()}.pdf",
        )
        writer.Write(fig)
    writer.Write("# All Algorithm Best")
    # "Relative Miss Ratio [LRU]": "Miss ratio relative to LRU"
    # "Relative Promotion [LRU]": "Promotions relative to LRU"
    lowest = data.loc[data.groupby("Algorithm")["Relative Miss Ratio [LRU]"].idxmin()]
    mean = data.groupby("Algorithm").mean(numeric_only=True).reset_index()
    print(
        lowest[["Algorithm", "Relative Promotion [LRU]", "Relative Miss Ratio [LRU]"]]
    )
    fig = plt_scatter(
        mean,
        y="Relative Promotion [LRU]",
        y_label="Promotions Relative to LRU",
        x="Relative Miss Ratio [LRU]",
        x_label="Miss Ratio Relative to LRU",
        hue="Algorithm",
        markers=markers,
        palette=palette,
        x_size=12,
        fontsize=41,
        order=[
            "Prob",
            "Batch",
            "Delay",
            "FR",
            "D-FR",
            "AGE",
        ],
        output_pdf="./docs/cover_mean.pdf",
    )
    writer.Write(fig)
    fig = plt_scatter(
        lowest,
        y="Relative Promotion [LRU]",
        y_label="Promotions relative to LRU",
        x="Relative Miss Ratio [LRU]",
        x_label="Miss ratio relative to LRU",
        hue="Algorithm",
        markers=markers,
        palette=palette,
        x_size=12,
        order=[
            "Prob",
            "Batch",
            "Delay",
            "FR",
            "D-FR",
            "AGE",
        ],
        # output_pdf=f"./docs/dclock_{key.replace(' ', '_').lower()}.pdf",
    )
    writer.Write(fig)
    fig = plt_scatter(
        data,
        y="Relative Promotion [LRU]",
        y_label="Promotions relative to LRU",
        x="Relative Miss Ratio [LRU]",
        x_label="Miss ratio relative to LRU",
        hue="Algorithm",
        markers=markers,
        palette=palette,
        x_size=12,
        order=[
            "Prob",
            "Batch",
            "Delay",
            "FR",
            "D-FR",
            "AGE",
        ],
        # output_pdf=f"./docs/dclock_{key.replace(' ', '_').lower()}.pdf",
    )
    writer.Write(fig)
    writer.Write("# Advanced Algorithm")
    advanced = df.query("Variant == Variant")
    # print(advanced)
    # exit(0)
    for key, val in list(measurements) + list(
        {
            "Relative Miss Ratio [Adv]": "Miss ratio relative to original",
            "Relative Promotion [Adv]": "Promotions relative to original",
        }.items()
    ):
        writer.Write(f"## {key}")
        for algo in ["ARC", "TwoQ"]:
            fig = plt_box(
                advanced.query("Variant != 'LRU' and Algorithm == @algo"),
                # title=algo,
                # show_legend=True,
                y=key,
                y_label=val.replace("original", algo if algo != "TwoQ" else "2Q"),
                x="Variant",
                x_label="",
                hue="Algorithm",
                dodge=True,
                palette=["#EE964B"],
                tick_step=0.2
                if "Relative Promotion" in key
                else 0.01
                if "Miss" in key
                else None,
                order=[
                    "Prob",
                    "Batch",
                    "Delay",
                    "FR",
                    # "LRU",
                ],
                x_size=12,
                width=0.7,
                fontsize=40,
                legend_font_size=40,
                output_pdf=f"./docs/{algo}_{key.replace(' ', '_').lower()}.pdf",
            )
            writer.Write(fig)
    writer.Write("# All Algorithm Comparison")
    for key, val in measurements:
        writer.Write(f"## {key}")
        fig = plt_box(
            data,
            y=key,
            y_label=val,
            x="Algorithm",
            x_label="",
            hue="Algorithm",
            dodge=False,
            palette=palette,
            tick_step=0.2
            if "Relative Promotion" in key
            else 0.025
            if "Miss" in key
            else None,
            order=[
                "Prob",
                "Batch",
                "Delay",
                "FR",
                "D-FR",
                "AGE",
            ],
            x_size=12,
            width=0.7,
            output_pdf=f"./docs/dclock_{key.replace(' ', '_').lower()}.pdf",
        )
        writer.Write(fig)
    # writer.Write("# DCLOCK Mean/Median Heatmap")
    # for aggfunc in ["mean", "median"]:
    #     writer.Write(f"## dclock {aggfunc} heatmap")
    #     for key, val in measurements:
    #         writer.Write(f"### {key}")
    #         fig = heat_map(
    #             dclock,
    #             x="Bit",
    #             x_label="frequency bits",
    #             y="Scale",
    #             y_label="delay ratio",
    #             title=val,
    #             values=key,
    #             aggfunc=aggfunc,
    #         )
    #         writer.Write(fig)
    writer.Write("# DCLOCK Bit Comparison")
    for dr in [0.05]:
        # print("dr: ", dr)
        data = pd.concat(
            [
                dclock.query("`Scale` == @dr"),
                clock,
            ],
            ignore_index=True,
        )
        data["Bit"] = data["Bit"].astype(int)
        # print(data)
        writer.Write(f"## Delay Ratio : {dr}")
        for key, val in measurements:
            writer.Write(f"### {key}")
            fig = plt_box(
                data,
                y=key,
                y_label=val,
                x="Bit",
                x_label="# Frequency bits",
                hue="Algorithm",
                dodge=True,
                palette=palette,
                tick_step=0.015 if "Miss" in key else None,
                x_size=12,
                width=0.7,
                show_legend=True,
                output_pdf=f"./docs/dclock_{dr}_{key.replace(' ', '_').lower()}.pdf",
            )
            writer.Write(fig)


def GenerateSite(
    title: str,
    df: pd.DataFrame,
):
    if df.empty:
        return

    html_path = os.path.join(
        OUTPUT_PATH,
        f"{title}.html",
    )
    writer = DocsWriter(html_path=html_path, md_path=None)
    PrintAGE(df, writer)
    PrintPaperFigures(df, writer)
    writer.Flush()
    print("Finished generating " + title)


CACHE = "df.feather"
USE_CACHE = True
USE_CACHE = False


def main():
    df = pd.read_feather("./data/processed.feather")
    print(df.columns)
    # delay_lru = df.query("Algorithm == 'Delay' and Scale == 0.1")
    # print(delay_lru)
    # delay_lru = delay_lru.query("`Relative Miss Ratio [LRU]` <= 1")
    # print(delay_lru)
    # exit(0)
    GenerateSite("index", df)


if __name__ == "__main__":
    main()
