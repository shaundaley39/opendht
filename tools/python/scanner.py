from cython.parallel import parallel, prange

from opendht import *
import time
from math import cos, sin, pi

from matplotlib.colors import colorConverter
from matplotlib.collections import RegularPolyCollection
import matplotlib.pyplot as plt
from numpy import nonzero

done = 0
all_nodes = PyNodeSet()


def gcb(v):
    print("get callback", v)
    return True

r = PyDhtRunner()
i = PyIdentity()
i.generate()

r.run(4222, i, True)
r.bootstrap("bootstrap.ring.cx", "4222")

time.sleep(2)


def step(cur_h, cur_depth):
    global done, all_nodes
    print("step", cur_h, cur_depth)
    done += 1
    r.get(cur_h, gcb, lambda d, nodes: nextstep(cur_h, cur_depth, d, nodes))

def nextstep(cur_h, cur_depth, ok, nodes):
    global done, all_nodes
    snodes = PyNodeSet()
    snodes.extend(nodes)
    all_nodes.extend(nodes)
    depth = PyInfoHash.commonBits(snodes.first(), snodes.last())+1
    print(cur_h, ":", snodes.size(), " ", depth, " cur ", cur_depth)
    if cur_depth < depth:
        for b in range(cur_depth, depth):
            new_h = PyInfoHash(cur_h.getId());
            new_h.setBit(b, 1);
            step(new_h, b+1);
    done -= 1

start_h = PyInfoHash()
start_h.setBit(159, 1)


step(start_h, 0)

plt.ion()

KEEP_BYTES = 4
ax = plt.axes(xlim=(-1.5,1.5), ylim=(-1.5,1.5), autoscale_on=False)

def update_plot():
    global done
    ax.cla()
    fig = ax.figure
    node_val = [int(n.getId()[:KEEP_BYTES*2], 16)/2**(KEEP_BYTES*8) for n in all_nodes]
    xys = [(cos(d*2*pi), sin(d*2*pi)) for d in node_val]
    #xys = [(cos(d*2*pi), sin(d*2*pi)) for d in range(150)]
    collection = RegularPolyCollection(
                fig.dpi, 6, sizes=(20,),
                facecolors=colorConverter.to_rgba('blue'),
                offsets = xys,
                transOffset = ax.transData)
    ax.add_collection(collection)
    fig.canvas.draw()

update_plot()
#plt.show()
while done > 0:
    time.sleep(1)
    update_plot()

print(all_nodes.size(), " nodes found :")

print(all_nodes)
