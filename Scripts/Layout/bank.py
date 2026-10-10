import sys; sys.path.insert(0,'.')
from sprites import *
SX,SY=0.7906,0.7912
def scaled(img):
    h,w=img.shape[:2]
    nw=max(2,int(round(w*SX))); nh=max(2,int(round(h*SY)))
    bgr=img[:,:,:3].astype(np.float32); a=img[:,:,3:4].astype(np.float32)/255
    pm=cv2.resize(bgr*a,(nw,nh),interpolation=cv2.INTER_AREA); ar=cv2.resize(a[:,:,0],(nw,nh),interpolation=cv2.INTER_AREA)
    col=np.clip(pm/np.maximum(ar[:,:,None],1e-3),0,255)
    mask=(ar>0.5)
    if mask.sum()==0: return None,None
    mean=col[mask].mean(0)
    col[~mask]=mean
    return col.astype(np.uint8),(mask*255).astype(np.uint8)
def build(clips,every=1):
    bank=[]
    for c in clips:
        fr=clip_frames(c)
        for k in range(0,len(fr),every):
            img,px,py=fr[k]
            col,mask=scaled(img)
            if col is None or mask.sum()/255<100: continue
            bank.append(dict(clip=c,idx=k,col=col,mask=mask,px=px*SX,py=py*SY))
    return bank
def match_bank(frm,bank,thr=0.75,nms=True):
    out=[]
    for b in bank:
        t=b['col']
        if t.shape[0]>=frm.shape[0] or t.shape[1]>=frm.shape[1]: continue
        R=cv2.matchTemplate(frm,t,cv2.TM_CCOEFF_NORMED)
        ys,xs=np.where(R>=thr)
        for y,x in zip(ys,xs): out.append((float(R[y,x]),int(x),int(y),t.shape[1],t.shape[0],b['clip'],b['idx']))
    return out
