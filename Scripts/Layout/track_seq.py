import sys; sys.path.insert(0,'.')
from common import *
md=mapdata(); P=80
Gp=np.pad(md['G'],((P,P),(P,P)))
H,W=md['G'].shape
rel=np.load(S+'/rel.npy')
fr=frames()
Gf_cache={}
def gradf(i):
    f=cv2.cvtColor(np.array(fr[i]),cv2.COLOR_BGR2GRAY)
    return grad(f)[Y0:Y1]
def match(i,px,py,rx,ry):
    Gf=gradf(i)
    x0=int(np.floor(px-rx)); y0=int(np.floor(py+Y0-ry))
    w=FW+2*rx+1; h=(Y1-Y0)+2*ry+1
    win=Gp[y0+P:y0+P+h, x0+P:x0+P+w]
    R=cv2.matchTemplate(win,Gf,cv2.TM_CCORR_NORMED)
    _,mx,_,ml=cv2.minMaxLoc(R)
    ox,oy=ml
    sx=sy=0.0
    if 0<ox<R.shape[1]-1:
        a,b,c=R[oy,ox-1],R[oy,ox],R[oy,ox+1]; d=a-2*b+c
        if d<0: sx=0.5*(a-c)/d
    if 0<oy<R.shape[0]-1:
        a,b,c=R[oy-1,ox],R[oy,ox],R[oy+1,ox]; d=a-2*b+c
        if d<0: sy=0.5*(a-c)/d
    return x0+ox+sx, y0+oy+sy-Y0, mx
def run(i0,pos0,i1,step):
    out={}
    px,py=pos0
    out[i0]=(px,py,1.0)
    i=i0
    vx=vy=0
    while i!=i1:
        j=i+step
        # rel[j] is shift cur(j) vs prev(j-1) ; for step -1 use rel[i]
        if step>0: dx,dy=-rel[j,0],-rel[j,1]
        else: dx,dy=rel[i,0],rel[i,1]
        dx=float(np.clip(dx,-40,40)); dy=float(np.clip(dy,-30,30))
        pxp,pyp=px+dx,py+dy
        ax,ay,sc=match(j,pxp,pyp,16,10)
        if sc<0.30:
            ax,ay=pxp,pyp  # rely on rel
        out[j]=(ax,ay,sc)
        px,py=ax,ay; i=j
    return out
if __name__=='__main__':
    N=int(250*FPS)
    f0=240; pos=(295.07,-3.35)
    fw=run(f0,pos,N,1)
    bw=run(f0,pos,0,-1)
    arr=np.zeros((N+1,3),np.float32)
    for k,v in {**bw,**fw}.items(): arr[k]=v
    np.save(S+'/seq.npy',arr)
    print('done')
