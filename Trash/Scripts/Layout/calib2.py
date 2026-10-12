import sys; sys.path.insert(0,'.')
from common import *
pts=np.load(S+'/map_kp.npy'); des=np.load(S+'/map_des.npy')
sift=cv2.SIFT_create(contrastThreshold=0.02)
fl=cv2.FlannBasedMatcher(dict(algorithm=1,trees=4),dict(checks=64))
res=[]
for t in [8,12,20,30,45,60,70,100,110,150,160,180,200,215,230,245]:
    f=tframe(t); g=cv2.cvtColor(f,cv2.COLOR_BGR2GRAY)
    m=np.zeros_like(g); m[52:338]=255
    kp,d=sift.detectAndCompute(g,m)
    if d is None or len(kp)<8: print(t,'few'); continue
    mm=fl.knnMatch(d,des,k=2)
    good=[a for a,b in mm if a.distance<0.7*b.distance]
    if len(good)<8: print(t,'good',len(good)); continue
    src=np.float32([kp[a.queryIdx].pt for a in good]); dst=pts[[a.trainIdx for a in good]]
    # full affine dst = A*src + b
    A,inl=cv2.estimateAffine2D(src,dst,ransacReprojThreshold=3,maxIters=5000)
    if A is None: print(t,'noaff'); continue
    n=int(inl.sum())
    # sim partial for compare
    print(t,len(good),n,'A',np.round(A,4).tolist())
