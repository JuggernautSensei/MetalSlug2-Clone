import sys; sys.path.insert(0,'.')
from common import *
md=mapdata(); G=md['G']; H,W=G.shape
rel=np.load(S+'/rel.npy')
N=len(rel)
cum=-np.cumsum(rel[:,:2],axis=0)  # relative camera pos (M px), frame0 = 0
fr=frames()
def match(i,cx,cy,rx,ry):
    f=cv2.cvtColor(np.array(fr[i]),cv2.COLOR_BGR2GRAY)
    Gf=grad(f)[Y0:Y1]
    x0=int(round(cx-rx)); y0=int(round(cy+Y0-ry))
    x1=int(round(cx+FW+rx)); y1=int(round(cy+Y1+ry))
    x0c=max(x0,0); y0c=max(y0,0); x1c=min(x1,W); y1c=min(y1,H)
    if x1c-x0c<FW or y1c-y0c<Y1-Y0: return None
    R=cv2.matchTemplate(G[y0c:y1c,x0c:x1c],Gf,cv2.TM_CCORR_NORMED)
    _,mx,_,ml=cv2.minMaxLoc(R)
    px=ml[0]+x0c; py=ml[1]+y0c-Y0
    # also correlation energy fraction: how much map gradient is in the window
    en=G[ml[1]+y0c:ml[1]+y0c+Y1-Y0, ml[0]+x0c:ml[0]+x0c+FW].mean()
    return px,py,mx,en
if __name__=='__main__':
    # sequential anchoring
    anchors=[]  # (frame, cx, cy, score)
    last=(int(8*FPS),295.07,-3.35)
    step=8
    i=last[0]
    anch=[]
    fails=0
    for i in range(int(8*FPS),N,step):
        dx,dy=cum[i]-cum[last[0]]
        px,py=last[1]+dx,last[2]+dy
        grow=min(10+0.02*abs(i-last[0])*2, 60)
        r=match(i,px,py,12+ (i-last[0])*0.5 if False else 14+min(abs(i-last[0])*0.3,60),10+min(abs(i-last[0])*0.1,20))
        if r is None: continue
        ax,ay,sc,en=r
        anch.append((i,ax,ay,sc,en,px,py))
        if sc>0.5 and en>3:
            last=(i,ax,ay)
    anch=np.array(anch)
    np.save(S+'/anch.npy',anch)
    print(len(anch)); 
    for a in anch[::10]: print(np.round(a,2))
