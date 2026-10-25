"""The sole Python image writer: one PNG per figure ID, always under figures/."""
import json
from pathlib import Path
from contextlib import contextmanager

from research.common.paths import central_figures

_active_pass = None


@contextmanager
def generation_pass():
    global _active_pass
    if _active_pass is not None:raise RuntimeError('nested image generation pass')
    _active_pass={}
    try:yield _active_pass
    finally:_active_pass=None


class FigureWriter:
    def __init__(self, group):
        self.directory = central_figures(group)
        self.names = []

    def save(self, figure, name, data=None, dpi=220):
        import matplotlib.pyplot as plt
        if name in self.names:
            raise ValueError('figure generated twice in one pass: ' + name)
        if Path(name).name != name or Path(name).suffix:
            raise ValueError('figure ID must be a filename stem')
        self.names.append(name)
        path=self.directory/(name+'.png')
        if _active_pass is not None:
            if str(path) in _active_pass:raise ValueError('duplicate image producer: '+str(path))
            _active_pass[str(path)]=1
        try:
            figure.savefig(path, dpi=dpi, bbox_inches='tight')
        finally:
            plt.close(figure)
        if data is not None:
            (self.directory/(name+'.data.json')).write_text(json.dumps(data,ensure_ascii=False,indent=2),encoding='utf-8')

    def gallery(self, title):
        import html
        (self.directory/'index.html').write_text(
            '<!doctype html><meta charset="utf-8"><title>'+html.escape(title)+
            '</title><style>body{font:16px sans-serif;max-width:1200px;margin:auto}img{max-width:100%}</style><h1>'+html.escape(title)+'</h1>'+''.join(
                '<h2>'+html.escape(name)+'</h2><img src="'+name+'.png" loading="lazy">' for name in self.names),encoding='utf-8')
