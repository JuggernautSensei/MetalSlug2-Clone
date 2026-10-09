import subprocess, os, sys
from PIL import Image, ImageDraw
env=dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_RENDER_DRIVER='software')
ticks=sys.argv[1]; pre='Captures/v9/b'
args=[os.path.join('Output','Release','MyMetalSlug2.exe'),'--state','stage0','--no-audio','--life','10','--warp-boss','--hold','J','--capture',pre,'--capture-ticks',ticks]+sys.argv[2:]
subprocess.run(args,env=env,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,timeout=300)
ts=[int(t) for t in ticks.split(',')]
ims=[]
for t in ts:
    f=f'{pre}_{t:05d}.bmp'
    if os.path.exists(f):
        im=Image.open(f).convert('RGB').resize((480,336)); ImageDraw.Draw(im).text((4,320),f'tick {t}',fill=(255,255,0)); ims.append(im)
W=Image.new('RGB',(480*3,336*((len(ims)+2)//3)))
for i,im in enumerate(ims): W.paste(im,((i%3)*480,(i//3)*336))
W.save('Captures/v9/boss_sheet.png'); print(len(ims))
