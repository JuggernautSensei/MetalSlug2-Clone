import sys; sys.path.insert(0,'.')
from common import *
import sift_win, pickle
rel=np.load(S+'/rel.npy')
STEP=4
def run(i0,pos0,i1,step):
    out={i0:(pos0[0],pos0[1],999)}
    px,py=pos0; i=i0; lastgood=i0; miss=0
    while (i<i1 if step>0 else i>i1):
        j=i+step if step>0 else max(i+step,i1)
        if step>0: d=-rel[i+1:j+1,:2]
        else: d=rel[j+1:i+1,:2]
        d=np.clip(d,-30,30).sum(0)*(1 if step>0 else 1)
        if step<0: d=d   # shift sign: going back, cam moves opposite of forward; forward cam=-rel => backward cam=+rel
        pxp,pyp=px+d[0],py+d[1]
        grow=min(miss*30,200)
        r=sift_win.locw(j,pxp,pyp,rx=110+grow,ry=70+grow//2)
        if r is not None and not isinstance(r[0],str) and r[0]>=10:
            px,py=r[2],r[3]; out[j]=(px,py,r[0]); miss=0
        else:
            px,py=pxp,pyp; out[j]=(px,py,-1 if r is None or isinstance(r[0],str) else -r[0]); miss+=1
        i=j
    return out
if __name__=='__main__':
    N=int(250*FPS)
    f=run(240,(295.07,-3.35),N,STEP)
    b=run(240,(295.07,-3.35),0,-STEP)
    allp={**b,**f}
    pickle.dump(allp,open(S+'/track_sift.pkl','wb'))
    ks=sorted(allp); n_bad=sum(1 for k in ks if allp[k][2]<10)
    print(len(ks),'anchors; weak',n_bad)
