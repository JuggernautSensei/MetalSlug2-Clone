import sys; sys.path.insert(0,'.')
from common import *
pts=np.load(S+'/map_kp.npy'); des=np.load(S+'/map_des.npy')
sift=cv2.SIFT_create(contrastThreshold=0.02)
fl=cv2.FlannBasedMatcher(dict(algorithm=1,trees=4),dict(checks=64))
def loc(i):
    f=frame(i); g=cv2.cvtColor(f,cv2.COLOR_BGR2GRAY)
    m=np.zeros_like(g); m[52:338]=255
    kp,d=sift.detectAndCompute(g,m)
    if d is None or len(kp)<6: return None
    mm=fl.knnMatch(d,des,k=2)
    good=[a for a,b in mm if a.distance<0.75*b.distance]
    if len(good)<6: return ('few',len(good))
    src=np.float32([kp[a.queryIdx].pt for a in good]); dst=pts[[a.trainIdx for a in good]]
    A,inl=cv2.estimateAffine2D(src,dst,ransacReprojThreshold=3,maxIters=5000)
    if A is None: return ('noaff',len(good))
    return (int(inl.sum()),len(good),np.round(A.ravel(),3).tolist())
if __name__=='__main__':
    for i in [int(a) for a in sys.argv[1:]]:
        print(i, round(i/FPS,1), loc(i))
