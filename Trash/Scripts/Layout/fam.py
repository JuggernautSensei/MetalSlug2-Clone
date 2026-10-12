import sys; sys.path.insert(0,'.')
from bank import *
FAMS={
 'sd':['SD_Idle_Left','SD_Idle_Right','SD_Move_Left','SD_Move_Right','SD_Throw_Bomb_Left','SD_Throw_Bomb_Right','SD_Slash_Left','SD_Slash_Right','SD_Jump_Slash_Left','SD_Jump_Slash_Right','SD_Jump_Slash_Ready_Left','SD_Jump_Slash_Ready_Right','SD_Hang_Idle_Left','SD_Hang_Throw_Bomb_Left','SD_Hang_Throw_Bomb_Right','SD_Jump_Left','SD_Jump_Right','SD_MoveJump_Left','SD_MoveJump_Right','SD_Throw_Missle_Left','SD_Throw_Missle_Right','SD_Missle_Left','SD_Missle_Right','SD_Land_Left','SD_Land_Right','SD_MoveStop_Left','SD_MoveStop_Right','SD_MoveDown_Left','SD_MoveDown_Right'],
 'bzk':['Bazuka_Idle_Left','Bazuka_Idle_Right','Bazuka_Move_Left','Bazuka_Move_Right','Bazuka_Shot_Left','Bazuka_Shot_Right','Bazuka_Shot_Ready_Left','Bazuka_Shot_Ready_Right','Bazuka_Jump_Left','Bazuka_Jump_Right','Bazuka_Fly_Shot_Ready_2_Left'],
 'shd':['Shield_Idle_Left','Shield_Idle_Right','Shield_Move_Left','Shield_Move_Right','Shield_Defence_Left','Shield_Defence_Right','Shield_Shot_Left','Shield_Shot_Right','Shield_Shot_Ready_Left','Shield_Shot_Ready_Right','Shield_Jump_Left','Shield_Jump_Right'],
 'pow':['POW_V1_Stuck_Left','POW_V1_Stuck_Right','POW_V2_Stuck_Left','POW_V2_Stuck_Right','POW_V3_Stuck_Left','POW_V3_Stuck_Right'],
}
_b={}
def bankof(f):
    if f not in _b: _b[f]=build([c for c in FAMS[f] if c in anims()])
    return _b[f]
def detect_family(frm,f,thr=0.62,y0=50,y1=338):
    out=[]
    for b in bankof(f):
        t=b['col']
        R=cv2.matchTemplate(frm,t,cv2.TM_CCOEFF_NORMED)
        h,w=t.shape[:2]
        ys,xs=np.where(R>=thr)
        for y,x in zip(ys,xs):
            if y+h<y0 or y>y1: continue
            out.append((float(R[y,x]),x,y,w,h,b['clip'],b['idx']))
    out.sort(reverse=True)
    keep=[]
    for d in out:
        cx=d[1]+d[3]/2; cy=d[2]+d[4]/2
        if all(abs(cx-(k[1]+k[3]/2))>0.6*max(d[3],k[3]) or abs(cy-(k[2]+k[4]/2))>0.6*max(d[4],k[4]) for k in keep): keep.append(d)
    return keep
if __name__=='__main__':
    import time
    t=float(sys.argv[1]); fam=sys.argv[2]; thr=float(sys.argv[3])
    i=int(round(t*FPS)); f=frame(i)
    t0=time.time(); d=detect_family(f,fam,thr); print(time.time()-t0,len(d))
    img=cv2.resize(f.copy(),None,fx=2,fy=2,interpolation=cv2.INTER_CUBIC)
    for s,x,y,w,h,c,k in d[:25]:
        cv2.rectangle(img,(2*x,2*y),(2*(x+w),2*(y+h)),(255,0,255),1)
        cv2.putText(img,'%.2f %s'%(s,c[:14]),(2*x,2*y-2),0,0.35,(255,255,255),1)
    cv2.imwrite(S+'/fam_test.png',img)
    for s,x,y,w,h,c,k in d[:12]: print(round(s,2),x,y,c,k)
