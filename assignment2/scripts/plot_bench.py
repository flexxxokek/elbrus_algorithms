"""
plot_bench.py — строит графики из JSON-вывода Google Benchmark.
Использование:
python plot_bench.py results.json
python plot_bench.py results_slow.json results_fast.json
python plot_bench.py results.json --out /custom/path

Как читать графики:
• Левый subplot (log-log): асимптотика видна как наклон прямой.
Наклон ≈1 → O(n), наклон ≈1.1–1.3 → O(n log n), наклон ≈2 → O(n²).
Пунктирные референсные линии помогают сравнивать.
• Правый subplot (T / (n log n)): если кривая горизонтальна — алгоритм O(n log n).
Рост вверх → хуже, чем n log n; падение → лучше (ближе к O(n)).
"""
import json
import re
import math
import argparse
from pathlib import Path
from collections import defaultdict
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np

# ==============================================================================
# Парсинг аргументов
# ==============================================================================
parser = argparse.ArgumentParser()
parser.add_argument("json_files", nargs="+", help="One or more benchmark JSON files")
parser.add_argument("--show", action="store_true", help="Show plots interactively")
parser.add_argument("--out", default=None, help="Output directory (default: plots/ in project root)")
args = parser.parse_args()

# ==============================================================================
# Папка для графиков
# ==============================================================================
if args.out:
    out_dir = Path(args.out)
else:
    # ИСПРАВЛЕНО: file -> __file__
    out_dir = Path(__file__).resolve().parent.parent / "plots"

out_dir.mkdir(parents=True, exist_ok=True)
print(f"Output directory: {out_dir}")

# ==============================================================================
# Стиль matplotlib
# ==============================================================================
plt.rcParams.update({
    "font.family": "DejaVu Sans",
    "font.size": 11,
    "axes.titlesize": 14,
    "axes.labelsize": 12,
    "legend.fontsize": 10,
    "xtick.labelsize": 10,
    "ytick.labelsize": 10,
    "axes.spines.top": False,
    "axes.spines.right": False,
    "figure.dpi": 150,
})

# ==============================================================================
# Загрузка и мерж всех JSON
# ==============================================================================
all_benchmarks = []
for json_path in args.json_files:
    with open(json_path) as f:
        data = json.load(f)
    all_benchmarks.extend(data["benchmarks"])
    print(f"Loaded {len(data['benchmarks'])} benchmarks from {json_path}")

# ==============================================================================
# Парсинг результатов
# ==============================================================================
results: dict[str, dict[str, list]] = defaultdict(lambda: defaultdict(list))
for bm in all_benchmarks:
    name = bm["name"]
    m = re.search(r"/(\d+)/(\d+)", name)
    if not m:
        continue
    size = int(m.group(1))
    label = bm.get("label", "").strip()
    if not label:
        continue
    sort_name = name.split("/")[0]
    results[sort_name][label].append((size, bm["real_time"]))

for sort_name in results:
    for label in results[sort_name]:
        results[sort_name][label].sort(key=lambda x: x[0])

# ==============================================================================
# Степени сортированности
# ==============================================================================
SORTEDNESS_ORDER = [
    "reverse sorted",
    "20% sorted",
    "40% sorted",
    "60% sorted",
    "80% sorted",
    "fully sorted",
    "random",
]
SORTEDNESS_COLORS = {
    "reverse sorted": "#d62728",
    "20% sorted": "#e07b39",
    "40% sorted": "#e6b800",
    "60% sorted": "#17becf",
    "80% sorted": "#1f77b4",
    "fully sorted": "#2ca02c",
    "random": "#9467bd",
}
SORTEDNESS_MARKERS = {
    "reverse sorted": "D",
    "20% sorted": "v",
    "40% sorted": "s",
    "60% sorted": "P",
    "80% sorted": "^",
    "fully sorted": "o",
    "random": "X",
}
SKIP_INDIVIDUAL = {"RadixSortBase10", "RadixSortBase20", "RadixSortBase100", "StdSort"}

# Алгоритмы с квадратичной сложностью — сравниваем между собой
QUADRATIC_SORTS = {"BubbleSort", "InsertionSort", "SelectionSort", "ShellSort",
                   "CocktailSort", "CocktailShakerSort", "ShakerSort",
                   "GnomeSort", "CombSort", "OddEvenSort"}

# Палитра для квадратичных сортировок
QUADRATIC_PALETTE = [
    "#d62728", "#e07b39", "#e6b800", "#2ca02c",
    "#17becf", "#9467bd", "#8c564b", "#bcbd22",
]

