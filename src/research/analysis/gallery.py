"""Refresh the persisted catalog without losing verified report links."""
from research.common.paths import FIGURES
from research.analysis.catalog import render as render_catalog


def render():
    return render_catalog(FIGURES)


if __name__=='__main__':render()
