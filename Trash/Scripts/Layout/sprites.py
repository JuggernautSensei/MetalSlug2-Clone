import sys; sys.path.insert(0,'.')
from common import *
_atl={}
_anim=None
def anims():
    global _anim
    if _anim is None: _anim=json.load(open(ROOT+r'\Assets\anim.json'))['animations']
    return _anim
def atlas(n):
    if n not in _atl: _atl[n]=cv2.imread(ROOT+r'\Assets\Atlas\atlas_%03d.png'%n,cv2.IMREAD_UNCHANGED)
    return _atl[n]
def clip_frames(name):
    out=[]
    for fr in anims()[name]['frames']:
        a=atlas(fr['atlas']); x,y,w,h=fr['x'],fr['y'],fr['w'],fr['h']
        out.append((a[y:y+h,x:x+w].copy(),fr['pivotX'],fr['pivotY']))
    return out
