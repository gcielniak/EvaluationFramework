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
	vector<Frame> v;
	LoadPascal(v, "gt.txt");

	BoundingVolume::Box box;
	Frame frame;

	frame.Add(&box);
	frame.Add(new BoundingVolume::Box());

	//for all frames f
	//for all gt i
	//for all outputs j
	//calcualte overlaps i,j
	//get tp/fp/fn/tn

	Frame gt, output;

	gt.Add(new BoundingVolume::Box(0, 2.8, 3.0, 2.8 + 4, 3.0 + 2.3));
	gt.Add(new BoundingVolume::Box(0, 10.8, 2.3, 10.8 + 4, 2.3 + 2.3));
	gt.Add(new BoundingVolume::Box(0, 16.9, 6.1, 16.9 + 4, 6.1 + 2.3));

	output.Add(new BoundingVolume::Box(0, 0.7, 1.1, 0.7 + 4, 1.1 + 2.3, 0.1));
	output.Add(new BoundingVolume::Box(0, 3.2, 3.4, 3.2 + 4, 3.4 + 2.3, 0.9));
	output.Add(new BoundingVolume::Box(0, 2.2, 5.0, 2.2 + 4, 5.0 + 2.3, 0.2));
	output.Add(new BoundingVolume::Box(0, 1.4, 7.2, 1.4 + 4, 7.2 + 2.3, 0.1));
	output.Add(new BoundingVolume::Box(0, 10.6, 2.8, 10.6 + 4, 2.8 + 2.3, 0.8));
	output.Add(new BoundingVolume::Box(0, 12.0, 6.7, 12.0 + 4, 6.7 + 2.3, 0.1));
	output.Add(new BoundingVolume::Box(0, 16.7, 1.0, 16.7 + 4, 1.0 + 2.3, 0.1));

	//sort the detection in descending order
	output.Sort();

	//calculate overlaps between gt and output
	vector< vector<double> > overlap(gt.annotations.size(), vector<double>(output.annotations.size(),0.0));

	for (unsigned int i = 0; i < gt.annotations.size(); i++)
		for (unsigned int j = 0; j < output.annotations.size(); j++)
			overlap[i][j] = gt.annotations[i]->overlap(*output.annotations[j]);

	//calculate assignments between gt and output
	vector<int> assigned(output.annotations.size(), -1);

	for (unsigned int i = 0; i < overlap.size(); i++)
	{
		for (unsigned int j = 0; j < overlap[i].size(); j++)
		{
			if ((assigned[j] == -1) && (overlap[i][j] > 0.0))
			{
				//true positive
				assigned[j] = i;
				break;
			}
		}
	}

	vector<double> tpc(assigned.size(), 0.0);
	vector<int> tp(assigned.size(), 0);
	vector<int> fp(assigned.size(), 0);
	vector<int> fn(assigned.size(), 0);

	//count
	for (unsigned int i = 0; i < output.annotations.size(); i++)
	{
		for (unsigned int j = 0; j < assigned.size(); j++)
		{
			if (output.annotations[j]->weight >= output.annotations[i]->weight)
			{
				//positives
				if (assigned[j] != -1)
				{
					//true positives
					tp[i]++;
					tpc[i] += overlap[assigned[j]][j];
				}
				else
				{
					//false positive
					fp[i]++;
				}
			}
			else
				break;
		}
		//calculate difference between tp and total gt - these are all missed detections
		fn[i] = gt.annotations.size() - tp[i];
	}

	cerr << "Overlaps: ";
	for (unsigned int i = 0; i < gt.annotations.size(); i++)
	{
		for (unsigned int j = 0; j < output.annotations.size(); j++)
			cerr << overlap[i][j] << " ";
		cerr << endl;
	}

	cerr << "Weights: ";
	for (unsigned int i = 0; i < output.annotations.size(); i++)
		cerr << output.annotations[i]->weight << " ";
	cerr << endl;

	cerr << "ASS: " << assigned << endl;
	cerr << "TPC: " << tpc << endl;
	cerr << "TP : " << tp << endl;
	cerr << "FP : " << fp << endl;
	cerr << "FN : " << fn << endl;

	return 0;
}
