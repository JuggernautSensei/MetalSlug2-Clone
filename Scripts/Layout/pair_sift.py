import sys; sys.path.insert(0,'.')
from common import *
from multiprocessing import Pool
sift=None
def kpdes(i):
    f=frame(i); g=cv2.cvtColor(f,cv2.COLOR_BGR2GRAY)
    m=np.zeros_like(g); m[52:338]=255
    kp,d=sift.detectAndCompute(g,m)
    return (np.float32([k.pt for k in kp]) if kp else np.zeros((0,2),np.float32)), d
def init():
    global sift
    sift=cv2.SIFT_create(contrastThreshold=0.03)
def work(rng):
    a,b=rng
    bf=cv2.BFMatcher(cv2.NORM_L2)
    out=[]
    pk,pd=kpdes(a-1)
    for i in range(a,b):
        k,d=kpdes(i)
        res=(0,0,0.0,0.0)
        if d is not None and pd is not None and len(k)>=4 and len(pk)>=4:
            mm=bf.knnMatch(pd,d,k=2)
            g=[x for x,y in mm if x.distance<0.8*y.distance]
            if len(g)>=4:
                s=pk[[x.queryIdx for x in g]]; t=k[[x.trainIdx for x in g]]
                dd=t-s
                best=None
                for q in dd:
                    inl=np.linalg.norm(dd-q,axis=1)<1.5
                    if best is None or inl.sum()>best.sum(): best=inl
                tt=np.median(dd[best],axis=0)
                res=(int(best.sum()),len(g),float(tt[0]),float(tt[1]))
        out.append((i,)+res)
        pk,pd=k,d
    return out
if __name__=='__main__':
    N=int(250*FPS)+1
    chunks=[(a,min(a+100,N)) for a in range(1,N,100)]
    with Pool(14,initializer=init) as p: r=p.map(work,chunks)
    arr=np.array([x for c in r for x in c],np.float32)
    np.save(S+'/pair.npy',arr); print(arr.shape)
