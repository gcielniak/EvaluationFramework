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

	gt.Add(new BoundingVolume::Box(0, 3, 3, 3 + 4, 3 + 2));
	gt.Add(new BoundingVolume::Box(0, 11, 2, 11 + 4, 2 + 2));
	gt.Add(new BoundingVolume::Box(0, 17, 6, 17 + 4, 6 + 2));

	output.Add(new BoundingVolume::Box(0, 1, 1, 1 + 4, 1 + 2, 0.1));
	output.Add(new BoundingVolume::Box(0, 3, 3, 3 + 4, 3 + 2, 0.8));
	output.Add(new BoundingVolume::Box(0, 2, 5, 2 + 4, 5 + 2, 0.2));
	output.Add(new BoundingVolume::Box(0, 1, 7, 1 + 4, 7 + 2, 0.1));
	output.Add(new BoundingVolume::Box(0, 11, 3, 11 + 4, 3 + 2, 0.8));
	output.Add(new BoundingVolume::Box(0, 12, 7, 12 + 4, 7 + 2, 0.1));
	output.Add(new BoundingVolume::Box(0, 17, 1, 17 + 4, 1 + 2, 0.1));

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
		}
		//calculate difference between tp and total gt - these are all missed detections
		fn[i] = gt.annotations.size() - tp[i];
	}

	cerr << "Weights: ";
	for (unsigned int i = 0; i < output.annotations.size(); i++)
		cerr << output.annotations[i] << " ";
	cerr << endl;

	cerr << "ASS: " << assigned << endl;
	cerr << "TPC: " << tpc << endl;
	cerr << "TP : " << tp << endl;
	cerr << "FP : " << fp << endl;
	cerr << "FN : " << fn << endl;

	return 0;
}

