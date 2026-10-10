import sys; sys.path.insert(0,'.')
from multiprocessing import Pool
def work(i):
    import sift_loc
    try: r=sift_loc.loc(i)
    except Exception as e: r=None
    return i,r
if __name__=='__main__':
    import pickle
    from common import *
    N=int(250*FPS)
    idx=list(range(0,N,6))
    with Pool(14) as p: res=p.map(work,idx,chunksize=8)
    pickle.dump(res,open(S+'/sift_all.pkl','wb'))
    print(len(res))
