import sys; sys.path.insert(0,'.')
from common import *
N=int(250*FPS)+2
Y0,Y1=52,336
win=cv2.createHanningWindow((FW,Y1-Y0),cv2.CV_32F)
prev=None
out=np.zeros((N,3),np.float32)
fr=frames()
for i in range(N):
    g=cv2.cvtColor(np.array(fr[i]),cv2.COLOR_BGR2GRAY)[Y0:Y1].astype(np.float32)
    g=cv2.GaussianBlur(g,(0,0),1.0)
    if prev is not None:
        (dx,dy),r=cv2.phaseCorrelate(prev,g,win)
        out[i]=(dx,dy,r)
    prev=g
np.save(S+'/rel.npy',out)
print(out[:10]); print((out[:,2]<0.1).mean(), np.abs(out[:,0]).max())