# ==============================================================================
# Вспомогательные функции
# ==============================================================================
def add_reference_lines(ax, xs, ys_all, include_n2=False):
    """
    Рисует референсные линии O(n), O(n log n), и опционально O(n²).
    После отрисовки ylim восстанавливается по данным — референсы
    не растягивают масштаб оси.
    """
    if not xs:
        return
    xs_arr = np.array(xs, dtype=float)
    ref_y = np.median([y for ys in ys_all for y in ys])
    ref_x = np.sqrt(xs_arr[0] * xs_arr[-1])

    def scale(curve):
        c_val = curve(ref_x)
        if c_val <= 0:
            return None
        return (ref_y / c_val) * curve(xs_arr)

    # Сохраняем ylim ДО отрисовки — референсы не должны менять масштаб
    ymin_before, ymax_before = ax.get_ylim()

    REF_COLOR = "#1a1a2e"
    styles = [
        (lambda x: x, "O(n)", (12, 4), 1.8),
        (lambda x: x * np.log2(x + 1), "O(n log n)", (5, 2, 1, 2), 1.8),
    ]
    if include_n2:
        styles.append((lambda x: x ** 2, "O(n²)", (2, 3), 1.8))

    for fn, lbl, dash, lw in styles:
        ys_ref = scale(fn)
        if ys_ref is None:
            continue
        ax.plot(xs_arr, ys_ref,
                linestyle=(0, dash), color=REF_COLOR, linewidth=lw,
                alpha=0.55, zorder=2)
        ax.annotate(
            lbl,
            xy=(xs_arr[-1], ys_ref[-1]),
            xytext=(4, 0), textcoords="offset points",
            va="center", ha="left",
            fontsize=9, fontweight="bold", color=REF_COLOR,
            bbox=dict(boxstyle="round,pad=0.15", fc="white", ec="none", alpha=0.85),
        )

    # Восстанавливаем ylim по данным
    ax.set_ylim(ymin_before, ymax_before)

def log_power_formatter(x, _):
    """Форматирует тики как 10^k."""
    if x <= 0:
        return ""
    exp = math.log10(x)
    if abs(exp - round(exp)) < 0.01:
        return f"$10^{{{int(round(exp))}}}$"
    return f"{x:,.0f}"

def apply_loglog_axes(ax):
    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.xaxis.set_major_formatter(ticker.FuncFormatter(log_power_formatter))
    ax.yaxis.set_major_formatter(ticker.FuncFormatter(
        lambda x, _: f"{x:,.0f}" if x >= 1 else f"{x:.2f}"
    ))
    ax.grid(True, which="major", linestyle="--", alpha=0.4, zorder=0)
    ax.grid(True, which="minor", linestyle=":", alpha=0.2, zorder=0)

# ==============================================================================
# Индивидуальные графики: два subplot — log-log и нормировка T/(n log n)
# ==============================================================================
for sort_name, label_data in results.items():
    if sort_name in SKIP_INDIVIDUAL:
        continue
    fig, (ax_ll, ax_norm) = plt.subplots(1, 2, figsize=(16, 6))

    all_xs_flat = []
    all_ys_flat = []

    # -- Левый subplot: log-log ---------------------------------------------
    for label in SORTEDNESS_ORDER:
        if label not in label_data:
            continue
        pairs = label_data[label]
        xs = np.array([p[0] for p in pairs], dtype=float)
        # ИСПРАВЛЕНО: fo r -> for
        ys = np.array([p[1] for p in pairs], dtype=float)
        all_xs_flat.extend(xs)
        all_ys_flat.append(ys)
        ax_ll.plot(
            xs, ys,
            marker=SORTEDNESS_MARKERS.get(label, "o"),
            color=SORTEDNESS_COLORS.get(label, "#888"),
            linewidth=1.2, markersize=4,
            label=label, zorder=3,
        )

    apply_loglog_axes(ax_ll)

    if all_xs_flat:
        # ИСПРАВЛЕНО: al l_ys_flat -> all_ys_flat
        add_reference_lines(ax_ll, sorted(set(all_xs_flat)), all_ys_flat, include_n2=True)

    ax_ll.set_title("Время vs. n (log-log шкала)", fontweight="bold", fontsize=12)
    ax_ll.set_xlabel("Размер массива n")
    ax_ll.set_ylabel("Время (µs)")

    ax_ll.legend(title="Степень сортированности", loc="upper left", framealpha=0.9)

    # -- Правый subplot: T / (n log n) — «нормированное» время ------------
    for label in SORTEDNESS_ORDER:
        if label not in label_data:
            continue
        # ИСПРАВЛЕНО: pai rs -> pairs
        pairs = label_data[label]
        xs = np.array([p[0] for p in pairs], dtype=float)
        ys = np.array([p[1] for p in pairs], dtype=float)
        norm = xs * np.log2(xs + 1)
        ys_norm = ys / norm * 1e6
        ax_norm.plot(
            xs, ys_norm,
            marker=SORTEDNESS_MARKERS.get(label, "o"),
            color=SORTEDNESS_COLORS.get(label, "#888"),
            linewidth=1.2, markersize=4,
            label=label, zorder=3,
        )

    ax_norm.set_xscale("log")
    ax_norm.xaxis.set_major_formatter(ticker.FuncFormatter(log_power_formatter))
    ax_norm.grid(True, which="major", linestyle="--", alpha=0.4)
    ax_norm.grid(True, which="minor", linestyle=":", alpha=0.2)
    ax_norm.set_title("T / (n·log₂n) — горизонталь = O(n log n)", fontweight="bold", fontsize=12)
    ax_norm.set_xlabel("Размер массива n")
    ax_norm.set_ylabel("T / (n·log₂n) [усл. ед.]")
    ax_norm.legend(title="Степень сортированности", loc="best", framealpha=0.9)

    # Аннотация-подсказка
    ax_norm.annotate(
        "Горизонталь → O(n log n)\nРост → хуже\nПадение → ближе к O(n)",
        xy=(0.97, 0.05), xycoords="axes fraction",
        ha="right", va="bottom", fontsize=8.5,
        bbox=dict(boxstyle="round,pad=0.4", fc="lightyellow", ec="#cccccc", alpha=0.9),
    )

    # ИСПРАВЛЕНО: f "text" -> f"text"
    fig.suptitle(f"Анализ асимптотики: {sort_name}",
                 fontsize=15, fontweight="bold")
    fig.subplots_adjust(left=0.07, right=0.95, top=0.88, bottom=0.12, wspace=0.35)

    path = out_dir / f"{sort_name}.png"
    fig.savefig(path, dpi=150, bbox_inches="tight")
    print(f"Saved: {path}")
    plt.close(fig)

