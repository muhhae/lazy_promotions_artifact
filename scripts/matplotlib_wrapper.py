from os import XATTR_SIZE_MAX
import numpy as np
import seaborn as sns
import pandas as pd
import matplotlib.pyplot as plt
import io
import base64
import matplotlib.ticker as ticker

sns.set_theme(
    style="whitegrid",
    rc={
        "lines.linewidth": 3,  # thicker lines everywhere
        "patch.linewidth": 3,  # bar/box edges thicker
        "axes.linewidth": 3,  # axis spines thicker
    },
)
plt.rcParams.update(
    {
        "axes.edgecolor": "black",
        "xtick.color": "black",
        "ytick.color": "black",
        "axes.labelcolor": "black",
        "text.color": "black",
        "xtick.labelsize": 12,
        "ytick.labelsize": 12,
        # "axes.labelweight": "bold",
    }
)


def heat_map(
    df,
    x,
    y,
    values,
    aggfunc,
    x_label=None,
    y_label=None,
    output_pdf=None,
    fontsize=32,
    x_size=12,
    y_size=7,
    title="",
):
    plt.figure(figsize=(x_size, y_size))
    data = df.pivot_table(
        columns=x,
        index=y,
        values=values,
        aggfunc=aggfunc,
    )
    data = data.sort_index(ascending=True)
    ax = sns.heatmap(
        data,
        annot=True,
        cmap="YlGnBu",
    )

    ax.invert_yaxis()
    ax.set_yticklabels(ax.get_yticklabels(), rotation=0)  # 0° = horizontal
    for _, patch in enumerate(ax.patches):
        patch.set_linewidth(3)
        patch.set_edgecolor("black")

    for line in ax.lines:
        if line.get_linestyle() == "-":
            line.set_color("black")
            line.set_linewidth(3)

    plt.title(title, fontsize=fontsize)

    plt.xlabel(x, fontsize=fontsize) if x_label is None else plt.xlabel(
        x_label, fontsize=fontsize
    )
    plt.ylabel(y, fontsize=fontsize) if y_label is None else plt.ylabel(
        y_label, fontsize=fontsize
    )

    plt.yticks(fontsize=fontsize)

    labels = [t.get_text() for t in ax.get_xticklabels()]
    if labels and max(len(lbl) for lbl in labels) > 8:
        plt.xticks(rotation=-30, ha="center", fontsize=fontsize)
    else:
        plt.xticks(fontsize=fontsize)
    buf = io.BytesIO()
    plt.savefig(buf, format="svg", bbox_inches="tight")
    plt.savefig(
        output_pdf, format="pdf", bbox_inches="tight"
    ) if output_pdf is not None else None
    plt.close()
    buf.seek(0)
    svg_base64 = base64.b64encode(buf.read()).decode("utf-8")
    md = f"![plot](data:image/svg+xml;base64,{svg_base64})"
    return md


