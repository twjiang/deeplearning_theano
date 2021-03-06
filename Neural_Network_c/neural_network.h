/*************************************************
Author: Tianwen Jiang
Date: 2016-01-27
Description: the implementation of neural network
**************************************************/

#ifndef NEURAL_NETWORK_H_INCLUDED
#define NEURAL_NETWORK_H_INCLUDED

#include <iostream>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

#include "tw_func.h"

using namespace std;

class Neural_Network
{
private:
    vector<int> nn_size;
    int layers_num;
    vector<vector<double> > biases;
    vector<vector<vector<double> > > weights;
    vector<vector<double> > nabla_biases;
    vector<vector<vector<double> > > nabla_weights;
    vector<vector<double> > delta_nabla_b;
    vector<vector<vector<double> > > delta_nabla_w;

    vector<double> cost_derivative(vector<double> &output_activation, vector<double> &y)
    {
        vector<double> result;
        if (output_activation.size() != y.size())
        {
            cout << "not match wrong!";
            exit(1);
        }
        for (unsigned int i = 0; i < output_activation.size(); i++)
        {
            result.push_back(output_activation[i]-y[i]);
        }
        return result;
    }

    double calc_loss(vector<double> &output1, vector<double> &output2)
    {
        double sqr_sum = 0;
        if (output1.size() != output2.size())
        {
            cout << "wrong!" << endl;
            exit(1);
        }
        for (unsigned i = 0; i < output1.size(); i++)
        {
            sqr_sum += sqr(output1[i]-output2[i]);
        }
        return sqr_sum / 2;
    }

    /** back propagation algorithm **/
    void backprop(vector<double> &x, vector<double> &y)
    {
        for (unsigned int i = 0; i < delta_nabla_b.size(); i++)
        {
            for (unsigned int j = 0; j < delta_nabla_b[i].size(); j++)
            {
                delta_nabla_b[i][j] = 0;
                for (unsigned int k = 0; k < delta_nabla_w[i][j].size(); k++)
                    delta_nabla_w[i][j][k] = 0;
            }
        }


        vector<double> activation;
        vector<double> z;
        vector<vector<double> > activations;
        vector<vector<double> > zs;

        // Input x: set the corresponding activation a^1 for input layer
        //activation = new vector<double>();
        for (unsigned int i = 0; i < x.size(); i++)
            activation.push_back(x[i]);
        activations.push_back(activation);

        // FeedForward: for each l=1,2, compute z^l=w^la^{l-1}+b^l and a^l=\sigma(z^l)
        // current level
        for (unsigned int i = 1; i < nn_size.size(); i++)
        {
            activation.clear();
            z.clear();
            // every neural in the current level
            for (int j = 0; j < nn_size[i]; j++)
            {
                double w_dot_a = 0;

                // every neural in the last level
                for (int k = 0; k < nn_size[i-1]; k++)
                    w_dot_a += weights[i-1][j][k] * (activations[i-1][k]);
                z.push_back(w_dot_a + biases[i-1][j]);
                activation.push_back(sigmoid(z.back()));

            }
            zs.push_back(z);
            activations.push_back(activation);
        }

        // output error delta^L: compute the vector delta^L=nabla_a C odot sigma'(z^L)
        vector<double> delta;
        vector<double> delta_tmp = cost_derivative(activations.back(), y);
        vector<double> z_L = zs.back();
        for (unsigned int i = 0; i < z_L.size(); i++)
        {
            delta.push_back(delta_tmp[i] * sigmoid_prime(z_L[i]));
        }

        // backpropagate the error;
        for (int j = 0; j < nn_size[layers_num-1]; j++)
        {
            (delta_nabla_b.back())[j] = delta[j];
            for (int k = 0; k < nn_size[layers_num-2]; k++)
                (delta_nabla_w.back())[j][k] = activations[layers_num-2][k] * delta[j];
        }
        for (int layer = layers_num-1; layer >= 2; layer--)
        {
            vector<double> old_delta = delta;
            delta.clear();
            for (int j = 0; j < nn_size[layer-1]; j++)
            {
                double delta_element = 0;
                for (int k = 0; k < nn_size[layer]; k++)
                    delta_element += weights[layer-1][k][j]*old_delta[k];
                delta_element = delta_element * sigmoid_prime(zs[layer-2][j]);
                delta.push_back(delta_element);

                //update delta_nabla_w and delta_nabla_b
                delta_nabla_b[layer-2][j] = delta[j];
                for (int k = 0; k < nn_size[layer-2]; k++)
                    delta_nabla_w[layer-2][j][k] = activations[layer-2][k] * delta[j];
            }
        }
    }

