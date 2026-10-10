import sys; sys.path.insert(0,'.')
from blobs import *
import pickle
res=pickle.load(open(S+'/blobs.pkl','rb'))
# res: (i,wx,wy,ww,wh,area,x,y,w,h) ; group by frame
byf={}
for r in res: byf.setdefault(r[0],[]).append(r)
fr=sorted(byf)
tracks=[]
for f in fr:
    for r in byf[f]:
        best=None;bd=1e9
        for t in tracks:
            if f-t['last']['f']>46: continue
            d=np.hypot(r[1]-t['last']['x'],r[2]-t['last']['y'])
            if d<95 and d<bd: best=t;bd=d
        o=dict(f=f,x=r[1],y=r[2],w=r[3],h=r[4],a=r[5],bx=r[6],by=r[7],bw=r[8],bh=r[9])
        if best is None: tracks.append(dict(obs=[o],last=o))
        else: best['obs'].append(o); best['last']=o
tr=[t for t in tracks if len(t['obs'])>=2 and max(o['a'] for o in t['obs'])>=140]
print(len(tracks),len(tr))
for k,t in enumerate(tr): t['id']=k
pickle.dump(tr,open(S+'/tracks.pkl','wb'))
# crops montage
def crop(t):
    o=max(t['obs'],key=lambda o:o['a'])
    f=frame(o['f']).copy()
    x0=max(0,o['bx']-14);y0=max(0,o['by']-14);x1=min(FW,o['bx']+o['bw']+14);y1=min(FH,o['by']+o['bh']+14)
    c=f[y0:y1,x0:x1]
    c=cv2.resize(c,None,fx=2.5,fy=2.5,interpolation=cv2.INTER_CUBIC)
    c=c[:150,:170]
    out=np.full((170,170,3),40,np.uint8); out[:c.shape[0],:c.shape[1]]=c
    cv2.putText(out,'#%d t%.0f x%.0f y%.0f'%(t['id'],o['f']/FPS,o['x'],o['y']),(1,162),0,0.4,(0,255,255),1)
    return out
import glob
for n in range(0,len(tr),35):
    tiles=[crop(t) for t in tr[n:n+35]]
    while len(tiles)<35: tiles.append(np.zeros((170,170,3),np.uint8))
    img=np.vstack([np.hstack(tiles[r:r+7]) for r in range(0,35,7)])
    cv2.imwrite(S+'/tm_%02d.png'%(n//35),img)
print((len(tr)+34)//35,'montages')
