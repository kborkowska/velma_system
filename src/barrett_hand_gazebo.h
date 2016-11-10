/*
 Copyright (c) 2014, Robot Control and Pattern Recognition Group, Warsaw University of Technology
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the Warsaw University of Technology nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL <COPYright HOLDER> BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef BARRETT_HAND_GAZEBO_H__
#define BARRETT_HAND_GAZEBO_H__

#include <ros/callback_queue.h>
#include <ros/advertise_options.h>
#include <std_msgs/Empty.h>
#include <std_msgs/Int32.h>

#include <gazebo/gazebo.hh>
#include <gazebo/physics/physics.hh>
//#include <gazebo/physics/dart/DARTModel.hh>
//#include <gazebo/physics/dart/DARTJoint.hh>
#include <gazebo/common/common.hh>

#include <ros/ros.h>
#include <kdl/chain.hpp>
#include <kdl/chaindynparam.hpp>
#include <kdl/chainfksolverpos_recursive.hpp>
#include <kdl/chainjnttojacsolver.hpp>
#include <kdl_parser/kdl_parser.hpp>

#include "Eigen/Dense"

#include <rtt/Component.hpp>
#include <rtt/Port.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/Logger.hpp>

//#include <geometry_msgs/Pose.h>
#include <geometry_msgs/Wrench.h>
//#include <geometry_msgs/Twist.h>

#include <kuka_lwr_fri/friComm.h>

class BarrettHandGazebo : public RTT::TaskContext
{
protected:
    typedef Eigen::Matrix<double, 4, 1> Dofs;
    typedef Eigen::Matrix<double, 8, 1> Joints;

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    // orocos ports
    RTT::InputPort<Dofs>  port_q_in_;
    RTT::InputPort<Dofs>  port_v_in_;
    RTT::InputPort<Dofs>  port_t_in_;
    RTT::InputPort<double>           port_mp_in_;
    RTT::InputPort<int32_t>          port_hold_in_;
    RTT::InputPort<Dofs > port_max_measured_pressure_in_;
    RTT::InputPort<std_msgs::Empty>  port_reset_in_;
    RTT::OutputPort<uint32_t>        port_status_out_;
    RTT::OutputPort<Joints> port_q_out_;
    RTT::OutputPort<Joints> port_t_out_;
    //RTT::OutputPort<barrett_hand_controller_msgs::BHTemp> port_temp_out_;

    Dofs q_in_;
    Dofs v_in_;
    Dofs t_in_;
    double          mp_in_;
    int32_t         hold_in_;
    Dofs max_measured_pressure_in_;
    std_msgs::Empty reset_in_;
    uint32_t        status_out_;
    Joints q_out_;
    Joints t_out_;
    //barrett_hand_controller_msgs::BHTemp temp_out_;

    // public methods
    BarrettHandGazebo(std::string const& name);
    ~BarrettHandGazebo();
    void updateHook();
    bool startHook();
    bool configureHook();
    bool gazeboConfigureHook(gazebo::physics::ModelPtr model);
    void gazeboUpdateHook(gazebo::physics::ModelPtr model);

  protected:

    enum {STATUS_OVERCURRENT1 = 0x0001, STATUS_OVERCURRENT2 = 0x0002, STATUS_OVERCURRENT3 = 0x0004, STATUS_OVERCURRENT4 = 0x0008,
        STATUS_OVERPRESSURE1 = 0x0010, STATUS_OVERPRESSURE2 = 0x0020, STATUS_OVERPRESSURE3 = 0x0040,
        STATUS_TORQUESWITCH1 = 0x0100, STATUS_TORQUESWITCH2 = 0x0200, STATUS_TORQUESWITCH3 = 0x0400,
        STATUS_IDLE1 = 0x1000, STATUS_IDLE2 = 0x2000, STATUS_IDLE3 = 0x4000, STATUS_IDLE4 = 0x8000 };

    double clip(double n, double lower, double upper) const;
    double getFingerAngle(int fidx) const;

    std::string prefix_;

    ros::NodeHandle *nh_;

    gazebo::physics::ModelPtr model_;
//    gazebo::physics::DARTModelPtr model_dart_;
//    dart::dynamics::Skeleton *dart_sk_;
//    dart::simulation::World *dart_world_;

    bool data_valid_;

    // BarrettHand
    std::vector<gazebo::physics::JointPtr> joints_;
    std::vector<std::string> joint_scoped_names_;

//    std::vector<dart::dynamics::Joint*>  joints_dart_;

    std::vector<int > too_big_force_counter_;
    bool move_hand_;

    bool clutch_break_[3];
    double clutch_break_angle_[3];

    double spread_int_;
    double finger_int_[3];

    gazebo::physics::JointController *jc_;

    //! Synchronization
    RTT::os::MutexRecursive gazebo_mutex_;
};

#endif  // BARRETT_HAND_GAZEBO_H__