# ==============================================================================
# График: RadixSort (base 10/20/100) vs std::sort
# ==============================================================================
RADIX_STYLES = {
    "RadixSortBase10": ("#2a9d8f", "s", "RadixSort (base 10)"),
    "RadixSortBase20": ("#e76f51", "D", "RadixSort (base 20)"),
    "RadixSortBase100": ("#8338ec", "^", "RadixSort (base 100)"),
    "StdSort": ("#1f77b4", "X", "std::sort"),
}
fig, (ax_ll, ax_norm) = plt.subplots(1, 2, figsize=(16, 6))
has_any = False
all_sizes_flat = []
all_avgs_flat = []

for sort_name, (color, marker, display_name) in RADIX_STYLES.items():
    if sort_name not in results:
        continue
    size_times: dict[int, list[float]] = defaultdict(list)
    for points in results[sort_name].values():
        for size, t in points:
            size_times[size].append(t)

    if not size_times:
        continue

    sizes = np.array(sorted(size_times.keys()), dtype=float)
    avg_times = np.array(
        [sum(size_times[int(s)]) / len(size_times[int(s)]) for s in sizes]
    )
    all_sizes_flat.extend(sizes)
    all_avgs_flat.append(avg_times)

    # log-log
    ax_ll.plot(sizes, avg_times,
               marker=marker, color=color,
               linewidth=1.5, markersize=5,
               label=display_name, zorder=3)

    # нормировка T/(n log n)
    norm = sizes * np.log2(sizes + 1)
    ax_norm.plot(sizes, avg_times / norm * 1e6,
                 marker=marker, color=color,
                 linewidth=1.5, markersize=5,
                 label=display_name, zorder=3)

    has_any = True

if has_any:
    apply_loglog_axes(ax_ll)
    if all_sizes_flat:
        add_reference_lines(ax_ll, sorted(set(all_sizes_flat)), all_avgs_flat)

    ax_ll.set_title("Время vs. n (log-log шкала)", fontweight="bold", fontsize=12)
    ax_ll.set_xlabel("Размер массива n")
    ax_ll.set_ylabel("Среднее время (µs)")

    ax_ll.legend(title="Алгоритм", loc="upper left", framealpha=0.9)

    ax_norm.set_xscale("log")
    ax_norm.xaxis.set_major_formatter(ticker.FuncFormatter(log_power_formatter))
    ax_norm.grid(True, which="major", linestyle="--", alpha=0.4)
    ax_norm.grid(True, which="minor", linestyle=":", alpha=0.2)
    ax_norm.set_title("T / (n·log₂n) — горизонталь = O(n log n)", fontweight="bold", fontsize=12)
    ax_norm.set_xlabel("Размер массива n")
    ax_norm.set_ylabel("T / (n·log₂n) [усл. ед.]")
    ax_norm.legend(title="Алгоритм", loc="best", framealpha=0.9)
    ax_norm.annotate(
        "Горизонталь → O(n log n)\nРост → хуже\nПадение → ближе к O(n)",
        xy=(0.97, 0.05), xycoords="axes fraction",
        ha="right", va="bottom", fontsize=8.5,
        bbox=dict(boxstyle="round,pad=0.4", fc="lightyellow", ec="#cccccc", alpha=0.9),
    )

    fig.suptitle("Сравнение RadixSort (base 10/20/100) и std::sort\n"
                 "(среднее по всем степеням сортированности)",
                 fontsize=15, fontweight="bold")
    fig.subplots_adjust(left=0.07, right=0.95, top=0.85, bottom=0.12, wspace=0.35)

    path = out_dir / "radix_vs_std.png"
    fig.savefig(path, dpi=150, bbox_inches="tight")
    print(f"Saved: {path}")
    plt.close(fig)

