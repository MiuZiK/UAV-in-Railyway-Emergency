#!/usr/bin/env python3
# scripts/rtsp_snapshot_node.py

import rospy
import cv2
import os
import time
import threading
from cv_bridge import CvBridge
from sensor_msgs.msg import Image
from nav_msgs.msg import Odometry

class RTSPSnapshotNode:
    def __init__(self):
        rospy.init_node('rtsp_snapshot_node', anonymous=False)

        # 从参数服务器获取配置（注意：参数名应为 ～，不是 ～）
        self.rtsp_url = rospy.get_param('~rtsp_url', 'rtsp://192.168.1.108:554/stream=1')
        self.save_dir = rospy.get_param('~save_dir', './snapshots')
        self.snapshot_interval = rospy.get_param('~snapshot_interval', 0.5)  # 秒
        self.publish_image = rospy.get_param('~publish_image', False)
        self.buffer_size = rospy.get_param('~buffer_size', 1)
        self.odom_topic = rospy.get_param('~odom_topic', '/odom')  # 可配置 odom topic

        # 创建保存目录
        os.makedirs(self.save_dir, exist_ok=True)

        # 设置 OpenCV FFMPEG 选项
        os.environ["OPENCV_FFMPEG_CAPTURE_OPTIONS"] = "rtsp_transport;udp|analyzeduration;0|probesize;32"

        # 初始化 VideoCapture
        self.cap = cv2.VideoCapture(self.rtsp_url, cv2.CAP_FFMPEG)
        self.cap.set(cv2.CAP_PROP_BUFFERSIZE, self.buffer_size)

        if not self.cap.isOpened():
            rospy.logfatal("[RTSP] RGB Can't open RTSP stream: %s", self.rtsp_url)
            rospy.signal_shutdown("RTSP stream failed to open")
            return

        # 初始化 cv_bridge
        self.bridge = CvBridge()
        if self.publish_image:
            self.image_pub = rospy.Publisher('rtsp/image_raw', Image, queue_size=1)

        # Odom 相关
        self.latest_odom = None
        self.odom_lock = threading.Lock()
        self.odom_sub = rospy.Subscriber(self.odom_topic, Odometry, self.odom_callback)

        # 注册关闭钩子
        rospy.on_shutdown(self.cleanup)

        rospy.loginfo("[RTSP] RGB Snapshot Node startup")
        rospy.loginfo("[RTSP] RGB URL: %s", self.rtsp_url)
        rospy.loginfo("[RTSP] RGB Odom Topic: %s", self.odom_topic)
        rospy.loginfo("[RTSP] RGB Savedir: %s", self.save_dir)
        rospy.loginfo("[RTSP] RGB snaptime: %.1f s", self.snapshot_interval)

    def odom_callback(self, msg):
        """回调函数：更新最新 odom 位姿"""
        with self.odom_lock:
            self.latest_odom = (
                msg.pose.pose.position.x,
                msg.pose.pose.position.y,
                msg.pose.pose.position.z
            )

    def get_latest_odom_str(self):
        """获取格式化的 x_y_z 字符串，若无 odom 则返回 0_0_0"""
        with self.odom_lock:
            if self.latest_odom is not None:
                x, y, z = self.latest_odom
                # 保留3位小数，避免文件名过长
                return f"{x:.3f}_{y:.3f}_{z:.3f}".replace('.', '_')  # 将小数点替换为下划线（避免文件名问题）
            else:
                return "0_0_0"

    def cleanup(self):
        rospy.loginfo("[RTSP] RGB Closing RTSP stream...")
        if hasattr(self, 'cap') and self.cap.isOpened():
            self.cap.release()

    def run(self):
        last_snapshot_time = 0
        while not rospy.is_shutdown():
            ret, frame = self.cap.read()
            if not ret:
                rospy.logwarn_throttle(5, "[RTSP] RGB Can't read RTSP frame")
                time.sleep(0.1)
                continue

            current_time = time.time()

            # 定时截图
            if current_time - last_snapshot_time >= self.snapshot_interval:
                timestamp = int(current_time)
                odom_str = self.get_latest_odom_str()
                filename = f"snapshot_{timestamp}_{odom_str}.jpg"
                filepath = os.path.join(self.save_dir, filename)
                try:
                    cv2.imwrite(filepath, frame)
                    rospy.loginfo("[RTSP] RGB save: %s", filename)
                    last_snapshot_time = current_time
                except Exception as e:
                    rospy.logerr("[RTSP] RGB save error: %s", str(e))

            # 发布图像到 ROS topic（可选）
            if self.publish_image:
                try:
                    img_msg = self.bridge.cv2_to_imgmsg(frame, encoding="bgr8")
                    img_msg.header.stamp = rospy.Time.now()
                    self.image_pub.publish(img_msg)
                except Exception as e:
                    rospy.logerr("[RTSP] RGB publish error: %s", str(e))

            time.sleep(0.01)

def main():
    try:
        node = RTSPSnapshotNode()
        if rospy.is_shutdown():
            return
        node.run()
    except KeyboardInterrupt:
        rospy.loginfo("[RTSP] Ctrl+C, exiting...")
    except Exception as e:
        rospy.logfatal("[RTSP] Node error: %s", str(e))
        sys.exit(1)

if __name__ == '__main__':
    main()