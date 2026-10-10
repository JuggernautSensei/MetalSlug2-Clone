import sys; sys.path.insert(0,'.')
from common import *
CAM=np.load(S+'/cam_game.npy')
SXg,SYg=0.7906,0.7912
def w2s(wx,wy,i):  # world game px -> video px
    return (wx-CAM[i,0])*SXg,(wy-CAM[i,1])*SYg
def s2w(u,v,i):
    return CAM[i,0]+u/SXg, CAM[i,1]+v/SYg
def render(i,scale=2.0,grid=100,extra=None):
    f=cv2.resize(frame(i),None,fx=scale,fy=scale,interpolation=cv2.INTER_CUBIC)
    H,W=f.shape[:2]
    x0=CAM[i,0]; y0=CAM[i,1]
    gx=int(np.ceil(x0/grid))*grid
    while True:
        u,_=w2s(gx,0,i)
        if u>FW: break
        U=int(u*scale)
        major=(gx%500==0)
        cv2.line(f,(U,0),(U,H),(0,255,255) if major else (0,200,0),1)
        cv2.putText(f,str(gx),(U+2,H-24 if not major else H-36),0,0.45,(0,255,255),1,cv2.LINE_AA)
        gx+=grid
    gy=int(np.ceil(y0/grid))*grid
    while True:
        _,v=w2s(0,gy,i)
        if v>FH: break
        V=int(v*scale)
        cv2.line(f,(0,V),(W,V),(0,200,0),1)
        cv2.putText(f,str(gy),(2,V-2),0,0.45,(0,255,255),1,cv2.LINE_AA)
        gy+=grid
    cv2.rectangle(f,(W-150,H-24),(W,H),(0,0,0),-1)
    cv2.putText(f,'t=%.2f f%d'%(i/FPS,i),(W-146,H-7),0,0.5,(255,255,255),1,cv2.LINE_AA)
    if extra: extra(f,i,scale)
    return f
def pair(ts,path,scale=2.0,**k):
    imgs=[render(int(round(t*FPS)),scale,**k) for t in ts]
    cv2.imwrite(path,np.vstack(imgs))
if __name__=='__main__':
    ts=[float(a) for a in sys.argv[2:]]
    pair(ts,sys.argv[1])
