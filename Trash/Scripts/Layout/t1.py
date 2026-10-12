import sys; sys.path.insert(0,'.')
from common import *
col=np.load(S+'/M_col.npy'); al=np.load(S+'/M_alpha.npy')
t=8
f=tframe(t)
x0=295.07; y0=-3.35
# crop M at (x0,y0), 480x360
x=int(round(x0)); 
M=np.zeros((360,480,3),np.uint8); Aa=np.zeros((360,480),np.uint8)
ys=4  # y0=-3.35 -> row offset
M[4:360]=col[0:356,x:x+480]; Aa[4:360]=al[0:356,x:x+480]
f2=f.copy(); 
out=np.hstack([f,M]); cv2.imwrite(S+'/t1.png',out)
d=np.abs(f.astype(int)-M.astype(int)).mean(2)
m=Aa>0
print('mean abs diff',d[m][ (np.arange(360)[:,None]*np.ones((1,480)))[m]>52].mean(), np.median(d[m]))
print('mean frame', f[m].mean((0)), 'map', M[m].mean(0))
