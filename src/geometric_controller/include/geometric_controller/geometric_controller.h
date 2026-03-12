/****************************************************************************
 *
 *   Copyright (c) 2018-2021 Jaeyoung Lim. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/
/**
 * @brief Geometric Controller
 *
 * Geometric controller
 *
 * @author Jaeyoung Lim <jalim@ethz.ch>
 */

#ifndef GEOMETRIC_CONTROLLER_H
#define GEOMETRIC_CONTROLLER_H

#include <ros/ros.h>
#include <ros/subscribe_options.h>
#include <tf/transform_broadcaster.h>

#include <stdio.h>
#include <cstdlib>
#include <sstream>
#include <string>

#include <geometry_msgs/Point.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/TwistStamped.h>
#include <mavros_msgs/AttitudeTarget.h>
#include <mavros_msgs/CommandBool.h>
#include <mavros_msgs/CompanionProcessStatus.h>
#include <mavros_msgs/SetMode.h>
#include <mavros_msgs/State.h>
#include <nav_msgs/Odometry.h>
#include <nav_msgs/Path.h>
#include <std_msgs/Float32.h>
#include <std_msgs/Bool.h>
#include <Eigen/Dense>
#include <geometry_msgs/Vector3.h>
#include <mavros_msgs/CommandLong.h>
#include <mavros_msgs/CommandHome.h>
#include <mavros_msgs/ParamGet.h>
#include <mavros_msgs/ParamSet.h>
#include <mavros_msgs/ParamValue.h>
/* #include <controller_msgs/FlatTarget.h> */
#include <dynamic_reconfigure/server.h>
#include <geometric_controller/GeometricControllerConfig.h>
#include <std_srvs/SetBool.h>
#include <trajectory_msgs/MultiDOFJointTrajectory.h>
#include <trajectory_msgs/MultiDOFJointTrajectoryPoint.h>

#include "geometric_controller/common.h"
#include <std_msgs/String.h>
#include "udp.h"
#include <memory>
#define ERROR_QUATERNION 1
#define ERROR_GEOMETRIC 2

using namespace std;
using namespace Eigen;

enum class MAV_STATE {
  MAV_STATE_UNINIT,
  MAV_STATE_BOOT,
  MAV_STATE_CALIBRATIN,
  MAV_STATE_STANDBY,
  MAV_STATE_ACTIVE,
  MAV_STATE_CRITICAL,
  MAV_STATE_EMERGENCY,
  MAV_STATE_POWEROFF,
  MAV_STATE_FLIGHT_TERMINATION,
};

class geometricCtrl {
 private:
  ros::NodeHandle nh_;
  ros::NodeHandle nh_private_;
  ros::Subscriber planner_point_occ_Sub_;
  ros::Subscriber referenceSub_;
  ros::Subscriber flatreferenceSub_;
  ros::Subscriber multiDOFJointSub_;
  ros::Subscriber mavstateSub_;
  ros::Subscriber mavposeSub_, gzmavposeSub_;
  ros::Subscriber mavtwistSub_;
  ros::Subscriber yawreferenceSub_;
  ros::Publisher rotorVelPub_, angularVelPub_, target_pose_pub_,planner_target_pub_, raw_pub_;
  ros::Publisher referencePosePub_;
  ros::Publisher posehistoryPub_;
  ros::Publisher systemstatusPub_;
  ros::ServiceClient arming_client_;
  ros::ServiceClient set_mode_client_;
  ros::ServiceServer ctrltriggerServ_;
  ros::ServiceServer land_service_;
  // 飞控参数获取/修改服务
  ros::ServiceClient px4_param_get_client_;
  ros::ServiceClient px4_param_set_client_;
  
  ros::Publisher gimbalCtrlPub_;
  ros::Timer cmdloop_timer_, statusloop_timer_,vision_point_process_timer_;
  ros::Time last_request_, reference_request_now_, reference_request_last_, last_control_, last_pos_receiver_;
  ros::Publisher eularPub_;
  string mav_name_;
  bool fail_detec_, ctrl_enable_, feedthrough_enable_,have_pose_;//have_pose_用于判断飞机是否具备位置信息，如果有，则证明定位信息给入
  int ctrl_mode_;
  bool landing_commanded_;
  bool sim_enable_;
  bool velocity_yaw_;
  double kp_rot_, kd_rot_;
  double reference_request_dt_;
  double attctrl_tau_;
  double norm_thrust_const_, norm_thrust_offset_;
  double max_fb_acc_;
  double dx_, dy_, dz_;
  bool planner_point_occ_state_;
  bool traj_receive_flag_;

    // 添加偏航率参数
  double mc_yawrate_max_ = 120.0;
  double mc_yawrate_land_ = 0.0;

