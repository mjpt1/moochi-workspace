# موچی — تبدیل یک GIF رنگی به فایل .h برای پخش روی نمایشگر ۲۴۰×۲۸۰
#
# روش (برای اینکه در حافظه‌ي فلش ESP32 جا شود):
#   ۱. هر فریم به ۲۴۰×۲۸۰ تغییر اندازه و برش داده می‌شود (صفحه پر شود)
#   ۲. همه‌ي فریم‌ها یک پالت مشترک ۲۵۶ رنگی می‌گیرند (هر پیکسل ۱ بایت)
#   ۳. فریم اول کامل ذخیره می‌شود؛ هر فریم بعدی فقط «مستطیلِ تغییرکرده»
#   ۴. داخل هر مستطیل ، پیکسل‌های تکراری پشت سر هم فشرده می‌شوند (RLE)
#
# استفاده:  python gif_to_anim.py  ورودی.gif  خروجی.h  [هر چند فریم یکی]
import sys
from PIL import Image, ImageSequence

W, H = 240, 280
src = sys.argv[1]
dst = sys.argv[2]
step = int(sys.argv[3]) if len(sys.argv) > 3 else 1

im = Image.open(src)
raw = [f.convert('RGB') for f in ImageSequence.Iterator(im)]
durs = []
for f in ImageSequence.Iterator(im):
    durs.append(f.info.get('duration', 40) or 40)

def fit(fr):
    sw, sh = fr.size
    s = max(W / sw, H / sh)                 # پر کردن صفحه (بخش اضافه بریده می‌شود)
    nw, nh = round(sw * s), round(sh * s)
    fr = fr.resize((nw, nh), Image.LANCZOS)
    x0, y0 = (nw - W) // 2, (nh - H) // 2
    return fr.crop((x0, y0, x0 + W, y0 + H))

frames = [fit(raw[i]) for i in range(0, len(raw), step)]
fdur = [sum(durs[i:i + step]) for i in range(0, len(raw), step)]

# پالت مشترک: از چند فریم نمونه ساخته می‌شود
sample = Image.new('RGB', (W, H * min(8, len(frames))))
for k in range(min(8, len(frames))):
    sample.paste(frames[k * len(frames) // min(8, len(frames))], (0, H * k))
pal_img = sample.quantize(colors=256, method=Image.Quantize.MEDIANCUT)
idx = [f.quantize(palette=pal_img, dither=Image.Dither.NONE) for f in frames]
pal = pal_img.getpalette()[:768]
pal += [0] * (768 - len(pal))

def rgb565(r, g, b):
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

pal565 = [rgb565(pal[i*3], pal[i*3+1], pal[i*3+2]) for i in range(256)]

def rle(data):
    out = bytearray()
    i = 0
    while i < len(data):
        v = data[i]; n = 1
        while i + n < len(data) and data[i + n] == v and n < 255:
            n += 1
        out += bytes((n, v))
        i += n
    return out

blob = bytearray()
index = []   # (offset, x, y, w, h, dur)
prev = None
for k, f in enumerate(idx):
    px = list(f.getdata()) if hasattr(f, 'getdata') else None
    if prev is None:
        x0, y0, x1, y1 = 0, 0, W - 1, H - 1
    else:
        xs = [i % W for i in range(W * H) if px[i] != prev[i]]
        if not xs:
            x0 = y0 = x1 = y1 = 0          # بدون تغییر: یک پیکسل
        else:
            ys = [i // W for i in range(W * H) if px[i] != prev[i]]
            x0, x1, y0, y1 = min(xs), max(xs), min(ys), max(ys)
    bw, bh = x1 - x0 + 1, y1 - y0 + 1
    box = [px[(y0 + yy) * W + x0 + xx] for yy in range(bh) for xx in range(bw)]
    off = len(blob)
    blob += rle(box)
    index.append((off, x0, y0, bw, bh, min(fdur[k], 65535)))
    prev = px

name = 'BOOT_GIF'
o = []
o.append('// ساخته‌شده خودکار با tools/gif_to_anim.py از %s' % src.replace('\\', '/').split('/')[-1])
o.append('// %d فریم ، %dx%d ، پالت ۲۵۶ رنگ ، فریم‌های تفاضلی + RLE ، داده %d بایت' % (len(idx), W, H, len(blob)))
o.append('#pragma once')
o.append('#include <Arduino.h>')
o.append('#define %s_W %d' % (name, W))
o.append('#define %s_H %d' % (name, H))
o.append('#define %s_FRAMES %d' % (name, len(idx)))
o.append('const uint16_t %s_PAL[256] PROGMEM = {' % name)
o.append(','.join('0x%04X' % c for c in pal565))
o.append('};')
o.append('// هر فریم: آفست در داده ، x ، y ، عرض ، ارتفاع ، مدت (میلی‌ثانیه)')
o.append('const uint32_t %s_IDX[%s_FRAMES][6] PROGMEM = {' % (name, name))
for e in index:
    o.append('{%d,%d,%d,%d,%d,%d},' % e)
o.append('};')
o.append('const uint8_t %s_DATA[%d] PROGMEM = {' % (name, len(blob)))
for i in range(0, len(blob), 32):
    o.append(','.join(str(b) for b in blob[i:i + 32]) + ',')
o.append('};')
open(dst, 'w', encoding='utf-8').write('\n'.join(o) + '\n')

# پیش‌نمایش همان چیزی که روی نمایشگر دیده می‌شود (با رنگ‌های ۵۶۵)
prevs = []
for f in idx:
    p = f.convert('RGB')
    prevs.append(p.point(lambda v: v & 0xF8))
prevs[0].save(dst.rsplit('.', 1)[0] + '_preview.gif', save_all=True, append_images=prevs[1:],
              duration=fdur, loop=0)
print('frames', len(idx), 'data bytes', len(blob),
      'avg box', sum(e[3] * e[4] for e in index) // len(index), 'px')
