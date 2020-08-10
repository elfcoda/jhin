import matplotlib.pyplot as plt
import graph as gr

tree = [("1", 34, 5), [("2", 30, 2)], [("3", 4, 1)]]

fig, ax = plt.subplots()
ax.set_xlim(0, 400)
ax.set_ylim(0, 400)

ax.plot([1, 2, 3, 4], [1, 4, 2, 3])

def draw():
    for idx in range(len(tree)):
        if idx == 0:
            ax.annotate(tree[0], xy=(300, 300), xycoords="data",
                    va="center", ha="center",
                    bbox=dict(boxstyle="round4", fc="cyan", ec="b"))
        else:


plt.show()