    /** use the batch_data to update weights and biases **/
    void update_weights_biases(vector<vector<double> > &batch_inputs,
                               vector<vector<double> > &batch_outputs, double rate)
    {
        int batch_len = batch_inputs.size();

        // renew the nabla_biases and nabla_weights
        for (unsigned int i = 0; i < nabla_biases.size(); i++)
        {
            for (unsigned int j = 0; j < nabla_biases[i].size(); j++)
            {
                nabla_biases[i][j] = 0;
                for (unsigned int k = 0; k < nabla_weights[i][j].size(); k++)
                    nabla_weights[i][j][k] = 0;
            }
        }

        for (int index = 0; index < batch_len; index++)
        {
            backprop(batch_inputs[index], batch_outputs[index]);
            for (unsigned int i = 0; i < nabla_biases.size(); i++)
            {
                for (unsigned int j = 0; j < nabla_biases[i].size(); j++)
                {
                    nabla_biases[i][j] += delta_nabla_b[i][j];
                    for (unsigned int k = 0; k < nabla_weights[i][j].size(); k++)
                        nabla_weights[i][j][k] += delta_nabla_w[i][j][k];
                }
            }

        }

        for (unsigned int i = 0; i < biases.size(); i++)
        {
            for (unsigned int j = 0; j < biases[i].size(); j++)
            {
                biases[i][j] = biases[i][j]-rate*nabla_biases[i][j]/batch_len;
                for (unsigned int k = 0; k < weights[i][j].size(); k++)
                    weights[i][j][k] = weights[i][j][k]-rate*nabla_weights[i][j][k]/batch_len;
            }
        }
    }

    vector<double> feedforword(vector<double> &input)
    {
        vector<double> activation;

        for (unsigned int i = 0; i < input.size(); i++)
            activation.push_back(input[i]);

        for (unsigned int i = 1; i < nn_size.size(); i++)
        {
            vector<double> old_activation = activation;
            activation.clear();
            /* every neural in the current level */
            for (int j = 0; j < nn_size[i]; j++)
            {
                double w_dot_a = 0;

                /* every neural in the last level */
                for (int k = 0; k < nn_size[i-1]; k++)
                    w_dot_a += weights[i-1][j][k] * (old_activation[k]);
                activation.push_back(sigmoid(w_dot_a + biases[i-1][j]));
            }
        }
        return activation;
    }

    int evaluate(vector<vector<double> > &testing_inputs, vector<vector<double> > &testing_outputs)
    {
        int right_count = 0;
        vector<double> output;
        for (unsigned int i = 0; i < testing_inputs.size(); i++)
        {
            output = feedforword(testing_inputs[i]);
            int max_pos = 0;
            double max_ele = 0;
            for (unsigned int j = 0; j < output.size(); j++)
            {
                if (output[j] > max_ele)
                {
                    max_ele = output[j];
                    max_pos = j;
                }
            }
            if (testing_outputs[i][max_pos] == 1.0)
                right_count++;
        }
        return right_count;
    }

public:
    /** construction fuction: initialize the weights and biases **/
    Neural_Network(vector<int> &nn_size_in)
    {
        layers_num = nn_size_in.size();
        nn_size = nn_size_in;

        cout << endl << "initializing the biases and weights ... ..." << endl;

        /** initializing the biases **/
        biases.resize(layers_num-1);
        nabla_biases.resize(layers_num-1);
        delta_nabla_b.resize(layers_num-1);
        for (unsigned int i = 0; i < biases.size(); i++)
        {
            biases[i].resize(nn_size[i+1]);
            for (unsigned int j = 0; j < biases[i].size(); j++)
                biases[i][j] = gaussrand();
            nabla_biases[i].resize(nn_size[i+1]);
            delta_nabla_b[i].resize(nn_size[i+1]);
        }

        /** initializing the weights **/
        weights.resize(layers_num-1);
        nabla_weights.resize(layers_num-1);
        delta_nabla_w.resize(layers_num-1);
        for (unsigned int i = 0; i < weights.size(); i++)
        {
            weights[i].resize(nn_size[i+1]);
            nabla_weights[i].resize(nn_size[i+1]);
            delta_nabla_w[i].resize(nn_size[i+1]);
            for (unsigned int j = 0; j < weights[i].size(); j++)
            {
                weights[i][j].resize(nn_size[i]);
                for (unsigned int k = 0; k < weights[i][j].size(); k++)
                    weights[i][j][k] = gaussrand();
                nabla_weights[i][j].resize(nn_size[i]);
                delta_nabla_w[i][j].resize(nn_size[i]);
            }
        }

        cout << "initialized ok. " << endl << endl;
    }

    /** train the neural network using stochastic gradient descent **/
    void SGD(vector<vector<double> > &training_inputs,
             vector<vector<double> > &training_outputs, int nepochs, int batch_size, double rate,
             vector<vector<double> > &testing_inputs, vector<vector<double> > &testing_outputs)
    {
        //cout << "in SGD." << endl;
        int training_len = training_inputs.size();
        int nbatchs = training_len/batch_size;

        vector<vector<double> > batch_inputs;
        vector<vector<double> > batch_outputs;
        batch_inputs.resize(batch_size);
        batch_outputs.resize(batch_size);

        for (int epoch = 0; epoch < nepochs ; epoch++)
        {
            for (int batch = 0; batch < nbatchs; batch++)
            {
                /** for each batch we select batch_size training data for training model **/
                for (int i = 0; i < batch_size; i++)
                {
                    int index = rand_max(training_len);
                    batch_inputs[i] = training_inputs[index];
                    batch_outputs[i] = training_outputs[index];
                }

                /** use the batch_data to update weights and biases **/
                update_weights_biases(batch_inputs, batch_outputs, rate);
            }
            //int right_count = 0;
            int right_count = evaluate(testing_inputs, testing_outputs);
            cout << "epoch: " << epoch << "\t" << right_count << "/" << testing_inputs.size() << endl;
            //loss = loss / training_len;
            //cout << "epoch: " << epoch << "\t" << "loss: " << loss << endl;
        }
    }
};

#endif // NEURAL_NETWORK_H_INCLUDED
