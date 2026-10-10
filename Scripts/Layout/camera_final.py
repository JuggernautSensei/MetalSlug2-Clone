import sys; sys.path.insert(0,'.')
from common import *
import pickle
from scipy.ndimage import median_filter
tr=pickle.load(open(S+'/track_sift.pkl','rb'))
pr=np.load(S+'/pair.npy'); rel=np.load(S+'/rel.npy'); seq=np.load(S+'/seq.npy')
N=len(pr)+1
inc=np.zeros((N,2))
for k in range(len(pr)):
    i=int(pr[k,0])
    d=-pr[k,3:5] if pr[k,1]>=5 else -rel[i,:2]
    inc[i]=np.clip(d,-60,60)
# trusted anchors
A={}
for k,v in tr.items():
    if v[2]>=25 and k<N: A[k]=(v[0],v[1])
for k in range(240,2540,4):   # NCC seq anchors (street) with decent score
    if seq[k,2]>0.5 and k not in A: A[k]=(seq[k,0],seq[k,1])
ak=np.array(sorted(A)); av=np.array([A[k] for k in ak])
# calibrate scale factor on increments between consecutive-ish anchors
cum=np.cumsum(inc,axis=0)
num=0;den=0
for a,b in zip(ak[:-1],ak[1:]):
    if 20<=b-a<=200:
        t=av[list(ak).index(b)]-av[list(ak).index(a)]; c=cum[b]-cum[a]
        if np.linalg.norm(c)>100: num+=np.dot(t,c); den+=np.dot(c,c)
fac=num/den; print('scale factor',fac)
cum=np.cumsum(inc*fac,axis=0)
# drift at anchors
d=av-cum[ak]
# robust smoothing of drift (median over 9 anchors)
ds=np.stack([median_filter(d[:,0],size=9,mode='nearest'),median_filter(d[:,1],size=9,mode='nearest')],1)
# remove anchors that deviate >6 px from smoothed -> outliers
ok=np.all(np.abs(d-ds)<6,axis=1); print('anchors',len(ak),'ok',ok.sum())
ak2=ak[ok]; d2=d[ok]
D=np.stack([np.interp(np.arange(N),ak2,d2[:,0]),np.interp(np.arange(N),ak2,d2[:,1])],1)
cam=cum+D
np.save(S+'/cam_M.npy',cam)
res=cam[ak2]-av[ok]; print('residual at anchors rms',np.sqrt((res**2).mean(0)))
print('last anchor t',ak2[-1]/FPS)
# step-wise discontinuity of D
jd=np.abs(np.diff(D,axis=0)).max(1); print('max drift step',jd.max(), 'n>2',(jd>2).sum())
for t in [0,8,20,40,60,80,86,100,120,140,160,175,190,210,227,235,240,245,248,250]:
    i=int(t*FPS); print(t,np.round(cam[i],0),'game',np.round(cam[i]/0.79,0))
