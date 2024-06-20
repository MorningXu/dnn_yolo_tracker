#pragma once

#include "dataType.h"

namespace byte_kalman {
    /*
     * KalmanFilter 类提供了一个卡尔曼滤波器的实现。
     */
    class KalmanFilter {
    public:
        // chi2inv95 存储了不同自由度下卡方分布的95%分位数。
        static const double chi2inv95[10];

        // 默认构造函数
        KalmanFilter();

        /*
         * initiate 函数根据测量值初始化卡尔曼滤波器的状态。
         * @param measurement 初始化的测量值。
         * @return 返回初始化后的状态数据。
         */
        KAL_DATA initiate(const DETECTBOX &measurement);

        /*
         * predict 函数根据当前状态预测下一个状态。
         * @param mean 预测后的均值。
         * @param covariance 预测后的协方差。
         */
        void predict(KAL_MEAN &mean, KAL_COVA &covariance);

        /*
         * project 函数将当前状态投影到测量空间。
         * @param mean 当前状态的均值。
         * @param covariance 当前状态的协方差。
         * @return 返回投影后的数据。
         */
        KAL_HDATA project(const KAL_MEAN &mean, const KAL_COVA &covariance);

        /*
         * update 函数根据测量值更新状态。
         * @param mean 更新后的均值。
         * @param covariance 更新后的协方差。
         * @param measurement 当前的测量值。
         * @return 返回更新后的状态数据。
         */
        KAL_DATA update(const KAL_MEAN &mean,
                        const KAL_COVA &covariance,
                        const DETECTBOX &measurement);

        /*
         * gating_distance 函数计算测量值与预测状态之间的门控距离。
         * @param mean 预测状态的均值。
         * @param covariance 预测状态的协方差。
         * @param measurements 多个测量值的集合。
         * @param only_position 是否仅考虑位置信息，默认为false。
         * @return 返回一个矩阵，包含每个测量值与预测状态的门控距离。
         */
        Eigen::Matrix<float, 1, -1> gating_distance(
            const KAL_MEAN &mean,
            const KAL_COVA &covariance,
            const std::vector<DETECTBOX> &measurements,
            bool only_position = false);

    private:
        // _motion_mat 为运动模型矩阵。
        Eigen::Matrix<float, 8, 8, Eigen::RowMajor> _motion_mat;
        // _update_mat 为更新模型矩阵。
        Eigen::Matrix<float, 4, 8, Eigen::RowMajor> _update_mat;
        // _std_weight_position 为位置的标准化权重。
        float _std_weight_position;
        // _std_weight_velocity 为速度的标准化权重。
        float _std_weight_velocity;
    };
}
