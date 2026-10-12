import sys; sys.path.insert(0,'.')
from viewer import *
def greenboxes(i):
    f=frame(i); hsv=cv2.cvtColor(f,cv2.COLOR_BGR2HSV)
    m=((hsv[:,:,0]>=27)&(hsv[:,:,0]<=48)&(hsv[:,:,1]>=70)&(hsv[:,:,2]>=35)).astype(np.uint8)
    m[:50]=0; m[338:]=0
    m=cv2.dilate(m,cv2.getStructuringElement(cv2.MORPH_ELLIPSE,(7,7)))
    n,lab,st,cen=cv2.connectedComponentsWithStats(m)
    out=[]
    for k in range(1,n):
        x,y,w,h,a=st[k]
        if a<90: continue
        out.append((x,y,w,h))
    return out
def extra(f,i,sc):
    for (x,y,w,h) in greenboxes(i):
        cv2.rectangle(f,(int(x*sc),int(y*sc)),(int((x+w)*sc),int((y+h)*sc)),(255,0,255),1)
        wx,wy=s2w(x+w/2,y+h,i)
        cv2.putText(f,'%d,%d'%(wx,wy),(int(x*sc),int(y*sc)-3),0,0.4,(255,255,255),1,cv2.LINE_AA)
if __name__=='__main__':
    out=S+'/v_%s.png'%sys.argv[1]
    ts=[float(a) for a in sys.argv[2:]]
    imgs=[render(int(round(t*FPS)),1.7,extra=extra) for t in ts]
    cv2.imwrite(out,np.vstack(imgs))
