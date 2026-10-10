import sys; sys.path.insert(0,'.')
from common import *
N=int(250*FPS)+2
fr=frames()
def relband(ya,yb,name,sig=0.8):
    win=cv2.createHanningWindow((FW,yb-ya),cv2.CV_32F)
    out=np.zeros((N,3),np.float32); prev=None
    for i in range(N):
        g=cv2.cvtColor(np.array(fr[i]),cv2.COLOR_BGR2GRAY)[ya:yb].astype(np.float32)
        g=cv2.GaussianBlur(g,(0,0),sig)
        if prev is not None:
            (dx,dy),r=cv2.phaseCorrelate(prev,g,win); out[i]=(dx,dy,r)
        prev=g
    np.save(S+'/'+name,out)
relband(240,336,'rel_low.npy')
relband(52,200,'rel_hi.npy')