def plt_box(
    df: pd.DataFrame,
    x,
    y,
    x_label=None,
    y_label=None,
    whis=[10, 90],
    fontsize=32,
    legend_font_size=28,
    show_legend=False,
    hue=None,
    dodge=True,
    title="",
    tick_step=None,
    showmeans=True,
    width=0.3,
    gap=0.3,
    x_size=None,
    y_size=None,
    output_pdf=None,
    **kwargs,
) -> str:
    tmp = df.sort_values(
        by=x,
        key=lambda col: col.map(
            lambda x: (0, float(x))
            if str(x).replace(".", "", 1).isdigit()
            else (1, str(x))
        ),
    )

    x_size = 2 * len(df[x].unique()) if x_size is None else x_size
    y_size = 7 if y_size is None else y_size
    plt.figure(figsize=(x_size, y_size))

    ax = sns.boxplot(
        data=tmp,
        x=x,
        y=y,
        hue=hue,
        patch_artist=True,
        showfliers=False,
        whis=whis,
        width=width,
        gap=gap,
        showmeans=showmeans,
        # legend=False if not show_legend else None,
        dodge=dodge,
        meanprops=dict(
            marker="o",
            markerfacecolor="white",
            markeredgewidth=2.5,
            markeredgecolor="black",
            markersize=14,
        ),
        medianprops=dict(linestyle="-", linewidth=1.25, color="black"),
        **kwargs,
    )
    for _, patch in enumerate(ax.patches):
        patch.set_linewidth(3)
        patch.set_edgecolor("black")

    for line in ax.lines:
        if line.get_linestyle() == "-":
            line.set_color("black")
            line.set_linewidth(3)

    if hue is None:
        for _, patch in enumerate(ax.patches):
            patch.set_facecolor("lightblue")

        ax.patches[-1].set_facecolor("lightgreen")
        ax.patches[-2].set_facecolor("lightgreen")

    ax.yaxis.set_major_locator(ticker.MaxNLocator(nbins=5))

    if tick_step is not None:
        ymin, ymax = ax.get_ylim()
        ticks_up = np.arange(1, ymax, tick_step)
        ticks_down = np.arange(1, ymin, -tick_step)
        new_ticks = np.unique(np.concatenate([ticks_up, ticks_down]))
        ax.set_yticks(new_ticks)

    if show_legend:
        legend = ax.legend(fontsize=legend_font_size)
        # legend.get_frame().set_edgecolor("black")
        legend.get_frame().set_linewidth(0)  # no border
        legend.get_frame().set_facecolor("none")
    else:
        ax.legend().remove()

    plt.title(title, fontsize=fontsize)
    plt.grid(axis="y", linestyle="--", alpha=0.7)
    plt.grid(True, color="lightgray", linestyle="--", linewidth=2)
    plt.xlabel(x, fontsize=fontsize) if x_label is None else plt.xlabel(
        x_label, fontsize=fontsize
    )
    plt.ylabel(y, fontsize=fontsize) if y_label is None else plt.ylabel(
        y_label, fontsize=fontsize
    )
    plt.yticks(fontsize=fontsize)

    labels = [t.get_text() for t in ax.get_xticklabels()]
    if labels and max(len(lbl) for lbl in labels) > 8:
        plt.xticks(rotation=-30, ha="center", fontsize=fontsize)
    else:
        plt.xticks(fontsize=fontsize)

    plt.tight_layout()
    buf = io.BytesIO()
    plt.savefig(buf, format="svg", bbox_inches="tight")
    plt.savefig(
        output_pdf, format="pdf", bbox_inches="tight"
    ) if output_pdf is not None else None
    plt.close()
    buf.seek(0)
    svg_base64 = base64.b64encode(buf.read()).decode("utf-8")
    md = f"![plot](data:image/svg+xml;base64,{svg_base64})"
    return md


def plt_scatter(
    df: pd.DataFrame,
    x,
    y,
    x_label=None,
    y_label=None,
    fontsize=38,
    legend_font_size=36,
    show_legend=True,
    hue=None,
    markers=None,
    order=None,
    title="",
    tick_step=None,
    marker_size=700,
    palette=None,
    x_size=11,
    y_size=6 * 3.8 / 3,
    output_pdf=None,
    **kwargs,
) -> str:
    plt.figure(figsize=(x_size, y_size))
    ax = sns.scatterplot(
        data=df,
        x=x,
        y=y,
        hue=hue,
        hue_order=order,
        style=hue,
        markers=markers,
        palette=palette,
        s=marker_size,
        edgecolor="black",
    )

    ax.yaxis.set_major_locator(ticker.MaxNLocator(nbins=5))
    ax.xaxis.set_major_locator(ticker.MaxNLocator(nbins=5))

    if tick_step is not None:
        ymin, ymax = ax.get_ylim()
        ticks_up = np.arange(1, ymax, tick_step)
        ticks_down = np.arange(1, ymin, -tick_step)
        new_ticks = np.unique(np.concatenate([ticks_up, ticks_down]))
        ax.set_yticks(new_ticks)

    if show_legend:
        plt.legend(
            loc="upper center",
            bbox_to_anchor=(0.5, 1.3),
            ncol=3,
            frameon=False,
            fontsize=legend_font_size,
        )
        # legend = ax.legend(fontsize=legend_font_size)
        # # legend.get_frame().set_edgecolor("black")
        # legend.get_frame().set_linewidth(0)  # no border
        # legend.get_frame().set_facecolor("none")
    else:
        ax.legend().remove()

    plt.title(title, fontsize=fontsize)
    plt.grid(axis="y", linestyle="--", alpha=0.7)
    plt.grid(True, color="lightgray", linestyle="--", linewidth=2)
    plt.xlabel(x, fontsize=fontsize) if x_label is None else plt.xlabel(
        x_label, fontsize=fontsize
    )
    plt.ylabel(y, fontsize=fontsize) if y_label is None else plt.ylabel(
        y_label, fontsize=fontsize
    )
    plt.yticks(fontsize=fontsize)

    labels = [t.get_text() for t in ax.get_xticklabels()]
    plt.xticks(fontsize=fontsize)

    buf = io.BytesIO()
    plt.savefig(buf, format="svg", bbox_inches="tight")
    plt.savefig(
        output_pdf, format="pdf", bbox_inches="tight"
    ) if output_pdf is not None else None
    plt.close()
    buf.seek(0)
    svg_base64 = base64.b64encode(buf.read()).decode("utf-8")
    md = f"![plot](data:image/svg+xml;base64,{svg_base64})"
    return md
