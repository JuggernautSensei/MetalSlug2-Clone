import sys; sys.path.insert(0,'.')
from viewer import *
def detect(i,minarea=70):
    f=frame(i); hsv=cv2.cvtColor(f,cv2.COLOR_BGR2HSV)
    m=((hsv[:,:,0]>=27)&(hsv[:,:,0]<=48)&(hsv[:,:,1]>=70)&(hsv[:,:,2]>=35)).astype(np.uint8)
    m[:50]=0; m[338:]=0
    raw=m.copy()
    m=cv2.dilate(m,cv2.getStructuringElement(cv2.MORPH_ELLIPSE,(7,7)))
    n,lab,st,cen=cv2.connectedComponentsWithStats(m)
    out=[]
    for k in range(1,n):
        x,y,w,h,a=st[k]
        a2=int(raw[y:y+h,x:x+w][lab[y:y+h,x:x+w]==k].sum())
        if a2<minarea: continue
        wx,wy=s2w(x+w/2,y+h,i)
        wl,_=s2w(x,0,i); wr,_=s2w(x+w,0,i)
        out.append((i,wx,wy,wr-wl,h/SYg,a2,x,y,w,h))
    return out
if __name__=='__main__':
    import pickle
    N=int(250*FPS)
    res=[]
    for t in np.arange(0,250,0.5):
        res+=detect(int(round(t*FPS)))
    pickle.dump(res,open(S+'/blobs.pkl','wb'))
    print(len(res), len(res)/500)