# ==============================================================================
# График: сравнение квадратичных сортировок (среднее по всем степеням сортированности)
# ==============================================================================
quad_names = [s for s in results if s in QUADRATIC_SORTS]

if quad_names:
    fig, (ax_ll, ax_norm) = plt.subplots(1, 2, figsize=(16, 6))
    all_sizes_q: list[float] = []
    all_avgs_q: list[np.ndarray] = []

    for idx, sort_name in enumerate(sorted(quad_names)):
        color = QUADRATIC_PALETTE[idx % len(QUADRATIC_PALETTE)]
        marker = list(SORTEDNESS_MARKERS.values())[idx % len(SORTEDNESS_MARKERS)]

        # Собираем все точки (size → список времён по всем случаям)
        size_times: dict[int, list[float]] = defaultdict(list)
        for points in results[sort_name].values():
            for size, t in points:
                size_times[size].append(t)

        if not size_times:
            continue

        sizes = np.array(sorted(size_times.keys()), dtype=float)
        avg_times = np.array(
            [sum(size_times[int(s)]) / len(size_times[int(s)]) for s in sizes]
        )
        all_sizes_q.extend(sizes)
        all_avgs_q.append(avg_times)

        # log-log
        ax_ll.plot(sizes, avg_times,
                   marker=marker, color=color,
                   linewidth=1.5, markersize=5,
                   label=sort_name, zorder=3)

        # нормировка T/(n²)  — горизонталь = O(n²)
        norm_n2 = sizes ** 2
        ax_norm.plot(sizes, avg_times / norm_n2 * 1e9,
                     marker=marker, color=color,
                     linewidth=1.5, markersize=5,
                     label=sort_name, zorder=3)

    apply_loglog_axes(ax_ll)
    if all_sizes_q:
        add_reference_lines(ax_ll, sorted(set(all_sizes_q)), all_avgs_q, include_n2=True)

    ax_ll.set_title("Время vs. n (log-log шкала)", fontweight="bold", fontsize=12)
    ax_ll.set_xlabel("Размер массива n")
    ax_ll.set_ylabel("Среднее время (µs)")
    ax_ll.legend(title="Алгоритм", loc="upper left", framealpha=0.9)

    ax_norm.set_xscale("log")
    ax_norm.xaxis.set_major_formatter(ticker.FuncFormatter(log_power_formatter))
    ax_norm.grid(True, which="major", linestyle="--", alpha=0.4)
    ax_norm.grid(True, which="minor", linestyle=":", alpha=0.2)
    ax_norm.set_title("T / n² — горизонталь = O(n²)", fontweight="bold", fontsize=12)
    ax_norm.set_xlabel("Размер массива n")
    ax_norm.set_ylabel("T / n² [усл. ед.]")
    ax_norm.legend(title="Алгоритм", loc="best", framealpha=0.9)
    ax_norm.annotate(
        "Горизонталь → O(n²)\nРост → хуже O(n²)\nПадение → ближе к O(n log n)",
        xy=(0.97, 0.05), xycoords="axes fraction",
        ha="right", va="bottom", fontsize=8.5,
        bbox=dict(boxstyle="round,pad=0.4", fc="lightyellow", ec="#cccccc", alpha=0.9),
    )

    fig.suptitle("Сравнение квадратичных сортировок\n"
                 "(среднее по всем степеням сортированности)",
                 fontsize=15, fontweight="bold")
    fig.subplots_adjust(left=0.07, right=0.95, top=0.85, bottom=0.12, wspace=0.35)

    path = out_dir / "quadratic_sorts_comparison.png"
    fig.savefig(path, dpi=150, bbox_inches="tight")
    print(f"Saved: {path}")
    plt.close(fig)
else:
    print("No quadratic sort benchmarks found (BubbleSort, InsertionSort, etc.) — skipping comparison chart.")

if args.show:
    plt.show()

print(f"\nDone! All plots saved to: {out_dir.resolve()}")