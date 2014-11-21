#include "Annotation.h"

#include <iostream>

using namespace EvaluationFramework;
using namespace std;

template<typename T>
std::ostream& operator<<(std::ostream& stm, const std::vector<T>& obj) {
	stm << "[";
	if (!obj.empty()) {
		for (size_t i = 0 ; i < obj.size()-1 ; ++i) {
			stm << obj[i] << " ";
		}
		stm << obj.back();
	}
	stm << "]";
	return stm;
}

int main()
{
	vector<Frame> gt, output;
	LoadPascal(gt, "gt.txt");
	LoadPascal(output, "output.txt");
	double eval_overlap = 0.5;

	vector< vector<double>> tpc(gt.size());
	vector< vector<int>> tp(gt.size());
	vector< vector<int>> fp(gt.size());
	vector< vector<int>> fn(gt.size());

	for (unsigned int k = 0; k < gt.size(); k++)
	{
		//sort the detection in descending order
		output[k].Sort();

		//calculate overlaps between gt and output
		vector< vector<double> > overlap(gt[k].annotations.size(), vector<double>(output[k].annotations.size(),0.0));

		for (unsigned int i = 0; i < gt[k].annotations.size(); i++)
			for (unsigned int j = 0; j < output[k].annotations.size(); j++)
				overlap[i][j] = gt[k].annotations[i]->overlap(*output[k].annotations[j]);

		//calculate assignments between gt and output
		vector<int> assigned(output[k].annotations.size(), -1);

		for (unsigned int i = 0; i < overlap.size(); i++)
		{
			double max_overlap = eval_overlap;
			int index = -1;

			for (unsigned int j = 0; j < overlap[i].size(); j++)
			{
				if ((assigned[j] == -1) && (overlap[i][j] > max_overlap))
				{
					index = j;
					max_overlap = overlap[i][j];
				}
			}
			if (index != -1)
			{
				assigned[index] = i;
			}
		}

		tpc[k] = vector<double>(assigned.size(), 0.0);
		tp[k] = vector<int>(assigned.size(), 0);
		fp[k] = vector<int>(assigned.size(), 0);
		fn[k] = vector<int>(assigned.size(), 0);

		//count
		for (unsigned int i = 0; i < output[k].annotations.size(); i++)
		{
			for (unsigned int j = 0; j < assigned.size(); j++)
			{
				if (output[k].annotations[j]->weight >= output[k].annotations[i]->weight)
				{
					//positives
					if (assigned[j] != -1)
					{
						//true positives
						tp[k][i]++;
						tpc[k][i] += overlap[assigned[j]][j];
					}
					else
					{
						//false positive
						fp[k][i]++;
					}
				}
				else
					break;
			}
			//calculate difference between tp and total gt - these are all missed detections
			fn[k][i] = gt[k].annotations.size() - tp[k][i];
		}

		if (0)
		{

			cerr << "Overlaps: ";
			for (unsigned int i = 0; i < gt[k].annotations.size(); i++)
			{
				for (unsigned int j = 0; j < output[k].annotations.size(); j++)
					cerr << overlap[i][j] << " ";
				cerr << endl;
			}

			cerr << "Weights: ";
			for (unsigned int i = 0; i < output[k].annotations.size(); i++)
				cerr << output[k].annotations[i]->weight << " ";
			cerr << endl;

			cerr << "ASS: " << assigned << endl;
			cerr << "TPC: " << tpc[k] << endl;
			cerr << "TP : " << tp[k] << endl;
			cerr << "FP : " << fp[k] << endl;
			cerr << "FN : " << fn[k] << endl;
		}
	}

	return 0;
}
