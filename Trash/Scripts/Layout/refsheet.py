import sys; sys.path.insert(0,'.')
from bank import *
def tile(name,k=0,sc=1.6):
    fr=clip_frames(name); k=min(k,len(fr)-1)
    img=fr[k][0]; col,mask=scaled(img)
    t=cv2.resize(col,None,fx=sc,fy=sc,interpolation=cv2.INTER_CUBIC)
    bg=np.full((int(120*sc/1.6*1.0)+10,150,3),(110,150,190),np.uint8)
    h,w=t.shape[:2]
    h=min(h,bg.shape[0]-14); w=min(w,148)
    bg[bg.shape[0]-3-h:bg.shape[0]-3,2:2+w]=t[:h,:w]
    cv2.putText(bg,name[:24],(1,10),0,0.32,(0,0,0),1)
    return bg
if __name__=='__main__':
    names=sys.argv[2:]
    tiles=[tile(n) for n in names]
    while len(tiles)%6: tiles.append(np.zeros_like(tiles[0]))
    img=np.vstack([np.hstack(tiles[i:i+6]) for i in range(0,len(tiles),6)])
    cv2.imwrite(sys.argv[1],img)
