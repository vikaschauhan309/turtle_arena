"""
Integration test for the cmd_vel -> Box2D -> odom wiring.

Launches the real turtle_arena_node in headless mode (no SFML window, so
this runs with no display), publishes a Twist on /turtle1/cmd_vel, and
asserts /turtle1/odom reports the robot has actually moved -- catches
regressions in the physics/ROS2 wiring without needing to look at a
rendered window.
"""
import time
import unittest

import launch
import launch_ros.actions
import launch_testing.actions
import launch_testing.markers
import pytest
import rclpy
from geometry_msgs.msg import Twist
from nav_msgs.msg import Odometry
from rclpy.node import Node


@pytest.mark.launch_test
@launch_testing.markers.keep_alive
def generate_test_description():
    turtle_arena_node = launch_ros.actions.Node(
        package="turtle_arena",
        executable="turtle_arena_node",
        name="turtle_arena_node",
        output="screen",
        parameters=[{"headless": True}],
    )

    return launch.LaunchDescription(
        [
            turtle_arena_node,
            launch_testing.actions.ReadyToTest(),
        ]
    )


class TestCmdVelMovesOdom(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        rclpy.init()

    @classmethod
    def tearDownClass(cls):
        rclpy.shutdown()

    def setUp(self):
        self.node = Node("test_cmd_vel_to_odom_driver")
        self.cmd_pub = self.node.create_publisher(Twist, "/turtle1/cmd_vel", 10)
        self.odom_positions = []

        def on_odom(msg):
            self.odom_positions.append(msg.pose.pose.position)

        self.node.create_subscription(Odometry, "/turtle1/odom", on_odom, 10)

    def tearDown(self):
        self.node.destroy_node()

    def _spin_for(self, seconds: float):
        deadline = time.time() + seconds
        while time.time() < deadline:
            rclpy.spin_once(self.node, timeout_sec=0.05)

    def test_cmd_vel_moves_robot(self):
        # Give the node time to come up and the subscription to connect.
        self._spin_for(1.0)
        self.assertTrue(len(self.odom_positions) > 0, "no /turtle1/odom messages received at all")
        start_x = self.odom_positions[-1].x

        twist = Twist()
        twist.linear.x = 1.0
        deadline = time.time() + 2.0
        while time.time() < deadline:
            self.cmd_pub.publish(twist)
            rclpy.spin_once(self.node, timeout_sec=0.05)

        self.cmd_pub.publish(Twist())  # stop
        self._spin_for(0.3)

        end_x = self.odom_positions[-1].x
        self.assertGreater(
            end_x, start_x + 0.5,
            f"expected /turtle1/odom x to advance after cmd_vel, start={start_x} end={end_x}",
        )
