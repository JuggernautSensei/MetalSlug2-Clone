import sys; sys.path.insert(0,'.')
from common import *
from multiprocessing import Pool
cam=np.load(S+'/cam_M2.npy')
def w(i):
    import sift_win
    r=sift_win.locw(i,cam[i,0],cam[i,1],rx=60,ry=40)
    if r is None or isinstance(r[0],str): return (i,0,0,0)
    return (i,r[0],r[2]-cam[i,0],r[3]-cam[i,1])
if __name__=='__main__':
    rng=np.random.RandomState(1)
    idx=sorted(set(int(x)*4+2 for x in rng.randint(30,1840,400)))   # off the anchor grid (anchors at 240+4k -> use +2)
    with Pool(14) as p: r=np.array(p.map(w,idx))
    ok=r[:,1]>=25
    print('n valid',ok.sum(),'of',len(r))
    e=r[ok]
    print('dx abs pct 50/90/99/max',np.percentile(abs(e[:,2]),[50,90,99,100]).round(2))
    print('dy abs pct',np.percentile(abs(e[:,3]),[50,90,99,100]).round(2))
    # by time bin
    for a,b in [(0,60),(60,100),(100,140),(140,180),(180,227)]:
        m=(e[:,0]>=a*FPS)&(e[:,0]<b*FPS)
        if m.sum(): print(a,b,m.sum(),'rms dx %.2f dy %.2f'%(np.sqrt((e[m,2]**2).mean()),np.sqrt((e[m,3]**2).mean())))
