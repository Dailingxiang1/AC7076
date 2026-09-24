from pathlib import Path
from xml.sax.saxutils import escape
import re
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle, PageBreak, Image
from reportlab.lib import colors
from reportlab.lib.styles import ParagraphStyle
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from pypdf import PdfReader

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'doc/pdf/AC7076A3_外设Demo调试手册.pdf'
OUT.parent.mkdir(parents=True, exist_ok=True)
pdfmetrics.registerFont(TTFont('CN', 'C:/Windows/Fonts/msyh.ttc', subfontIndex=0))
pdfmetrics.registerFont(TTFont('CNB', 'C:/Windows/Fonts/msyhbd.ttc', subfontIndex=0))
pdfmetrics.registerFontFamily('CN', normal='CN', bold='CNB', italic='CN', boldItalic='CNB')
navy = colors.HexColor('#142D47')
teal = colors.HexColor('#087F8C')
pale = colors.HexColor('#EEF5F8')
styles = {
    'body': ParagraphStyle('body', fontName='CN', fontSize=9.3, leading=15, spaceAfter=8, wordWrap='CJK', textColor=navy),
    'title': ParagraphStyle('title', fontName='CNB', fontSize=23, leading=32, spaceAfter=14, textColor=navy),
    'h2': ParagraphStyle('h2', fontName='CNB', fontSize=16, leading=24, spaceAfter=14, keepWithNext=True, textColor=teal),
    'cell': ParagraphStyle('cell', fontName='CN', fontSize=8, leading=12, wordWrap='CJK', textColor=navy),
    'head': ParagraphStyle('head', fontName='CNB', fontSize=8, leading=12, wordWrap='CJK', textColor=colors.white),
    'code': ParagraphStyle('code', fontName='CN', fontSize=8.2, leading=13, spaceAfter=0, wordWrap='CJK', textColor=navy),
}

def inline(s):
    s = escape(s)
    s = re.sub(r'\[([^\]]+)\]\((https?://[^)]+)\)', r'<link href="\2" color="#087F8C">\1</link>', s)
    s = re.sub(r'\*\*(.+?)\*\*', r'<b>\1</b>', s)
    s = re.sub(r'`([^`]+)`', r'<font color="#087F8C">\1</font>', s)
    return s

story = []
text = (ROOT / 'doc/AC7076A3_DEMO.md').read_text(encoding='utf-8').splitlines()
i = 0
while i < len(text):
    line = text[i].strip()
    if not line:
        i += 1
        continue
    if line.startswith('# '):
        story.append(Paragraph('AC7076A3 外设 Demo 调试手册', styles['title']))
        story.append(Paragraph('Board1 / SDK 3.2.1 / 2026-09-24<br/>依据用户原理图。编译验证与硬件验收分开记录。', styles['body']))
        i += 1
    elif line.startswith('## '):
        if line.startswith('## 1.'):
            story.extend([Spacer(1, 18), Image(str(ROOT / 'tmp/pdfs/schematic.png'), width=499, height=353), Spacer(1, 12)])
            story.append(Paragraph('原理图缩略图。精确引脚请查第 3 页接线表及 doc 目录下的原始 PDF。<br/>当前 demo 默认通过 USB CDC 虚拟 COM 输出日志；JD9855 模组参数仍待确认。早期编译下载指南记录的是原 SDK 基线。', styles['body']))
        story.append(PageBreak())
        story.append(Paragraph(inline(line[3:]), styles['h2']))
        i += 1
    elif line.startswith('|'):
        rows = []
        while i < len(text) and text[i].strip().startswith('|'):
            cells = [c.strip() for c in text[i].strip().strip('|').split('|')]
            if not all(re.match(r'^:?-+:?$', c) for c in cells):
                rows.append(cells)
            i += 1
        data = [[Paragraph(inline(c), styles['head' if r == 0 else 'cell']) for c in row] for r, row in enumerate(rows)]
        widths = [185, 38, 276] if rows[0][0] == '开关' else [112, 169, 218]
        tab = Table(data, colWidths=widths, repeatRows=1, hAlign='LEFT')
        tab.setStyle(TableStyle([
            ('BACKGROUND', (0,0), (-1,0), navy), ('VALIGN', (0,0), (-1,-1), 'TOP'),
            ('ROWBACKGROUNDS', (0,1), (-1,-1), [pale, colors.white]),
            ('LEFTPADDING', (0,0), (-1,-1), 7), ('RIGHTPADDING', (0,0), (-1,-1), 7),
            ('TOPPADDING', (0,0), (-1,-1), 6), ('BOTTOMPADDING', (0,0), (-1,-1), 6),
        ]))
        story.extend([tab, Spacer(1,10)])
    elif line.startswith('```'):
        code = []
        i += 1
        while i < len(text) and not text[i].startswith('```'):
            code.append(Paragraph(escape(text[i]).replace(' ', '&#160;') or '&#160;', styles['code']))
            i += 1
        i += 1
        box = Table([[code]], colWidths=[499])
        box.setStyle(TableStyle([('BACKGROUND',(0,0),(-1,-1),pale),('LEFTPADDING',(0,0),(-1,-1),10),('TOPPADDING',(0,0),(-1,-1),8),('BOTTOMPADDING',(0,0),(-1,-1),8)]))
        story.extend([box, Spacer(1,10)])
    else:
        parts = [line]
        i += 1
        while i < len(text) and text[i].strip() and not text[i].startswith(('#','|','```','- ')) and not re.match(r'^\d+\. ', text[i]):
            parts.append(text[i].strip())
            i += 1
        story.append(Paragraph(inline(''.join(parts)), styles['body']))

def footer(c, d):
    c.setStrokeColor(teal)
    c.line(48, 42, 547, 42)
    c.setFont('CN', 8)
    c.setFillColor(navy)
    c.drawString(48, 27, 'AC7076A3 / Board1 / 外设验证')
    c.drawRightString(547, 27, str(d.page))

SimpleDocTemplate(str(OUT), pagesize=(595.28,841.89), leftMargin=48, rightMargin=48,
    topMargin=44, bottomMargin=57, title='AC7076A3 外设 Demo 调试手册', author='Board1 SDK engineering notes').build(story, onFirstPage=footer, onLaterPages=footer)
reader = PdfReader(OUT)
print(f'{OUT}\nPages: {len(reader.pages)}')
for n, p in enumerate(reader.pages, 1):
    print(n, len(p.extract_text()), p.extract_text()[:60].replace('\n', ' '))