  mavros_msgs::State current_state_;
  mavros_msgs::SetMode offb_set_mode_;
  mavros_msgs::CommandBool arm_cmd_;
  std::vector<geometry_msgs::PoseStamped> posehistory_vector_;

  MAV_STATE companion_state_ = MAV_STATE::MAV_STATE_ACTIVE;

  double initTargetPos_x_, initTargetPos_y_, initTargetPos_z_,takeoff_height_,sample_distance_;
  Eigen::Vector3d targetPos_, targetVel_, targetAcc_, targetJerk_, targetSnap_, targetPos_prev_, targetVel_prev_;
  Eigen::Vector3d mavPos_, mavVel_, mavRate_, newPose_, newRate_;
  Eigen::Vector3d last_ref_acc_{Eigen::Vector3d::Zero()};
  double mavYaw_;
  double fix_mavYaw_;
  Eigen::Quaterniond fix_q_;
  Eigen::Vector3d g_;
  Eigen::Vector4d mavAtt_, q_des;
  Eigen::Vector4d cmdBodyRate_;  //{wx, wy, wz, Thrust}
  Eigen::Vector3d Kpos_, Kvel_, D_, Ki_vel_;
  Eigen::Vector3d a0, a1, tau;
  Eigen::Vector3d Vel_int;
  Eigen::Vector3d target_point_,target_point_prev_,vision_target_point_, temp_target_point_;
  double tau_x, tau_y, tau_z;
  double Kpos_x_, Kpos_y_, Kpos_z_, Kvel_x_, Kvel_y_, Kvel_z_, Ki_vel_x_, Ki_vel_y_, Ki_vel_z_;
  int posehistory_window_;
  int yaw_type_,control_mode_,takeoff_mode_,auto_type_;
  int Visiontarget_count_;
  bool Visiontarget_detect_flag_;
  std::vector<Eigen::Vector3d> path_to_end;  // 如果需要一个粗略的终点，则从param读取end pt后，按照一定比率计算下采样
  std::vector<Eigen::Vector3d> vision_path;  
  std::vector<Eigen::Vector3d> temp_vec; //临时的轨迹点
  std::vector<Eigen::Vector2i> yt_to_end; //yuntai value
  std::vector<Eigen::Vector4d> pose_to_end; //pose quanternoin w, x, y, z
  
  
  Eigen::Vector2i yt_point_;
  Eigen::Vector4d pose_point_;

  
  float pose_yaw, prexYaw_, yaw_diff, mavros_yaw; //uav yaw 2025-12-22
  float holdingtime_; //悬停时间 2025-12-22

  std::vector<float> posyaw_to_end; //uav yaw 2025-12-22
  float posyaw_;

  std::vector<float> posvel_to_end;
  float posvel_; //fly speed 2025-12-22

  std::vector<float> poshold_to_end;
  float poshold_;

  std::vector<int> fsm_to_end; //状态机变换  VISION_PT_GEN， HOLD_PT
  int fsm_flag_;

  std::string vision_goals_file_;
  // 这里要注意，要找寻Vision_PATH最后一个点和path_to_end未执行点的关系（X），
  // 若执行完成vision_path，发现其X大于path_to_end未执行点，则清掉对比下一个，若path_to_end删除干净，则认为当前点为end点，执行end操作
  void plannerpointstateCallback(const std_msgs::Bool &state);

  void pubMotorCommands();
  void pubRateCommands(const Eigen::Vector4d &cmd, const Eigen::Vector4d &target_attitude);
  void pubReferencePose(const Eigen::Vector3d &target_position, const Eigen::Vector4d &target_attitude);
  void pubPoseHistory();
  void pubSystemStatus();
  void appendPoseHistory();
  
  void odomCallback(const nav_msgs::OdometryConstPtr &odomMsg);
  void VisiontargetCallback(const geometry_msgs::Point &msg);
  /* void flattargetCallback(const controller_msgs::FlatTarget &msg); */
  void yawtargetCallback(const std_msgs::Float32 &msg);
  void multiDOFJointCallback(const trajectory_msgs::MultiDOFJointTrajectory &msg);
  void keyboardCallback(const geometry_msgs::Twist &msg);
  void cmdloopCallback(const ros::TimerEvent &event);
  void mavstateCallback(const mavros_msgs::State::ConstPtr &msg);
  void mavposeCallback(const geometry_msgs::PoseStamped &msg);
  void mavtwistCallback(const geometry_msgs::TwistStamped &msg);
  void statusloopCallback(const ros::TimerEvent &event);
  void visionloopCallback(const ros::TimerEvent &event);
  bool ctrltriggerCallback(std_srvs::SetBool::Request &req, std_srvs::SetBool::Response &res);
  bool landCallback(std_srvs::SetBool::Request &request, std_srvs::SetBool::Response &response);
  geometry_msgs::PoseStamped vector3d2PoseStampedMsg(Eigen::Vector3d &position, Eigen::Vector4d &orientation);
  void computeBodyRateCmd(Eigen::Vector4d &bodyrate_cmd, const Eigen::Vector3d &target_acc);
  Eigen::Vector3d controlPosition(const Eigen::Vector3d &target_pos, const Eigen::Vector3d &target_vel,
                                  const Eigen::Vector3d &target_acc);
  Eigen::Vector3d poscontroller(const Eigen::Vector3d &pos_error, const Eigen::Vector3d &vel_error);
  Eigen::Vector4d attcontroller(const Eigen::Vector4d &ref_att, const Eigen::Vector3d &ref_acc,
                                Eigen::Vector4d &curr_att);
  Eigen::Vector4d geometric_attcontroller(const Eigen::Vector4d &ref_att, const Eigen::Vector3d &ref_acc,
                                          Eigen::Vector4d &curr_att);
  Eigen::Vector4d jerkcontroller(const Eigen::Vector3d &ref_jerk, const Eigen::Vector3d &ref_acc,
                                 Eigen::Vector4d &ref_att, Eigen::Vector4d &curr_att);

