#include "rclcpp/rclcpp.hpp"
#include "ai_msgs/msg/perception_targets.hpp"

#include <string>

#include "byte_tracker/BYTETracker.h"

class YoloTracker : public rclcpp::Node {
public:
    YoloTracker();

    YoloTracker(const std::string &node_name): Node(node_name) {
        RCLCPP_INFO(this->get_logger(), "YoloTracker node is created");
        this->declare_parameter("msg_sub_topic", "dnn_node_sample");
        this->get_parameter("msg_sub_topic", msg_sub_topic);

        this->declare_parameter("msg_pub_topic", "yolo_tracker_res");
        this->get_parameter("msg_pub_topic", res_pub_topic);

        target_subscription_ = this->create_subscription<ai_msgs::msg::PerceptionTargets>(
            msg_sub_topic, 10, std::bind(&YoloTracker::AiMsgCallBack, this, std::placeholders::_1));
        result_publisher_ = this->create_publisher<ai_msgs::msg::PerceptionTargets>(res_pub_topic, 10);

        tracker_ = std::make_shared<BYTETracker>(30, 30);
    }

private:
    std::string msg_sub_topic;
    std::string res_pub_topic;
    std::shared_ptr<BYTETracker> tracker_;
    rclcpp::Subscription<ai_msgs::msg::PerceptionTargets>::SharedPtr target_subscription_;
    rclcpp::Publisher<ai_msgs::msg::PerceptionTargets>::SharedPtr result_publisher_;

    void AiMsgCallBack(const ai_msgs::msg::PerceptionTargets::SharedPtr msg) {

        ai_msgs::msg::PerceptionTargets result;
        std::vector<STrack> output_stracks;
        tracker_->update(msg, output_stracks);
        for (auto &output_strack: output_stracks) {
            float xmin;
            float ymin;
            float xmax;
            float ymax;
            bool vertical = output_strack.tlwh[2] / output_strack.tlwh[3] > 1.6;
            if (output_strack.tlwh[2] * output_strack.tlwh[3] > 20 && !vertical) {
                xmin = output_strack.tlwh[0];
                ymin = output_strack.tlwh[1];
                xmax = output_strack.tlwh[0] + output_strack.tlwh[2];
                ymax = output_strack.tlwh[1] + output_strack.tlwh[3];
            }

            ai_msgs::msg::Roi roi;
            roi.rect.set__x_offset(xmin);
            roi.rect.set__y_offset(ymin);
            roi.rect.set__width(xmax - xmin);
            roi.rect.set__height(ymax - ymin);
            roi.set__confidence(output_strack.score);

            ai_msgs::msg::Target target;
            target.set__type(output_strack.class_name);
            target.rois.emplace_back(roi);
            target.set__track_id(output_strack.track_id);
            result.targets.emplace_back(std::move(target));
        }
        result.header = msg->header;
        result.fps = msg->fps;
        result_publisher_->publish(result);
    }
};


int main(int argc, char **argv) {
    rclcpp::init(argc,argv);
    auto node = std::make_shared<YoloTracker>("yolo_tracker");
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
