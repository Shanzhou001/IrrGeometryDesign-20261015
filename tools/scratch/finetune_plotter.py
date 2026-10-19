# -*- coding: utf-8 -*-
with open('IrrGeometryDesign/scientificplotter.h', 'rb') as f:
    code = f.read().decode('utf-8')

# 1. Figure 02 badge position: move up to rSa.top() - 34
code = code.replace(
    'QRect badge(rSa.left() - 10, rSa.top() - 28, barWidth + 20, 18);',
    'QRect badge(rSa.left() - 10, rSa.top() - 34, barWidth + 20, 18);'
)

# 2. Figure 03 gridRect: shift right to 115 and width 395
code = code.replace(
    'QRect gridRect(95, 105, 410, 410);',
    'QRect gridRect(115, 105, 390, 390);'
)
code = code.replace(
    'QRect colorbarRect(520, 105, 18, 410);',
    'QRect colorbarRect(525, 105, 18, 390);'
)

# 3. Figure 06 trace vertical margin: don't overlap with legend box
# replace:
# int y1 = traceRect.top() + (int)(((float)(t - 1) / (float)Nt) * traceRect.height());
# with drawing inside traceH = traceRect.height() - 70
old_trace = '''            for (int t = 1; t < Nt; t++)
            {
                int y1 = traceRect.top() + (int)(((float)(t - 1) / (float)Nt) * traceRect.height());
                int x1 = midX + (int)((d_full[t - 1][sampleTraceIdx] / maxA) * 110.0f);
                int y2 = traceRect.top() + (int)(((float)t / (float)Nt) * traceRect.height());
                int x2 = midX + (int)((d_full[t][sampleTraceIdx] / maxA) * 110.0f);
                p.drawLine(x1, y1, x2, y2);
            }

            // 红色粗线：POCS 重构波形
            p.setPen(QPen(QColor(220, 38, 38), 2, Qt::SolidLine));
            for (int t = 1; t < Nt; t++)
            {
                int y1 = traceRect.top() + (int)(((float)(t - 1) / (float)Nt) * traceRect.height());
                int x1 = midX + (int)((d_recon[t - 1][sampleTraceIdx] / maxA) * 110.0f);
                int y2 = traceRect.top() + (int)(((float)t / (float)Nt) * traceRect.height());
                int x2 = midX + (int)((d_recon[t][sampleTraceIdx] / maxA) * 110.0f);
                p.drawLine(x1, y1, x2, y2);
            }'''

new_trace = '''            int availH = traceRect.height() - 75;
            for (int t = 1; t < Nt; t++)
            {
                int y1 = traceRect.top() + 10 + (int)(((float)(t - 1) / (float)Nt) * availH);
                int x1 = midX + (int)((d_full[t - 1][sampleTraceIdx] / maxA) * 110.0f);
                int y2 = traceRect.top() + 10 + (int)(((float)t / (float)Nt) * availH);
                int x2 = midX + (int)((d_full[t][sampleTraceIdx] / maxA) * 110.0f);
                p.drawLine(x1, y1, x2, y2);
            }

            // 红色粗线：POCS 重构波形
            p.setPen(QPen(QColor(220, 38, 38), 2, Qt::SolidLine));
            for (int t = 1; t < Nt; t++)
            {
                int y1 = traceRect.top() + 10 + (int)(((float)(t - 1) / (float)Nt) * availH);
                int x1 = midX + (int)((d_recon[t - 1][sampleTraceIdx] / maxA) * 110.0f);
                int y2 = traceRect.top() + 10 + (int)(((float)t / (float)Nt) * availH);
                int x2 = midX + (int)((d_recon[t][sampleTraceIdx] / maxA) * 110.0f);
                p.drawLine(x1, y1, x2, y2);
            }'''

code = code.replace(old_trace.replace('\n', '\r\n'), new_trace.replace('\n', '\r\n'))
code = code.replace(old_trace, new_trace)

lines = code.splitlines()
normalized = '\r\n'.join(lines) + '\r\n'
out_bytes = b'\xef\xbb\xbf' + normalized.encode('utf-8')

with open('IrrGeometryDesign/scientificplotter.h', 'wb') as f:
    f.write(out_bytes)

print('Updated scientificplotter.h!')