  enum FlightState { WAITING_FOR_HOME_POSE, TARGET_PT_GEN, VISION_PT_GEN, TEMP_PT, HOLD_PT, PLANNER_TRAJ_EXEC, LANDING, LANDED } node_state_;

  template <class T>
  void waitForPredicate(const T *pred, const std::string &msg, double hz = 2.0) {
    ros::Rate pause(hz);
    ROS_INFO_STREAM(msg);
    while (ros::ok() && !(*pred)) {
      ros::spinOnce();
      pause.sleep();
    }
  };

  void rosinfopub(const std::string &msg, double hz = 2.0) {
    ros::Rate pause(hz);
    ROS_INFO_STREAM(msg);
    while (ros::ok()) {
      ros::spinOnce();
      pause.sleep();
    }
  };
  geometry_msgs::Pose home_pose_;
  bool received_home_pose_;
  bool px4_param_set(const std::string param_id, double value);
  bool px4_param_get(const std::string param_id, float& value);
  void set_px4_mode_func(string mode);
  bool checkParamValue(const std::string& param_id, double expected_value, double tolerance );
  
    // 添加时间戳成员变量
  std::chrono::steady_clock::time_point vision_start_time_;
  std::chrono::steady_clock::time_point overload_start_time_;
  const double VISION_TIMEOUT = 3.0;   // 3秒超时
  const double OVERLOAD_TIMEOUT = 30.0; // 30秒超时
  
 public:
  void dynamicReconfigureCallback(geometric_controller::GeometricControllerConfig &config, uint32_t level);
  geometricCtrl(const ros::NodeHandle &nh, const ros::NodeHandle &nh_private);
  virtual ~geometricCtrl();
  void getStates(Eigen::Vector3d &pos, Eigen::Vector4d &att, Eigen::Vector3d &vel, Eigen::Vector3d &angvel) {
    pos = mavPos_;
    att = mavAtt_;
    vel = mavVel_;
    angvel = mavRate_;
  };
  void getErrors(Eigen::Vector3d &pos, Eigen::Vector3d &vel) {
    pos = mavPos_ - targetPos_;
    vel = mavVel_ - targetVel_;
  };
  void setBodyRateCommand(Eigen::Vector4d bodyrate_command) { cmdBodyRate_ = bodyrate_command; };
  void setFeedthrough(bool feed_through) { feedthrough_enable_ = feed_through; };
  void setDesiredAcceleration(Eigen::Vector3d &acceleration) { targetAcc_ = acceleration; };
  static Eigen::Vector4d acc2quaternion(const Eigen::Vector3d &vector_acc, const double &yaw);
  static double getVelocityYaw(const Eigen::Vector3d velocity) { return atan2(velocity(1), velocity(0)); };

  void changeFSMExecState(FlightState new_state, string pos_call);
  void printFSMExecState();
  std::vector<Eigen::Vector3d> generatePoint(Eigen::Vector3d &start_pt,Eigen::Vector3d &end_pt, double samples_distance, bool vision_flag);
  void Gen_temp_points(Eigen::Vector3d p_start, Eigen::Vector3d p_end, std::vector<Eigen::Vector3d>& pts);
  void pubGimbal(const std::string str)
  {
    std_msgs::String msg;
    msg.data = str;
    gimbalCtrlPub_.publish(msg);
  }
  std::shared_ptr<UDPClient> udp_shock;
  std::shared_ptr<UDPClient> udp_offboard;
  std::shared_ptr<UDPServer> udp_server_;
  void udp_msgHandler(const uint8_t* data, size_t len, const sockaddr_in& sender);
  Eigen::Vector3d estimate_pos;
  float estimate_yaw; //deg
  bool start_requested_{false};
};

#endif
