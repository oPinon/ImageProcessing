#include <vector>

#include "SuperSample.h"

using namespace std;

struct Neuron
{
	double input; // sum of all incoming synapses
	double value; // = activation( input )
	double diff; // difference between the desired value and the value
};

// connections between several neural layers
struct Synapses
{
	const double learningRate = 0.01;

	int inputLayer; // nb of neurons in the input layer
	int outputLayer; // nb of neurons in the output layer
private :
	vector<double> coefficients; // coefficients of each connection
	vector<double> gradient; // delta to add to the coefficient for the next step

public :
	double get(int input, int output) { return coefficients[output * inputLayer + input]; } // get coefficient
	void set(int input, int output, double value) { coefficients[output * inputLayer + input] = value; } // set coefficient
	void addDiff(int input, int output, double value) { gradient[output * inputLayer + input] += value; }

	Synapses(int input, int output);

	void updateCoeffs() { for (int i = 0; i < coefficients.size(); i++) { coefficients[i] += learningRate * gradient[i]; gradient[i] = 0; } }
};

class Network
{
	static double sigmoid(double x) { return 1 / (1 + exp(-x)); }
	static double sigmoidDeriv(double x) { return sigmoid(x) * (1 - sigmoid(x)); } // TODO : optimize

public:

	vector<vector<Neuron>> layers;
	vector<Synapses> synapses;

	Network(vector<int> layers);
	void setInput(const double* values);
	void activate();
	void setDesiredOutput(const double* values);
	vector<double> getOuput();
	void update();
	void backtrack();

	Image imageCompletion(const Image& src, int imageSize, int nbIterations);
};