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

#include "lwr_gazebo.h"
#include <rtt/Logger.hpp>

using namespace RTT;

    void LWRGazebo::updateHook() {
        Logger::In in("LWRGazebo::updateHook");

        // Synchronize with gazeboUpdate()
        RTT::os::MutexLock lock(gazebo_mutex_);


        if (!data_valid_) {
            Logger::log() << Logger::Debug << "gazebo is not initialized" << Logger::endl;
            return;
        }
        else {
            Logger::log() << Logger::Debug << Logger::endl;
        }

        port_MassMatrix_out_.write(MassMatrix_out_);
        port_GravityTorque_out_.write(GravityTorque_out_);
        port_JointTorque_out_.write(JointTorque_out_);
        port_JointPosition_out_.write(JointPosition_out_);
        port_JointVelocity_out_.write(JointVelocity_out_);

        if (port_KRL_CMD_in_.read(KRL_CMD_in_) == RTT::NewData) {
            if (1 == KRL_CMD_in_.data) {
                if (!command_mode_) {
                    command_mode_ = true;
                    Logger::log() << Logger::Info <<  "switched to command mode" << Logger::endl;
                }
                else {
                    Logger::log() << Logger::Warning <<  "tried to switch to command mode while in command mode" << Logger::endl;
                }
            }
            else if (2 == KRL_CMD_in_.data) {
                if (command_mode_) {
                    command_mode_ = false;
                    Logger::log() << Logger::Info << "switched to monitor mode" << Logger::endl;
                }
                else {
                    Logger::log() << Logger::Warning <<  "tried to switch to monitor mode while in monitor mode" << Logger::endl;
                }
            }
        }

        if (port_JointTorqueCommand_in_.read(JointTorqueCommand_in_) == RTT::NewData) {
        }


        // FRI comm state
        FRIState_out_.quality = FRI_QUALITY_PERFECT;
        if (command_mode_) {
            FRIState_out_.state = FRI_STATE_CMD;
        }
        else {
            FRIState_out_.state = FRI_STATE_MON;
        }
        port_FRIState_out_.write(FRIState_out_);
        port_FRIState_out_.write(FRIState_out_);

        // FRI robot state
        RobotState_out_.power = 0x7F;
        RobotState_out_.error = 0;
        RobotState_out_.warning = 0;
        RobotState_out_.control = FRI_CTRL_JNT_IMP;
        port_RobotState_out_.write(RobotState_out_);

        port_CartesianWrench_out_.write(CartesianWrench_out_);
    }

    bool LWRGazebo::startHook() {
      return true;
    }

    bool LWRGazebo::configureHook() {
        Logger::In in("LWRGazebo::configureHook");

        if (init_joint_names_.size() != init_joint_positions_.size()) {
            Logger::log() << Logger::Error <<
                "ERROR: LWRGazebo::configureHook: init_joint_names_.size() != init_joint_positions_.size(), " <<
                init_joint_names_.size() << "!=" << init_joint_positions_.size() << Logger::endl;
            return false;
        }

        std::map<std::string, double> init_joint_map;
        for (int i=0; i<init_joint_names_.size(); i++) {
            Logger::log() << Logger::Info <<
                "LWRGazebo::configureHook: inital position: " << init_joint_names_[i] << " " <<
                init_joint_positions_[i] << Logger::endl;
            init_joint_map[init_joint_names_[i]] = init_joint_positions_[i];
        }
        setInitialPosition(init_joint_map);

        std::string joint_suffix[7] = {"_arm_0_joint", "_arm_1_joint", "_arm_2_joint",
                                        "_arm_3_joint", "_arm_4_joint", "_arm_5_joint",
                                        "_arm_6_joint"};
        joint_names_.resize(7);
        for (int i = 0; i < 7; ++i) {
            joint_names_[i] = name_ + joint_suffix[i];
        }
        Logger::log() << Logger::Info <<
            "tool parameters for " << name_ << " LWR: " << tool_.m << " " << tool_.com.x << " " <<
            tool_.com.y << " " << tool_.com.z << Logger::endl;

        mm_.reset( new manipulator_mass_matrix::Manipulator(model_, name_ + "_arm_0_joint", name_ + "_arm_6_joint",
                    tool_.m, gazebo::math::Vector3(tool_.com.x,tool_.com.y,tool_.com.z), tool_.ixx, tool_.ixy, tool_.ixz, tool_.iyy, tool_.iyz, tool_.izz) );

        return true;
    }

