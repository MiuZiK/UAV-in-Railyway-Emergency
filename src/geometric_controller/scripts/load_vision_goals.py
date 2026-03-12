#!/usr/bin/env python3
# scripts/load_vision_goals.py

import rospy
import sys
import os

def load_goals_from_txt(txt_path):
    goals = []
    with open(txt_path, 'r') as f:
        for line_num, line in enumerate(f, 1):
            line = line.strip()
            if not line or line.startswith('#'):
                continue  # 跳过空行和注释
            try:
                # 支持空格或逗号分隔
                if ',' in line:
                    parts = [float(x.strip()) for x in line.split(',')]
                else:
                    parts = [float(x) for x in line.split()]
                if len(parts) != 11:
                    rospy.logwarn(f"第 {line_num} 行有 {len(parts)} 个值，期望 11 个，跳过")
                    continue
                goals.extend(parts)
            except ValueError as e:
                rospy.logerr(f"第 {line_num} 行解析失败: {line} | 错误: {e}")
                continue
    return goals

def main():
    rospy.init_node('vision_goals_loader', anonymous=True)

    # 获取参数文件路径（可通过 rosparam 或 argv 传入）
    if len(sys.argv) < 2:
        rospy.logfatal("请提供 vision_goals.txt 路径作为参数")
        sys.exit(1)

    txt_path = sys.argv[1]
    if not os.path.isfile(txt_path):
        rospy.logfatal(f"文件不存在: {txt_path}")
        sys.exit(1)

    goal_list = load_goals_from_txt(txt_path)
    if not goal_list:
        rospy.logwarn("未加载到任何有效目标点")

    # 设置参数（注意：参数名需与 controller 节点一致）
    rospy.set_param('/geometric_controller/vision_goal_point_list', goal_list)
    rospy.loginfo(f"成功加载 {len(goal_list)//11} 个目标点到参数服务器")

if __name__ == '__main__':
    main()