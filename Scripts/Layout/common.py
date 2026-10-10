import os, numpy as np, cv2, json
ROOT = r"C:\Users\Ahnjiwoo\Desktop\Projects\windowAPIProject(24.02.05)\MyMetalSlug2"
S = r"C:\Users\Ahnjiwoo\AppData\Local\Temp\claude\C--Users-Ahnjiwoo-Desktop-Projects-windowAPIProject-24-02-05-\ddade356-abdc-4267-a45e-a11d5b5f18a2\scratchpad\layout2"
FPS = 29.5
FW, FH = 480, 360
FBYTES = FW*FH*3
NFRAMES = os.path.getsize(os.path.join(S,'frames.bgr'))//FBYTES
_mm = None
def frames():
    global _mm
    if _mm is None:
        _mm = np.memmap(os.path.join(S,'frames.bgr'), dtype=np.uint8, mode='r', shape=(NFRAMES,FH,FW,3))
    return _mm
def frame(i): return np.array(frames()[i])
def tframe(t): return frame(int(round(t*FPS)))

def grad(gray, s=0.9, clip=80.0):
    g = cv2.GaussianBlur(gray.astype(np.float32),(0,0),s)
    gx = cv2.Sobel(g,cv2.CV_32F,1,0,ksize=3); gy = cv2.Sobel(g,cv2.CV_32F,0,1,ksize=3)
    return np.minimum(cv2.magnitude(gx,gy),clip)
Y0,Y1 = 52,336
_mapc = {}
def mapdata():
    if not _mapc:
        col=np.load(S+'/M_col.npy'); al=np.load(S+'/M_alpha.npy')
        gray=cv2.cvtColor(col,cv2.COLOR_BGR2GRAY)
        ale=cv2.erode(al,np.ones((7,7),np.uint8))
        G=grad(gray); G[ale==0]=0
        _mapc.update(col=col,gray=gray,al=al,ale=ale,G=G)
    return _mapc

def mapcrop(cx,cy,w=FW,h=FH):
    """crop of M color+alpha at float pos (cx,cy) via bilinear shift (zeros outside)"""
    md=mapdata(); col=md['col']; H,W=col.shape[:2]
    M=np.float32([[1,0,-cx],[0,1,-cy]])
    c=cv2.warpAffine(col,M,(w,h),flags=cv2.INTER_LINEAR,borderMode=cv2.BORDER_CONSTANT,borderValue=0)
    a=cv2.warpAffine(md['al'],M,(w,h),flags=cv2.INTER_LINEAR,borderMode=cv2.BORDER_CONSTANT,borderValue=0)
    return c,a
def overlay_check(i,cx,cy):
    f=frame(i); c,a=mapcrop(cx,cy)
    d=np.abs(f.astype(int)-c.astype(int)).sum(2).clip(0,255).astype(np.uint8)
    d[a<250]=0
    return np.hstack([f,c,cv2.cvtColor(d,cv2.COLOR_GRAY2BGR)])

def sheet(idx,cols=4,scale=0.5,path=None,label=True):
    tiles=[]
    for i in idx:
        f=cv2.resize(frame(i),None,fx=scale,fy=scale)
        if label: cv2.putText(f,'f%d t%.1f'%(i,i/FPS),(3,int(350*scale)),0,0.5,(0,255,255),1)
        tiles.append(f)
    while len(tiles)%cols: tiles.append(np.zeros_like(tiles[0]))
    img=np.vstack([np.hstack(tiles[r:r+cols]) for r in range(0,len(tiles),cols)])
    if path: cv2.imwrite(path,img)
    return img
