import sys; sys.path.insert(0,'.')
from common import *
md=mapdata(); G=md['G']
def gsearch(i,xr=(0,11134),yr=(0,1049),topk=5):
    f=cv2.cvtColor(frame(i),cv2.COLOR_BGR2GRAY); Gf=grad(f)[Y0:Y1]
    xa,xb=xr; ya,yb=yr
    win=G[ya:yb, xa:xb]
    R=cv2.matchTemplate(win,Gf,cv2.TM_CCORR_NORMED)
    res=[]
    R2=R.copy()
    for k in range(topk):
        _,mx,_,ml=cv2.minMaxLoc(R2)
        res.append((round(float(mx),3),ml[0]+xa,ml[1]+ya-Y0))
        cv2.rectangle(R2,(ml[0]-40,ml[1]-40),(ml[0]+40,ml[1]+40),0,-1)
    return res
if __name__=='__main__':
    for i in [int(a) for a in sys.argv[1:]]:
        print(i,round(i/FPS,1),gsearch(i,(4000,9000),(100,1049)))
