#include "NeuralNetwork.h"

Synapses::Synapses(int input, int output) : inputLayer(input), outputLayer(output) {

	coefficients = vector<double>(input*output);
	gradient = vector<double>(input*output, 0);
	for (int i = 0; i < coefficients.size(); i++) { coefficients[i] = 1 - 2 * ((double)rand()) / RAND_MAX; }
}

Network::Network(vector<int> layerSizes) : layers(), synapses() {

	for (int i = 0; i < layerSizes.size(); i++) {
		vector<Neuron> layer = vector<Neuron>(layerSizes[i] + 1);
		layer[layer.size() - 1].value = -1; // the bias neuron has a constant value of -1
		if (i < layerSizes.size() - 1) {  // for every layer but the last :

			Synapses s = Synapses(layerSizes[i] + 1, layerSizes[i + 1]);  // don't forget the bias in the input layer !
			synapses.push_back(s);
		}
		layers.push_back(layer);
	}
}

void Network::setInput(const double* values) {

	vector<Neuron>& inputLayer = layers[0];
	for (int i = 0; i < inputLayer.size()-1; i++) { inputLayer[i].value = values[i]; }
}

void Network::activate() {

	for (int i = 1; i < layers.size(); i++) {  // for every layer but the input

		vector<Neuron>& layer = layers[i];
		Synapses& synapse = synapses[i - 1]; // synapses between layer i-1 and i
		vector<Neuron>& prevLayer = layers[i - 1];
		for (int nextNeuron = 0; nextNeuron < layer.size() - 1; nextNeuron++) {
			layer[nextNeuron].input = 0;
			for (int prevNeuron = 0; prevNeuron < synapse.inputLayer; prevNeuron++) {
				layer[nextNeuron].input += synapse.get(prevNeuron, nextNeuron) * prevLayer[prevNeuron].value;
			}
			layer[nextNeuron].value = sigmoid(layer[nextNeuron].input);
		}
	}
}

void Network::setDesiredOutput(const double* values) {

	vector<Neuron>& outputLayer = layers[layers.size() - 1];
	for (int i = 0; i < outputLayer.size() - 1; i++) {
		outputLayer[i].diff = sigmoidDeriv(outputLayer[i].input) * (values[i] - outputLayer[i].value); // delta = g'(in) * (y - a)
	}
}

vector<double> Network::getOuput() {

	vector<Neuron>& outputLayer = layers[layers.size() - 1];
	vector<double> dst(outputLayer.size() - 1);
	for (int i = 0; i < outputLayer.size() - 1; i++) {
		dst[i] = outputLayer[i].value;
	}
	return dst;
}

void Network::update() {

	for (auto& s : synapses) { s.updateCoeffs(); }
}

void Network::backtrack() {

	for (int l = layers.size() - 2; l >= 0; l--) {
		vector<Neuron>& layer = layers[l];; // local input layer
		Synapses& synapse = synapses[l];
		vector<Neuron>& nextLayer = layers[l + 1]; // local output layer
		for (int j = 0; j < layer.size(); j++) {
			double diffSum = 0;
			for (int i = 0; i < nextLayer.size() - 1; i++) {
				diffSum += synapse.get(j, i) * nextLayer[i].diff;
				double diff = layer[j].value * nextLayer[i].diff;
				synapse.addDiff(j, i, diff);
			}
			layer[j].diff = sigmoidDeriv(layer[j].input) * diffSum;
		}
	}
}

struct Value {
	double coords[2];
	double colors[3];
};

Image Network::imageCompletion(const Image& src, int imageSize, int nbIterations) {

	ImageF srcF = convert(src);
	ImageF low = resize(srcF, (srcF.width * imageSize) / srcF.height, imageSize, NEAREST); free(srcF.img);

	vector<Value> values(0);
	for (uint x = 0; x < low.width; x++) {
		for (uint y = 0; y < low.height; y++) {
			float alpha = low.img[low.channels*(low.width*y + x) + 3];
			if (alpha >= 255) {
				Value v;
				v.coords[0] = ((double)x) / low.width;
				v.coords[1] = ((double)y) / low.height;
				v.colors[0] = ((double)low.img[low.channels*(low.width*y + x) + 0]) / 255;
				v.colors[1] = ((double)low.img[low.channels*(low.width*y + x) + 1]) / 255;
				v.colors[2] = ((double)low.img[low.channels*(low.width*y + x) + 2]) / 255;
				values.push_back(v);
			}
		}
	}
	cout << "learning on " << values.size() << " pixels" << endl;

	for (uint it = 0; it < nbIterations; it++) {
		for (const auto& v : values) {
			setInput(v.coords);
			activate();
			setDesiredOutput(v.colors);
			backtrack();
		}
		update();
	}

	Image dst;
	dst.width = src.width; dst.height = src.height;
	dst.channels = 4;
	dst.img = (uchar*)malloc(dst.width*dst.height*dst.channels*sizeof(uchar));
	for (uint x = 0; x < dst.width; x++) {
		for (uint y = 0; y < dst.height; y++) {

			double coords[2];
			coords[0] = ((double)x) / dst.width;
			coords[1] = ((double)y) / dst.height;

			setInput(coords);
			activate();

			const auto color = getOuput();
			dst.img[dst.channels*(dst.width*y + x) + 0] = 255 * color[0];
			dst.img[dst.channels*(dst.width*y + x) + 1] = 255 * color[1];
			dst.img[dst.channels*(dst.width*y + x) + 2] = 255 * color[2];
			dst.img[dst.channels*(dst.width*y + x) + 3] = 255;
		}
	}
	free(low.img);
	return dst;
}

void imageCompletion(int argc, char** argv) {

	int imageSize = 32;
	int nbIterations = 1000;

	if (argc < 3) {
		cout << "command line arguments are : <input_image.png> <output_image.png> (<nbIterations=" << nbIterations << "> <imageSize=" << imageSize << "> optional)" << endl;
		return;
	}

	char* srcFileName = argv[1];
	char* dstFileName = argv[2];
	Image src = loadImage(srcFileName);

	if (argc >= 4) {
		nbIterations = stoi(argv[3]);
	}
	if (argc >= 5) {
		imageSize = stoi(argv[4]);
	}

	Network net({ 2, 10, 10, 3 });
	Image dst = net.imageCompletion(src, imageSize, nbIterations);
	free(src.img);
	writeImage(dst, dstFileName);
	free(dst.img);

}