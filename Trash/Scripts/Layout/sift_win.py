import sys; sys.path.insert(0,'.')
from common import *
pts=np.load(S+'/map_kp.npy'); des=np.load(S+'/map_des.npy')
sift=cv2.SIFT_create(contrastThreshold=0.02)
bf=cv2.BFMatcher(cv2.NORM_L2)
def locw(i,px,py,rx=150,ry=100,ratio=0.8):
    f=frame(i); g=cv2.cvtColor(f,cv2.COLOR_BGR2GRAY)
    m=np.zeros_like(g); m[52:338]=255
    kp,d=sift.detectAndCompute(g,m)
    if d is None or len(kp)<6: return None
    sel=(pts[:,0]>px-rx)&(pts[:,0]<px+FW+rx)&(pts[:,1]>py-ry)&(pts[:,1]<py+FH+ry)
    ix=np.where(sel)[0]
    if len(ix)<10: return ('nomap',len(ix))
    mm=bf.knnMatch(d,des[ix],k=2)
    good=[a for a,b in mm if a.distance<ratio*b.distance]
    if len(good)<5: return ('few',len(good))
    src=np.float32([kp[a.queryIdx].pt for a in good]); dst=pts[ix[[a.trainIdx for a in good]]]
    # translation-only RANSAC (scale ~1)
    dd=dst-src
    best=None
    for t in dd:
        inl=np.linalg.norm(dd-t,axis=1)<2.5
        if best is None or inl.sum()>best.sum(): best=inl
    t=np.median(dd[best],axis=0)
    return (int(best.sum()),len(good),float(t[0]),float(t[1]))
if __name__=='__main__':
    a=np.load(S+'/seq.npy')
    for i in range(5090,5700,20):
        print(i,np.round(a[i,:2],0),locw(i,a[i,0],a[i,1]))
