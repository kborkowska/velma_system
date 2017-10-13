#!/usr/bin/env python
import roslib; roslib.load_manifest('velma_task_cs_ros_interface')

import rospy
import math
import PyKDL
from threading import Thread

from velma_common.velma_interface import *
from planner.planner import *
from ros_utils.marker_publisher import MarkerPublisher

from moveit_msgs.msg import AttachedCollisionObject, CollisionObject
from shape_msgs.msg import SolidPrimitive
from geometry_msgs.msg import Pose
from visualization_msgs.msg import Marker
import tf_conversions.posemath as pm

def exitError(code):
    if code == 0:
        print "OK"
        exit(0)
    print "ERROR:", code
    exit(code)

class MarkerPublisherThread:
    def threaded_function(self, obj):
        pub = MarkerPublisher("attached_objects")
        while not self.stop_thread:
            pub.publishSinglePointMarker(PyKDL.Vector(), 1, r=1, g=0, b=0, a=1, namespace='default', frame_id=obj.link_name, m_type=Marker.CYLINDER, scale=Vector3(0.02, 0.02, 1.0), T=pm.fromMsg(obj.object.primitive_poses[0]))
            try:
                rospy.sleep(0.1)
            except:
                break

    def __init__(self, obj):
        self.thread = Thread(target = self.threaded_function, args = (obj, ))

    def start(self):
        self.stop_thread = False
        self.thread.start()

    def stop(self):
        self.stop_thread = True
        self.thread.join()

if __name__ == "__main__":

    rospy.init_node('test_jimp_planning_attached', anonymous=True)

    rospy.sleep(0.5)

    print "This test/tutorial executes complex motions"\
        " in Joint Impedance mode with additional objects"\
        " attached to end-effectors. Planning is used"\
        " in this example.\n"

    print "Running python interface for Velma..."
    velma = VelmaInterface("/velma_task_cs_ros_interface")
    print "Waiting for VelmaInterface initialization..."
    if not velma.waitForInit(timeout_s=10.0):
        print "Could not initialize VelmaInterface\n"
        exitError(1)
    print "Initialization ok!\n"

    print "Waiting for Planner initialization..."
    p = Planner(velma.maxJointTrajLen())
    if not p.waitForInit(timeout_s=10.0):
        print "Could not initialize Planner"
        exitError(2)
    print "Planner initialization ok!"

    print "Motors must be enabled every time after the robot enters safe state."
    print "If the motors are already enabled, enabling them has no effect."
    print "Enabling motors..."
    if velma.enableMotors() != 0:
        exitError(3)

    print "Moving to the current position..."
    js_start = velma.getLastJointState()
    velma.moveJoint(js_start[1], None, 1.0, start_time=0.5, position_tol=15.0/180.0*math.pi)
    error = velma.waitForJoint()
    if error != 0:
        print "The action should have ended without error, but the error code is", error
        exitError(4)

    print "Creating a virtual object attached to gripper..."

    # for more details refer to ROS docs for moveit_msgs/AttachedCollisionObject
    object1 = AttachedCollisionObject()
    object1.link_name = "right_HandGripLink"
    object1.object.header.frame_id = "right_HandGripLink"
    object1.object.id = "object1"
    object1_prim = SolidPrimitive()
    object1_prim.type = SolidPrimitive.CYLINDER
    object1_prim.dimensions=[None, None]    # set initial size of the list to 2
    object1_prim.dimensions[SolidPrimitive.CYLINDER_HEIGHT] = 1.0
    object1_prim.dimensions[SolidPrimitive.CYLINDER_RADIUS] = 0.02
    object1_pose = pm.toMsg(PyKDL.Frame(PyKDL.Rotation.RotY(math.pi/2)))
    object1.object.primitives.append(object1_prim)
    object1.object.primitive_poses.append(object1_pose)
    object1.object.operation = CollisionObject.ADD

    print "Publishing the attached object marker on topic /attached_objects"
    pub = MarkerPublisherThread(object1)
    pub.start()

    q_map_goal = {'torso_0_joint':0,
        'right_arm_0_joint':-0.3,
        'right_arm_1_joint':-1.8,
        'right_arm_2_joint':-1.25,
        'right_arm_3_joint':1.57,
        'right_arm_4_joint':0,
        'right_arm_5_joint':-0.5,
        'right_arm_6_joint':0,
        'left_arm_0_joint':0.3,
        'left_arm_1_joint':1.8,
        'left_arm_2_joint':-1.25,
        'left_arm_3_joint':-0.85,
        'left_arm_4_joint':0,
        'left_arm_5_joint':0.5,
        'left_arm_6_joint':0
        }

    print "Planning motion to the goal position using set of all joints..."

    print "Moving to valid position, using planned trajectory."
    goal_constraint_1 = qMapToConstraints(q_map_goal, 0.01)
    for i in range(3):
        rospy.sleep(0.5)
        js = velma.getLastJointState()
        print "Planning (try", i, ")..."
        traj, jn = p.plan(js, [goal_constraint_1], "impedance_joints", max_velocity_scaling_factor=0.2, planner_id="RRTConnect", attached_collision_objects=[object1])
        if traj == None:
            continue
        print "Executing trajectory..."
        if not velma.moveJointTraj(traj, jn, start_time=0.5):
            exitError(5)
        if velma.waitForJoint() == 0:
            break
        else:
            print "The trajectory could not be completed, retrying..."
            continue

    rospy.sleep(0.5)
    js = velma.getLastJointState()
    if not isConfigurationClose(q_map_goal, js):
        exitError(6)

    rospy.sleep(1.0)

    q_map_end = {'torso_0_joint':0,
        'right_arm_0_joint':-0.3,
        'right_arm_1_joint':-1.8,
        'right_arm_2_joint':1.25,
        'right_arm_3_joint':0.85,
        'right_arm_4_joint':0,
        'right_arm_5_joint':-0.5,
        'right_arm_6_joint':0,
        'left_arm_0_joint':0.3,
        'left_arm_1_joint':1.8,
        'left_arm_2_joint':-1.25,
        'left_arm_3_joint':-0.85,
        'left_arm_4_joint':0,
        'left_arm_5_joint':0.5,
        'left_arm_6_joint':0
        }

    print "Moving to starting position, using planned trajectory."
    goal_constraint_2 = qMapToConstraints(q_map_end, 0.01)
    for i in range(3):
        rospy.sleep(0.5)
        js = velma.getLastJointState()
        print "Planning (try", i, ")..."
        traj, jn = p.plan(js, [goal_constraint_2], "impedance_joints", max_velocity_scaling_factor=0.2, planner_id="RRTConnect", attached_collision_objects=[object1])
        if traj == None:
            continue
        print "Executing trajectory..."
        if not velma.moveJointTraj(traj, jn, start_time=0.5):
            exitError(7)
        if velma.waitForJoint() == 0:
            break
        else:
            print "The trajectory could not be completed, retrying..."
            continue
    rospy.sleep(0.5)
    js = velma.getLastJointState()
    if not isConfigurationClose(q_map_end, js):
        exitError(8)

    pub.stop()

    exitError(0)
