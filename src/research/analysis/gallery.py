"""Generate one lightweight gallery from the images already in figures/."""
import html
from pathlib import Path
from urllib.parse import quote

from research.common.paths import FIGURES


def render():
    images = sorted(path for path in FIGURES.rglob('*')
                    if path.suffix.lower() in {'.png', '.jpg', '.jpeg', '.gif', '.webp', '.svg'})
    cards = []
    labels = {'paper': '论文图', 'research': '研究图', 'gui': '程序界面',
              'legacy': '示例图', 'assets': '图标素材'}
    for image in images:
        relative = image.relative_to(FIGURES)
        group = labels.get(relative.parts[0], '其他')
        name = html.escape(image.stem.replace('_', ' '))
        url = quote(relative.as_posix(), safe='/')
        cards.append(f'<a class="card" href="{url}" data-group="{group}">'
                     f'<div class="preview"><img src="{url}" loading="lazy" alt="{name}"></div>'
                     f'<div class="caption"><small>{group}</small><span>{name}</span></div></a>')
    buttons = '<button class="active" data-group="all">全部</button>' + ''.join(
        f'<button data-group="{label}">{label}</button>' for label in labels.values())
    page = '''<!doctype html>
<html lang="zh-CN"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>IrrGeometryDesign · 图片</title>
<style>
*{box-sizing:border-box}body{margin:0;background:#f4f5f7;color:#192b37;font:15px/1.6 system-ui,"Microsoft YaHei",sans-serif}
main{max-width:1440px;margin:auto;padding:48px 28px}header{border-bottom:1px solid #d6dfe4;padding-bottom:24px}
.eyebrow{font-size:12px;letter-spacing:2px;color:#58717d}h1{margin:8px 0;font-size:34px;font-weight:650}p{color:#58717d;margin:0}
nav{display:flex;gap:8px;flex-wrap:wrap;padding:24px 0}button{border:1px solid #d6dfe4;background:white;color:#36515e;border-radius:24px;padding:8px 18px;cursor:pointer;font:inherit}
button.active{background:#214c5e;color:white;border-color:#214c5e}.grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(280px,1fr));gap:20px}
.card{display:block;background:white;border:1px solid #e0e5e8;border-radius:12px;overflow:hidden;text-decoration:none;color:inherit;transition:transform .15s,box-shadow .15s}
.card:hover{transform:translateY(-3px);box-shadow:0 8px 24px #214c5e12}.preview{height:220px;display:flex;align-items:center;justify-content:center;background:#fff;padding:14px}
img{max-width:100%;max-height:100%;object-fit:contain}.caption{border-top:1px solid #edf0f2;padding:14px 18px}small{display:block;color:#6b858e;font-size:11px}span{display:block;font-size:14px;overflow-wrap:anywhere}
[hidden]{display:none!important}footer{margin-top:32px;font-size:12px;color:#6b858e}@media(max-width:600px){main{padding:28px 16px}h1{font-size:28px}}
</style><main><header><div class="eyebrow">IRR GEOMETRY DESIGN</div><h1>图片资料库</h1>
<p>论文、研究、程序界面与原始图标 · 点击图片查看原图</p></header><nav>BUTTONS</nav>
<section class="grid">CARDS</section><footer>COUNT 张图片 · 原图保存在当前目录</footer></main>
<script>document.querySelectorAll('button').forEach(button=>button.onclick=()=>{
document.querySelectorAll('button').forEach(item=>item.classList.toggle('active',item===button));
document.querySelectorAll('.card').forEach(card=>card.hidden=button.dataset.group!=='all'&&card.dataset.group!==button.dataset.group);
});</script></html>'''
    (FIGURES/'index.html').write_text(page.replace('BUTTONS', buttons).replace('CARDS', ''.join(cards))
                                     .replace('COUNT', str(len(images))), encoding='utf-8')
    print(f'Gallery: {len(images)} images, {FIGURES / "index.html"}')


if __name__ == '__main__':
    render()
