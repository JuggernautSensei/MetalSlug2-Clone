import sys; sys.path.insert(0,'.')
from common import *
from PIL import Image
Image.MAX_IMAGE_PIXELS=None
im=cv2.imread(ROOT+r'\Assets\Textures\Stage0_Main.png',cv2.IMREAD_UNCHANGED)
print(im.shape)
sc=0.79
bgr=im[:,:,:3].copy(); a=im[:,:,3]
# premult onto mid-gray for features
g=cv2.cvtColor(bgr,cv2.COLOR_BGR2GRAY)
g[a<128]=128
M=cv2.resize(g,None,fx=sc,fy=sc,interpolation=cv2.INTER_AREA)
print(M.shape)
sift=cv2.SIFT_create(nfeatures=0,contrastThreshold=0.02)
kp,des=sift.detectAndCompute(M,None)
print(len(kp))
pts=np.array([k.pt for k in kp],np.float32)
np.save(S+'/map_kp.npy',pts); np.save(S+'/map_des.npy',des)
