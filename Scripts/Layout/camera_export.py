import sys; sys.path.insert(0,'.')
from common import *
cam=np.load(S+'/cam_M.npy').copy()
N=len(cam)
# open land correction (t>=227): ramp 0->27 M px over 227..233.5 s, hold
f0,f1=int(227*FPS),int(233.5*FPS)
corr=np.clip((np.arange(N)-f0)/(f1-f0),0,1)*27.0
cam[:,0]+=corr
np.save(S+'/cam_M2.npy',cam)
sc=0.79
gx=cam[:,0]/sc; gy=cam[:,1]/sc
import csv
with open(ROOT+r'\Assets\Source\OriginalCamera.csv','w',newline='') as fh:
    w=csv.writer(fh); w.writerow(['frame','t_s','cam_x','cam_y'])
    for i in range(N): w.writerow([i,round(i/FPS,3),round(gx[i],1),round(gy[i],1)])
np.save(S+'/cam_game.npy',np.stack([gx,gy],1))
print(N, gx[-1])
# verification sheet
ts=[3,30,60,100,150,200]
tiles=[]
for t in ts:
    i=int(t*FPS); o=overlay_check(i,cam[i,0],cam[i,1])
    cv2.putText(o,'t=%d cam=(%.0f,%.0f) game'%(t,gx[i],gy[i]),(5,350),0,0.6,(0,255,255),2)
    tiles.append(cv2.resize(o,None,fx=0.6,fy=0.6))
cv2.imwrite(S+'/verify_cam.png',np.vstack(tiles))
