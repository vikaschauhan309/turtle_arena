#!/usr/bin/env python3
"""
Generate comparison images showing turtle_arena vs turtlesim differences:
1. Physics/collision detection
2. Lidar sensor visualization
3. Multi-robot support
"""
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches

DARK = "#2c3e50"
GREEN = "#2ecc71"
RED = "#e74c3c"
YELLOW = "#f1c40f"


def comparison_physics():
    """Side-by-side: turtlesim (no collision) vs turtle_arena (real physics)"""
    fig, axes = plt.subplots(1, 2, figsize=(14, 4.6))

    for idx, ax in enumerate(axes):
        ax.set_xlim(0, 10)
        ax.set_ylim(0, 6.6)
        ax.axis('off')

        # Title
        title = "turtlesim (teleport)" if idx == 0 else "turtle_arena (physics)"
        ax.text(5, 6.3, title, ha="center", fontsize=12, fontweight="bold")

        # Arena boundary
        border = mpatches.Rectangle((0, 0), 10, 6, linewidth=2, edgecolor=DARK, facecolor='white')
        ax.add_patch(border)

        if idx == 0:
            # turtlesim: robot clips through wall
            wall = mpatches.Rectangle((7, 1), 0.4, 4, linewidth=1, edgecolor=DARK, facecolor='gray')
            ax.add_patch(wall)
            robot = mpatches.Circle((8.2, 3), 0.3, facecolor=GREEN, edgecolor=DARK, linewidth=1)
            ax.add_patch(robot)
            ax.text(5, 0.5, "Robot commanded to (8.5, 3) → clips through wall",
                   ha="center", fontsize=9, color=RED, style='italic')
        else:
            # turtle_arena: robot stops at collision
            wall = mpatches.Rectangle((7, 1), 0.4, 4, linewidth=1, edgecolor=DARK, facecolor='gray')
            ax.add_patch(wall)
            robot = mpatches.Circle((6.5, 3), 0.3, facecolor=GREEN, edgecolor=DARK, linewidth=1)
            ax.add_patch(robot)
            ax.text(5, 0.5, "Robot commanded to (8.5, 3) → stops at wall collision",
                   ha="center", fontsize=9, color=GREEN, style='italic')

    plt.tight_layout()
    plt.savefig('/home/vikaschauhan/claude/ros2/turtle_arena/media/comparison_physics.png', dpi=100, bbox_inches='tight')
    plt.close()
    print("generated comparison_physics.png")


def comparison_sensors():
    """turtlesim: no sensors vs turtle_arena: lidar /scan"""
    fig, axes = plt.subplots(1, 2, figsize=(14, 4.6))

    for idx, ax in enumerate(axes):
        ax.set_xlim(0, 10)
        ax.set_ylim(0, 6.6)
        ax.axis('off')

        title = "turtlesim (blind)" if idx == 0 else "turtle_arena (lidar)"
        ax.text(5, 6.3, title, ha="center", fontsize=12, fontweight="bold")

        border = mpatches.Rectangle((0, 0), 10, 6, linewidth=2, edgecolor=DARK, facecolor='white')
        ax.add_patch(border)

        # Maze wall
        wall = mpatches.Rectangle((4.5, 1), 0.4, 4, linewidth=1, edgecolor=DARK, facecolor='gray')
        ax.add_patch(wall)

        robot = mpatches.Circle((2, 3), 0.3, facecolor=GREEN, edgecolor=DARK, linewidth=1)
        ax.add_patch(robot)

        if idx == 0:
            ax.text(5, 0.5, "No sensors → must hardcode maze navigation",
                   ha="center", fontsize=9, color=RED, style='italic')
        else:
            # Draw lidar rays
            for angle in range(-60, 61, 20):
                rad = angle * 3.14159 / 180
                import math
                ex = 2 + 2 * math.cos(rad)
                ey = 3 + 2 * math.sin(rad)
                ax.plot([2, ex], [3, ey], 'r-', linewidth=0.5, alpha=0.6)

            ax.text(5, 0.5, "/scan published → autonomous navigation possible",
                   ha="center", fontsize=9, color=GREEN, style='italic')

    plt.tight_layout()
    plt.savefig('/home/vikaschauhan/claude/ros2/turtle_arena/media/comparison_sensors.png', dpi=100, bbox_inches='tight')
    plt.close()
    print("generated comparison_sensors.png")


def comparison_multirobot():
    """turtlesim: single turtle vs turtle_arena: multiple robots"""
    fig, axes = plt.subplots(1, 2, figsize=(14, 4.6))

    for idx, ax in enumerate(axes):
        ax.set_xlim(0, 10)
        ax.set_ylim(0, 6.6)
        ax.axis('off')

        title = "turtlesim (single)" if idx == 0 else "turtle_arena (multi-robot)"
        ax.text(5, 6.3, title, ha="center", fontsize=12, fontweight="bold")

        border = mpatches.Rectangle((0, 0), 10, 6, linewidth=2, edgecolor=DARK, facecolor='white')
        ax.add_patch(border)

        if idx == 0:
            robot = mpatches.Circle((5, 3), 0.3, facecolor=GREEN, edgecolor=DARK, linewidth=1)
            ax.add_patch(robot)
            ax.text(5, 0.5, "Only turtle1 — /spawn fails",
                   ha="center", fontsize=9, color=RED, style='italic')
        else:
            robot1 = mpatches.Circle((3, 3), 0.3, facecolor=GREEN, edgecolor=DARK, linewidth=1)
            robot2 = mpatches.Circle((7, 3), 0.3, facecolor=YELLOW, edgecolor=DARK, linewidth=1)
            ax.add_patch(robot1)
            ax.add_patch(robot2)
            ax.text(3, 0.8, "turtle1", ha="center", fontsize=8)
            ax.text(7, 0.8, "turtle2", ha="center", fontsize=8)
            ax.text(5, 0.2, "/spawn works → each robot: /cmd_vel, /odom, /scan",
                   ha="center", fontsize=9, color=GREEN, style='italic')

    plt.tight_layout()
    plt.savefig('/home/vikaschauhan/claude/ros2/turtle_arena/media/comparison_multirobot.png', dpi=100, bbox_inches='tight')
    plt.close()
    print("generated comparison_multirobot.png")


if __name__ == "__main__":
    comparison_physics()
    comparison_sensors()
    comparison_multirobot()
    print("all comparison images generated")
