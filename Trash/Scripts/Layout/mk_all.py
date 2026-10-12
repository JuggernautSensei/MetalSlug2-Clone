import sys; sys.path.insert(0,'.')
from vp2 import *
t0,t1,dt=float(sys.argv[1]),float(sys.argv[2]),float(sys.argv[3]); k=int(sys.argv[4]) if len(sys.argv)>4 else 3
ts=np.arange(t0,t1,dt)
for n in range(0,len(ts),k):
    grp=ts[n:n+k]
    imgs=[render(int(round(t*FPS)),1.6,extra=extra) for t in grp]
    cv2.imwrite(S+'/s_%05.1f.png'%grp[0],np.vstack(imgs))
print(len(ts))
