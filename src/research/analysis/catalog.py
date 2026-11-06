"""One persisted catalog for Qt images, report links and the HTML index."""
import html
import json
from pathlib import Path
from urllib.parse import quote

from research.common.hashing import digest
from research.common.paths import ROOT,FIGURES


def publish(batch,images,reports,scope,figures=FIGURES):
    batch,figures=Path(batch).resolve(),Path(figures).resolve()
    for item in images:
        path=(figures/item['path']).resolve()
        if not path.is_relative_to(figures) or digest(path)!=item['sha256']:raise ValueError('Unverified catalog image')
    links=[]
    for title,path in reports:
        path=Path(path).resolve()
        if not path.is_relative_to(ROOT) or not path.is_file():raise ValueError('Missing catalog report')
        links.append(dict(title=title,path=path.relative_to(ROOT).as_posix(),sha256=digest(path)))
    state=dict(schema='research_catalog_v1',batch=batch.relative_to(ROOT).as_posix(),scope=scope,
               image_count=len(images),images=images,reports=links)
    figures.mkdir(parents=True,exist_ok=True)
    (figures/'current.catalog.json').write_text(json.dumps(state,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    (figures/'current.tsv').write_text(''.join(f"{i['path']}\t{i['title']}\t{i['description']}\n" for i in images),encoding='utf-8')
    (figures/'current.meta.tsv').write_text(f"{state['batch']}\t{len(images)}\t{scope}\n",encoding='utf-8')
    render(figures)
    return state


def render(figures=FIGURES):
    figures=Path(figures);catalog=figures/'current.catalog.json'
    if catalog.exists():state=json.loads(catalog.read_text(encoding='utf-8'))
    else:
        from html.parser import HTMLParser
        state=dict(batch='legacy',scope='legacy',images=[],reports=[])
        listing=figures/'current.tsv'
        if listing.exists():
            for line in listing.read_text(encoding='utf-8').splitlines():
                fields=line.split('\t')
                if len(fields)==3:state['images'].append(dict(zip(('path','title','description'),fields)))
        class Reports(HTMLParser):
            def handle_starttag(self,tag,attrs):
                if tag!='a':return
                href=dict(attrs).get('href','');path=(figures/href).resolve()
                if href.startswith('../results/') and path.is_file() and path.is_relative_to(figures.parent):
                    state['reports'].append(dict(title=path.parent.name,path=path.relative_to(figures.parent).as_posix()))
        index=figures/'index.html'
        if index.exists():Reports().feed(index.read_text(encoding='utf-8'))
    cards=[]
    for item in state['images']:
        path=(figures/item['path']).resolve()
        if not path.is_relative_to(figures.resolve()) or not path.is_file():raise ValueError('Catalog image missing')
        if item.get('sha256') and digest(path)!=item['sha256']:raise ValueError('Catalog image changed')
        cards.append('<h2>'+html.escape(item['title'])+'</h2><p>'+html.escape(item['description'])+
                     '</p><img style="width:100%" src="'+quote(item['path'],safe='/')+'">')
    links=[]
    for item in state['reports']:
        target=figures.parent/item['path']
        if not target.is_file() or (item.get('sha256') and digest(target)!=item['sha256']):raise ValueError('Catalog report changed')
        links.append('<p><a href="../'+quote(item['path'],safe='/')+'">'+html.escape(item['title'])+'</a></p>')
    page='<!doctype html><meta charset="utf-8"><title>研究实验目录</title><body style="max-width:1200px;margin:30px auto;font:17px sans-serif">'
    page+='<h1>实验图件与报告</h1><p>批次：'+html.escape(state['batch'])+'；范围：'+html.escape(state['scope'])+'</p>'
    (figures/'index.html').write_text(page+''.join(links+cards),encoding='utf-8')
    return state
