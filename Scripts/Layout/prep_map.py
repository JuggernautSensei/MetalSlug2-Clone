import sys; sys.path.insert(0,'.')
from common import *
im=cv2.imread(ROOT+r'\Assets\Baked\Textures\Stage0_Main.png',cv2.IMREAD_UNCHANGED)
sc=0.79
bgr=im[:,:,:3].astype(np.float32); a=im[:,:,3].astype(np.float32)/255
# premultiplied area resample to avoid fringes
pm=bgr*a[:,:,None]
W=int(round(im.shape[1]*sc)); H=int(round(im.shape[0]*sc))
pmr=cv2.resize(pm,(W,H),interpolation=cv2.INTER_AREA)
ar=cv2.resize(a,(W,H),interpolation=cv2.INTER_AREA)
col=np.clip(pmr/np.maximum(ar[:,:,None],1e-3),0,255).astype(np.uint8)
alpha=(ar>0.98).astype(np.uint8)*255
col[alpha==0]=0
np.save(S+'/M_col.npy',col); np.save(S+'/M_alpha.npy',alpha)
cv2.imwrite(S+'/M_col.png',col)
print(col.shape)
