#ifndef ROS_DAILY_LOGGER_H
#define ROS_DAILY_LOGGER_H

#include <ros/ros.h>
#include <mutex>
#include <ctime>
#include <sstream>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <system_error>
#include <iomanip>

namespace LOGGER {

enum LogLevel {
  DEBUG,
  INFO,
  WARN,
  ERROR,
  FATAL
};

class DailyLogger {
public:
  static DailyLogger& getInstance() {
    static DailyLogger instance;
    return instance;
  }

  void init(const std::string& log_dir,
           bool enable_ros = true,
           const std::string& prefix = "",
           bool ensure_directory = true) {
    std::lock_guard<std::mutex> lock(mutex_);
    log_dir_ = log_dir;
    prefix_ = prefix;
    ros_enabled_ = enable_ros;
    ensure_directory_ = ensure_directory;

    if(ensure_directory_) {
      createLogDirectory();
    }
    rotateLog(true);
  }

  static void Debug(const std::string& msg) { log(DEBUG, msg); }
  static void Info(const std::string& msg)  { log(INFO, msg); }
  static void Warn(const std::string& msg)  { log(WARN, msg); }
  static void Error(const std::string& msg) { log(ERROR, msg); }
  static void Fatal(const std::string& msg) { log(FATAL, msg); }

private:
  DailyLogger() = default;
  ~DailyLogger() {
    if(file_.is_open()) {
      file_.close();
    }
  }

  DailyLogger(const DailyLogger&) = delete;
  DailyLogger& operator=(const DailyLogger&) = delete;

  // 时间格式化工具
  static std::string currentDate() {
    auto now = ros::Time::now();
    std::time_t t = now.sec;
    std::tm* tm = std::localtime(&t);
    char buf[9];
    std::strftime(buf, sizeof(buf), "%Y%m%d", tm);
    return buf;
  }

  static std::string currentDateTime() {
    auto now = ros::Time::now();
    std::time_t t = now.sec;
    std::tm* tm = std::localtime(&t);
    char buf[20];
    std::strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", tm);
    return std::string(buf) + "." + std::to_string(now.nsec / 1000000).substr(0,3);
  }

  // 目录创建
  void createLogDirectory() {
    std::error_code ec;
    
    std::filesystem::path dir(log_dir_);
    if (!std::filesystem::exists(dir, ec)) {
      if(std::filesystem::create_directories(dir, ec)) 
      {
        logToRos(INFO, "Created log directory: " + log_dir_);
      } 
      else 
      {
        logToRos(ERROR, "Directory creation failed: " + log_dir_ + " - " + ec.message());
      }
    }
  }

  // 日志轮转
  void rotateLog(bool force_create = false) {
    std::error_code ec;
    
    const std::string new_date = currentDate();
    if(force_create || new_date != current_date_) {
      current_date_ = new_date;
      if(file_.is_open()) file_.close();

      if(ensure_directory_ && !std::filesystem::exists(log_dir_, ec)) {
        createLogDirectory();
      }

      std::ostringstream filename;
      filename << log_dir_ << "/"
              << (prefix_.empty() ? "" : prefix_ + "_")
              << "log_" << current_date_
              << "_" << currentDateTime() << ".log";

      std::filesystem::path file_path(filename.str());
      if(ensure_directory_ && !std::filesystem::exists(file_path.parent_path(), ec)) {
        createLogDirectory();
      }

      file_.open(file_path.string(), std::ios::app);
      if(!file_.is_open()) {
        logToRos(ERROR, "File creation failed: " + filename.str());
      } else {
        logToRos(DEBUG, "New log file created: " + filename.str());
      }
    }
  }

  // 核心日志方法
  static void log(LogLevel level, const std::string& msg) {
    auto& instance = getInstance();
    std::lock_guard<std::mutex> lock(instance.mutex_);
    
    instance.rotateLog();
    
    const std::string entry = "[" + currentDateTime() + "]"
                            + "[" + levelToString(level) + "] "
                            + msg;
    
    if(instance.file_.is_open()) {
      instance.file_ << entry << std::endl;
    }
    
    instance.logToRos(level, msg);
  }

  static std::string levelToString(LogLevel level) {
    switch(level) {
      case DEBUG: return "DEBUG";
      case INFO:  return "INFO";
      case WARN:  return "WARN";
      case ERROR: return "ERROR";
      case FATAL: return "FATAL";
      default:    return "UNKNOWN";
    }
  }

  void logToRos(LogLevel level, const std::string& msg) {
    if(!ros_enabled_) return;
    
    switch(level) {
      case DEBUG: ROS_DEBUG_STREAM(msg); break;
      case INFO:  ROS_INFO_STREAM(msg);  break;
      case WARN:  ROS_WARN_STREAM(msg);  break;
      case ERROR: ROS_ERROR_STREAM(msg); break;
      case FATAL: ROS_FATAL_STREAM(msg); break;
      default: break;
    }
  }

  // 成员变量
  std::ofstream file_;
  std::mutex mutex_;
  std::string log_dir_;
  std::string prefix_;
  std::string current_date_;
  bool ros_enabled_ = true;
  bool ensure_directory_ = true;
};

} // namespace your_package

#endif // ROS_DAILY_LOGGER_H