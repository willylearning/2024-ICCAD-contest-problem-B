#include <iostream>
#include <Eigen/Dense>
#include <vector>

using namespace Eigen;
using namespace std;

class RNN {
public:
    RNN(int input_size, int hidden_size, int output_size) {
        this->input_size = input_size;
        this->hidden_size = hidden_size;
        this->output_size = output_size;

        // Initialize weights
        Wxh = MatrixXd::Random(hidden_size, input_size);
        Whh = MatrixXd::Random(hidden_size, hidden_size);
        Why = MatrixXd::Random(output_size, hidden_size);

        // Initialize biases
        bh = VectorXd::Zero(hidden_size);
        by = VectorXd::Zero(output_size);
    }

    vector<VectorXd> forward(const vector<VectorXd>& inputs) {
        h = VectorXd::Zero(hidden_size); // Initial hidden state

        vector<VectorXd> outputs;
        for (const auto& x : inputs) {
            h = (Wxh * x + Whh * h + bh).array().tanh();
            VectorXd y = Why * h + by;
            outputs.push_back(y);
        }

        return outputs;
    }

    void backward(const vector<VectorXd>& inputs, const vector<VectorXd>& targets, double learning_rate = 0.001) {
        // Forward pass
        vector<VectorXd> hs;
        hs.push_back(VectorXd::Zero(hidden_size));
        vector<VectorXd> ys;

        for (const auto& x : inputs) {
            VectorXd h_new = (Wxh * x + Whh * hs.back() + bh).array().tanh();
            hs.push_back(h_new);
            ys.push_back(Why * h_new + by);
        }

        // Initialize gradients
        MatrixXd dWxh = MatrixXd::Zero(Wxh.rows(), Wxh.cols());
        MatrixXd dWhh = MatrixXd::Zero(Whh.rows(), Whh.cols());
        MatrixXd dWhy = MatrixXd::Zero(Why.rows(), Why.cols());
        VectorXd dbh = VectorXd::Zero(bh.size());
        VectorXd dby = VectorXd::Zero(by.size());
        VectorXd dh_next = VectorXd::Zero(hidden_size);

        // Backward pass
        for (int t = inputs.size() - 1; t >= 0; --t) {
            VectorXd dy = ys[t] - targets[t];
            dWhy += dy * hs[t + 1].transpose();
            dby += dy;

            VectorXd dh = Why.transpose() * dy + dh_next;
            VectorXd dh_raw = (1 - hs[t + 1].array().square()) * dh.array();
            dbh += dh_raw;
            dWxh += dh_raw * inputs[t].transpose();
            dWhh += dh_raw * hs[t].transpose();
            dh_next = Whh.transpose() * dh_raw;
        }

        // Gradient descent
        Wxh -= learning_rate * dWxh;
        Whh -= learning_rate * dWhh;
        Why -= learning_rate * dWhy;
        bh -= learning_rate * dbh;
        by -= learning_rate * dby;
    }

private:
    int input_size;
    int hidden_size;
    int output_size;

    MatrixXd Wxh;
    MatrixXd Whh;
    MatrixXd Why;

    VectorXd bh;
    VectorXd by;

    VectorXd h; // Hidden state
};

int main() {
    int input_size = 3;
    int hidden_size = 5;
    int output_size = 2;

    RNN rnn(input_size, hidden_size, output_size);

    // Create dummy data
    vector<VectorXd> inputs = {VectorXd::Random(input_size), VectorXd::Random(input_size), VectorXd::Random(input_size)};
    vector<VectorXd> targets = {VectorXd::Random(output_size), VectorXd::Random(output_size), VectorXd::Random(output_size)};

    // Training
    for (int i = 0; i < 1000; ++i) {
        rnn.backward(inputs, targets);
    }

    // Prediction
    vector<VectorXd> outputs = rnn.forward(inputs);
    for (const auto& y : outputs) {
        cout << "Output: " << y.transpose() << endl;
    }

    return 0;
}
