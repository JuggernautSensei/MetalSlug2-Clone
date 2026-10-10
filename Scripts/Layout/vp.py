import sys; sys.path.insert(0,'.')
from viewer import *
out=S+'/v_%s.png'%sys.argv[1]
ts=[float(a) for a in sys.argv[2:]]
sc=1.7
pair(ts,out,scale=sc)
