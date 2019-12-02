import tensorflow.compat.v1 as tf
tf.disable_v2_behavior()
import numpy as np
from functools import wraps

import sys, os
sys.path.append(os.getcwd() + "/..")
import config

# cache decorator
def memoize(func):
    memo = {}
    
    @wraps(func)
    def wrapper(*args):
        if args in memo:
            return memo[args]
        else:
            rv = func(*args)
            memo[args] = rv
            return rv
    
    return memoize

class AbstrctNN:

    def __init__(self, weights, bias, core, err):
        self.weights = weights
        self.bias = bias
        self.core = core
        self.err = err
    
    #@memoize
    def predict(self, key):
        result = np.mat(key) * np.mat(self.weights[0]) + np.mat(self.bias[0])
        for i in range(1, len(self.core)-1):
            result = np.mat(result) * np.mat(self.weights[i]) + np.mat(self.bias[i])
        return int(result[0][0])

class NeuralNet:

    def __init__(self, distribution, x, y, learningRate, trainStepNum, keepRatio, batchSize, core):
        self.distribution = distribution
        self.x = x
        self.y = y
        self.learningRate = learningRate
        self.trainStepNum = trainStepNum
        self.keepRatio = keepRatio
        # initialize batch
        self.batchSize = batchSize
        self.batchCount = 1
        self.batchX = np.array([self.x[0:self.batchSize]]).T
        self.batchY = np.array([self.y[0:self.batchSize]]).T
        # initialize weight and bias matrix (ax +b)
        self.core = core
        self.w_fc, self.b_fc = [], []
        for i in range(len(self.core)-1):
            self.w_fc.append(self._weight_varible([self.core[i], self.core[i+1]]))
            self.b_fc.append(self._bias_variable([self.core[i+1]]))
        # intialize running states
        self.y_ = tf.placeholder(tf.float32, shape=[None, self.core[-1]])
        self.h_fc = [None for i in range(len(self.core))]
        self.h_fc_drop = [None for i in range(len(self.core))]
        self.h_fc_drop[0] = tf.placeholder(tf.float32, shape=[None, self.core[0]])
        self.keepProb = tf.placeholder(tf.float32)
        # intialize tf session
        self.sess = tf.Session()
    
    def _weight_varible(self, shape):
        if self.distribution == config.Distribution.SEQUENCE:
            w = tf.constant(0.1, shape=shape)
        elif self.distribution == config.Distribution.RANDOM:
            w = tf.constant(0.1, shape=shape)
        elif self.distribution == config.Distribution.EXPONENTIAL:
            w = tf.constant(0.1, shape=shape)
        return tf.Variable(w)
    
    def _bias_variable(self, shape):
        b = tf.constant(0.1, shape=shape)
        return tf.Variable(b)

    def next_batch(self):
        batchStart = self.batchCount * self.batchSize
        batchEnd = (self.batchCount + 1) * self.batchSize
        if batchEnd < len(self.x):
            self.batchX = np.array([self.x[batchStart:batchEnd]]).T
            self.batchY = np.array([self.y[batchStart:batchEnd]]).T
            self.batchCount += 1
        else:
            self.batchX = np.array([self.x[batchStart:len(self.x)]]).T
            self.batchY = np.array([self.y[batchStart:len(self.y)]]).T
            self.batchCount = 0
    
    def train(self):
        # training logic
        for i in range(len(self.core)-1):
            self.h_fc[i] = tf.nn.relu(tf.matmul(self.h_fc_drop[i], self.w_fc[i]) + self.b_fc[i])
            self.h_fc_drop[i+1] = tf.nn.dropout(self.h_fc[i], self.keepProb)
        self.crossEntropy = tf.reduce_mean(tf.losses.mean_squared_error(self.y_, self.h_fc[len(self.core)-2]))
        self.trainStep = tf.train.AdamOptimizer(self.learningRate).minimize(self.crossEntropy)
        self.sess.run(tf.global_variables_initializer())

        for step in range(self.trainStepNum):
            self.sess.run(self.trainStep, feed_dict={
                self.h_fc_drop[0]: self.batchX,
                self.y_: self.batchY,
                self.keepProb: self.keepRatio
            })
            
            if step % 1000 == 0:
                err = self.get_err()
                print("Error: " + str(err))
            
            self.next_batch()
    
    def get_err(self):
        err = self.sess.run(self.crossEntropy, feed_dict={
            self.h_fc_drop[0]: np.array([self.x]).T,
            self.y_: np.array([self.y]).T,
            self.keepProb: 1.0
        })
        return err
    
    def get_weights(self):
        weights = []
        for i in range(len(self.core)-1):
            weights.append(self.sess.run(self.w_fc[i], feed_dict={
                self.h_fc_drop[0]: np.array([self.x]).T,
                self.y_: np.array([self.y]).T,
                self.keepProb: 1.0
            }).tolist())
        return weights
    
    def get_bias(self):
        bias = []
        for i in range(len(self.core)-1):
            bias.append(self.sess.run(self.b_fc[i], feed_dict={
                self.h_fc_drop[0]: np.array([self.x]).T,
                self.y_: np.array([self.y]).T,
                self.keepProb: 1.0
            }).tolist())
        return bias
    
    def save(self, path):
        saver = tf.train.Saver()
        saver.save(self.sess, path)