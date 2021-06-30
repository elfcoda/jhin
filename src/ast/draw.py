### 其实这个脚本还不够通用，应该能够根据任意的简单list结构，包括多叉树，来生成树

import matplotlib.pyplot as plt
import graph as gr

tree = gr.tree

# width 个节点宽度
width = tree[0][1]
depth = gr.depth

width_len = 20
depth_len = 20

width_margin = 50
depth_margin = 50

fig, ax = plt.subplots()
x = width * width_len + 2 * width_margin
y = depth * depth_len + 2 * depth_margin
ax.set_xlim(0, x)
ax.set_ylim(0, y)

fc_pick = ['b', 'g', 'r', 'c', 'm', 'y', 'k', 'w']
ec_pick = ['b', 'g', 'r', 'c', 'm', 'y', 'k', 'w']
# 挑选颜色
def color_pick():
    for i in range(len(fc_pick)):
        for j in range(len(ec_pick)):
            ax.annotate("PICK ME: (" + fc_pick[i] + ", " + ec_pick[j] + ")", xy=(i * 10, j * 1), xycoords="data",
                        va="center", ha="center",
                        bbox=dict(boxstyle="round4", fc=fc_pick[i], ec=ec_pick[j]))

fc_color = ['w', 'c', 'r', 'w', 'm', 'y']
ec_color = ['k', 'g', 'm', 'r', 'b', 'm']
# 叶子节点等密度算法
def draw(root, leftOffset, topOffset):
    children_total_width = leftOffset
    x1 = 0
    y1 = 0
    x2 = 0
    y2 = 0
    for idx in range(len(root)):
        if idx == 0:
            curWidth = root[0][1]
            x1 = leftOffset + curWidth / 2 * width_len
            y1 = y - topOffset - depth_len / 2
            ax.annotate(root[0][0], xy=(x1, y1), xycoords="data",
                    va="center", ha="center",
                    bbox=dict(boxstyle="round4", fc=fc_color[root[0][2]], ec=ec_color[root[0][2]]))
        else:
            draw(root[idx], children_total_width, topOffset + depth_len)
            delta = root[idx][0][1] * width_len
            children_total_width += delta
            # line
            x2 = children_total_width - delta / 2
            y2 = y - (topOffset + depth_len / 2 * 3)
            ax.plot([x1, x2], [y1, y2])

# color_pick()
draw(tree, width_margin, depth_margin)

plt.show()

